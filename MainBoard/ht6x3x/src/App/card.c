/*card.c
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "card.h"
#include "aes.h"
#include "ui.h"
#include "blueProto.h"
#include "productTest.h"
#include "rtc.h"
#include "usart.h"
#include "sc8042.h"
#include "flash.h"
#include "lcd.h"
#include "server.h"
#include "proto.h"
#include "relayCtrlTask.h"

TaskHandle_t CardUpgradeHandle_t  = NULL;

uint8_t readCardFlg = 0;
extern KEY_VALUES keyVal;

uint8_t gWaitMsgAck = 0;
uint8_t gCKBSendBuff[OUT_NET_PKT_LEN];
MUX_SEM_STR gCKBSendMux = {0,0};
CKB_BASIC_INFO_REPORT_STR gBlueInfo;
uint8_t gBlueRxData[OUT_NET_PKT_LEN];

static unsigned char SecretCardKEY_A[6] = {'6', 'f', '7', 'd', '2', 'k'};



void ResetKeyBoard(void)
{
    //�ȸ��������µ�
    NVIC_DisableIRQ(UART1_IRQn);				//��ֹ�����ж�
    GPIO_Cfg(HT_GPIOC, GPIO_Pin_0|GPIO_Pin_1, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
    GPIO_ResetBits(HT_GPIOC, GPIO_Pin_0);
    GPIO_ResetBits(HT_GPIOC, GPIO_Pin_1);
	CkbPowerOff();
    vTaskDelay(4000);
    Init_Uart1();
    CkbPowerOn();
}


//���Ͱ���������
int SendCKBPkt(uint8_t sn, uint8_t module, uint8_t cmd, CKB_STR *pPkt, uint16_t len)
{
    uint8_t *pbuff = (void*)pPkt;

    pPkt->head.aa = 0xaa;
    pPkt->head.five = 0x55;
    pPkt->head.len = len + 5;
    pPkt->head.ver = 0x01;
    pPkt->head.sn = sn;
	pPkt->head.module = module;
    pPkt->head.cmd = cmd;
    pbuff[sizeof(CKB_HEAD_STR) + len] = GetPktSum((void*)&pPkt->head.ver, len+4);
    //PrintfData("SendCKBPkt", (void*)pPkt, sizeof(CKB_HEAD_STR)+len+1);
	UsartSend(CKB_UART, (void*)pPkt, sizeof(CKB_HEAD_STR)+len+1);
	return CL_OK;
}


//dest: 0:���� 1:2.4g
int SendBlueNetPkt(uint8_t dest, uint8_t *pPkt, uint16_t len)
{
    CKB_STR *pkt = (void*)gCKBSendBuff;
	BT_MESSAGE_RECIVE_STR* pBlueSend = (void*)pkt->data;
    int i;

    MuxSempTake(&gCKBSendMux);
    i = 0;
    while (gWaitMsgAck) {
        Feed_WDT();
        vTaskDelay(50);
        if (20 < ++i) {
            CL_LOG("to.\n");
            gWaitMsgAck = 0;
            break;
        }
    }
	pBlueSend->type = dest;
    pBlueSend->len = len;
    memcpy(pBlueSend->data, pPkt, len);
    gWaitMsgAck = 1;
	SendCKBPkt(gChgInfo.ckb_sn++, BT_MODULE, BT_MESSAGE_CMD_SEND, pkt, sizeof(CKB_HEAD_STR)+1+2+len);
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}


//������Ϣ�ϱ��ذ�
int SendBasicInfoReportAck(uint8_t result)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
	CKB_BASIC_INFO_REPORT_ACK_STR* basicInfoAck = (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	basicInfoAck->result = result;
    PrintfData("SendBasicInfoReportAck", (void*)basicInfoAck, sizeof(CKB_BASIC_INFO_REPORT_ACK_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, WHOLE_MODULE, WHOLE_MESSAGE_CMD_BASIC_INFO, pkt, sizeof(CKB_BASIC_INFO_REPORT_ACK_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

/**
 * ��������
 */ 
