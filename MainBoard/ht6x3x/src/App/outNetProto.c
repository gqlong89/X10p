
#include "includes.h"
#include "outNetProto.h"
#include "proto.h"
#include "card.h"
#include "productTest.h"
#include "rtc.h"
#include "usart.h"
#include "sim800c.h"
#include "sc8042.h"
#include "flash.h"
#include "server.h"
#include "blueProto.h"
#include "ui.h"


OUT_NET_STATUS_STR gOutNetStatus;
uint16_t gSendPktLen = 0;
uint8_t gSendPktBuff[OUT_NET_PKT_LEN];
uint8_t gRecvPktBuff[NET_485_PKT_LEN];


void CheckDialValue(void)
{
    int ret = GetDialValue();

    if (gOutNetStatus.rs485Node != ret) {
        CL_LOG("dial switch change, %d ->%d.\n",gOutNetStatus.rs485Node,ret);
        gOutNetStatus.rs485Node = ret;
        Sc8042bSpeech(ret+31);
    }
}


//len为报文静荷长度
int SendOutNetPktFuncy(uint8_t ifType, OUT_ACK_PKT_STR *pFrame, uint16_t len)
{
    pFrame->head.aa = 0xaa;
    pFrame->head.five = 0x55;
    pFrame->head.len = 4+len;
    pFrame->head.ver = 1;
    pFrame->data[len-sizeof(OUT_ACK_HEAD_STR)] = GetPktSum((void*)&pFrame->head.ver, len+3);
    if (OUT_485_NET == ifType) {
        pFrame->head.src[0] = gOutNetStatus.rs485Node;
        //PrintfData("SendOutNetPktFuncy 485", (void*)pFrame, sizeof(OUT_NET_HEAD_STR)+len+1);
        UsartFlush(UART_485_PORT);
        return UsartSend(UART_485_PORT, (void*)pFrame, sizeof(OUT_NET_HEAD_STR)+len+1);
    }else{
        CL_LOG("ifType=%d,error.\n",ifType);
    }
    return CL_FAIL;
}


//发送电桩sn，识别码
//如果没有发送返回-1
int SendChargerInfo(uint8_t ifType, OUT_ACK_PKT_STR *pkt)
{
    OUT_HEART_BEAT_ACK_STR *pFrame = (void*)&pkt->ackHead;
    CHARGER_INFO_STR *pChargerInfo = (void*)pFrame->pkt;

    pkt->head.sn = gOutNetStatus.frameSn++;
    pkt->head.cmd = OUT_NET_HEART_BEAT;
    pFrame->ackHead.status = 0;
    pFrame->ackHead.reqOpt = SET_SN_ID;
    pFrame->ackHead.len = sizeof(CHARGER_INFO_STR);
    memcpy(pChargerInfo->chargerSn, &system_info.station_id[3], sizeof(pChargerInfo->chargerSn));
    memcpy(pChargerInfo->deviceId, system_info.idCode, sizeof(pChargerInfo->deviceId));
    //PrintfData("SendChargerInfo", (void*)&pkt->ackHead, sizeof(OUT_ACK_HEAD_STR)+sizeof(CHARGER_INFO_STR));
    return SendOutNetPktFuncy(ifType, pkt, sizeof(OUT_ACK_HEAD_STR)+sizeof(CHARGER_INFO_STR));
}


int SendReqFwUpdata(uint8_t ifType, OUT_ACK_PKT_STR *pkt)
{
    OUT_HEART_BEAT_ACK_STR *pFrame = (void*)&pkt->ackHead;

    pkt->head.sn = gOutNetStatus.frameSn++;
    pkt->head.cmd = OUT_NET_HEART_BEAT;
    pFrame->ackHead.status = 0;
    pFrame->ackHead.reqOpt = REQ_FW;
    pFrame->ackHead.len = 0;
    return SendOutNetPktFuncy(ifType, pkt, sizeof(OUT_ACK_HEAD_STR));
}


//pPkt报文前面预留有 OUT_NET_HEAD_STR 和 OUT_ACK_HEAD_STR 的内存空间
int SendOutNetPkt(uint8_t *pPkt, uint16_t len, uint8_t type)
{
    OUT_ACK_PKT_STR *pFrame = (OUT_ACK_PKT_STR*)(pPkt-sizeof(OUT_NET_HEAD_STR)-sizeof(OUT_ACK_HEAD_STR));

    pFrame->head.sn = gOutNetStatus.frameSn++;
    pFrame->head.cmd = OUT_NET_HEART_BEAT;
    pFrame->ackHead.status = gOutNetStatus.status;
    gOutNetStatus.status = 0;
    pFrame->ackHead.reqOpt = type;
    pFrame->ackHead.len = len;
    //PrintfData("SendOutNetPkt", (void*)pFrame, sizeof(OUT_NET_HEAD_STR)+sizeof(OUT_ACK_HEAD_STR)+len);
    return SendOutNetPktFuncy(gOutNetStatus.connect, pFrame, sizeof(OUT_ACK_HEAD_STR)+len);
}


