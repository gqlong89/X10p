/*key.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "blueProto.h"
#include "proto.h"
#include "sc8042.h"
#include "card.h"
#include "ntcad.h"
#include "emu.h"
#include "rtc.h"
#include "sim800c.h"
#include "flash.h"
#include "lcd.h"
#include "gun.h"
#include "server.h"
#include "upgrade.h"
#include "ui.h"



BLUE_STATUS_STR gBlueStatus;
uint8_t current_send_order_gunid = 0;
extern uint16_t oneUploadTradeNum;
extern uint32_t uploadHistoryOrderTimes;



//计费模板上传
void BlueCostTemplateUpload(uint8_t gunid)
{
	BLUE_PROTO_STR pBluePkt;
    uint16_t temp;
	COST_TEMPLATE_HEAD_STR *cost = (void*)pBluePkt.data;
	uint16_t datalen = 6;

    memset(&pBluePkt,0,sizeof(BLUE_PROTO_STR));
	cost->gunId = system_info.cost_template.gunId;
	cost->template_id = system_info.cost_template.template_id;
	cost->mode = system_info.cost_template.mode;
	if(system_info.cost_template.mode == COST_UNIFY){
		temp = sizeof(unify_t);
	}else if(system_info.cost_template.mode == COST_POWER){
		temp = sizeof(segment_str)*system_info.cost_template.Data.powerInfo.segmentCnt + 1;
	}
    memcpy(cost->Data, &system_info.cost_template.Data, temp);
    cost->Data[temp] = system_info.chargerMethod;
	cost->Data[temp+1] = system_info.chargerStartingGold;
	cost->Data[temp+2] = system_info.chargerStartingGold>>8;
	datalen += temp+3;

	pBluePkt.head.start = 0xee;
	pBluePkt.head.cmd = B_COST_TEMPLATE_UPLOAD;
	pBluePkt.head.len = datalen;
	pBluePkt.data[pBluePkt.head.len] = GetPktSum((void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len);
	SendBlueNetPkt(NODE_BLUE, (void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len+1);
    PrintfData("BlueCostTemplateUpload", (void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len+1);
}


void BlueSendCostTemplate(BLUE_PROTO_STR *pMsg)
{
	BLUE_COST_TEMPLATE_REQ *cost = (void*)pMsg->data;
	if(cost->template_id != system_info.cost_template.template_id  && system_info.cost_template.template_id != 0){
		BlueCostTemplateUpload(cost->gun_id);
	}
}


//手机时间暂时不处理
void ShakeReqProc(BLUE_PROTO_STR *pMsg)
{
	BLUE_SHAKE_REQ_STR *shakeReq = (void*)pMsg->data;

	LcdDisplayBlutooth(LCD_DISPLAY);
	Sc8042bSpeech(VOIC_BLUETOOTH_ONLINE);
	gBlueStatus.lastRecvHeartBeat = GetRtcCount();
	memcpy(gChgInfo.current_usr_card_id, shakeReq->phonesSn, sizeof(shakeReq->phonesSn));
    gChgInfo.current_usr_card_id[sizeof(shakeReq->phonesSn)] = 0;
	CL_LOG("blue connect phonesSn:%s Time=%d\n",shakeReq->phonesSn,shakeReq->time);
	//ACK
    BLUE_SHAKE_ACK_STR *pAck = (void*)pMsg->data;

    memset(pAck, 0, sizeof(BLUE_SHAKE_ACK_STR));
    memcpy(pAck->name, CHARGER_NAME, strlen(CHARGER_NAME));
    memcpy(pAck->chargerSn, &system_info.station_id[3], sizeof(pAck->chargerSn));
    pAck->fwVer = FW_VERSION;
    pAck->portCnt = GUN_NUM_MAX;
    pAck->startNo = 1;
    pMsg->head.len = sizeof(BLUE_SHAKE_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);

    vTaskDelay(500);
    BlueSendHeartBeat();

	//请求计费模板
	vTaskDelay(500);
	BlueCostTemplateReq(0);
	uploadHistoryOrderTimes = GetRtcCount()-5;
	gBlueStatus.status = 1;
	oneUploadTradeNum = 0;//当有APP连接时，从新计算订单上传的个数
}


//计费模板下发TEMPLATE
void BlueCostTemplateDown(BLUE_PROTO_STR *pMsg)
{
	uint8_t result = 0;
	COST_TEMPLATE_HEAD_STR *pcost = (void*)pMsg->data;

    PrintfData("BlueCostTemplateDown",pMsg->data,pMsg->head.len);
	CL_LOG("cost_template mode=%d,id=%d.\n",pcost->mode,pcost->template_id);
    result = CostTempCopy(pcost);

	//ACK
	pMsg->data[0] = result;
	pMsg->head.len = 1;
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
	SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}

void OppoSiteAuthProc(BLUE_PROTO_STR *pMsg)
{

}


void BlueSendStartChargingAck(BLUE_PROTO_STR *pMsg, uint8_t gunId, uint8_t result, uint8_t reason)
{
    BLUE_START_CHARGING_ACK_STR *pStartAck = (void*)pMsg->data;
	gun_info_t *pGunInfo = &gun_info[gunId-1];

    pStartAck->port = gunId;
    pStartAck->result = result;
    pStartAck->reason = reason;
	if(result == 0){
		pStartAck->startTime = pGunInfo->start_time;
	}else{
		pStartAck->startTime = 0;
	}

    pStartAck->startElec = 0;
    pMsg->head.len = sizeof(BLUE_START_CHARGING_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
    PrintfData("BlueSendStartChargingAck", (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}


//需要判断离线的时候如果已经没有缓存订单空间就不能再开启充电
void BlueStartCharging(BLUE_PROTO_STR *pMsg)
{
    BLUE_START_CHARGING_STR *pStart = (void*)pMsg->data;
    int ret;
    uint8_t gun_id = pStart->port;
	gun_info_t *pGunInfo = &gun_info[gun_id-1];

	//如果该枪头正在充电，订单号一致 ，认为充电已经开启
	if(pGunInfo->is_load_on){
         if (memcmp(gZeroArray, pStart->order, sizeof(pStart->order))) {
            if(memcmp(pGunInfo->order,pStart->order,sizeof(pGunInfo->order)) == 0){
                BlueSendStartChargingAck(pMsg, gun_id, 0, 0);
                return;
            }
         }
	}

	//离线订单数量判断
	if (CL_OK != CheckOrderBuffStatus()) {
		CL_LOG("HistoryOrderNum is to many.\n");
		BlueSendStartChargingAck(pMsg, gun_id, 1, ret);
        return;
	}

	//充电参数判断
    if (CL_OK != (ret = CheckStartPara(gun_id))) {
        CL_LOG("call CheckStartPara fail.\n");
        BlueSendStartChargingAck(pMsg, gun_id, 1, ret);
        return;
    }

    gChgInfo.money = pStart->para/100;
	gChgInfo.mode = pStart->mode;
	gChgInfo.subsidyType = pStart->subsidyType;
	gChgInfo.subsidyPararm = pStart->subsidyPararm;
    gChgInfo.current_usr_gun_id = gun_id;
    StartCharging(START_TYPE_BLUE, pStart->para, gChgInfo.current_usr_card_id, pStart->order, pStart->orderSource);

	//ACK
    BlueSendStartChargingAck(pMsg, gun_id, 0, 0);
}


void BlueSendStopChargingAck(BLUE_PROTO_STR *pMsg, uint8_t gunId, uint8_t result)
{
    BLUE_STOP_CHARGING_ACK_STR *pStopAck = (void*)pMsg->data;
    gun_info_t *pGunInfo = &gun_info[gunId-1];

    pMsg->head.len = sizeof(BLUE_STOP_CHARGING_ACK_STR);
    pStopAck->port = gunId;
    pStopAck->result = result;
	pStopAck->stopTime = GetRtcCount();
	pStopAck->stopElec = pGunInfo->chargingElec;
    pStopAck->reason = pGunInfo->stopReason;
    pStopAck->startTime = pGunInfo->start_time;
    pStopAck->startElec = 0;
    pStopAck->money = pGunInfo->money;
    pStopAck->chargingPower = pGunInfo->chargingPower/10;
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
    PrintfData("BlueSendStopChargingAck", (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}


void BlueStopCharging(BLUE_PROTO_STR *pMsg)
{
    BLUE_STOP_CHARGING_STR *pStop = (void*)pMsg->data;
    gun_info_t *pGunInfo = &gun_info[pStop->port-1];

    if ((0 == pStop->port) || (GUN_NUM_MAX < pStop->port)) {
        CL_LOG("port=%d,error.\n",pStop->port);
        BlueSendStopChargingAck(pMsg, pStop->port, 1);
        return;
    }

    if (0 == pGunInfo->gun_state) {
        CL_LOG("gun is idle,err.\n");
        BlueSendStopChargingAck(pMsg, pStop->port, 0);
        return;
    }

    if (memcmp(pStop->order, pGunInfo->order, sizeof(pStop->order))) {
        CL_LOG("order is diff,err.\n");
        BlueSendStopChargingAck(pMsg, pStop->port, 1);
        return;
    }

    pGunInfo->stopReason = STOP_BLUE;
    pGunInfo->reasonDetail = REASON_BLUE;
    StopCharging(pStop->port);
    BlueSendStopChargingAck(pMsg, pStop->port, 0);
}


void BlueReqBreak(BLUE_PROTO_STR *pMsg)
{
    LcdDisplayBlutooth(LCD_CLEAR);
    gBlueStatus.status = 0;
    OpenBluetoothRadio();//打开蓝牙广播
    Sc8042bSpeech(VOIC_BLUETOOTH_OFFLINE);
}


//每30秒发送一次蓝牙心跳
void BlueSendHeartBeat(void)
{
    int i;
    GUN_STATUS_STR gunStatus;
	//GUN_CHARGING_STR *pGunCharging ;
    uint8_t  sendBuff[64] = {0};
    BLUE_PROTO_STR *pBluePkt = (void*)sendBuff;
    HEART_BEAT_STR *pHeart = (void*)pBluePkt->data;
    BLUE_GUN_HEART_BEAT_STR *pGunHeart = pHeart->gunStatus;
    gun_info_t *pGunInfo = NULL;
    uint16_t leftTime;

    pHeart->simSignal = GetNetSignal();
    pHeart->temp = GetCpuTemp() + 50;
    pHeart->portCnt = GUN_NUM_MAX;
    for (i=1; i<=GUN_NUM_MAX; i++) 
	{
		//pGunCharging = &gChgInfo.gunCharging[i-1];
        pGunHeart->port = i;
        pGunInfo = &gun_info[i-1];
        if (pGunInfo->is_load_on) 
		{
            pGunHeart->status = 1;
            leftTime = GetLeftChargingTime(i);
            pGunHeart->errCode = (255 <= leftTime) ? 255 : leftTime;
        }
		else
		{
            pGunHeart->status = 0;
            GetGunStatus(i, &gunStatus);
            if (0 == gunStatus.status) 
			{
                if (CL_OK != IsSysOnLine()) 
				{
                    pGunHeart->status = 3;
                }
            }
			else
			{
                pGunHeart->errCode = pGunHeart->status;
                pGunHeart->status = 2;
            }
        }
        pGunHeart++;
    }
    pBluePkt->head.start = 0xee;
    pBluePkt->head.cmd = B_HEART_BEAT;
    pBluePkt->head.len = sizeof(HEART_BEAT_STR);
    pBluePkt->data[pBluePkt->head.len] = GetPktSum((void*)pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len+1);
    PrintfData("BlueSendHeartBeat 发送BT心跳包", (void*)pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len+1);
}


//上传历史订单
void BlueUpLoadHistoryOrder(HISTORY_ORDER_STR *order)
{
	BLUE_PROTO_STR pBluePkt;
	memset(&pBluePkt,0,sizeof(BLUE_PROTO_STR));

	BLUE_UPLOAD_HISTORY_ORDER_REQ_STR pTradeRecord;
	memset(&pTradeRecord,0,sizeof(BLUE_UPLOAD_HISTORY_ORDER_REQ_STR));

	pTradeRecord.chargerMode = order->chargerMode;
	pTradeRecord.chargerPararm = order->chargerPararm;
	pTradeRecord.cost_price = order->cost_price;
	pTradeRecord.fw_version = order->fw_version;
	pTradeRecord.gun_id = order->gun_id;
	memcpy(pTradeRecord.order,order->order,sizeof(order->order));
	pTradeRecord.ordersource = order->ordersource;
	memcpy(pTradeRecord.phoneSn,order->user_account,sizeof(pTradeRecord.phoneSn));
	pTradeRecord.power = order->power;
	pTradeRecord.startElec = order->startElec;
    pTradeRecord.stopElec = order->stopElec;
	pTradeRecord.startTime = order->startTime;
	pTradeRecord.stopDetails = order->stopDetails;
	pTradeRecord.stopTime = order->stopTime;
	pTradeRecord.subsidyPararm = order->subsidyPararm;
	pTradeRecord.subsidyType = order->subsidyType;
	pTradeRecord.template_id = order->template_id;
	memcpy(pBluePkt.data,&pTradeRecord,sizeof(BLUE_UPLOAD_HISTORY_ORDER_REQ_STR));

	pBluePkt.head.start = 0xee;
    pBluePkt.head.cmd = B_HISTORY_ORDER_UPLOAD;
    pBluePkt.head.len = sizeof(BLUE_UPLOAD_HISTORY_ORDER_REQ_STR);
    pBluePkt.data[pBluePkt.head.len] = GetPktSum((void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len);
    current_send_order_gunid = pTradeRecord.gun_id;
	SendBlueNetPkt(NODE_BLUE, (void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len+1);

	CL_LOG("BlueUpLoadHistoryOrder : gun_id=%d startTime=%d stopTime=%d money=%d \n",order->gun_id,order->startTime,order->stopTime,order->cost_price);
}


//订单上传响应
void BlueHistoryOrderUpload_ACK(BLUE_PROTO_STR *pMsg)
{
	BLUE_UPLOAD_HISTORY_ORDER_ACK_STR *orderAck = (void*)pMsg->data;

	if(orderAck->result == 0 && current_send_order_gunid != 0){
		oneUploadTradeNum++;
		TradeIndex_Add();//订单上传完成，进行下一个订单
		uploadHistoryOrderTimes = 0;
        current_send_order_gunid = 0;
	}
}


//计费模板请求
void BlueCostTemplateReq(uint8_t gunid)
{
	BLUE_PROTO_STR pBluePkt;
	BLUE_COST_TEMPLATE_REQ *pcost = (void*)pBluePkt.data;

	memset(&pBluePkt,0,sizeof(BLUE_PROTO_STR));
	pcost->gun_id = gunid;
	pcost->template_id = system_info.cost_template.template_id;
	pBluePkt.head.start = 0xee;
    pBluePkt.head.cmd = B_REQ_COST_TEMPLATE;
    pBluePkt.head.len = sizeof(BLUE_COST_TEMPLATE_REQ);
    pBluePkt.data[pBluePkt.head.len] = GetPktSum((void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len+1);
    PrintfData("BlueCostTemplateReq", (void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len+1);
}


//历史订单上传确认
void BlueHistoryOrderEnsure(BLUE_PROTO_STR *pMsg)
{
	BLUE_Record_ENSURE_REQ_STR *record = (void*)pMsg->data;
	uint8_t gun_id = record->gun_id;
	uint32_t startTime = record->startTime;
    CL_LOG("OrderEnsure gun_id=%d  startTime=%d  stopTime=%d\n",gun_id,startTime,record->stopTime);

	int result = Remove_RecordOrder(gun_id,startTime,record->stopTime);

	if(result == CL_OK){
        oneUploadTradeNum--;
		CL_LOG("Remove_RecordOrder success.\n");
	}else{
		CL_LOG("Remove_RecordOrder failed.\n");
	}

	//ACK
	BLUE_Record_ENSURE_ACK_STR *pACK = (void*)pMsg->data;
	pACK->result = 0;
	pACK->startTime = startTime;
	pACK->gun_id = gun_id;

    pMsg->head.len = sizeof(BLUE_Record_ENSURE_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}


//蓝牙断开的时候，每隔15分钟，发送一次打开蓝牙广播的命令
void BlueTimingOpen(void)
{
    uint32_t rtcTime = GetRtcCount();

    if (0 == gBlueStatus.status) {
        if ((60*15) < (uint32_t)(rtcTime - gChgInfo.blueCheck)) {
            OpenBluetoothRadio();//打开蓝牙广播
            gChgInfo.blueCheck = rtcTime;
        }
    }
}


void ProcBtHeartBeat(void)
{
    uint32_t rtcTime = GetRtcCount();

    if (gBlueStatus.status) {
        if (30 < (uint32_t)(rtcTime - gBlueStatus.lastRecvHeartBeat)) {
            LcdDisplayBlutooth(LCD_CLEAR);
            gBlueStatus.status = 0;
            OpenBluetoothRadio();//打开蓝牙广播
            Sc8042bSpeech(VOIC_BLUETOOTH_OFFLINE);
        }
    }
}


void BtSetChargerSn(BLUE_PROTO_STR *pMsg)
{
    uint8_t result = 0;

    PrintfData("BtSetChargerSn set sn", pMsg->data, sizeof(system_info.station_id));
    if (GetOrderCnt() || GetChargingGunCnt() > 0){
        result = 1;
    }else{
        if (memcmp(system_info.station_id,pMsg->data,CHARGER_SN_LEN) != 0) {
            WriteCfgInfo(CFG_CHARGER_SN, CHARGER_SN_LEN, pMsg->data);

            memset(gun_info, 0, sizeof(gun_info));
		    FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
            memcpy(system_info.station_id, pMsg->data, CHARGER_SN_LEN);
			memset(system_info.idCode, 0, sizeof(system_info.idCode));
            WriteCfgInfo(CFG_CHARGER_DEVICEID, CHARGER_SN_LEN, system_info.idCode);
			system_info.cost_template.template_id = 0;
            UpdataGunDataSum();
            Clear_RecordOrder();
        }
    }

    pMsg->head.len = 1;
    pMsg->data[0] = result;
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
    ResetSysTem();
}


void BtSetChargerDeviceId(BLUE_PROTO_STR *pMsg)
{
    PrintfData("BtSetChargerDeviceId deviceId", pMsg->data, sizeof(system_info.idCode));
    WriteCfgInfo(CFG_CHARGER_DEVICEID, CHARGER_SN_LEN, pMsg->data);

    memcpy(system_info.idCode, pMsg->data, sizeof(system_info.idCode));
    FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
    pMsg->head.len = 1;
    pMsg->data[0] = 0;
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
    ResetSysTem();
}


void SendBtRemoCtrlAck(BLUE_PROTO_STR *pMsg, uint8_t result)
{
    BLUE_REMO_CTRL_ACK_STR *pAck = (void*)pMsg->data;

    pAck->result = result;
    pMsg->head.len = sizeof(BLUE_REMO_CTRL_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}


//设备注册响应
void BlueDevRegisterACK(BLUE_PROTO_STR *pMsg)
{
	BLUE_REGISTER_ACK_STR *pRegister = (void*)pMsg->data;

	if (pRegister->result == 0) {
		PrintfData("idcode:", (void*)pRegister->idcode, 8);
        WriteCfgInfo(CFG_CHARGER_DEVICEID, CHARGER_SN_LEN, pRegister->idcode);

		memcpy((void*)system_info.idCode, (void*)pRegister->idcode, sizeof(system_info.idCode));
		FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
		CL_LOG("register reboot.\n");
        ResetSysTem();
	}
}

//设备注册 设备第一次使用时会先注册
void BlueRegister(void)
{
	BLUE_PROTO_STR pBluePkt;
	memset(&pBluePkt,0,sizeof(BLUE_PROTO_STR));

	BLUE_REGISTER_REQ_STR *msg = (void*)pBluePkt.data;

    memcpy(msg->device_type, STATION_MACHINE_TYPE, strlen(STATION_MACHINE_TYPE));
    memset(msg->register_code, 0, sizeof(msg->register_code));
    memcpy(msg->register_code, REGISTER_CODE, strlen(REGISTER_CODE));

	pBluePkt.head.start = 0xee;
    pBluePkt.head.cmd = B_DEV_REGISTER;
    pBluePkt.head.len = sizeof(BLUE_REGISTER_REQ_STR);
    pBluePkt.data[pBluePkt.head.len] = GetPktSum((void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)&pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt.head.len+1);
}

void BtRemoCtrlReset(BLUE_PROTO_STR *pMsg)
{
    SendBtRemoCtrlAck(pMsg, CL_OK);
    ResetSysTem();
}


void BtRemoCtrlPrint(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;

    SetPrintSwitch(pReq->para);
    SendBtRemoCtrlAck(pMsg, CL_OK);
}


void BtSetChargeChangePower(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;

    SetChargeChangePowerFuncy(pReq->para);
    SendBtRemoCtrlAck(pMsg, CL_OK);
}


void BtRemoCtrlOpenGun(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;
    int ret = TestOpenGunProc(pReq->para, 3*60, TEST_OPEN);

    SendBtRemoCtrlAck(pMsg, ret);
}


void BtRemoCtrlCloseGun(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;
    int ret = TestCloseGunProc(pReq->para);

    SendBtRemoCtrlAck(pMsg, ret);
}


void BtSetFullTime(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;

    CL_LOG("chargingFullTime=%d.\n",pReq->para);
    system_info.chargingFullTime = (CHARGING_FULL_TIME < pReq->para) ? pReq->para : CHARGING_FULL_TIME;
    FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
    SendBtRemoCtrlAck(pMsg, CL_OK);
}


void BtSetPullGunTime(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;

    CL_LOG("pgst=%d.\n",pReq->para);
    SetPullGunStopTimeFuncy(pReq->para);
    SendBtRemoCtrlAck(pMsg, CL_OK);
}


// pMsg前面预留有 BLUE_HEAD_STR 的内存
void RemoCtrlProc(BLUE_PROTO_STR *pMsg)
{
    REMO_CTRL_REQ_STR *pReq = (void*)pMsg->data;

    switch (pReq->optCode) {
        case SYSTEM_REBOOT:
            BtRemoCtrlReset(pMsg);
            break;
        case CTRL_OPEN_GUN:
            BtRemoCtrlOpenGun(pMsg);
            break;
        case CTRL_CLOSE_GUN:
            BtRemoCtrlCloseGun(pMsg);
            break;
        case CTRL_SET_FULL_TIME:
            BtSetFullTime(pMsg);
            break;
        case CTRL_SET_PULL_GUN_TIME:
            BtSetPullGunTime(pMsg);
            break;
        case CTRL_SET_PRINT_SWITCH:
            BtRemoCtrlPrint(pMsg);
            break;
        case CTRL_SET_CHARGE_CHANGE_POWER:
            BtSetChargeChangePower(pMsg);
            break;
    }
}


void SendFwUpgradeNoticeAck(BLUE_PROTO_STR *pMsg, uint8_t result)
{
	BLUE_DOWN_FW_ACK_STR *pACK = (void*)pMsg->data;

	pACK->result = result;
	pMsg->head.len = sizeof(BLUE_DOWN_FW_ACK_STR);
	pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}


//固件升级通知
void BlueFWUpgradeNotice(BLUE_PROTO_STR *pMsg)
{
	uint8_t result = 0;
	BLUE_DOWN_FW_REQ_STR *pFW = (void*)pMsg->data;

    if (gChgInfo.netStatus & (1<<OUT_NET_UPGRADE)) {
        CL_LOG("错误,外网升级中,无法进行蓝牙升级.\n");
        SendFwUpgradeNoticeAck(pMsg, 1);
        return;
    }

	if(pFW->fw_version != FW_VERSION){
		//记录固件信息
		CL_LOG("upgrade package size=%d checkSum=%X package_num=%d fw_version=%d.\n",pFW->fw_size,pFW->checkSum,pFW->package_num,pFW->fw_version);
		Upgrade_Init(pFW->package_num,pFW->fw_version);
        gChgInfo.size = GetRtcCount();
        gChgInfo.netStatus |= (1<<BLUE_UPGRADE);
	}else{
		CL_LOG("fw_version is equal,cann't upgrade.\n");
		result = 1;
	}
    SendFwUpgradeNoticeAck(pMsg, result);
}


//固件下发
void BlueFWDownLoad(BLUE_PROTO_STR *pMsg)
{
	BLUE_FW_DOWNLOAD_REQ_STR *pFW = (void*)pMsg->data;
	uint8_t index = pFW->data[0];
	//保存数据
	uint8_t result = Upgrade_Write(pFW->data,pMsg->head.len);

	//ACK
	BLUE_FW_DOWNLOAD_ACK_STR *pACK = (void*)pMsg->data;
	pACK->result = (result == 0 ? 1:0);
	pACK->index = index;
	pMsg->head.len = sizeof(BLUE_FW_DOWNLOAD_ACK_STR);
	pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    SendBlueNetPkt(NODE_BLUE, (void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
	if(result == 2){
        OptSuccessNotice(805);
		CL_LOG("system will reboot.\n");
		ResetSysTem();
	}
}


void CheckBlueUpgrade(void)
{
    if (gChgInfo.netStatus & (1<<BLUE_UPGRADE)) 
	{
        if ((8*60) < (uint32_t)(GetRtcCount() - gChgInfo.size)) 
		{
            gChgInfo.netStatus &= (~(1<<BLUE_UPGRADE));
        }
    }
}


void BlueProtoProc(BLUE_PROTO_STR *pMsg, uint16_t len)
{
    switch (pMsg->head.cmd) 
	{
		case B_DEV_REGISTER:	//注册响应
			CL_LOG("BlueProtoProc dev register ack.\n");
			BlueDevRegisterACK(pMsg);
			break;
        case B_SHAKE_REQ:	//握手请求
            PrintfData("蓝牙握手请求", (void*)pMsg, len);
            ShakeReqProc(pMsg);
            break;
        case B_OPPO_SITE_AUTH:	//
            PrintfData("用户鉴权", (void*)pMsg, len);
            OppoSiteAuthProc(pMsg);
            break;
        case B_START_CHARGING:	//开启充电
            PrintfData("开启充电", (void*)pMsg, len);
            BlueStartCharging(pMsg);
            break;
        case B_STOP_CHARGING:	//结束充电
            PrintfData("结束充电", (void*)pMsg, len);
            BlueStopCharging(pMsg);
            break;
		case B_HEART_BEAT:	//心跳
            gBlueStatus.lastRecvHeartBeat = GetRtcCount();
			CL_LOG("蓝牙心跳.\n");
			break;
        case B_REQ_COST_TEMPLATE://请求计费模板
            PrintfData("请求计费模板", (void*)pMsg, len);
            BlueSendCostTemplate(pMsg);
            break;
		case B_COST_TEMPLATE_DOWNLOAD://计费模板下发
			CL_LOG("计费模板下发.\n");
			BlueCostTemplateDown(pMsg);
			break;
        case B_REQ_BREAK:	//请求断开蓝牙链接
            PrintfData("请求断开蓝牙链接", (void*)pMsg, len);
            BlueReqBreak(pMsg);
            break;
        case BT_CMD_SET_CHARGER_SN:	//设置充电桩编号
            BtSetChargerSn(pMsg);
            break;
        case BT_CMD_SET_DEVICE_ID:	//设置充电桩识别码
            BtSetChargerDeviceId(pMsg);
            break;
        case BT_CMD_REMO_CTRL:	//远程控制
            RemoCtrlProc(pMsg);
            break;
		case B_HISTORY_ORDER_UPLOAD:	//历史订单上传ack
			CL_LOG("历史订单上传ack.\n");
			BlueHistoryOrderUpload_ACK(pMsg);
			break;
		case B_HISTORY_ORDER_ENSURE://历史订单确认
			CL_LOG("历史订单确认.\n");
			BlueHistoryOrderEnsure(pMsg);
			break;
		case B_FW_UPGRADE_NOTICE:	//固件升级开始通知
			BlueFWUpgradeNotice(pMsg);
			break;
		case B_FW_DOWN_LOAD://固件下发
			BlueFWDownLoad(pMsg);
			break;
    }
}