int App_CB_SendStartUpgrade(uint32_t fileSize, uint32_t package, uint16_t checkSum, uint8_t verson)
{
    CKB_STR *pkt = (void*)gCKBSendBuff;
    START_UPGRADE_REQUEST_STR *startUpgrade = (void*)pkt->data;
	
	MuxSempTake(&gCKBSendMux);
    memset(startUpgrade, 0, sizeof(START_UPGRADE_REQUEST_STR));

    startUpgrade->filesize = fileSize;
    startUpgrade->package = package;
    startUpgrade->checkSum = checkSum;
    startUpgrade->fw_verson = verson;
    
	PrintfData("���Ϳ�ʼ����", (void*)startUpgrade, sizeof(START_UPGRADE_REQUEST_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, ENUM_MODUL_UPGRADE, ENUM_UPGRADE_REQUEST, pkt, sizeof(START_UPGRADE_REQUEST_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

/**
 * �̼��·�
 */ 
void App_CB_DownFW(uint8_t package,uint8_t *data,uint16_t len)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
    CB_DOWN_FW_t *fw = (void*)pkt->data;
	
	if(len > UPGRADE_PACKAGE_SIZE)
    {
        CL_LOG("fw length is too loog.\r\n");
        return;
    } 
	MuxSempTake(&gCKBSendMux);
    memset(fw, 0, sizeof(CB_DOWN_FW_t));

    fw->index = package;
    memcpy(fw->data, data, len);
//	PrintfData("������������", (void*)fw, sizeof(CB_DOWN_FW_t));
	SendCKBPkt(gChgInfo.ckb_sn++, ENUM_MODUL_UPGRADE, ENUM_SEND_UPGRADE_PKT, pkt, len + 1);
    MuxSempGive(&gCKBSendMux);
}

void BswSrv_Upgrade_SendNotify(uint8_t transAction)
{
   if(CardUpgradeHandle_t != NULL)
   {
       xTaskNotify((TaskHandle_t  )CardUpgradeHandle_t,    //��������֪ͨ��������
                   (uint32_t    )transAction,            //����ֵ֪ͨ
                   (eNotifyAction  )eSetValueWithOverwrite);  //��д�ķ�ʽ��������֪ͨ
   }
}

//Զ������
void App_CB_Handle_UpgradeInfo(CKB_STR *ptk)
{
    uint8_t cmd = ptk->head.cmd;
	static uint8_t IndexTimesFlag = 0;
		
    if(cmd == ENUM_UPGRADE_REQUEST) //��ʼ����
    {
        CB_RESULE_ACK_t *pRet = (void*)ptk->data;
        if(pRet->result == 0)
        {
        	#if 0
            BswSrv_Upgrade_SendNotify(0);
			#else
			gChgInfo.UpgradeRunning = 0xa5;
			printf("�յ����������������־\r\n");
			#endif
        }
		else
		{
			printf("�յ�����������־\r\n");
		}
    }
    else if(cmd == ENUM_SEND_UPGRADE_PKT)//�̼��·�
    {
        CB_DOWN_FW_ACK_t *FWAck = (void*)ptk->data;
        if(FWAck->result == 0)
        {
			#if 0
            BswSrv_Upgrade_SendNotify(FWAck->index);
			#else
			gChgInfo.UpgradeRunning = 0xa5;
			gChgInfo.UpgradeIndex = FWAck->index;
//			printf("����index = %d\r\n", gChgInfo.UpgradeIndex);
			IndexTimesFlag = 0;
			#endif
        }   
		else
		{
			printf("�յ������·��̼�\r\n");
			IndexTimesFlag++;
			if(20 < IndexTimesFlag)
			{
				printf("cxsj\n");
				OptFailNotice(208);
				gChgInfo.UpgradeRunning = 0;
				ResetSysTem();
			}
		}
    }
}

#if 0
//�ط�����
#define RETRY_TIMERS    8
void CardBoard_UpgradeTask(void)
{
    uint8_t buf[UPGRADE_PACKAGE_SIZE];
 //   FW_HEAD_INFO_T info;
    uint8_t index = 0;
    uint32_t package = 0;
    uint32_t readAddr = 0;
    uint32_t remain = 0;
    uint16_t read_len ;
    uint32_t i = 0;
    BaseType_t result;
    uint32_t value;

    CL_LOG("ˢ����������������..\r\n");

    OS_DELAY_MS(1000);

    package = system_info.X10KeyBoardFwInfo.filesize / UPGRADE_PACKAGE_SIZE;
    if((system_info.X10KeyBoardFwInfo.filesize % UPGRADE_PACKAGE_SIZE) != 0)
    {
        package ++;
    }
    for(i = 0;i < RETRY_TIMERS; i++)
    {
        //������������
        App_CB_SendStartUpgrade(system_info.X10KeyBoardFwInfo.filesize, package, system_info.X10KeyBoardFwInfo.checkSum, system_info.X10KeyBoardFwInfo.fw_verson);
        CL_LOG("SendStartUpgrade...\r\n");
        //�ȴ���Ӧ
        result = xTaskNotifyWait(0,                
                               (uint32_t  )0xFFFFFFFF,            //�˳�������ʱ��������е�bit
                               (uint32_t*  )&value,               //��������ֵ֪ͨ
                               (TickType_t  )3000); 
        if(result == pdTRUE) 
		{
            CL_LOG("recv req ack .\r\n");
            break;
        }
    }
    if(i >= RETRY_TIMERS)
    {
        CL_LOG("no recv req ack.. \r\n");
        goto EXIT;
    }
	
	readAddr = KeyBoardBackAddr;
    remain = system_info.X10KeyBoardFwInfo.filesize;

    //���͹̼�
	while(remain)
	{
		printf("www\n");
		if(remain >= UPGRADE_PACKAGE_SIZE)
		{
            read_len = UPGRADE_PACKAGE_SIZE;
        }
		else
		{
            read_len = remain;
        }
        remain = remain - read_len;
		HT_Flash_ByteRead(&buf[0], readAddr, read_len);
		readAddr += read_len;
		
        for(i = 0; i < RETRY_TIMERS; i++)
        {
            //��������
            App_CB_DownFW(index, buf, read_len);
            //�ȴ���Ӧ
            result = xTaskNotifyWait(0,
                               (uint32_t  )0xFFFFFFFF,  
                               (uint32_t*  )&value,  
                               (TickType_t  )2000); 
            if(result == pdTRUE && index == value) 
			{
                break;
            }
		//	CL_LOG("wwwwww index=%d value=%d \r\n",index, value);
            if(index != value)
			{
                CL_LOG("index error..index=%d value=%d \r\n",index, value);
            }
        }
        if(i >= RETRY_TIMERS)
        {
            CL_LOG("no recv fw ack..index=%d \r\n",index);
            goto EXIT;
        }
    
        index++;
		OS_DELAY_MS(50);
	}
	CL_LOG("�����������ɹ�.\r\n");
    system_info.KeyBoard.isUpgradeFlag = 0;
    FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
EXIT:
    CL_LOG("ˢ���������˳�..\r\n");
    CardUpgradeHandle_t = NULL;
	vTaskDelete(NULL);
}

void BswSrv_StartCardBoard_UpgradeTask(void)
{
	if(UPGRADE_SUCCESS_FLAG != system_info.KeyBoard.isUpgradeFlag)
    {
        CL_LOG("û����������.\r\n");
		return;
    }
    if((CardUpgradeHandle_t == NULL))
    {
        xTaskCreate((TaskFunction_t)CardBoard_UpgradeTask, "CardBoard_UpgradeTask", 256, NULL, 1, &CardUpgradeHandle_t);    
    }
    else
    {
        CL_LOG("����������������,����ˢ���治����.\r\n");
    }
}
#endif

//����ά��
int OperateMaintain(uint8_t type, uint8_t para)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
	CKB_OPERATION_MAINTENANCE_STR* OperatePkt = (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	OperatePkt->type = type;
	if (type == 3) {
		OperatePkt->rsv = para;
	}
    PrintfData("OperateMaintain", (void*)OperatePkt, sizeof(CKB_OPERATION_MAINTENANCE_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, WHOLE_MODULE, WHOLE_MESSAGE_CMD_OPERATION_MAINTENANCE, pkt, sizeof(CKB_OPERATION_MAINTENANCE_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

//�������㲥
void OpenBluetoothRadio(void)
{
	OperateMaintain(OptMainTainOpenBroCase, 1);
}

//�ر������㲥
void CloseBluetoothRadio(void)
{
	OperateMaintain(OptMainTainOpenBroCase, 0);
}

//����PCB����
int SendCkbPcbSn(uint8_t *pcb)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
	CKB_PCB_STR* SetPcbPkt = (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	memcpy(SetPcbPkt->pcb, pcb, 8);
    PrintfData("SendCkbPcbSn", (void*)SetPcbPkt, sizeof(CKB_PCB_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, WHOLE_MODULE, WHOLE_MESSAGE_CMD_SET_PCB, pkt, sizeof(CKB_PCB_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

//��ȡPCB����
int GetCkbPcbSn(void)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
    MuxSempTake(&gCKBSendMux);
	SendCKBPkt(gChgInfo.ckb_sn++, WHOLE_MODULE, WHOLE_MESSAGE_CMD_GET_PCB, pkt, 0);
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

//�����ϱ��ذ�
int SendKeyReportAck(uint8_t result)
{
    CKB_STR *pkt = (void*)gCKBSendBuff;
	KEY_VAL_REPORT_ACK_STR* key_report_ack = (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	key_report_ack->result = result;
    PrintfData("SendKeyReportAck", (void*)key_report_ack, sizeof(KEY_VAL_REPORT_ACK_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, KEY_MODULE, 0x01, pkt, sizeof(KEY_VAL_REPORT_ACK_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

//����Ϣ�ϱ��ذ�
int SendCardInfoReportAck(uint8_t result)
{
    CKB_STR *pkt = (void*)gCKBSendBuff;
	CARD_INFO_REPORT_ACK_STR* pCardInfoAck =  (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	pCardInfoAck->result = result;
    PrintfData("SendCardInfoReportAck", (void*)pCardInfoAck, sizeof(CARD_INFO_REPORT_ACK_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, CARD_MODULE, 0x01, pkt, sizeof(CARD_INFO_REPORT_ACK_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}


/*****************************************************************************
** Function name:       WriteToCardBlock
** Descriptions:        //д��
** input parameters:
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int WriteToCardBlock(uint8_t* keyA, uint8_t sectorNum, uint8_t blockNum)
{
    CKB_STR *pkt = (void*)gCKBSendBuff;
	WRITE_CARD_REQ_STR* pWriteToCardBlock =  (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	memcpy(pWriteToCardBlock->keyA, keyA, sizeof(pWriteToCardBlock->keyA));
    memcpy(pWriteToCardBlock->data, ((WRITE_CARD_REQ_STR*)keyA)->data, sizeof(pWriteToCardBlock->data));
	pWriteToCardBlock->sectorNum = sectorNum;
	pWriteToCardBlock->blockNun = blockNum;
    //PrintfData("WriteToCardBlock", (void*)pWriteToCardBlock, sizeof(WRITE_CARD_REQ_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, CARD_MODULE, 0x03, pkt, sizeof(WRITE_CARD_REQ_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

//����������
int ReadCardNumReq(uint8_t keyA[], uint8_t sectorNum, uint8_t blockNum)
{
    CKB_STR *pkt = (void*)gCKBSendBuff;
	READ_CARD_REQ_STR* pReadCardReq =  (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
	memcpy(pReadCardReq->keyA, keyA, 6);
	pReadCardReq->sectorNum = sectorNum;
	pReadCardReq->blockNum = blockNum;
    PrintfData("ReadCardReq", (void*)pReadCardReq, sizeof(READ_CARD_REQ_STR));
    //CL_LOG("\n ����sectorNum = %d, blockNum = %d\n", sectorNum, blockNum);
	SendCKBPkt(gChgInfo.ckb_sn++, CARD_MODULE, CMD_CARD_READ, pkt, sizeof(READ_CARD_REQ_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}

//д������
int WriteCardReq(void)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
	WRITE_CARD_REQ_STR* pWriteCardReq =  (void*)pkt->data;
    MuxSempTake(&gCKBSendMux);
    PrintfData("WriteCardReq", (void*)pWriteCardReq, sizeof(WRITE_CARD_REQ_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, CARD_MODULE, CMD_CARD_WRITE, pkt, sizeof(WRITE_CARD_REQ_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;

}

//������������
int SetBtName(uint8_t *name)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
	BT_SET_NAME_STR* btSetName =  (void*)pkt->data;
	memcpy(btSetName->btName, name, sizeof(btSetName->btName));
    MuxSempTake(&gCKBSendMux);
    PrintfData("SetBtName", (void*)btSetName, sizeof(BT_SET_NAME_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, BT_MODULE, BT_MESSAGE_CMD_SET_NAME, pkt, sizeof(BT_SET_NAME_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}


//������Ϣ���ջذ�
int BtMessageRecAck(uint8_t result)
{
	CKB_STR *pkt = (void*)gCKBSendBuff;
	BT_MESSAGE_RECIVE_ACK_STR* btMessageRecAck =  (void*)pkt->data;
	btMessageRecAck->result = result;
    MuxSempTake(&gCKBSendMux);
    //PrintfData("BtMessageRecAck", (void*)btMessageRecAck, sizeof(BT_MESSAGE_RECIVE_ACK_STR));
	SendCKBPkt(gChgInfo.ckb_sn++, BT_MODULE, BT_MESSAGE_CMD_RECIVE, pkt, sizeof(BT_MESSAGE_RECIVE_ACK_STR));
    MuxSempGive(&gCKBSendMux);
    return CL_OK;
}


//status ����״̬: 0������1����
void BuleStatusSendProc(uint8_t status)
{
    uint8_t  sendStatus = status ? EVENT_RECOVER : EVENT_OCCUR;

    if (status) 
	{
        CL_LOG("bt status=%d,err.\n",status);
    }

    if (status != gChgInfo.lastBlueStatus) 
	{
        if (CL_OK == IsSysOnLine()) 
		{
            if (gChgInfo.sendBlueStatus < 10) 
			{
                SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_BLUE, 0, sendStatus, NULL);
                gChgInfo.sendBlueStatus++;
            }
			else
			{
                gChgInfo.lastBlueStatus = status;
                gChgInfo.sendBlueStatus = 0;
            }
        }
    }
    gBlueInfo.btState = status;
}


//������Ϣ����
void WholeModuleHandle(CKB_STR* pFrame)
{
    char name[12] = {0};
    CKB_STATU_REPORT_STR *pStatus = NULL;

	if (pFrame->head.cmd == WHOLE_MESSAGE_CMD_BASIC_INFO) 
	{//������Ϣ�ϱ�
		SendBasicInfoReportAck(0);
		CKB_BASIC_INFO_REPORT_STR* basicInfo = (CKB_BASIC_INFO_REPORT_STR*)pFrame->data;
        CL_LOG("ckb fw=%d,state=%d,version=%s,btName=%s.\n", basicInfo->fwVersion,basicInfo->btState,basicInfo->btVersion,basicInfo->btName);
		//PrintfData("WholeModuleHandle btMacAddr", (void*)basicInfo->btMacAddr, sizeof(basicInfo->btMacAddr));
        memcpy(&gBlueInfo, pFrame->data, sizeof(gBlueInfo));
        if (system_info.fwVersion != basicInfo->fwVersion) 
		{
            system_info.fwVersion = basicInfo->fwVersion;
            FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
        }
        if (0 == basicInfo->btState) 
		{
		    memcpy(system_info.btVersion, basicInfo->btVersion, sizeof(system_info.btVersion));
            DeviceBcd2str(name, &system_info.station_id[3], BLUE_GWADDR_LEN);
            if (memcmp(basicInfo->btName, name, BLUE_NAME_LEN)) 
			{
                system_info.setBtNameFlag = 0;
                CL_LOG("������������.\n");
            }
            gChgInfo.netStatus |= 4;
            if (NULL == strstr((void*)basicInfo->btVersion, "BLE")) 
			{
                gChgInfo.netStatus |= 8;
            }
    	}
    }
	else if (pFrame->head.cmd == WHOLE_MESSAGE_CMD_OPERATION_MAINTENANCE) 
	{ //����ά��
        if (pFrame->data[0] == 0) 
		{
			//CL_LOG("operation maintenance success.\n");
		} 
		else 
		{
			CL_LOG("fail.\n");
		}
	} 
	else if (pFrame->head.cmd == WHOLE_MESSAGE_CMD_STATU_REPORT) 
	{//״̬�ϱ�
		pStatus = (void*)pFrame->data;
        BuleStatusSendProc(pStatus->blueStatus);
	} 
	else if (pFrame->head.cmd == WHOLE_MESSAGE_CMD_SET_PCB) 
	{//���ð�����PCB
		if (pFrame->data[0]) 
		{
            CL_LOG("set pcb sn fail.\n");
		}
	} 
	else if (pFrame->head.cmd == WHOLE_MESSAGE_CMD_GET_PCB) 
	{//��ȡ������PCB
		PrintfData("tx kb pcb sn to pc:", pFrame->data, 8);
		//���Ͱ�����PCB���뵽PC��
		uint8_t tmp[10]={0};
		tmp[0] = 0x01;
		memcpy(&tmp[1], &pFrame->data, 8);
		SendTestPktAck(TEST_CMD_READ_SN, (void *)tmp, 9);
	}
}


//������Ϣ����
void KeyModuleHandle(CKB_STR* pFrame)
{
	SendKeyReportAck(0);
	KEY_VAL_REPORT_STR* pKey = (KEY_VAL_REPORT_STR*)pFrame->data;
	CL_LOG("key=%d.\n", pKey->value);
	Sc8042bSpeech(VOIC_KEY);
	if (TestKeyFlag == 1) 
	{
		uint8_t ackData[2] = {0};
		ackData[0] = 0x01;
		ackData[1] = pKey->value;
		SendTestPktAck(TEST_CMD_KEY, (void*)&ackData, 2);
		return;
	}
	KeyToUiProc(pKey->value);
}


//֧�������Ž��������ݵĴ���
//sectorOffset: =0 block8��=1 block48
void ReadPayCardBlock8Handle(BLOCK8_STRUCT_TYPE *pGetBlockData, uint8_t sectorOffset)
{
    BLOCK8_STRUCT_TYPE* pBlockData = (BLOCK8_STRUCT_TYPE*)(gChgInfo.PayCardBlockBuff.block8);
    uint32_t money;

    if (pGetBlockData->CheckSum != GetPktSum((void*)pGetBlockData, 15)) {
        CL_LOG("c err.\n");
        return;
    }

    if (8 == gChgInfo.PayCardReadFlag) {
        uint8_t MiyaoWithBlock[32];
        WRITE_CARD_REQ_STR* pWriteCardReq = (WRITE_CARD_REQ_STR*)MiyaoWithBlock;
        if (0 != memcmp(pGetBlockData, gChgInfo.PayCardBlockBuff.block8, sizeof(gChgInfo.PayCardBlockBuff.block8))) {
			CL_LOG("data diff,ofs=%d.\n",sectorOffset);
            Sc8042bSpeech(VOIC_CARD_INVALID);
            gChgInfo.PayCardReadFlag = 0;
            return;
        }

        money = gChgInfo.money * 100;
        if (pBlockData->CardBalance >= money) {
            pBlockData->CardBalance -= money;
        }else{
            pBlockData->CardBalance = 0;
        }

        gChgInfo.PayCardBlockBuff.block8[15] = GetPktSum(&gChgInfo.PayCardBlockBuff.block8[0], 15);
        memcpy(pWriteCardReq->keyA, gChgInfo.PayCardBlockBuff.MiyaoPayment, sizeof(gChgInfo.PayCardBlockBuff.MiyaoPayment));
        memcpy(pWriteCardReq->data, gChgInfo.PayCardBlockBuff.block8, sizeof(gChgInfo.PayCardBlockBuff.block8));
        WriteToCardBlock(&MiyaoWithBlock[0], sectorOffset + 2, 0);
    }
	else
	{
        Sc8042bSpeech(VOIC_CARD);
        //���ڶ���
        Sc8042bSpeech(VOIC_READING_CARD);
        vTaskDelay(650 / VOICE_DELAY_DIV);
        PrintfData("������", &pGetBlockData->CardUserFlag, 16);
        memcpy(gChgInfo.PayCardBlockBuff.block8, pGetBlockData, sizeof(gChgInfo.PayCardBlockBuff.block8));
        gChgInfo.PaymentCardMoney = pGetBlockData->CardBalance;
        SwitchToUI_ShowBalance(gChgInfo.PaymentCardMoney);
        SpeechCardBalance(gChgInfo.PaymentCardMoney);
        vTaskDelay(750 / VOICE_DELAY_DIV);
        Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
        SwitchToUi_InputSocketNum();
        vTaskDelay(600 / VOICE_DELAY_DIV);
        clearKeyBuf(&keyVal);
    }
}


//֧�������Ž��������ݵĴ���
void ReadPayCardBlock9Handle(BLOCK9_STRUCT_TYPE *pGetBlock9Data, uint8_t* ucardkeyA, uint8_t sectorOffset)
{
    if (pGetBlock9Data->CheckSum != GetPktSum((void*)pGetBlock9Data, 15)) 
	{
		CL_LOG("fail,ofs=%d.\n",sectorOffset);
        return;
    }

    if (8 == gChgInfo.PayCardReadFlag) {
		//��⿨���Ƿ���ͬ
        if (0 != memcmp(pGetBlock9Data, gChgInfo.PayCardBlockBuff.block9, sizeof(gChgInfo.PayCardBlockBuff.block9))) {
            Sc8042bSpeech(VOIC_CARD_INVALID);
			//��������ͽ���, ʹ������һ��֧�������������, ���ｫgChgInfo.PayCardReadFlag = 0��ϣ�����ſ���������ˢ��
            gChgInfo.PayCardReadFlag = 0;
            return;
        } else {
            gChgInfo.payCardBlock = sectorOffset*4 + 8;
            //����������
            ReadCardNumReq(&ucardkeyA[0], (sectorOffset+2), 0);    //����8����
        }
    } else{
        gChgInfo.PayCardReadFlag = 1;
        PrintfData("������", &pGetBlock9Data->isPayCard, 16);
        memcpy(&gChgInfo.PayCardBlockBuff.block9[0], pGetBlock9Data, sizeof(BLOCK9_STRUCT_TYPE));
        sprintf((char*)gChgInfo.current_usr_card_id, "%x%02x%02x%02x%02x",pGetBlock9Data->PayCardID[3], pGetBlock9Data->PayCardID[4], pGetBlock9Data->PayCardID[5],pGetBlock9Data->PayCardID[6], pGetBlock9Data->PayCardID[7]);

        if ((pGetBlock9Data->UnixTime + (((uint32_t)8) * 60 * 60)) < GetRtcCount()) {
            Sc8042bSpeech(VOIC_CARD_INVALID);
            gChgInfo.PayCardReadFlag = 0;
            CL_LOG("����,time=%d.\n",pGetBlock9Data->UnixTime);
            return;
        }
        gChgInfo.payCardBlock = sectorOffset*4 + 5;
        ReadCardNumReq(&ucardkeyA[0], sectorOffset+1, 1);
    }
}

/*****************************************************************************
** Function name:       WritePayCardHandle
** Descriptions:        //
** input parameters:    pFrame
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void WritePayCardHandle(CKB_STR* pFrame)
{
    WRITE_CARD_REPORT_STR* pWriteCard = (WRITE_CARD_REPORT_STR*)pFrame->data;
    uint8_t  PayCardOrder[ORDER_SECTION_LEN] = {0,};

    Sc8042bSpeech(VOIC_CARD);
    vTaskDelay(100);

	if (pWriteCard->result == 0) 
{  //д���ɹ�
        CL_LOG("gun=%d.\n", gChgInfo.current_usr_gun_id);
        gChgInfo.mode = 1;
        gChgInfo.subsidyType = 0;
        StartCharging(START_TYPE_WALLET_CARD, gChgInfo.money*100, gChgInfo.current_usr_card_id, PayCardOrder, (uint8_t)ORDER_SOURCE_PAYMENT_CARD);
    }
	else 
	{
        Sc8042bSpeech(VOIC_START_CHARGER_FAIL);
        CL_LOG("fail\n");
	}
    memset(&gChgInfo.PayCardBlockBuff, 0, sizeof(gChgInfo.PayCardBlockBuff));
}

/*****************************************************************************
** Function name:       ReadAuthCardIDHandle
** Descriptions:        //
** input parameters:    pFrame
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void ReadAuthCardIDHandle(CKB_STR* pFrame)
{
    READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
    BLOCK9_STRUCT_TYPE *pGetBlock9Data = ((BLOCK9_STRUCT_TYPE*)(pReadCard->data));

    //����
    sprintf((char*)gChgInfo.current_usr_card_id, "%x%02x%02x%02x%02x",
                        pGetBlock9Data->PayCardID[3], pGetBlock9Data->PayCardID[4], pGetBlock9Data->PayCardID[5],
                        pGetBlock9Data->PayCardID[6], pGetBlock9Data->PayCardID[7]);
    CL_LOG("cardNum=%s.\n", gChgInfo.current_usr_card_id);

    Sc8042bSpeech(VOIC_CARD);
    vTaskDelay(200);

    if (TestCardFlag == 1) {
        SendTestPktAck(TEST_CMD_CARD, (void*)gChgInfo.current_usr_card_id, sizeof(gChgInfo.current_usr_card_id));
        vTaskDelay(3000);
        return;
    }

    //û�мƷ�ģ�岻������
    if (CL_OK != CheckCostTemplate()) 
    {
       // CL_LOG("û�мƷ�ģ��.\n");
        OptFailNotice(206);
        return;
    }

    //׮���߲�������
    if (CL_FALSE == system_info.is_socket_0_ok) {
        Sc8042bSpeech(VOIC_DEVICE_EXCEPTION);
        return;
    }

    //���ڶ���
    Sc8042bSpeech(VOIC_READING_CARD);
    readCardFlg = 1;
    //��ѯ�����
    SwitchToUi_ReadingCard();
    SendCardAuthReq(CARD_CHECK_MONEY);
    gChgInfo.sendAuthCnt = 0;
    vTaskDelay(100);
}

/*****************************************************************************
** Function name:       EntranceGuardPayCardReadIDHandle
** Descriptions:        �Ž�Ǯ������ȡ������
** input parameters:    pFrame�������巢�͹�����֡����
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void EntranceGuardPayCardReadIDHandle(CKB_STR* pFrame, uint8_t* ucardkeyA)
{
    READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
    BLOCK9_STRUCT_TYPE *pGetBlock49Data = ((BLOCK9_STRUCT_TYPE*)(pReadCard->data));
    BLOCK8_STRUCT_TYPE *pReadBlock48Data = ((BLOCK8_STRUCT_TYPE*)(pReadCard->data));

    CL_LOG("block=%d\n", gChgInfo.payCardBlock);
    switch(gChgInfo.payCardBlock) {
        case 49:
        {
            CL_LOG("����49\n");
            ReadPayCardBlock9Handle(pGetBlock49Data, ucardkeyA, ENTRANCE_GUARD_CARD_SECTOR_OFFSET);
        }
        break;
        case 48:
            ReadPayCardBlock8Handle(pReadBlock48Data, ENTRANCE_GUARD_CARD_SECTOR_OFFSET);
        break;
        case 45:
        {
            PrintfData("��45����", &pReadCard->data[0], 16);
            if(0 != memcmp(pReadCard->data, system_info.OperatorNum, 10))    //�����Ӫ�̱���Ƿ���ͬ
            {
                CL_LOG("opn is dif!\n");
                Sc8042bSpeech(VOIC_CARD_INVALID);
                gChgInfo.PayCardReadFlag = 0;
                return;
            }
            //CL_LOG("��Ӫ�̱����ͬ.\n");
            gChgInfo.payCardBlock = ((uint8_t)ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2) * 4 + 0;

            //����������
            ReadCardNumReq(&ucardkeyA[0], ((uint8_t)ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2), 0);    //����48����
        }
        break;
        default:
        break;
    }
}

/*****************************************************************************
** Function name:       ReadPayCardIDHandle
** Descriptions:        //
** input parameters:    pFrame
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void ReadPayCardIDHandle(CKB_STR* pFrame, uint8_t* ucardkeyA)
{
    READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
    BLOCK9_STRUCT_TYPE *pGetBlock9Data = ((BLOCK9_STRUCT_TYPE*)(pReadCard->data));
    BLOCK8_STRUCT_TYPE *pReadBlock8Data = ((BLOCK8_STRUCT_TYPE*)(pReadCard->data));

    switch(gChgInfo.payCardBlock)
    {
        case 9:
        {
            ReadPayCardBlock9Handle(pGetBlock9Data, ucardkeyA, 0);
        }
        break;
        case 8:
            ReadPayCardBlock8Handle(pReadBlock8Data, 0);
        break;
        case 5:
        {
            PrintfData("��5����", &pReadCard->data[0], 16);
            if(0 != memcmp(pReadCard->data, system_info.OperatorNum, 10))    //�����Ӫ�̱���Ƿ���ͬ
            {
                CL_LOG("opn is dif!\n");
                Sc8042bSpeech(VOIC_CARD_INVALID);
                gChgInfo.PayCardReadFlag = 0;
                return;
            }
            //CL_LOG("��Ӫ�̱����ͬ.\n");
            gChgInfo.payCardBlock = 8;

            //����������
            ReadCardNumReq(&ucardkeyA[0], 2, 0);    //����8����
        }
        break;
    }
}


/*****************************************************************************
** Function name:       MiYaoCardReadIDHandle
** Descriptions:        //
** input parameters:    pFrame
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void MiYaoCardReadIDHandle(CKB_STR* pFrame)
{
    READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
    BLOCK_STRUCT_TYPE* pblockData = ((BLOCK_STRUCT_TYPE*)(pReadCard->data));

    if (0 != memcmp(system_info.OperatorNum, pblockData->OperatorNumb, sizeof(system_info.OperatorNum))) {   //�����Ӫ�̱���Ƿ���ͬ
        WriteCfgInfo(CFG_OPERATORNUM, sizeof(system_info.OperatorNum), pblockData->OperatorNumb);

        memcpy(system_info.OperatorNum, pblockData->OperatorNumb, sizeof(system_info.OperatorNum));
        FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
    }
    PrintfData("opn:", &system_info.OperatorNum[0], 10);

    Sc8042bSpeech(VOIC_CARD);
    vTaskDelay(200);
    SwitchToUi_AdministratorTestMode();
    gChgInfo.isMiYaoCard = 0;
}

/*****************************************************************************
** Function name:       EntranceGuardCardReadIDHandle
** Descriptions:        ������Ž������ܵĿ�Ƭ��ȡ������
** input parameters:    pFrame����ȡ����֡����
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void EntranceGuardCardReadIDHandle(CKB_STR* pFrame)
{
    READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
    BLOCK9_STRUCT_TYPE *pReadBlock9Data = ((BLOCK9_STRUCT_TYPE*)(pReadCard->data));

    if(0 == gChgInfo.DataFlagBit.isPayCardFlag)      //֧����?
    {
        gChgInfo.DataFlagBit.isPayCardFlag = 1;
        gChgInfo.isPayCard = pReadBlock9Data->isPayCard;
    }

    if(0x11 == gChgInfo.isPayCard)      //֧����
    {
        CL_LOG("֧����.\n");
        if (CL_OK != CheckCostTemplate()) 
        { //û�мƷ�ģ�岻������
            OptFailNotice(206);
            return;
        }
        EntranceGuardPayCardReadIDHandle(pFrame, &gChgInfo.PayCardBlockBuff.MiyaoPayment[0]);
    }
    else if(0x12 == gChgInfo.isPayCard)     //��Ȩ��
    {
        gChgInfo.DataFlagBit.isPayCardFlag = 0;
        gChgInfo.PayCardReadFlag = 0;
        CL_LOG("��Ȩ��.\n");
        ReadAuthCardIDHandle(pFrame);
        gChgInfo.DataFlagBit.ReadFromElevenSector = 0;
    }
    else {
        //CL_LOG("�Ȳ���֧����, Ҳ���Ǽ�Ȩ��!\n");
        CL_LOG("err\n");
        gChgInfo.DataFlagBit.ReadFromElevenSector = 0;
    }
}

/*****************************************************************************
** Function name:       PayCardConfirmAddEntranceGuard
** Descriptions:        ȷ��֧�������Ȩ���Ƿ�����Ž�������, ��������Ž�����, ���ƫ�����Ǹ������Ŀ�
** input parameters:    ucardkeyA: ��ԿA
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void PayCardConfirmAddEntranceGuard(uint8_t *ucardkeyA)
{
    CL_LOG("set=%d,f=%d\n", gChgInfo.DataFlagBit.isReadFromElevenSector, gChgInfo.PayCardReadFlag);
    if(ENTRANCE_GUARD_CARD_CHECK_TIMES <= ++gChgInfo.DataFlagBit.isReadFromElevenSector)  //�������ζ��������ж���Ϊ�Ž���
    {
        ReadCardNumReq(&ucardkeyA[0], (ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2), 1);   //��Ȩ����֧���������Ž�����������ƫ��10
        //CL_LOG("��������ƫ��10!\n");
        CL_LOG("send 10!\n");
        if((8 != gChgInfo.PayCardReadFlag))
        {
            gChgInfo.PayCardReadFlag = 1;           //
        }
        gChgInfo.DataFlagBit.ReadFromElevenSector = 1;            //���Ž�����־λ
        gChgInfo.DataFlagBit.isReadFromElevenSector = 0;
        gChgInfo.payCardBlock = ((uint8_t)ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2)*4 + 1;
        return;
    }
    else if(((8 == gChgInfo.PayCardReadFlag) && (2 > ++gChgInfo.DataFlagBit.CardMisoperationTimes)) ||
        (0 == gChgInfo.PayCardReadFlag))
    {
        //����������
        ReadCardNumReq(&ucardkeyA[0], 2, 1);
        //CL_LOG("������ٴζ�����ȷ���Ƿ������Ž�����!\n");
        CL_LOG("read ag\n");
        return;
    }
    CL_LOG("��M1��ʧ��.\n");
    gChgInfo.PayCardReadFlag = 0;
}

/*****************************************************************************
** Function name:       KeyCardConfirmAddEntranceGuard
** Descriptions:        ȷ����Կ���Ƿ�����Ž�������, ��������Ž�����, ���ƫ�����Ǹ���������Կ��
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void KeyCardConfirmAddEntranceGuard(void)
{
    CL_LOG("st=%d,f=%d \n", gChgInfo.DataFlagBit.isReadFromElevenSector, gChgInfo.PayCardReadFlag);
    if(ENTRANCE_GUARD_CARD_CHECK_TIMES <= ++gChgInfo.DataFlagBit.isReadFromElevenSector)  //�������ζ��������ж���Ϊ�Ž���
    {
        ReadCardNumReq(&SecretCardKEY_A[0], (ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2), 0);   //��Կ�������Ž�����������ƫ��10
        if((8 != gChgInfo.PayCardReadFlag))
        {
            gChgInfo.PayCardReadFlag = 1;           //
        }
        gChgInfo.DataFlagBit.ReadFromElevenSector = 1;           //���Ž�����־λ
        gChgInfo.DataFlagBit.isReadFromElevenSector = 0;
        //gChgInfo.payCardBlock = ((uint8_t)ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2)*4 + 1;
        return;
    }
    else if((0 == gChgInfo.PayCardReadFlag))
    {
        //����������
        ReadCardNumReq(&SecretCardKEY_A[0], 2, 0);
        //CL_LOG("������ٴζ�����ȷ���Ƿ������Ž�����!\n");
        CL_LOG("read ag\n");
        return;
    }
    CL_LOG("��M1��ʧ��.\n");
    gChgInfo.PayCardReadFlag = 0;
}

/*****************************************************************************
** Function name:       CardReadIDHandle
** Descriptions:        û������Ž������ܵĿ�Ƭ��ȡ������
** input parameters:    pFrame����ȡ����֡����
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void CardReadIDHandle(CKB_STR* pFrame)
{
    READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
    BLOCK9_STRUCT_TYPE *pReadBlock9Data = ((BLOCK9_STRUCT_TYPE*)(pReadCard->data));

    if(0 == gChgInfo.DataFlagBit.isPayCardFlag)      //֧����?
    {
        gChgInfo.DataFlagBit.isPayCardFlag = 1;
        gChgInfo.isPayCard = pReadBlock9Data->isPayCard;
    }

    if(0x11 == gChgInfo.isPayCard)      //֧����
    {
        CL_LOG("֧����.\n");
        if (CL_OK != CheckCostTemplate()) 
        { //û�мƷ�ģ�岻������
            OptFailNotice(206);
            return;
        }
        ReadPayCardIDHandle(pFrame, &gChgInfo.PayCardBlockBuff.MiyaoPayment[0]);
    }
    else if(0x12 == gChgInfo.isPayCard)     //��Ȩ��
    {
        gChgInfo.DataFlagBit.isPayCardFlag = 0;
        gChgInfo.PayCardReadFlag = 0;
        CL_LOG("��Ȩ��.\n");
        ReadAuthCardIDHandle(pFrame);
    }
    else {
        //CL_LOG("�Ȳ���֧����, Ҳ���Ǽ�Ȩ��!\n");
		gChgInfo.DataFlagBit.ReadFromElevenSector = 0;
    }
}

//ˢ����Ϣ����
void CardModuleHandle(CKB_STR* pFrame)
{
	uint8_t tmp[16] = { 0 };
	AES_KEY aes;
    static uint8_t ucardkeyA[16];
    static uint32_t TickOfMiYaoCardOutTime = 0;

    if ((0x01 == gChgInfo.isMiYaoCard) && ((uint32_t)((GetRtcCount() - TickOfMiYaoCardOutTime) > 15))) 
	{
        TickOfMiYaoCardOutTime = GetRtcCount();
        gChgInfo.isMiYaoCard = 0;
    }
	if (pFrame->head.cmd == CMD_CARD_UP) 
	{//����Ϣ
		//Ӧ����ճɹ�
		SendCardInfoReportAck(0);
		CARD_INFO_REPORT_STR* pCardInfo = (CARD_INFO_REPORT_STR*)pFrame->data;
		if (pCardInfo->type == 0x01) 
		{//���뿨
		    gChgInfo.PayCardReadFlag = 0;   //���֧������������ͽ�����ˢ��Կ�������������¿�ʼ����
		    ReadCardNumReq(&SecretCardKEY_A[0], 2, 0);
            gChgInfo.isMiYaoCard = 1;
            TickOfMiYaoCardOutTime = GetRtcCount();     //�Է���Կ����û��ȡ�����ݾ��ƿ���
            gChgInfo.DataFlagBit.CardMisoperationTimes = 0;
		} 
		else if (pCardInfo->type == 0x02) 
		{//�����뿨
			//��ȡ�����к�
			AES_set_encrypt_key("chargerlink1234", 128, &aes);
			memcpy(tmp, pCardInfo->serial, 4);
			PrintfData("phySn:", tmp, 4);
			//keyA������ԿΪkey,������Ϊ����,�������ɸÿ�����ԿkeyA,��PCardInfo.ucardkeyA.
			AES_encrypt(tmp, ucardkeyA, &aes);
			PrintfData("card keyA:", ucardkeyA, 6);
            gChgInfo.DataFlagBit.CardMisoperationTimes = 0;

            if((0 == gChgInfo.PayCardReadFlag) || (GetRtcCount() - gChgInfo.SecondCardTick > 15) || ((1 == gChgInfo.PayCardReadFlag) && (UI_STANDBY == UIGetState()))) 
			{
                CL_LOG("��1��ˢ��.\n");
                //gChgInfo.DataFlagBit.isReadFromElevenSector = 0;
                gChgInfo.DataFlagBit.ReadFromElevenSector = 0;   //����û�м��Ž������ܵĿ�, ��������־
                gChgInfo.PayCardReadFlag = 0;
                gChgInfo.SecondCardTick = GetRtcCount();
                //����������
                ReadCardNumReq(&ucardkeyA[0], 2, 1);
                memcpy(gChgInfo.PayCardBlockBuff.MiyaoPayment, ucardkeyA, sizeof(gChgInfo.PayCardBlockBuff.MiyaoPayment));
                gChgInfo.DataFlagBit.isPayCardFlag = 0;
                gChgInfo.payCardBlock = 9;
            }
			else if((5 == gChgInfo.PayCardReadFlag)) 
			{
                CL_LOG("��2��ˢ��.\n");
                gChgInfo.DataFlagBit.ReadFromElevenSector = 0;  //����û�м��Ž������ܵĿ�, ��������־
                gChgInfo.PayCardReadFlag = 8;
                //����������
                ReadCardNumReq(&ucardkeyA[0], 2, 1);
                gChgInfo.DataFlagBit.isPayCardFlag = 0;
                gChgInfo.payCardBlock = 9;
            }
		}
	}
	else if (pFrame->head.cmd == CMD_CARD_READ) 
	{ //����
        READ_CARD_REPORT_STR* pReadCard = (READ_CARD_REPORT_STR*)pFrame->data;
        if(1 == gChgInfo.isMiYaoCard)   //��Կ����
        {
            //CL_LOG("��Կ��?\n");
            if (pReadCard->result == 0)     //�����ɹ�
            {
                CL_LOG("�����ɹ�.\n");
                MiYaoCardReadIDHandle(pFrame);
                gChgInfo.DataFlagBit.ReadFromElevenSector = 0;
            }
            else
			{      //����ʧ�ܻ�ȷ���Ƿ������Ž�����
                KeyCardConfirmAddEntranceGuard();
                return;
            }
        }
        else
        {
    		if (pReadCard->result == 0)     //�����ɹ�
            {
                CL_LOG("�����ɹ�.\n");
                switch(gChgInfo.DataFlagBit.ReadFromElevenSector)
                {
                    case 1:
                        EntranceGuardCardReadIDHandle(pFrame);
                    break;
                    default:
                        CardReadIDHandle(pFrame);
                    break;
                }
            }
            else
			{   //����ʧ�ܻ�ȷ���Ƿ������Ž�����
                PayCardConfirmAddEntranceGuard(ucardkeyA);
                return;
            }
        }
    }
    else if (pFrame->head.cmd == CMD_CARD_WRITE)      //д��
    {
        WritePayCardHandle(pFrame);
        gChgInfo.SecondCardTick = GetRtcCount();
        gChgInfo.PayCardReadFlag = 0;
        gChgInfo.DataFlagBit.isReadFromElevenSector = 0;
        gChgInfo.DataFlagBit.ReadFromElevenSector = 0;
    }
}


//������Ϣ����
void BtModuleHandle(CKB_STR* pFrame)
{
	if (pFrame->head.cmd == BT_MESSAGE_CMD_SET_NAME) 
	{
		BT_SET_NAME_REPORT_STR *btSetName = (BT_SET_NAME_REPORT_STR*)pFrame->data;
		if (btSetName->result == 0) 
		{
            system_info.setBtNameFlag = 1;
            FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
		}
		else
		{
            OptFailNotice(115);
        }
        CL_LOG("set bt name=%d.\n", btSetName->result);
	} 
	else if (pFrame->head.cmd == BT_MESSAGE_CMD_RECIVE) 
	{
		//BtMessageRecAck(0);
		BT_MESSAGE_RECIVE_STR *btMessageRec = (BT_MESSAGE_RECIVE_STR*)pFrame->data;
        CL_LOG("recLen=%d,type=%d 0:blue,1:2.4G.\n", btMessageRec->len,btMessageRec->type);
        PrintfData("BtModuleHandle", btMessageRec->data, btMessageRec->len);
		if (btMessageRec->type == 0) 
		{//����
			for(uint16_t i = 0; i < btMessageRec->len;)
			{
				if (CL_OK == FIFO_S_Put(&gBlueStatus.rxBtBuff, btMessageRec->data[i])) 
				{
					i++;
				}
				else
				{
					CL_LOG("bt buff of.\n");
					OS_DELAY_MS(2);
				}
			}
            LcdTurnOnLed();
		}
		else
		{
            CL_LOG("type=%d,err.\n",btMessageRec->type);
        }
	} 
	else if (pFrame->head.cmd == BT_MESSAGE_CMD_SEND) 
	{
	    gWaitMsgAck = 0;
		BT_MESSAGE_SEND_REPORT_STR *btMessageSendReport = (BT_MESSAGE_SEND_REPORT_STR*)pFrame->data;
		if (btMessageSendReport->result == 0) 
		{
			//CL_LOG("bt message send success.\n");
		} 
		else 
		{
			CL_LOG("bt tx fail.\n");
            OptFailNotice(116);
		}
	}
}


//ˢ����������Ϣ���մ���
void HandleCKBData(void *pPkt)
{
	CKB_STR* pFrame = pPkt;

	switch (pFrame->head.module) 
	{
		case WHOLE_MODULE://������Ϣ
		{
			WholeModuleHandle(pFrame);
		}
		break;
		case KEY_MODULE://������Ϣ
		{
		    if(1 == gChgInfo.DataFlagBit.KeyBoardModuleFlag)
            {
                if(GetRtcCount() - gChgInfo.KeyBoardModuleTick > 1)  //Ϊ�˷�ֹˢ�����̾����Ӧ������ȥ��ˢ������û��ִ���갴�İ���
                {
                    gChgInfo.KeyBoardModuleTick = GetRtcCount();
                    gChgInfo.DataFlagBit.KeyBoardModuleFlag = 0;
                    KeyModuleHandle(pFrame);
                    LcdTurnOnLed();
                }
            }
			else
			{
                KeyModuleHandle(pFrame);
                LcdTurnOnLed();
            }
		}
		break;
		case CARD_MODULE://ˢ����Ϣ
		{
            LcdTurnOnLed();
			CardModuleHandle(pFrame);
            gChgInfo.DataFlagBit.KeyBoardModuleFlag = 1;
            gChgInfo.KeyBoardModuleTick = GetRtcCount();
		}
		break;
		case BT_MODULE://������Ϣ
		{
			BtModuleHandle(pFrame);
		}
		break;
		case ENUM_MODUL_UPGRADE:	//����������
		{
			App_CB_Handle_UpgradeInfo(pFrame);
		}
		break;
		default:
			CL_LOG("����.\n");
		break;
	}
}

void CkbPowerOn(void)
{
	//CL_LOG("CKB power on.\n");
	GPIO_Cfg(HT_GPIOG, GPIO_Pin_9, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	GPIO_SetBits(HT_GPIOG, GPIO_Pin_9);
}


void CkbPowerOff(void)
{
	CL_LOG("CKB pof.\n");
	GPIO_ResetBits(HT_GPIOG, GPIO_Pin_9);
}


//setIndex: 1������������ 2��������mac
void CheckBlueInfo(uint8_t setIndex)
{
    int i;
    char name[12] = {0};

	if (gBlueInfo.btState != 0) 
	{
		CL_LOG("����״̬����.\n");
		return;
	}

    if (1 == setIndex) 
	{
        if (0 == system_info.setBtNameFlag) 
		{
            if (memcmp(gZeroArray, system_info.station_id, sizeof(system_info.station_id))) 
			{
                DeviceBcd2str(name, &system_info.station_id[3], 5);
                for (i=0; i<10; i++) 
				{
                    if ((0x30 > name[i]) || (0x39 < name[i])) 
					{
                        CL_LOG("sn err.\n");
                        return;
                    }
                }
                SetBtName((void*)name);
            }
        }
    }
}


void ProcKBData(void)
{
	uint8_t  data;
	static uint8_t  pktBuff[256]={0};
	static uint8_t  step = CKB_FIND_AA;
	static uint16_t pktLen;
	static uint16_t length;
	static uint8_t  sum;
    static uint32_t time;

    if (CKB_FIND_AA != step) 
	{
        if (2 < (uint32_t)(GetRtcCount() - time)) 
		{
            CL_LOG("no rx data,step=%d,err.\n",step);
            step = CKB_FIND_AA;
        }
    }
	while (CL_OK == FIFO_S_Get(&gUartPortAddr[1].rxBuffCtrl, &data)) 
	{
//		printf("data = %x\n", data);
    	switch (step) 
		{
    		case CKB_FIND_AA:
    		{
    			if (data == 0xAA) 
				{
                    time = GetRtcCount();
    				pktLen=0;
    				pktBuff[pktLen] = 0xAA;
    				pktLen++;
    				step = CKB_FIND_FIVE;
    			}
    		}
    		break;
    		case CKB_FIND_FIVE:
    		{
    			if (data == 0x55) 
				{
    				pktBuff[pktLen] = 0x55;
    				pktLen++;
    				step = CKB_FIND_LEN;
    			} 
				else 
				{
    				step = CKB_FIND_AA;
    			}
    		}
    		break;
    		case CKB_FIND_LEN:
    		{
    			if (pktLen == 2) 
				{
    				length = data;
    			} 
				else if (pktLen == 3) 
				{
    				length |= (data << 8);
    				sum = 0;
                    if (5 > length) 
					{
                        CL_LOG("length=%d,err.\n", length);
                        step = CKB_FIND_AA;
                        break;
                    }
                    step = CKB_FIND_VER;
    			}
    			pktBuff[pktLen] = data;
    			pktLen++;
    		}
    		break;
    		case CKB_FIND_VER:
    		{
    			if (data == 0x01) 
				{
    				sum += data;
    				pktBuff[pktLen] = data;
    				length--;
    				pktLen++;
    				step = CKB_FIND_SN;
    				//CL_LOG("CKB find version = %d.\n", data);
    			} 
				else 
				{
    				step = CKB_FIND_AA;
    			}
    		}
    		break;
    		case CKB_FIND_SN:
    		{
    			sum += data;
    			pktBuff[pktLen] = data;
    			length--;
    			pktLen++;
    			step = CKB_FIND_MODULE;
    			//CL_LOG("CKB find sn = %d.\n", sn);
    		}
    		break;
    		case CKB_FIND_MODULE:
    		{
    			sum += data;
    			pktBuff[pktLen] = data;
    			length--;
    			pktLen++;
    			step = CKB_FIND_CMD;
    			//CL_LOG("CKB find module = %d.\n", module);
    		}
    		break;
    		case CKB_FIND_CMD:
    		{
    			sum += data;
    			pktBuff[pktLen] = data;
    			if (--length) 
				{
                    step = CKB_FIND_DATA;
                }
				else
				{
                    step = CKB_FIND_CHECKSUM;
                }
    			pktLen++;
    			//CL_LOG("CKB find cmd = %d.\n", cmd);
    		}
    		break;
    		case CKB_FIND_DATA:
    		{
    			sum += data;
    			pktBuff[pktLen] = data;
    			pktLen++;
    			length--;
    			if (length==1) 
				{
    				step = CKB_FIND_CHECKSUM;
    			}
    		}
    		break;
    		case CKB_FIND_CHECKSUM:
    		{
    			pktBuff[pktLen] = data;
    			pktLen++;
    			if (data == sum) 
				{
    				gChgInfo.lastRecvKbMsgTime = GetRtcCount();
				//	PrintfData("���հ���������", (uint8_t*)pktBuff, pktLen + sizeof(CKB_HEAD_STR) + 2);
    				HandleCKBData((void*)pktBuff);
    			} 
				else 
				{
    				CL_LOG("CKB cs err.\n");
    			}
    			step = CKB_FIND_AA;
    		}
    		break;
    		default:
    		{
    			step = CKB_FIND_AA;
    		}
    		break;
    	}
    }
}


//�ֻ��������ݽ��մ���
void RecvBtData(void)
{
    uint8_t  *pBuff = gBlueRxData;
    static uint16_t len;
    static uint16_t pktLen;
    uint8_t  data;
    static uint8_t  step = BT_FIND_EE;
    static uint8_t  sum;
    static uint32_t time;

    if (BT_FIND_EE != step) 
	{
        if (2 < (uint32_t)(GetRtcCount() - time)) 
		{
            CL_LOG("no rx data,step=%d,err.\n",step);
            step = BT_FIND_EE;
        }
    }
    while (CL_OK == FIFO_S_Get(&gBlueStatus.rxBtBuff, &data)) 
	{
        switch (step) 
		{
            case BT_FIND_EE:
                if (0xee == data) 
				{
                    time = GetRtcCount();
                    pktLen = 0;
                    pBuff[pktLen++] = data;
                    step = BT_FIND_CMD;
                    sum = 0xee;
                }
                break;

            case BT_FIND_CMD:
                pBuff[pktLen++] = data;
                sum += data;
                step = BT_FIND_LEN;
                break;

            case BT_FIND_LEN:
                pBuff[pktLen++] = data;
                sum += data;
                len = data;
                if (OUT_NET_PKT_LEN < len) 
				{
                    step = BT_FIND_EE;
                }
				else if (0 == len) 
				{
                    step = FIND_CHK;
                }
				else
				{
                    step = BT_RX_DATA;
                }
                break;

            case BT_RX_DATA:
                pBuff[pktLen++] = data;
                sum += data;
                if (0 == --len) 
				{
                    step = FIND_CHK;
                }
                break;

           case FIND_CHK:
                pBuff[pktLen++] = data;
                if (sum == data) 
				{
                 //   PrintfData("������������", pBuff, pktLen);
					BlueProtoProc((void*)pBuff, pktLen);
                }
				else
				{
				//	PrintfData("������������", pBuff, pktLen);
                    CL_LOG("sum=%02x,psum=%02x,err.\n",sum,data);
                }
                step = BT_FIND_EE;
                break;
        }
    }
}

void KeyBoardUpgradeCheck(void)
{
	static uint32_t package = 0;
    static uint32_t readAddr = 0;
    static uint32_t remain = 0;
    static uint8_t index = 0;
	static uint32_t UpgradeIndexSum = 0;
    uint8_t buf[UPGRADE_PACKAGE_SIZE];
    static uint16_t read_len ;
	
	if((gChgInfo.KeyBoardUpgradeTick + 25) < GetRtcCount())
    {
        gChgInfo.KeyBoardUpgradeTick = GetRtcCount();
       // printf("����Ƿ���Ҫ����\r\n");
        if((UPGRADE_SUCCESS_FLAG == system_info.KeyBoard.isUpgradeFlag) && (gChgInfo.UpgradeRunning == 0))
        {
        	readAddr = KeyBoardBackAddr;
			remain = system_info.X10KeyBoardFwInfo.filesize;
            //������������
            package = system_info.X10KeyBoardFwInfo.filesize / UPGRADE_PACKAGE_SIZE;
            if((system_info.X10KeyBoardFwInfo.filesize % UPGRADE_PACKAGE_SIZE) != 0)
            {
                package ++;
            }
			index = 0;
			UpgradeIndexSum = 0;
			printf("package��С = %d\r\n", package);
            App_CB_SendStartUpgrade(system_info.X10KeyBoardFwInfo.filesize, package, 
                                    system_info.X10KeyBoardFwInfo.checkSum, 
                                    system_info.X10KeyBoardFwInfo.fw_verson);
            CL_LOG("������������\r\n");
        }
    }

	if((UPGRADE_SUCCESS_FLAG == system_info.KeyBoard.isUpgradeFlag) && (gChgInfo.UpgradeRunning == 0xa5))
    {
    	Feed_WDT();
        if((gChgInfo.UpgradeIndex == index) || (0 == index))
        {
        	if(remain >= UPGRADE_PACKAGE_SIZE)
		    {
		        read_len = UPGRADE_PACKAGE_SIZE;
		    }
		    else
		    {
		        read_len = remain;
		    }
            remain = remain - read_len;
            HT_Flash_ByteRead(&buf[0], readAddr, read_len);
            readAddr += read_len;
            index = gChgInfo.UpgradeIndex + 1;
			UpgradeIndexSum++;
		//	printf("index = %d, %d, %d\r\n", gChgInfo.UpgradeIndex, index, UpgradeIndexSum);
			printf("total %d [%d%%].\n", package, UpgradeIndexSum * 100 / package);
			if(UpgradeIndexSum == (package + 1))
			{
				system_info.KeyBoard.isUpgradeFlag = 0;
				FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
				gChgInfo.UpgradeRunning = 0;
				//Sc8042bSpeech(VOIC_SUCCESS);
				printf("�����ɹ�\r\n");
				vTaskDelay(1000);
				ResetSysTem();
			}
			gChgInfo.UpgradeIndex = 0;
        }
		if (60 > (uint32_t)(GetRtcCount() - gChgInfo.lastRecvKbMsgTime)) 
		{
			//��������
        	App_CB_DownFW(index, buf, read_len);
		}
    }
}

void ProcKbStatus(void)
{
    if (180 < (uint32_t)(GetRtcCount() - gChgInfo.lastRecvKbMsgTime)) 
	{
        gChgInfo.lastRecvKbMsgTime = GetRtcCount();
        gChgInfo.statusErr |= 0x01;
		CL_LOG("no rx ckbs.\n");
        ResetKeyBoard();
        //OptFailNotice(30);
	}
}


void CkbTask(void)
{
    uint8_t  btRxBuff[OUT_NET_PKT_LEN];
	uint32_t old;
    uint32_t second = 0;
    
    
    ResetKeyBoard();
    memset(&gBlueStatus, 0, sizeof(gBlueStatus));
	FIFO_S_Init(&gBlueStatus.rxBtBuff, (void*)btRxBuff, sizeof(btRxBuff));
	memset(&gBlueInfo, 0, sizeof(gBlueInfo));
    gBlueInfo.btState = 1;
    
	while(1) 
    {
        vTaskDelay(20);
		if (old != GetRtcCount()) 
        {
            old = GetRtcCount();
            Feed_WDT();
            second++;
            if (0 == (second & 0x1f)) 
            {
                CheckBlueInfo(1);
                CheckBlueUpgrade();
            }
            ProcKbStatus();
            LcdStatusCheck();
            UiStateLoop();
		}

		//���հ���������
		ProcKBData();
		//���ղ�������������
		RecvBtData();
        
        KeyBoardUpgradeCheck();
	 	RelayCtrlTask();
	}
}