//type: REQ_SEND_PKT
int PutOut485NetPkt(uint8_t *pkt, uint16_t len, uint8_t type)
{
    int i = 0;
    OUT_ACK_PKT_STR *pFrame = (void*)gSendPktBuff;

    while (gSendPktLen) {
        vTaskDelay(100);
        if (200 < ++i) {
            CL_LOG("wait fail.\n");
            return CL_FAIL;
        }
    }
    if (sizeof(gSendPktBuff) < len) {
        CL_LOG("len=%d,error.\n",len);
        return CL_FAIL;
    }

    memcpy(pFrame->data, pkt, len);
    pFrame->ackHead.reqOpt = type;
    gSendPktLen = len;
    CL_LOG("ready send pkt to dut,len=%d.\n",len);
    return CL_OK;
}


//pkt:指向数据体，如:aa 55 02
int PutOutNetPkt(uint8_t *pkt, uint16_t len, uint8_t type)
{
    if (OUT_485_NET == system_info.netType) 
	{
        return PutOut485NetPkt(pkt, len, type);
    }
	else
	{
        return SendOutNetPkt(pkt, len, type);
    }
}


int SendFwInfo(FRAME_STR *pkt, DOWN_FW_REQ_STR *pFwInfo)
{
    memcpy(&pkt->protoHead, pFwInfo, sizeof(DOWN_FW_REQ_STR));
    PrintfData("SendFwInfo", (void*)pFwInfo, sizeof(DOWN_FW_REQ_STR));
    return PutOutNetPkt((void*)&pkt->protoHead, sizeof(DOWN_FW_REQ_STR), SET_UPGRADE);
}


// 拉远2g模块检测
int OutSizeNetCheck(void)
{
    uint32_t i;

    CL_LOG("check out net...\n");
	for (i = 0; i < 15; i++) 
	{
        if (0 != gOutNetStatus.connect) 
		{
            CL_LOG("check out net ok,connect=%d, 2:485,3:2.4G.\n",gOutNetStatus.connect);
            return CL_OK;
        }
        vTaskDelay(1000);
    }
    CL_LOG("check out net fail.\n");
    return CL_FAIL;
}


void RecvAckProc(OUT_PKT_STR *pPkt)
{
    OUT_ACK_PKT_STR *pFrame = NULL;

    if (gSendPktLen) {
        pFrame = (void*)gSendPktBuff;
        SendOutNetPkt((void*)pFrame->data, gSendPktLen, pFrame->ackHead.reqOpt);
        gSendPktLen = 0;
    }else{
        if (gChgInfo.fwInfo.size && ((system_info.fwInfo.size != gChgInfo.fwInfo.size) || (system_info.fwInfo.checkSum != gChgInfo.fwInfo.checkSum))) {
            CL_LOG("local fw size=%d,sum=%#x, is diff from net gw fw size=%d,sum=%#x,need to upgrade.\n",system_info.fwInfo.size,system_info.fwInfo.checkSum,gChgInfo.fwInfo.size,gChgInfo.fwInfo.checkSum);
            gChgInfo.size = 0;
            gChgInfo.sum = 0;
            SendReqFwUpdata(OUT_485_NET, (void*)pPkt);
        }else{
            SendChargerInfo(OUT_485_NET, (void*)pPkt);
        }
    }
}


