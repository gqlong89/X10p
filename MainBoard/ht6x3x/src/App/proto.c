/*proto.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "proto.h"
#include "card.h"
#include "aes.h"
#include "tfs.h"
#include "md5.h"
#include "ntcad.h"
#include "ui.h"
#include "emu.h"
#include "rtc.h"
#include "sim800c.h"
#include "sc8042.h"
#include "flash.h"
#include "lcd.h"
#include "gun.h"
#include "server.h"
#include "history_order.h"


uint8_t req_cnt[GUN_NUM_MAX] = {0};
MUX_SEM_STR gProtoSendMux = {0,0};
uint8_t gProtoSendBuff[OUT_NET_PKT_LEN];
uint8_t gCardReqFlag = 0;
uint8_t  gSendHearBeatCnt = 0;
uint8_t AesKeyUpdateFlag=0;
uint8_t  gID2[TFS_ID2_LEN+1] = {0};
uint8_t SysToken[16] = "1234567891234567";

extern KEY_VALUES keyVal;
extern uint8_t readCardFlg;
extern TaskHandle_t MainTaskHandle_t;
extern TaskHandle_t gEmuTaskHandle_t;


void UpdateSysToken(void)
{
    if (0x7e < ++SysToken[0]) {
        SysToken[0] = 0x21;
    }
}


int GetPktSum(uint8_t *pData, uint16_t len)
{
    uint32_t i;
    uint8_t sum = 0;

    for (i = 0; i < len; i++) 
	{
        sum += pData[i];
    }
	
    return sum;
}


int GetNoZeroSum(uint8_t *pData, uint16_t len)
{
    int sum = GetPktSum(pData, len);

    if (0 == sum) 
	{
        sum = 1;
    }
	
    return sum;
}


int CheckCostTemplate(void)
{
    if ((COST_POWER != system_info.cost_template.mode) && (COST_UNIFY != system_info.cost_template.mode)) {
        return CL_FAIL;
    }
    return CL_OK;
}


//pData 指向静荷 len静荷长度
int SendProtoPkt(uint16_t sn, uint8_t cmd, PKT_STR *pPkt, uint16_t len, uint8_t decrypetFlag)
{
    uint16_t encryLen;

    if (gChgInfo.sendPktFlag) {
        CL_LOG("skf=%d,cns.\n",gChgInfo.sendPktFlag);
        return CL_FAIL;
    }

    pPkt->head.aa = 0xaa;
    pPkt->head.five = 0x55;
    pPkt->head.type = CHARGER_TYPE;
    if ((MQTT_CMD_REGISTER == cmd) || (MQTT_CMD_AES_REQ == cmd) || (MQTT_CMD_UPDATE_AES_NOTICE == cmd)) {
        memcpy(pPkt->head.chargerSn, system_info.station_id, sizeof(pPkt->head.chargerSn));
    }else{
        memcpy(pPkt->head.chargerSn, system_info.idCode, sizeof(system_info.idCode));
    }
    pPkt->head.len = len + 5;
    pPkt->head.ver = MESSAGE_VER_NOENCRYPT;
    pPkt->head.sn = sn;
    pPkt->head.cmd = cmd;
    pPkt->data[len] = GetPktSum((void*)&pPkt->head.ver, len+4);

    if (LOCAL_NET != system_info.netType) { //非本地网络发送不加密
        return PutOutNetPkt((void*)pPkt, sizeof(PKT_HEAD_STR)+len+1, REQ_SEND_PKT);
    }
    if (0 == decrypetFlag) {
        return SocketSendData(SOCKET_ID, (void*)pPkt, sizeof(PKT_HEAD_STR)+len+1);
    }

    if (0 == AesKeyUpdateFlag) {
        CL_LOG("key err.\n");
        return CL_FAIL;
    }
	//加密数据
	ENCRYPT_PKT_STR *encryptPkt = (void*)pPkt->data;
	memmove((void*)encryptPkt->data, (void*)pPkt->data, len);
	encryLen = (len+7+15) & (~0x000F);
    pPkt->head.len = encryLen + 5;
    pPkt->head.ver = MESSAGE_VER_ENCRYPT;
	encryptPkt->dataLen = len;
	encryptPkt->time = GetRtcCount();
	encryptPkt->data[encryLen - 7] = GetPktSum((void*)encryptPkt, encryLen-1);
	EncrypData((void*)encryptPkt, (void*)pPkt->data, encryLen, &gChgInfo.enAes);
	pPkt->data[encryLen] = GetPktSum((void*)&pPkt->head.ver, encryLen+4);
    PrintfData("SendProtoPkt encryp data", (void*)pPkt, sizeof(PKT_HEAD_STR)+encryLen+1);
    return SocketSendData(SOCKET_ID, (void*)pPkt, sizeof(PKT_HEAD_STR)+encryLen+1);
}


//调用该函数需要注意是否发生递归
int SendSysLog(uint8_t gunId, uint8_t logType, char *pStr)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    SYS_LOG_STR *msg = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
    msg->gun_id = gunId;
    msg->logType = logType;
    memcpy(msg->data, pStr, strlen(pStr));
    SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_SYS_LOG, (void*)&pkt->protoHead, 1+1+strlen(pStr), 0);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int CheckStartPara(uint8_t gun_id)
{
    gun_info_t *pGunInfo = &gun_info[gun_id-1];

    if ((gun_id > GUN_NUM_MAX) || (gun_id < 1)) {
        CL_LOG("gun=%d,err.\n",gun_id);
        OptFailNotice(102);
		return START_FAIL_PARA_ERR;
	}

    if (pGunInfo->is_load_on) {
        CL_LOG("gun %d busy.\n", gun_id);
        //OptFailNotice(103);
        Sc8042bSpeech(VOIC_SOCKET_OCCUPID);
        return START_FAIL_CHARGING;
    }

    if ((COST_POWER != GetCostMode()) && (COST_UNIFY != GetCostMode())) {
        CL_LOG("cm=%d,err.\n", GetCostMode());
        OptFailNotice(104);
        return START_FAIL_NO_TEMPLATE;
    }

    if (CL_OK != CheckGunStatu(gun_id)) {
        CL_LOG("cgs fail.\n");
        OptFailNotice(105);
        return START_FAIL_FAULT;
    }

    if (START_POWER_LIMIT < gChgInfo.chargingTotalPower) {
        CL_LOG("p=%d.\n",gChgInfo.chargingTotalPower);
        OptFailNotice(113);
        return START_FAIL_POWER_LARGE;
    }
    return CL_OK;
}


int SendRegister(void)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    REGISTER_REQ_STR *msg = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    memcpy(msg->device_type, STATION_MACHINE_TYPE, strlen(STATION_MACHINE_TYPE));
    memcpy(msg->register_code, REGISTER_CODE, strlen(REGISTER_CODE));
    memcpy(msg->hwId, system_info.iccid, sizeof(msg->hwId));
	PrintfData("SendRegister origin data", (void*)msg, sizeof(REGISTER_REQ_STR));
	SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_REGISTER, (void*)&pkt->protoHead, sizeof(REGISTER_REQ_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


//flag 1:上电启动			2:离线恢复重发
int SendStartUpNotice(int flag)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	START_UP_REQ_STR *mqtt_start_up_req = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    memcpy(mqtt_start_up_req->device_type, STATION_MACHINE_TYPE, strlen(STATION_MACHINE_TYPE));
    memcpy((void*)&mqtt_start_up_req->chargerSn[3], (void*)&system_info.station_id[3], sizeof(mqtt_start_up_req->chargerSn)-3);
	mqtt_start_up_req->fw_version = FW_VERSION;
    mqtt_start_up_req->fw_1_ver = FW_VERSION_SUB;
    memcpy(mqtt_start_up_req->sim_iccid, system_info.iccid, ICCID_LEN);
    mqtt_start_up_req->onNetWay = system_info.netType;
    mqtt_start_up_req->modeType = 1;
	mqtt_start_up_req->login_reason = flag;
    mqtt_start_up_req->gun_number = GUN_NUM_MAX;
    mqtt_start_up_req->device_status = gChgInfo.netStatus;
    mqtt_start_up_req->statistics_info[0] = gSimStatus;
    mqtt_start_up_req->statistics_info[1] = gSimStatus>>8;
    mqtt_start_up_req->statistics_info[2] = gSimStatus>>16;
    mqtt_start_up_req->statistics_info[3] = system_info.chargingFullTime/60;
    mqtt_start_up_req->statistics_info[4] = system_info.pullGunStop & 0x01;
    mqtt_start_up_req->statistics_info[4] |= ((system_info.chargingFullStop & 0x01) << 1);
    mqtt_start_up_req->statistics_info[5] = system_info.disturbingStartTime;
    mqtt_start_up_req->statistics_info[6] = system_info.disturbingStopTime;
    mqtt_start_up_req->statistics_info[7] = system_info.fwVersion;
    mqtt_start_up_req->fwDownProto = 0;
	PrintfData("startup origin data", (void*)mqtt_start_up_req, sizeof(START_UP_REQ_STR));
	SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_START_UP, (void*)&pkt->protoHead, sizeof(START_UP_REQ_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


// flag: 1：仅查询余额 0：刷卡鉴权，开始充电
int SendCardAuthReq(int flag)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	CARD_AUTH_REQ_STR *pMsg = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    pMsg->gun_id = gChgInfo.current_usr_gun_id;
	memcpy((void*)pMsg->card_id, (void*)gChgInfo.current_usr_card_id, sizeof(pMsg->card_id));
	memset(pMsg->card_psw, 0, sizeof(pMsg->card_psw));
	if (CARD_CHECK_MONEY == flag) {
        gCardReqFlag = 0;
		pMsg->card_type = 0;
		pMsg->optType = flag;
        gChgInfo.money = 0;
	}  else {
		gCardReqFlag = 1;
		pMsg->optType = 0;
		if (CARD_AUTH == flag) {
			pMsg->mode = 1;
			pMsg->card_type = 0;
			pMsg->chargingPara = gChgInfo.money*100;
            gChgInfo.cardType = ORDER_AUTH_CARD;
	    } else if (CARD_MONTH == flag) {//月卡
			pMsg->card_type = 1;
			pMsg->mode = 0;
            pMsg->chargingPara = 0;
            gChgInfo.money = 99;    //月卡是免费的,充电金额给99元
            gChgInfo.cardType = ORDER_MONTH_CARD;
		}
	}

	gChgInfo.mode = pMsg->mode;
	PrintfData("card auth,origin data", (void*)pMsg, sizeof(CARD_AUTH_REQ_STR));
    SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_CARD_ID_REQ, (void*)&pkt->protoHead, sizeof(CARD_AUTH_REQ_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendEventNotice(uint8_t gunId, uint8_t event, uint8_t para1, uint32_t para2, uint8_t status, char *pDisc)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	EVENT_NOTICE_STR *eventNotice = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    eventNotice->gun_id = gunId;
    eventNotice->code = event;
    eventNotice->para1 = para1;
    eventNotice->para2 = para2;
    eventNotice->status = status;
    eventNotice->level = EVENT_ALARM;
    memset(eventNotice->discrip, 0, sizeof(eventNotice->discrip));
    memcpy(eventNotice->discrip, pDisc, strlen(pDisc));
    PrintfData("SendEventNotice", (void*)eventNotice, sizeof(EVENT_NOTICE_STR));
    SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_EVENT_NOTICE, (void*)&pkt->protoHead, sizeof(EVENT_NOTICE_STR), 0);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendStartChargingNotice(int gun_id, uint32_t startTime, int flag)
{
    gun_info_t *pGunInfo = &gun_info[gun_id-1];
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	START_CHARGING_NOTICE_STR *pMqtt_report_power_on_req = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    pMqtt_report_power_on_req->gun_id = gun_id;
    pMqtt_report_power_on_req->ordersource = pGunInfo->ordersource;
    pMqtt_report_power_on_req->optType = flag;
    memcpy((void*)pMqtt_report_power_on_req->user_account, (void*)pGunInfo->user_account, sizeof(pMqtt_report_power_on_req->user_account));
	memcpy((void*)pMqtt_report_power_on_req->order, (void*)pGunInfo->order, ORDER_SECTION_LEN);
	pMqtt_report_power_on_req->start_time  = startTime;
	pMqtt_report_power_on_req->start_power = 0;
    pMqtt_report_power_on_req->money = pGunInfo->current_usr_money;
    pMqtt_report_power_on_req->template_id = pGunInfo->costTempId;
    pMqtt_report_power_on_req->subsidyType = pGunInfo->subsidyType;
    pMqtt_report_power_on_req->subsidyPararm = pGunInfo->subsidyPararm;
	PrintfData("SendStartChargingNotice origin data", (void*)pMqtt_report_power_on_req, sizeof(START_CHARGING_NOTICE_STR));
	SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_REPORT_POWER_ON, (void*)&pkt->protoHead, sizeof(START_CHARGING_NOTICE_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendStartChargingAck(uint8_t gunId, int sn, int result, int reason)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	START_CHARGING_ACK_STR *mqtt_remote_set_power_on_ack = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_remote_set_power_on_ack->gun_id = gunId;
	mqtt_remote_set_power_on_ack->result = result;
    mqtt_remote_set_power_on_ack->failReason = reason;
	PrintfData("SendStartChargingAck origin data", (void*)mqtt_remote_set_power_on_ack, sizeof(START_CHARGING_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_REMOTE_SET_POWER_ON, (void*)&pkt->protoHead, sizeof(START_CHARGING_ACK_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendTradeRecordNotice(HISTORY_ORDER_STR *order)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	STOP_CHARGING_NOTICE_REQ_STR pTradeRecord;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	pTradeRecord.charger_cost = order->cost_price;
	pTradeRecord.chargingPower = order->power;
	pTradeRecord.addChargingTimeCnt = order->addChargingTimeCnt;
    pTradeRecord.ctrlFlag = order->ctrlFlag;
    pTradeRecord.pullGunStopTime = order->pullGunStopTime;
	pTradeRecord.fwVer = order->fw_version;
	pTradeRecord.gun_id = order->gun_id;
	memcpy(pTradeRecord.order,order->order,sizeof(order->order));
	pTradeRecord.ordersource = order->ordersource;
	pTradeRecord.startElec = order->startElec;
	pTradeRecord.startTime = order->startTime;
	pTradeRecord.stopDetal = order->stopDetails;
	pTradeRecord.stop_elec = order->stopElec;
	pTradeRecord.stop_reason = order->stopReason;
	pTradeRecord.stop_time = order->stopTime;
	pTradeRecord.subsidyPararm = order->subsidyPararm;
	pTradeRecord.subsidyType = order->subsidyType;
	pTradeRecord.template_id = order->template_id;
	memcpy(pTradeRecord.user_account,order->user_account,sizeof(order->user_account));
    pTradeRecord.chargerMode = order->chargerMode;
    pTradeRecord.chargerPararm = order->chargerPararm;
    pTradeRecord.chargingTime = order->realChargingTime;
    memcpy(pkt->data, &pTradeRecord, sizeof(STOP_CHARGING_NOTICE_REQ_STR));
	PrintfData("SendTradeRecordNotice origin data", (void*)pkt->data, sizeof(STOP_CHARGING_NOTICE_REQ_STR));
	SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_REPORT_POWER_OFF, (void*)&pkt->protoHead, sizeof(STOP_CHARGING_NOTICE_REQ_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendStopChargingAck(uint8_t gunId, int sn, int result)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	STOP_CHARGING_ACK_STR *mqtt_remote_set_power_off_ack = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_remote_set_power_off_ack->gun_id = gunId;
	mqtt_remote_set_power_off_ack->result = result;
	PrintfData("SendStopChargingAck origin data", (void*)mqtt_remote_set_power_off_ack, sizeof(STOP_CHARGING_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_REMOTE_SET_POWER_OFF, (void*)&pkt->protoHead, sizeof(STOP_CHARGING_ACK_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int GetLeftChargingTime(uint8_t gun_id)
{
    gun_info_t *pGunInfo = &gun_info[gun_id-1];
    uint16_t leftTime;

    if (COST_UNIFY == pGunInfo->cost_mode) {
        leftTime = pGunInfo->charger_time_plan - pGunInfo->realChargingTime;
    }else{
        leftTime = pGunInfo->getPowerFlag ? pGunInfo->charger_time_plan - pGunInfo->realChargingTime : 255;
    }
    return leftTime;
}


int MakeGunHearBeatInfo(GUN_HEART_BEAT_STR *pInfo, uint8_t gun_id)
{
    gun_info_t *pGunInfo = &gun_info[gun_id-1];
    GUN_STATUS_STR gunStatus;
    int len = 4;
    uint16_t leftTime;

    pInfo->gunIndex = gun_id;
	pInfo->status = pGunInfo->is_load_on ? PLUG_USED_OPEN : PLUG_IDLE;
    GetGunStatus(gun_id, &gunStatus);
    pInfo->chipTemp = gunStatus.temp;
    pInfo->faultCode = gunStatus.status;
    if (PLUG_USED_OPEN == pInfo->status) {
        leftTime = GetLeftChargingTime(gun_id);
        pInfo->faultCode = (255 <= leftTime) ? 255 : leftTime;
        pInfo->voltage = gunStatus.voltage/10;
        pInfo->current = gunStatus.current/100;
        pInfo->power = gunStatus.power/10;
        pInfo->elec = pGunInfo->chargingElec;
        pInfo->money = pGunInfo->money;
        memcpy(pInfo->order, pGunInfo->order, sizeof(pInfo->order));
        len = sizeof(GUN_HEART_BEAT_STR);
    }
    return len;
}


int SendHeartBeatFun(uint8_t gun_id)
{
	int len = 0;
    int i;
    FRAME_STR *pkt = (void*)gProtoSendBuff;
	HEART_BEAT_REQ_STR *mqtt_heart_beat = (void*)pkt->data;
    GUN_HEART_BEAT_STR *pGunInfo = mqtt_heart_beat->gunStatus;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_heart_beat->netSigle = GetNetSignal();
    mqtt_heart_beat->envTemp = GetCpuTemp() + 50;
    mqtt_heart_beat->status = gChgInfo.statusErr;
    mqtt_heart_beat->learnFlag = EmuCalation.autoLearnFlag;
    mqtt_heart_beat->gunCnt = GUN_NUM_MAX;
    for (i=1; i<=GUN_NUM_MAX; i++) {
        len += MakeGunHearBeatInfo((void*)((char*)pGunInfo + len), i);
    }
    PrintfData("SendHeartBeatFun", (void*)mqtt_heart_beat, len+7);
    SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_HEART_BEAT, (void*)&pkt->protoHead, len+7, ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


void HeartBeatHandle(void)
{
	if (CL_OK == IsSysOnLine()) {
		SendHeartBeatFun(0);
        gSendHearBeatCnt++;
        if (2 < gSendHearBeatCnt) {
            gSendHearBeatCnt = 0;
            system_info.is_socket_0_ok = CL_FALSE;
            CL_LOG("no rx hb ack,err.\n");
            gChgInfo.netStatus |= 0x01;
            OptFailNotice(111);
        }
	}
}


int SendReqCostTemplate(uint8_t gunId)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    REQ_COST_TEMPLATE_STR *reqCostTemplate = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    reqCostTemplate->gun_id = gunId;
    PrintfData("SendReqCostTemplate", (void*)reqCostTemplate, sizeof(REQ_COST_TEMPLATE_STR));
    SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_COST_REQ, (void*)&pkt->protoHead, sizeof(REQ_COST_TEMPLATE_STR), 0);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


void CostTemplateReq(void)
{
    if ((COST_POWER != system_info.cost_template.mode) && (COST_UNIFY != system_info.cost_template.mode)) {
        CL_LOG("send req.\n");
        SendReqCostTemplate(0);
    }
}


int SendCostTemplateAck(uint32_t sn, uint8_t result, uint8_t gunId)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    COST_TEMPLATE_ACK_STR *mqtt_cost_down_ack = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    mqtt_cost_down_ack->gunId = gunId;
    mqtt_cost_down_ack->result = result;
	PrintfData("SendCostTemplateAck origin data", (void*)mqtt_cost_down_ack, sizeof(COST_TEMPLATE_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_COST_DOWN, (void*)&pkt->protoHead, sizeof(COST_TEMPLATE_ACK_STR), ID2);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendDeviceAesReq(uint32_t time_utc, uint8_t reason)
{
	uint8_t authCodeBuf[100] = {0};
	uint64_t timeStamp = (uint64_t)(time_utc) * 1000;
	FRAME_STR *pkt = (void*)gProtoSendBuff;
    DEVICE_AES_REQ_STR *pAesReq = (void*)pkt->data;

	if (CL_FAIL == TfsId2GetTimeStampAuthCode(timeStamp, (void*)extraData, TFS_EXTA_LEN, authCodeBuf)) {
        SendEventNotice(0, EVENT_PSW_UPDATE, 5, 0, EVENT_OCCUR, NULL);
        OptFailNotice(32);
		return CL_FAIL;
	}

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
	memcpy((void*)pAesReq->id2, gID2, TFS_ID2_LEN);
	memset((void*)pAesReq->authCode, 0, sizeof(pAesReq->authCode));
	memcpy((void*)pAesReq->authCode, authCodeBuf, TFS_TIMESTAMP_AUTH_CODE_LEN);
	memcpy((void*)pAesReq->extra, extraData, TFS_EXTA_LEN);
	pAesReq->reason = reason;
	UpdateSysToken();
	memcpy(pAesReq->token, SysToken, 16);
	pAesReq->method = 0;
	PrintfData("SendDeviceAesReq", (void*)pkt->data, sizeof(DEVICE_AES_REQ_STR));
	SendProtoPkt(system_info.mqtt_sn++, MQTT_CMD_AES_REQ, (void*)&pkt->protoHead, sizeof(DEVICE_AES_REQ_STR), 0);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


int SendUpdateAesAck(uint16_t sn, uint8_t result)
{
	FRAME_STR *pkt = (void*)gProtoSendBuff;
    UPDATE_AES_ACK_STR *pAesAckStr = (void*)pkt->data;

    MuxSempTake(&gProtoSendMux);
	memset(gProtoSendBuff, 0, sizeof(gProtoSendBuff));
    pAesAckStr->result = result;
	PrintfData("SendUpdateAesAck", (void*)pkt->data, sizeof(UPDATE_AES_ACK_STR));
	SendProtoPkt(sn, MQTT_CMD_UPDATE_AES_NOTICE, (void*)&pkt->protoHead, sizeof(UPDATE_AES_ACK_STR), 0);
    MuxSempGive(&gProtoSendMux);
    return CL_OK;
}


void InitChargingCtrlPara(uint8_t  gunId)
{
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];

    pGunCharging->loopCnt = system_info.pullGunStopTime/POWER_CHECK_CNT/2;
    //固定计费模式的情况下，功率低于20w开始判满，实时和起步金模式下功率低于45w开始判满
    pGunCharging->chargingFullPower = ((CHARGING_FIXED == pGunInfo->chargerMethod) || (START_TYPE_MONTH_CARD == pGunInfo->startMode) || (START_TYPE_ELE_MONTH_CARD == pGunInfo->startMode)) ? 200 : 450;
    pGunCharging->chargingFullTime = system_info.chargingFullTime;
    //月卡至少给4个小时续冲时长
    if ((START_TYPE_MONTH_CARD == pGunInfo->startMode) || (START_TYPE_ELE_MONTH_CARD == pGunInfo->startMode)) {
        if (pGunCharging->chargingFullTime < 14400) {
            pGunCharging->chargingFullTime = 14400;
        }
    }
    pGunCharging->pullGunStopTime = system_info.pullGunStopTime;
    pGunCharging->pullGunStop = system_info.pullGunStop;
    pGunCharging->chargingFullStop = system_info.chargingFullStop;
}


void StartCharging(uint8_t startMode, uint32_t money, uint8_t *pCardSn, uint8_t *pOrder, uint8_t ordersource)
{
    uint8_t  gunId = gChgInfo.current_usr_gun_id;
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    GUN_STATUS_STR gunStatus;
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];

    memset(pGunInfo, 0, sizeof(gun_info_t));
	pGunInfo->gun_id = gunId;
    pGunInfo->gun_state = GUN_STATE_WAIT_PLUG_IN;
	pGunInfo->ordersource = ordersource;
    if (NULL != pCardSn) {
        memcpy(pGunInfo->user_account, (void*)pCardSn, sizeof(pGunInfo->user_account));
    }
    memcpy(pGunInfo->order, pOrder, sizeof(pGunInfo->order));
    pGunInfo->cost_mode = system_info.cost_template.mode;
	pGunInfo->chargerMethod = system_info.chargerMethod;
	pGunInfo->chargerStartingGold = system_info.chargerStartingGold;
    if (COST_UNIFY == system_info.cost_template.mode) {
        pGunInfo->charger_time_plan = money * system_info.cost_template.Data.unifyInfo.duration / system_info.cost_template.Data.unifyInfo.price;
        pGunInfo->powerInfo.segmet[0].price = system_info.cost_template.Data.unifyInfo.price;
        pGunInfo->powerInfo.segmet[0].duration = system_info.cost_template.Data.unifyInfo.duration;
		if (pGunInfo->chargerMethod == CHARGING_START) {
			//起步价充电时间
			pGunInfo->startGoldTime = pGunInfo->chargerStartingGold * system_info.cost_template.Data.unifyInfo.duration/ system_info.cost_template.Data.unifyInfo.price;
		}
    }else{
        pGunInfo->charger_time_plan = NO_LOAD_TIME;
        memcpy(&pGunInfo->powerInfo, &system_info.cost_template.Data.powerInfo, sizeof(pGunInfo->powerInfo));
        pGunInfo->startGoldTime = NO_LOAD_TIME;
    }
    CL_LOG("cm=%d,cw=%d,sg=%d,m=%d,id=%d,cp=%d,pct=%dm,sgt=%dm,gun=%d.\n",
        pGunInfo->cost_mode,pGunInfo->chargerMethod,pGunInfo->chargerStartingGold,money,system_info.cost_template.template_id,system_info.changePower,pGunInfo->charger_time_plan,pGunInfo->startGoldTime,gunId);
    pGunInfo->costTempId = system_info.cost_template.template_id;
    pGunInfo->changePower = system_info.changePower;
	pGunInfo->start_time = GetRtcCount();
    GetGunStatus(gunId, &gunStatus);
    pGunInfo->startElec = gunStatus.elec;
    pGunInfo->startMode = startMode;
    pGunInfo->current_usr_money = money;
    pGunInfo->stopReason = STOP_UNKNOW;
    pGunInfo->reasonDetail = REASON_UNKNOW;
    pGunInfo->chargingMode = gChgInfo.mode;
	pGunInfo->subsidyType = gChgInfo.subsidyType;
	pGunInfo->subsidyPararm = gChgInfo.subsidyPararm;
    GunTurnOn(gunId);
    chargingOldTime = pGunInfo->start_time;
    pGunCharging->chargingTime = pGunInfo->start_time;
    pGunCharging->isTesting = 0;
    pGunCharging->checkPowerCnt = 0;
    pGunCharging->inCnt = 0;
    pGunCharging->isFull = 0;
    pGunCharging->powerIndex = 0;
    pGunCharging->resetEmuChipFlag = 0;
    pGunCharging->startChargerTimers = 0;
    InitChargingCtrlPara(gunId);
    pGunInfo->is_load_on = GUN_CHARGING_UNKNOW_POWER;
    pGunInfo->isSync = FIRST_START_CHARGING;
    SendStartChargingNoticeTime = 0;
    SwitchToUi_Charging(0);
	SpeechChargeing(gunId);
    FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
    UpdataGunDataSum();
}


void StopCharging(uint8_t gunId)
{
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];
    segment_str *pPowerSement = NULL;

    GunTurnOff(gunId);
    pPowerSement = &pGunInfo->powerInfo.segmet[pGunInfo->powerSemenIndex];
    //为了防止用户主动停止充电来不急计费更新，强制刷新一次充电金额
    pGunInfo->money = pGunInfo->realChargingTime * pPowerSement->price / pPowerSement->duration;
    if (pGunInfo->money > pGunInfo->current_usr_money) {
        pGunInfo->money = pGunInfo->current_usr_money;
    }
    pGunInfo->stop_time = GetRtcCount();
    pGunCharging->isTesting = 0;
	pGunInfo->is_load_on = GUN_CHARGING_IDLE;
    pGunInfo->gun_state = GUN_STATE_IDLE;
	FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
    UpdataGunDataSum();

	//生成订单
	HISTORY_ORDER_STR order;
	memset(&order,0,sizeof(HISTORY_ORDER_STR));
	MakeTradeInfo(&order, gunId);
	Write_HistoryOrder(&order);
    SendTradeRecordNoticeTime = 0;
    CL_LOG("gun=%d,stat=%d,stt=%d,sr=%d,rd=%d,m=%d.\n",
        gunId,pGunInfo->start_time,pGunInfo->stop_time,pGunInfo->stopReason,pGunInfo->reasonDetail,pGunInfo->money);
    //PrintfData("StopCharging",order.order,ORDER_SECTION_LEN);
}


int CostTempCopy(COST_TEMPLATE_HEAD_STR *pcost)
{
    int ret = CL_OK;
    uint16_t temp;

    if (0 == pcost->template_id) {
        CL_LOG("id=0,err.\n");
        OptFailNotice(207);
        return CL_FAIL;
    }

    if (COST_POWER == pcost->mode) {
		multiPower_t *powerInfo = (void*)pcost->Data;
		if (COST_TEMP_CNT < powerInfo->segmentCnt) {
			OptFailNotice(203);
			ret = 1;
		}else{
			temp = sizeof(segment_str)*powerInfo->segmentCnt + sizeof(powerInfo->segmentCnt);
			memcpy(&system_info.cost_template.Data.powerInfo, powerInfo, temp);
			system_info.cost_template.mode = COST_POWER;
		}
	}else if (COST_UNIFY == pcost->mode) {//统一计费
		unify_t *unifyInfo = (void*)pcost->Data;
        temp = sizeof(unify_t);
		memcpy(&system_info.cost_template.Data.unifyInfo, unifyInfo, sizeof(unify_t));
		system_info.cost_template.mode = COST_UNIFY;
	}else{
        CL_LOG("mode=%d,err.\n",pcost->mode);
        OptFailNotice(205);
        ret = 1;
    }

	if (CL_OK == ret) {
        system_info.chargerMethod = pcost->Data[temp];
        system_info.chargerStartingGold = pcost->Data[temp+1] | (pcost->Data[temp+2]<<8);
        system_info.cost_template.template_id = pcost->template_id;
		system_info.cost_template.gunId = pcost->gunId;
		FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
        ShowCostTemplate();
    }
    return ret;
}


void CostTemplateProc(PKT_STR *pPkt)
{
    COST_TEMPLATE_HEAD_STR *pcost = (void*)pPkt->data;
    int ret = CostTempCopy((void*)pPkt->data);

    SendCostTemplateAck(pPkt->head.sn, ret, pcost->gunId);
}


void SetUpgradeInfo(DOWN_FW_REQ_STR *pfwInfo)
{
    char url[50] = {0};
    char usrName[6] = {0};
    char psw[6] = {0};
    char fileName[10] = {0};
    int i;

	vTaskDelete(MainTaskHandle_t);
    vTaskDelete(gEmuTaskHandle_t);
    gChgInfo.lastOpenTime = GetRtcCount();
    gChgInfo.sendPktFlag = 2;
    memcpy(url, pfwInfo->url, sizeof(pfwInfo->url));
    memcpy(usrName, pfwInfo->usrName, sizeof(pfwInfo->usrName));
    memcpy(psw, pfwInfo->psw, sizeof(pfwInfo->psw));
    memcpy(fileName, pfwInfo->fileName, sizeof(pfwInfo->fileName));
    CL_LOG("url=%s,un=%s,psw=%s,fn=%s,cs=%#x.\n",url,usrName,psw,fileName,pfwInfo->checkSum);
    for (i=0; i<4; i++) {
        Sc8042bSpeech(VOIC_START_UPGRADE);
		SwitchToUi_EquipUpgrade();
        if (CL_OK == FtpGet(url, usrName, psw, fileName, pfwInfo->checkSum)) {
            break;
        }
        OS_DELAY_MS(5000);
    }
    ResetSysTem();
}


void SendRemoCtrlAck(PKT_STR *pRemoCtrlReq, uint8_t result)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;
    REMO_CTRL_ACK_STR *pRemoCtrlack = (void*)pkt->data;
    REMO_CTRL_REQ_STR *pReq = (void*)pRemoCtrlReq->data;

    MuxSempTake(&gProtoSendMux);
    pRemoCtrlack->optCode = pReq->optCode;
    pRemoCtrlack->result = result;
    PrintfData("SendRemoCtrlAck", (void*)pRemoCtrlack, sizeof(REMO_CTRL_ACK_STR));
	SendProtoPkt(pRemoCtrlReq->head.sn, MQTT_CMD_REMOTE_CTRL, (void*)&pkt->protoHead, sizeof(REMO_CTRL_ACK_STR), 0);
    MuxSempGive(&gProtoSendMux);
}


void SetChargingTime(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    CL_LOG("cft=%d.\n",pReq->para);
    system_info.chargingFullTime = (CHARGING_FULL_TIME < pReq->para) ? pReq->para : CHARGING_FULL_TIME;
    FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
    WriteCfgInfo(CFG_CHARGING_FULL_TIME, system_info.chargingFullTime, NULL);
    SendRemoCtrlAck(pFrame, CL_OK);
}


void SetPullGunStopTimeFuncy(uint32_t time)
{
    if (0 == time) {
        WriteCfgInfo(CFG_PULL_GUN_STOP, 2, NULL);
        system_info.pullGunStop = 2;
    }else{
        WriteCfgInfo(CFG_PULL_GUN_STOP, 1, NULL);
        system_info.pullGunStop = 1;
        WriteCfgInfo(CFG_PULL_GUN_STOP_TIME, time, NULL);
        system_info.pullGunStopTime = time;
        system_info.cfgFlag = 0x55aa;
    }
    FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
}


void SetPullGunStopTime(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    CL_LOG("pgst=%ds.\n", pReq->para);
    SetPullGunStopTimeFuncy(pReq->para);
    SendRemoCtrlAck(pFrame, CL_OK);
}


//timeLimit 时间限制，秒
int OpenGunEmergency(uint8_t gunId, uint16_t timeLimit, uint8_t openType)
{
	gun_info_t *pGunInfo = &gun_info[gunId-1];
	GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];

	if (pGunInfo->is_load_on) {
		CL_LOG("gun %d,busy.\n",gunId);
		return CL_OK;
	}

    //如果已经是应急码或远程打开插座，就不再打开
    if ((0 == pGunCharging->isTesting) || (TEST_OPEN == pGunCharging->isTesting)) {
    	pGunCharging->isTesting = openType;
    	pGunCharging->beginTime = GetRtcCount();
    	pGunCharging->timeLimit = timeLimit;
    	GunTurnOn(gunId);
    }
	return CL_OK;
}


int TestOpenGunProc(uint8_t gunId, uint16_t timeLimit, uint8_t openType)
{
	if (GUN_NUM_MAX < gunId) {
        CL_LOG("gun=%d,err.\n",gunId);
        return CL_FAIL;
    }else if(gunId >= 1) {
		OpenGunEmergency(gunId, timeLimit, openType);
	}else if(0 == gunId) {
		for(uint8_t i = 1;i<= GUN_NUM_MAX;i++) {
			OpenGunEmergency(i, timeLimit, openType);
		}
	}
    return CL_OK;
}


void RemoteOpenGun(PKT_STR *pFrame)
{
    int ret;
    uint8_t  gunId = pFrame->data[1];
	uint16_t time = (uint16_t)((pFrame->data[3]<<8) | pFrame->data[2]);

    CL_LOG("open,gun=%d,t=%dm.\n",gunId,time);
	if (time > 720) { //最大时间12小时--720分钟
		ret = CL_FAIL;
	}else{
		ret = TestOpenGunProc(gunId, time*60, REMOTE_CTRL_OPEN);
	}
    SendRemoCtrlAck(pFrame, ret);
}


int TestCloseGun(uint8_t gunId)
{
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];

    if (pGunInfo->is_load_on) {
        CL_LOG("gun %d buys.\n",gunId);
        return CL_FAIL;
    }

    if (pGunCharging->isTesting) {
        pGunCharging->isTesting = 0;
        GunTurnOff(gunId);
    }
    return CL_OK;
}


int TestCloseGunProc(uint8_t gunId)
{
    int i;

    if (GUN_NUM_MAX < gunId) {
        CL_LOG("gunid=%d,err.\n",gunId);
        return CL_FAIL;
    }

    if (0 == gunId) {
        for (i=1; i<=GUN_NUM_MAX; i++) {
            TestCloseGun(i);
        }
    }else{
        return TestCloseGun(gunId);
    }
    return CL_OK;
}


void RemoteCloseGun(PKT_STR *pFrame)
{
    int ret;
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    CL_LOG("gun %d close.\n",pReq->para);
    ret = TestCloseGunProc(pReq->para);
    SendRemoCtrlAck(pFrame, ret);
}


void SetPrintfSwitch(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    SetPrintSwitch(pReq->para);
    SendRemoCtrlAck(pFrame, CL_OK);
}


void SetPullGunStop(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    CL_LOG("para=%d.\n", pReq->para);
    system_info.pullGunStop = (0 == pReq->para) ? 1 : 2;
    WriteCfgInfo(CFG_PULL_GUN_STOP, system_info.pullGunStop, NULL);
    SendRemoCtrlAck(pFrame, CL_OK);
}


void SetChargerFullStop(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    CL_LOG("para=%d.\n", pReq->para);
    system_info.chargingFullStop = (0 == pReq->para) ? 1 : 2;
    WriteCfgInfo(CFG_CHARGING_FULL_STOP, system_info.chargingFullStop, NULL);
    SendRemoCtrlAck(pFrame, CL_OK);
}

void SetChargeChangePowerFuncy(uint8_t chargePower)
{
    CL_LOG("cfcp=%d.\n", chargePower);
    system_info.changePower = chargePower;
    WriteCfgInfo(CFG_CHARGE_POWER, chargePower, NULL);
}

void RemoSetChargeChangePower(PKT_STR *pFrame)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pFrame->data;

    SetChargeChangePowerFuncy(pReq->para);
    SendRemoCtrlAck(pFrame, CL_OK);
}


void RemoteCtrlProc(PKT_STR *pFrame)
{
    switch (pFrame->data[0]) {
        case SYSTEM_REBOOT:
            CL_LOG("reboot.\n");
            SendRemoCtrlAck(pFrame, CL_OK);
            ResetSysTem();
            break;
        case CTRL_OPEN_GUN:
            RemoteOpenGun(pFrame);
            break;
        case CTRL_CLOSE_GUN:
            RemoteCloseGun(pFrame);
            break;
        case CTRL_SET_FULL_TIME:
			SetChargingTime(pFrame);
            break;
        case CTRL_SET_PULL_GUN_TIME:
            SetPullGunStopTime(pFrame);
            break;
        case CTRL_SET_PULL_GUN_STOP:
            SetPullGunStop(pFrame);
            break;
        case CTRL_SET_PRINT_SWITCH:
            SetPrintfSwitch(pFrame);
            break;
		case CTRL_SET_DISTURBING_TIME:
			system_info.disturbingStartTime = pFrame->data[1];
			system_info.disturbingStopTime = pFrame->data[2];
			CL_LOG("sta=%d,sto=%d.\n", system_info.disturbingStartTime,system_info.disturbingStopTime);
            SendRemoCtrlAck(pFrame, CL_OK);
			FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
			break;
        case CTRL_SET_CHARGING_FULL_STOP:
            SetChargerFullStop(pFrame);
            break;
        case CTRL_SET_CHARGE_CHANGE_POWER:
            RemoSetChargeChangePower(pFrame);
            break;
		default:
			break;
    }
}


void ProcEventNoticeAck(PKT_STR *pFrame)
{
    EVENT_NOTICE_ACK_STR* pEventNoticeAck = (void*)pFrame->data;
    //GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[pEventNoticeAck->gun_id-1];

    if (GUN_NUM_MAX < pEventNoticeAck->gun_id) {
        CL_LOG("gun=%d,err.\n",pEventNoticeAck->gun_id);
        return;
    }

    switch (pEventNoticeAck->code) {
        case EVENT_PLUG_PULL_OUT:
            //pGunCharging->isPullOutGun = 0xff;
            break;
        case EVENT_CHIP_FAULT:
            if (CHIP_BLUE == pEventNoticeAck->para1) {
                gChgInfo.sendBlueStatus = 0xff;
            }
            break;
    }
}


int DecryptAndCheckServeData(uint8_t *inData, uint8_t *outData, uint16_t len)
{
	uint8_t checkSum = 0;

	DecryptData((void*)inData, (void*)outData, len, &gChgInfo.deAes);
	for (uint8_t i=0; i<(len-1); i++) {
		checkSum += outData[i];
	}

	if (checkSum == outData[len-1]) {
		//CL_LOG("checkSum ok.\n");
		return CL_OK;
	} else {
	    CL_LOG("cs error.\n");
	    //PrintfData("DecryptAndCheckServeData: checkSum error, origin data", outData, len);
		return CL_FAIL;
	}
}


int AesInfoHandle( PKT_STR* pFrame, uint16_t len)
{
    uint8_t AesKey[18];
	uint8_t AesInfo[64] = {0};
	uint8_t hashVal[16] = {0};
	uint8_t hashValBcdToStr[32]={0};
	DEVICE_AES_REQ_ACK_STR* pAesInfo = (DEVICE_AES_REQ_ACK_STR*)pFrame->data;

	PrintfData("AesInfoHandle: recv aes update req ack", (void*)pFrame->data, len);
	//更新系统时间
	SycTimeCount(pAesInfo->time_utc);
	SetRtcCount(pAesInfo->time_utc);
	if (pAesInfo->result == 0) {
		//PrintfData("recv aes update req ack cipherText",(void*)pAesInfo->aesInfo, CIPHER_SIZE);
		TfsId2Decrypt(pAesInfo->aesInfo, CIPHER_SIZE, AesInfo);
		//PrintfData("AesInfoHandle: id2 decrypt aes data",(void*)AesInfo, 64);

		//判断token
		if (memcmp((void*)SysToken, (void*)&AesInfo, 16) != 0) {
			CL_LOG("Systoken err.\n");
            SendEventNotice(0, EVENT_PSW_UPDATE, 2, 0, EVENT_OCCUR, NULL);
			return CL_FAIL;
		}

		//计算token与秘钥的哈希值
		md5(AesInfo, 32, hashVal);
		//PrintfData("AesInfoHandle: local hashVal", (void*)hashVal, 16);
		DeviceBcd2str((void*)hashValBcdToStr, (void*)hashVal, 16);

		//比较哈希值
		if (memcmp((void*)hashValBcdToStr, (void*)&AesInfo[32], 32) == 0) {
			memcpy(AesKey, (void*)&AesInfo[16], 16);
            AES_set_encrypt_key(AesKey, 128, &gChgInfo.enAes);
            AES_set_decrypt_key(AesKey, 128, &gChgInfo.deAes);
			AesKeyUpdateFlag = 1;
            gChgInfo.netStatus |= (1<<6);
            SendEventNotice(0, EVENT_PSW_UPDATE, 0, 0, EVENT_OCCUR, NULL);
			CL_LOG("update aes Aeskey:%s.\n", AesKey);
			return CL_OK;
		} else {
			CL_LOG("hash err.\n");
            SendEventNotice(0, EVENT_PSW_UPDATE, 3, 0, EVENT_OCCUR, NULL);
			return CL_FAIL;
		}
	} else {
		AesKeyUpdateFlag = 0;
        SendEventNotice(0, EVENT_PSW_UPDATE, 4, 0, EVENT_OCCUR, NULL);
		CL_LOG("update aes fail.\n");
		return CL_FAIL;
	}
}


void ProcChargerInfo(void)
{
    CHARGER_STR charger;

    HT_Flash_ByteRead((void*)&charger, CHARGER_INFO_FLASH_ADDR, sizeof(charger));
    if (memcmp(charger.station_id, system_info.station_id, sizeof(system_info.station_id))) {
        WriteCfgInfo(CFG_CHARGER_SN, sizeof(system_info.station_id), system_info.station_id);
        CL_LOG("w sn.\n");
    }

    if (memcmp(charger.idCode, system_info.idCode, sizeof(system_info.idCode))) {
        WriteCfgInfo(CFG_CHARGER_DEVICEID, sizeof(system_info.idCode), system_info.idCode);
        CL_LOG("w id.\n");
    }
}


int RecvServerData(PKT_STR *pFrame, uint16_t len)
{
    int ret;
    gun_info_t *pGunInfo = NULL;
    uint32_t now = GetRtcCount();

    if (CHARGER_TYPE != pFrame->head.type) {
        //CL_LOG("type=%d,error, pkt drop.\n",pFrame->head.type);
        OptFailNotice(110);
        return CL_FAIL;
    }

    if ((MQTT_CMD_REGISTER == pFrame->head.cmd) || (MQTT_CMD_AES_REQ == pFrame->head.cmd) || (MQTT_CMD_UPDATE_AES_NOTICE == pFrame->head.cmd)) {
        if (memcmp(pFrame->head.chargerSn, system_info.station_id, sizeof(pFrame->head.chargerSn))) {
            //CL_LOG("sn diff error, pkt drop,cmd=%d.\n",pFrame->head.cmd);
            OptFailNotice(109);
            return CL_FAIL;
        }
    } else {
		if (memcmp(pFrame->head.chargerSn, system_info.idCode, 8)) {
			//CL_LOG("idCode diff error, pkt drop,cmd=%d.\n",pFrame->head.cmd);
			OptFailNotice(109);
			return CL_FAIL;
		}
    }

	//CL_LOG("RecvServerData: ver = %d, cmd = %d.\n", pFrame->head.ver, pFrame->head.cmd);
	#if (1 == ID2)
	if (MESSAGE_VER_ENCRYPT == pFrame->head.ver) {  //外接485网络不会走进该分支
		PrintfData("RecvServerData encryp data:", (void*)pFrame, len);
		if(CL_OK == DecryptAndCheckServeData((void*)pFrame->data, (void*)pFrame->data, len-sizeof(PKT_HEAD_STR)-1)) {
            gChgInfo.id2DecrypErrCnt = 0;
			ENCRYPT_PKT_STR *encryptPkt = (ENCRYPT_PKT_STR*)pFrame->data;
			//CL_LOG("currentRtcCount=%d, getServerRtcCount=%d.\n", currentRtcCount, encryptPkt->time);
			now = (now > encryptPkt->time) ? now - encryptPkt->time : encryptPkt->time - now;
			if (600 < now) { //10分钟相差
				CL_LOG("rx time>600.\n");
				return CL_FAIL;
			}
			memmove((void*)pFrame->data, (void*)&pFrame->data[6], len);
		} else {
		    OptFailNotice(35);
			CL_LOG("decrypt err.\n");
            if (1 <= gChgInfo.id2DecrypErrCnt) {
                gChgInfo.id2DecrypErrCnt = 0;
                AesKeyUpdateFlag = 0;
                gChgInfo.ReqKeyReason = 1;
            }else{
                gChgInfo.id2DecrypErrCnt++;
            }
			return CL_FAIL;
		}
	}
    #endif

    switch (pFrame->head.cmd) {
        case MQTT_CMD_REGISTER:
        {
            PrintfData("RecvServerData: register ack", (void*)pFrame, len);
            REGISTER_ACK_STR *pRegister = (void*)pFrame->data;

            if (0 == pRegister->result) {
                PrintfData("idcode:", (void*)pRegister->idcode, 8);
                memcpy((void*)system_info.idCode, (void*)pRegister->idcode, sizeof(system_info.idCode));
                FlashWriteSysInfo(&system_info, sizeof(system_info), 1);

                WriteCfgInfo(CFG_CHARGER_DEVICEID, sizeof(system_info.idCode), pRegister->idcode);
                CL_LOG("register ok,reboot.\n");
                OptSuccessNotice(802);
                ResetSysTem();
            }else{
                CL_LOG("register ack=%d,err.\n",pRegister->result);
                OptFailNotice(100);
            }
        }
        break;

        case MQTT_CMD_START_UP:
        {
            PrintfData("RecvServerData: start up ack", (void*)pFrame, len);
            START_UP_ACK_STR* pStartUp = (START_UP_ACK_STR*)pFrame->data;
            SycTimeCount(pStartUp->time_utc);
            SetRtcCount(pStartUp->time_utc);
            gSimStatus = 0;
            gChgInfo.errCode = 0;
            gChgInfo.netStatus = 0;
            if (0 == pStartUp->result) {
                system_info.isRecvStartUpAck = 1;
                ProcChargerInfo();
            }else{
                CL_LOG("start up ack=%d,err.\n",pStartUp->result);
                OptFailNotice(101);
            }
            LcdTurnOnLed();
        }
        break;

        case MQTT_CMD_CARD_ID_REQ:
        {
            LcdTurnOnLed();
            PrintfData("RecvServerData: card auth ack", (void*)pFrame, len);
            //CL_LOG("gCardReqFlag=%d.\n",gCardReqFlag);
            CARD_AUTH_ACK_STR* pCardAuth = (CARD_AUTH_ACK_STR*)pFrame->data;
            switch(pCardAuth->result) {
                case 0: // success
					if (1 == readCardFlg) {  //查询余额应答处理
						readCardFlg = 0;
						if (pCardAuth->cardType == 1) {	//月卡
							gChgInfo.mode = 0;
                            //#if (1 == SPECIAL_NET_CARD)
                            //PointOutNumber(pCardAuth->user_momey);
                            //#endif
							SwitchToUi_InputSocketNum();
							clearKeyBuf(&keyVal);
							Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
						} else {
							gChgInfo.user_card_balance = pCardAuth->user_momey;
							CL_LOG("card balance = %d\n", gChgInfo.user_card_balance);
							if (gChgInfo.user_card_balance < 100) {
								Sc8042bSpeech(VOIC_CARD_RECHARGER);
								SwitchToUi_Standby();
								break;
							}
							gChgInfo.mode = 1;
							SwitchToUI_ReadCardSuccess();
							SpeechCardBalance(gChgInfo.user_card_balance);
						}
					} else {
						if (gCardReqFlag == 1) {
							if (CL_OK == CheckStartPara(gChgInfo.current_usr_gun_id) && (CL_OK == CheckOrderBuffStatus())) {
								gChgInfo.subsidyType = 0;
								gChgInfo.subsidyPararm = 0;
								StartCharging((ORDER_AUTH_CARD == gChgInfo.cardType) ? START_TYPE_AUTH_CARD : START_TYPE_MONTH_CARD, gChgInfo.money*100, gChgInfo.current_usr_card_id, pCardAuth->order, ORDER_SOURCE_AUTH_CARD);
							}
						} else {
							CL_LOG("card req fail.\n");
						}
						gCardReqFlag = 0;
					}
                    break;
                case 2: //账户余额不足或次数已经用完
					Sc8042bSpeech(VOIC_CARD_RECHARGER);
					if (pCardAuth->cardType == 0) {
						gChgInfo.user_card_balance = pCardAuth->user_momey;
						//gui显示卡余额
						SwitchToUi_VerifyCardFailure(1);
					}
                    break;
				case 3: //正在使用,不能同时使用
					SwitchToUi_VerifyCardFailure(2);
				 	Sc8042bSpeech(VOIC_CARD_INVALID);
					break;
                default:
               	 	SwitchToUi_VerifyCardFailure(2);
                    Sc8042bSpeech(VOIC_CARD_INVALID);
                    CL_LOG("card auth %d,err.\n",pCardAuth->result);
                    break;
            }
        }
        break;

        case MQTT_CMD_REMOTE_SET_POWER_ON:
        {
            LcdTurnOnLed();
            PrintfData("RecvServerData: charging start req", (void*)pFrame, len);
            START_CHARGING_REQ_STR* startChargingReq = (START_CHARGING_REQ_STR*)pFrame->data;

			if (CL_OK != CheckOrderBuffStatus()) {
				CL_LOG("order cnt=%d,err.\n",GetOrderCnt());
				return SendStartChargingAck(startChargingReq->gun_id, pFrame->head.sn, 1, START_FAIL_NO_MEMORY);
			}

            if (CL_OK != (ret = CheckStartPara(startChargingReq->gun_id))) {
                return SendStartChargingAck(startChargingReq->gun_id, pFrame->head.sn, 1, ret);
            }

            gChgInfo.user_card_balance = startChargingReq->money;
			gChgInfo.money = startChargingReq->chargingPara/100;
			SendStartChargingAck(startChargingReq->gun_id, pFrame->head.sn, 0, 0);
            vTaskDelay(50);
			gChgInfo.current_usr_gun_id = startChargingReq->gun_id;
            gChgInfo.mode = startChargingReq->mode;
			gChgInfo.subsidyType = 0;
			gChgInfo.subsidyPararm = 0;
            StartCharging((1 == startChargingReq->userAttribute) ? START_TYPE_ELE_MONTH_CARD : START_TYPE_SCAN_CODE, startChargingReq->chargingPara, startChargingReq->user_account, startChargingReq->order, startChargingReq->ordersource);
        }
        break;

        case MQTT_CMD_REMOTE_SET_POWER_OFF:
        {
            LcdTurnOnLed();
            //PrintfData("RecvServerData: recv power off req", (void*)pFrame, len);
            STOP_CHARGING_REQ_STR* stopChargingReq = (STOP_CHARGING_REQ_STR*)pFrame->data;
            pGunInfo = &gun_info[stopChargingReq->gun_id-1];

            if ((stopChargingReq->gun_id < 1) || (12 < stopChargingReq->gun_id)) {
                CL_LOG("gun=%d,err.\n",stopChargingReq->gun_id);
                SendStopChargingAck(stopChargingReq->gun_id, pFrame->head.sn, 1);
                OptFailNotice(106);
                return CL_FAIL;
            }
            SendStopChargingAck(stopChargingReq->gun_id, pFrame->head.sn, 0);
            pGunInfo->stopReason = STOP_PHONE;
            StopCharging(stopChargingReq->gun_id);
        }
        break;

        case MQTT_CMD_REPORT_POWER_ON://开启充电通知
        {
            LcdTurnOnLed();
            START_CHARGING_NOTICE_ACK_STR *ack = (void*)pFrame->data;
            CL_LOG("recv start charging notice ack,result=%d,gun=%d.\n",ack->result,ack->gun_id);
            if(ack->result == 0){
                gun_info_t *pGunInfo = &gun_info[ack->gun_id-1];
                pGunInfo->isSync = 0;
                if (0 != memcmp(ack->order, pGunInfo->order, sizeof(pGunInfo->order))) {
                    memcpy(pGunInfo->order,ack->order,sizeof(pGunInfo->order));
                }
                FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
                UpdataGunDataSum();
            }
        }
        break;

        case MQTT_CMD_REPORT_POWER_OFF://停止充电通知响应
        {
            LcdTurnOnLed();
            PrintfData("RecvServerData: stop charging notice ack", (void*)pFrame, len);
            STOP_CHARGING_NOTICE_ACK_STR* pStopChargingNoticeAck = (STOP_CHARGING_NOTICE_ACK_STR*)pFrame->data;
            if ((pStopChargingNoticeAck->gun_id < 1) || (12 < pStopChargingNoticeAck->gun_id)) {
                CL_LOG("gun=%d,err.\n",pStopChargingNoticeAck->gun_id);
                OptFailNotice(107);
                return CL_FAIL;
            }
			//删除正在上传的订单信息
			if ((pStopChargingNoticeAck->result == 0) && (gChgInfo.sendOrderGunId == pStopChargingNoticeAck->gun_id)) {
				Remove_RecordOrder_first();
				SendRecordTimers = 0;
                gChgInfo.sendOrderGunId = 0;
				SendTradeRecordNoticeTime = 0;
			}
        }
        break;

        case MQTT_CMD_HEART_BEAT:
        {
            int cnt;
            HEART_BEAT_ACK_STR *pHeartBeatAck = (HEART_BEAT_ACK_STR*)pFrame->data;
            if (pHeartBeatAck->time) {
                now = GetRtcCount();
                cnt = (now > pHeartBeatAck->time) ? now - pHeartBeatAck->time : pHeartBeatAck->time - now;
                if ((30 < cnt) && (0 == GetChargingGunCnt())) {
                    SycTimeCount(pHeartBeatAck->time);
                    SetRtcCount(pHeartBeatAck->time);
                    CL_LOG("set rtc,now=%d,acktime=%d.\n",now,pHeartBeatAck->time);
                }
            }
            CL_LOG("hb ack.\n");
            gSendHearBeatCnt = 0;
            if ((pHeartBeatAck->traffiId) && (pHeartBeatAck->traffiId != system_info.cost_template.template_id)) {
                SendReqCostTemplate(0);
            }
        }
        break;

        case MQTT_CMD_COST_DOWN:
        {
            //PrintfData("RecvServerData: recv cost template", (void*)pFrame, len);
            CostTemplateProc((void*)pFrame);
        }
        break;

        case MQTT_CMD_DFU_DOWN_FW_INFO:
        {
            DOWN_FW_REQ_STR* pfwInfo = (DOWN_FW_REQ_STR*)pFrame->data;
            if (LOCAL_NET == system_info.netType) {
                SetUpgradeInfo(pfwInfo);
            }else{
                OutNetUpgradeProc(pfwInfo);
            }
        }
        break;

        case MQTT_CMD_REMOTE_CTRL:
        {
            LcdTurnOnLed();
            PrintfData("RecvServerData: recv remote ctrl", (void*)pFrame, len);
            RemoteCtrlProc((void*)pFrame);
        }
        break;

        case MQTT_CMD_EVENT_NOTICE:
            PrintfData("RecvServerData: event notice ack", (void*)pFrame, len);
            ProcEventNoticeAck((void*)pFrame);
            break;

        #if (1 == ID2)
		case MQTT_CMD_AES_REQ://秘钥请求回复
		{
			AesInfoHandle(pFrame, len);
		}
		break;

		case MQTT_CMD_UPDATE_AES_NOTICE:
		{
			PrintfData("RecvServerData: server update aes notice", (void*)pFrame, len);
			UPDATE_AES_NOTICE_STR* pUpdateAesNotic = (UPDATE_AES_NOTICE_STR*)pFrame->data;
			//CL_LOG("server update aes notice, reason: %d.\n", pUpdateAesNotic->reason);
			//更新系统时间
			SycTimeCount(pUpdateAesNotic->time_utc);
			SetRtcCount(pUpdateAesNotic->time_utc);
			SendUpdateAesAck(pFrame->head.sn, 0);
            AesKeyUpdateFlag = 0;
            gChgInfo.ReqKeyReason = 2;
            gChgInfo.netStatus |= (1<<7);
            SoundCode(34);
		}
		break;
        #endif

        default:
            CL_LOG("unkown cmd %d.\n",pFrame->head.cmd);
            OptFailNotice(108);
            break;
    }
    return 0;
}