void PktRecvProc(OUT_PKT_STR *pPkt)
{
    int i;
    OUT_HEART_BEAT_REQ_STR *pMsg = (void*)pPkt->data;

    if (REQ_RECV_PKT == pMsg->cmd) {
        //PrintfData("OutNetRecvProc", pMsg->pkt, pMsg->len);
        for (i=0; i<pMsg->len;) {
            if (CL_OK == FIFO_S_Put(&gSocketPktRxCtrl, pMsg->pkt[i])) {
                i++;
            }else {
                CL_LOG("call FIFO_S_Put fail.\n");
                vTaskDelay(10);
            }
        }
        gOutNetStatus.status = 1;
    }else if (REQ_GET_CCID == pMsg->cmd) {
        if (memcmp(system_info.iccid, pMsg->pkt, ICCID_LEN)) {
            memcpy(system_info.iccid, pMsg->pkt, ICCID_LEN);
            system_info.iccid[ICCID_LEN] = 0;
            FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
        }
        memcpy(&gChgInfo.fwInfo, (void*)&pMsg->pkt[ICCID_LEN], sizeof(gChgInfo.fwInfo));
        gOutNetStatus.status = 0;
    }else if (REQ_DEVICEID == pMsg->cmd) {
    }else if (REQ_RECV_FW_INFO == pMsg->cmd) {
        if (0 == gChgInfo.size) { //停止网络动作
            gChgInfo.lastOpenTime = GetRtcCount();
            gChgInfo.sendPktFlag = 1;
            system_info.fwInfo.size = gChgInfo.fwInfo.size;
            system_info.fwInfo.checkSum = gChgInfo.fwInfo.checkSum;
            gChgInfo.netStatus = (1<<OUT_NET_UPGRADE);
            FlashEraseAppBackup();
            Sc8042bSpeech(VOIC_START_UPGRADE);
            SwitchToUi_EquipUpgrade();
        }

        //固件写flash
        CL_LOG("write %d byte fw to flash, all size=%d, writed size=%d.\n",pMsg->len,gChgInfo.fwInfo.size,gChgInfo.size);
        FlashWriteAppBackup(AppUpBkpAddr+gChgInfo.size, pMsg->pkt, pMsg->len);

        gChgInfo.size += pMsg->len;
        for (i=0; i<pMsg->len; i++) {
            gChgInfo.sum += pMsg->pkt[i];
        }

        if (gChgInfo.size >= gChgInfo.fwInfo.size) {
            RecvAckProc(pPkt);
            CL_LOG("local size=%d,size=%d.\n",gChgInfo.size,gChgInfo.fwInfo.size);
            if (gChgInfo.sum == gChgInfo.fwInfo.checkSum) {
                WriteUpdateInfo(gChgInfo.size, gChgInfo.sum);
                FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
                OptSuccessNotice(803);
                CL_LOG("upgrade fw ok.\n");
            }else{
                CL_LOG("local sum=%#x,checkSum=%#x,error.\n",gChgInfo.sum,gChgInfo.fwInfo.checkSum);
                memset(&system_info.fwInfo, 0, sizeof(system_info.fwInfo));
                FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
                OptFailNotice(119);
            }
            ResetSysTem();
        }
    }

    RecvAckProc(pPkt);
}


void Out485NetRecvProc(OUT_PKT_STR *pPkt)
{
    OUT_HEART_BEAT_REQ_STR *pMsg = (void*)pPkt->data;

    if (pPkt->head.dest[0] == gOutNetStatus.rs485Node) { //485网络需判断主节点本次是否轮询该从节点，是才能回报文
        if (DIAL_SUCCESS == pMsg->localStatus) {
            gNetSignal = pMsg->simSignel;
        }
        gOutNetStatus.lastRecvTime = GetRtcCount();
        gOutNetStatus.connect = OUT_485_NET;
        PktRecvProc(pPkt);
    }
}


void OutNetRecvProc(OUT_PKT_STR *pPkt, uint16_t len)
{
    OUT_HEART_BEAT_REQ_STR *pMsg = (void*)pPkt->data;

    if (OUT_NET_START == pPkt->head.cmd) 
	{
        gOutNetStatus.mode = 1;
    }
	else if (TEST_START == pPkt->head.cmd) 
	{
        gOutNetStatus.mode = 2;
        TestDataHandle(pPkt, len);
        return;
    }

    if (2 == gOutNetStatus.mode) 
	{
        TestDataHandle(pPkt, len);
        return;
    }

    if ((OUT_NET_HEART_BEAT == pPkt->head.cmd) || (OUT_NET_START == pPkt->head.cmd)) 
	{
        if (OUT_485_NET == pMsg->netType) 
		{
            Out485NetRecvProc(pPkt);
        }
    }
}


void OutNetConnetProc(void)
{
    if (gOutNetStatus.connect) 
	{
        if (60 < (uint32_t)(GetRtcCount() - gOutNetStatus.lastRecvTime)) 
		{
            CL_LOG("out net connect break,connect type=%d, change to 0(error).\n", gOutNetStatus.connect);
            gOutNetStatus.connect = 0;
        }
    }
}


void OutNetUpgradeProc(void *pFwInfo)
{
    FRAME_STR *pkt = (void*)gProtoSendBuff;

    MuxSempTake(&gProtoSendMux);
	SendFwInfo((void*)pkt, pFwInfo);
    MuxSempGive(&gProtoSendMux);
}


void UpgradeTimeOutProc(void)
{
    if (1 == gChgInfo.sendPktFlag) 
	{
        if (900 < (uint32_t)(GetRtcCount()- gChgInfo.lastOpenTime)) 
		{
            OptFailNotice(118);
            CL_LOG("upgrade timeout,or connect break,connect=%d,root system.\n",gOutNetStatus.connect);
            ResetSysTem();
        }
    }
	else if (2 == gChgInfo.sendPktFlag) 
	{
        if (600 < (uint32_t)(GetRtcCount()- gChgInfo.lastOpenTime)) 
		{
            OptFailNotice(120);
            CL_LOG("local upgrade timeout.\n");
            ResetSysTem();
        }
    }
}


void SysTask(void)
{
    int i;
    uint8_t  *pRecvBuff = gRecvPktBuff;
    uint8_t  data;
    uint8_t  step = FIND_AA;
    uint8_t  length;
    uint8_t  sum;
    uint16_t frameLen;
    uint32_t oldTime;

    gOutNetStatus.rs485Node = GetDialValue();
    CL_LOG("rs485 node=%d.\n", gOutNetStatus.rs485Node);
    while (1) 
	{
        vTaskDelay(25);
        if (oldTime != GetRtcCount()) 
		{
            oldTime = GetRtcCount();
            OutNetConnetProc();
            UpgradeTimeOutProc();
        }

        while (CL_OK == UsartGetOneData(UART_485_PORT, &data)) 
		{
            switch (step) 
			{
                case FIND_AA:
                    if (0xaa == data) {
                        i = 0;
                        pRecvBuff[i++] = data;
                        step = FIND_55;
                    }
                    break;
                case FIND_55:
                    if (0x55 == data) {
                        step = FIND_SRC;
                        pRecvBuff[i++] = data;
                        length = 0;
                    }else {
                        step = FIND_55;
                    }
                    break;
                case FIND_SRC:
                    pRecvBuff[i++] = data;
                    if (5 <= ++length) {
                        step = FIND_DEST;
                        length = 0;
                    }
                    break;
                case FIND_DEST:
                    pRecvBuff[i++] = data;
                    if (5 <= ++length) {
                        step = FIND_LEN;
                        length = 0;
                    }
                    break;
                case FIND_LEN:
                    pRecvBuff[i++] = data;
                    if (2 <= ++length) {
                        frameLen = (pRecvBuff[i-1]<<8) | pRecvBuff[i-2];
                        if (NET_485_PKT_LEN <= frameLen) {
                            step = FIND_AA;
                            CL_LOG("frameLen=%d,error.\n",frameLen);
                        }else{
                            step = FIND_VER;
                            sum = 0;
                        }
                    }
                    break;
                case FIND_VER:
                    pRecvBuff[i++] = data;
                    sum += data;
                    step = FIND_SERNUM;
                    break;
                case FIND_SERNUM:
                    pRecvBuff[i++] = data;
                    sum += data;
                    step = FIND_CMD;
                    break;
                case FIND_CMD:
                    pRecvBuff[i++] = data;
                    sum += data;
                    frameLen = (frameLen >= 4) ? frameLen - 4 : 0;
                    if (0 == frameLen) {
                        step = FIND_CHK;
                    }else{
                        step = RX_DATA;
                    }
                    break;
                case RX_DATA:
                    pRecvBuff[i++] = data;
                    sum += data;
                    if (0 == --frameLen) {
                        step = FIND_CHK;
                    }
                    break;
               case FIND_CHK:
                    pRecvBuff[i++] = data;
                    if (sum == data) 
					{
                        OutNetRecvProc((void*)pRecvBuff, i);
                    }
					else
					{
                        //PrintfData("SysTask", pRecvBuff, i);
                        CL_LOG("local sum=%d,pkt sum=%d,error.\n",sum,data);
                    }
                    step = FIND_AA;
                    break;
            }
        }
    }
}


int OutNetInit(void)
{
    memset(&gOutNetStatus, 0, sizeof(gOutNetStatus));
    GPIO_Cfg(HT_GPIOA, GPIO_Pin_12|GPIO_Pin_13, GPIO_Mode_IOIN, GPIO_Input_Floating, GPIO_Output_PP);
    return CL_OK;
}


