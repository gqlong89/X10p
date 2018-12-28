/*productTest.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "productTest.h"
#include "card.h"
#include "ui.h"
#include "emu.h"
#include "usart.h"
#include "sim800c.h"
#include "sc8042.h"
#include "flash.h"
#include "lcd.h"
#include "gun.h"
#include "server.h"
#include "rtc.h"



extern __IO uint8_t UART2_RxBuff[UART2_RX_SIZE];
extern GUN_STATUS_STR gun[13];
extern ip_state istage;
extern CKB_BASIC_INFO_REPORT_STR gBlueInfo;
uint8_t TestKeyFlag = 0;
uint8_t TestCardFlag = 0;


/**********************************************************/
void LcdTest(void)
{
	CL_LOG("test lcd!\n");
	LcdAllOn();
    LcdTurnOnLed();
    vTaskDelay(500);

}


void SendTestPktAck(uint8_t cmd, uint8_t *pkt, uint8_t len)
{
	uint8_t TestPktStrAck[128] = {0};
    OUT_PKT_STR *pFrame = (void*)TestPktStrAck;

    pFrame->head.aa = 0xaa;
    pFrame->head.five = 0x55;
    pFrame->head.len = 4+len;
    pFrame->head.ver = 1;
    pFrame->head.cmd = cmd;
    memcpy(pFrame->data, (void*)pkt, len);
    pFrame->data[len] = GetPktSum((void*)&pFrame->head.ver, len+3);
	UsartSend(UART_485_PORT, (void*)pFrame, sizeof(OUT_NET_HEAD_STR)+len+1);
}


void GetAllPower(uint8_t power[])
{
	uint8_t i = 0;
	for (i=0; i<12; i++) {
		power[2*i] = (gun[i+1].power>>8) & 0xFF;
		power[2*i+1] = gun[i+1].power & 0xFF;
	}
}


void TestDataHandle(OUT_PKT_STR *pFrame, uint8_t len)
{
	uint8_t gunId=0;
	uint8_t ackData[64] = {0};

	switch (pFrame->head.cmd) {
		case TEST_CMD_UART485:
			if (pFrame->data[0] == 0x00) {//测试请求开始
				SwitchToUi_ProductTest();
				ackData[0] = 0x00;
				ackData[1] = 0x00;
				SendTestPktAck(TEST_CMD_UART485, (void*)ackData, 2);
			} else if (pFrame->data[0] == 0x01){//测试请求结束
				ackData[0] = 0x01;
				ackData[1] = 0x00;
				SendTestPktAck(TEST_CMD_UART485, (void*)ackData, 2);
                ResetSysTem();
			}
			break;

		case TEST_CMD_KEY:
			TestKeyFlag = 1;
			ackData[0] = 0x00;
			SendTestPktAck(TEST_CMD_KEY, (void*)ackData, 1);
			break;

		case TEST_CMD_CARD:
			TestCardFlag = 1;
			break;

		case TEST_CMD_LCD:
			ackData[0] = 0x00;
			SendTestPktAck(TEST_CMD_LCD, (void*)ackData, 1);
			LcdTest();
		break;

		case TEST_CMD_2G:
			GprsSendCmd("AT+CSQ\r","\r\nOK\r\n",100, 0);
			GprsSendCmd("AT+CCID\r","OK",100, 0);
			vTaskDelay(200);
			ackData[0] = 0x00;
			ackData[1] =  GetNetSignal();
            memcpy(&ackData[2], system_info.iccid, ICCID_LEN);
			SendTestPktAck(TEST_CMD_2G, (void*)ackData, 2+ICCID_LEN);
		break;

		case TEST_CMD_VOICE:
			ackData[0] = 0x00;
			SendTestPktAck(TEST_CMD_VOICE, (void*)ackData, 1);
			Sc8042bSpeech(VOIC_WELCOME);
			break;

		case TEST_CMD_RELAY:
			gunId = pFrame->data[1];
			if (pFrame->data[0] == 0x00) 
            {//继电器打开,上报功率
				//TurnOnAllGun();
				GunTurnOn(gunId+1);
				GunTurnOn(gunId+2);
				GunTurnOn(gunId+3);
				ackData[0] = 0x00;
			} 
            else 
            {//继电器关闭, 上报功率
				//TurnOffAllGun();
				GunTurnOff(gunId+1);
				GunTurnOff(gunId+2);
				GunTurnOff(gunId+3);
				ackData[0] = 0x01;
			}
			vTaskDelay(3000);
			//GetAllPower(&ackData[1]);
			ackData[1] = gunId;
			ackData[2] = (gun[gunId+1].power>>8);
			ackData[3] = gun[gunId+1].power;
			ackData[4] = (gun[gunId+2].power>>8);
			ackData[5] = gun[gunId+2].power;
			ackData[6] = (gun[gunId+3].power>>8);
			ackData[7] = gun[gunId+3].power;

			SendTestPktAck(TEST_CMD_RELAY, (void*)ackData, 8);
			break;

		case TEST_CMD_PCB_CODE:
			switch(pFrame->data[0])
			{
				case 0x00:
					//设置主板编码
					PrintfData("PC set main board sn:", &pFrame->data[1], 8);
					memset(system_info.pcb_sn, 0, sizeof(system_info.pcb_sn));
					memcpy(system_info.pcb_sn, &pFrame->data[1], sizeof(system_info.pcb_sn));
					FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
					break;
				case 0x01:
					//设置按键板编码
					PrintfData("PC set ckb sn:", &pFrame->data[1], 8);
					SendCkbPcbSn(&pFrame->data[1]);
					break;
			}

			ackData[0]=pFrame->data[0];
			ackData[1] = 0x00;//设置成功00 设置失败01
			SendTestPktAck(TEST_CMD_PCB_CODE, (void*)ackData, 2);
			break;

		case TEST_CMD_SET_DEVICE_ID:
            if (memcmp(system_info.station_id, pFrame->data, sizeof(system_info.station_id))) {
                WriteCfgInfo(CFG_CHARGER_SN, CHARGER_SN_LEN, pFrame->data);

                memset(gun_info, 0, sizeof(gun_info));
    		    FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
    			memcpy(system_info.station_id, pFrame->data, sizeof(system_info.station_id));
    			PrintfData("PC write station id:", system_info.station_id, sizeof(system_info.station_id));
    			//更改桩号,删除识别码
    			memset(system_info.idCode, 0, sizeof(system_info.idCode));
                WriteCfgInfo(CFG_CHARGER_DEVICEID, CHARGER_SN_LEN, system_info.idCode);
                UpdataGunDataSum();
                Clear_RecordOrder();
            }
            vTaskDelay(100);
			ackData[0] = 0x00;
			SendTestPktAck(TEST_CMD_SET_DEVICE_ID, (void*)ackData, 1);
			break;

		case TEST_CMD_BLE:
			if (gBlueInfo.btState == 0) {
				ackData[0]=0x01;
			} else {
				ackData[0]=0x02;
			}
			SendTestPktAck(TEST_CMD_BLE, (void*)ackData, 1);
			break;

		case TEST_CMD_FW_VERSION:
			if (pFrame->data[0] == 0) {//0:主板   1：按键板
                ackData[0] = pFrame->data[0];
                ackData[1] = (uint8_t)(FW_VERSION>>8);
                ackData[2] = (uint8_t)(FW_VERSION);
                ackData[3] = (uint8_t)FW_VERSION_SUB;
                SendTestPktAck(TEST_CMD_FW_VERSION, (void*)ackData, 4);
            } else {
                ackData[0] = pFrame->data[0];
                ackData[1] = system_info.fwVersion;
                SendTestPktAck(TEST_CMD_FW_VERSION, (void*)ackData, 2);
            }
			break;

		case TEST_CMD_READ_SN:
			memset(ackData,0,64);
			ackData[0] = pFrame->data[0];
			switch(ackData[0])
			{
				case 0://读取主板编码
					CL_LOG("pc read mian board sn.\n");
					PrintfData("main board pcb SN:", system_info.pcb_sn, 8);
					memcpy(&ackData[1], system_info.pcb_sn, sizeof(system_info.pcb_sn));
					SendTestPktAck(TEST_CMD_READ_SN,(void *)ackData, 9);
					break;

				case 1://读取按键板编码
					CL_LOG("get ckb pcb sn req.\n");
					GetCkbPcbSn();
					break;

				default:
					break;
			}
		    break;
        case DEBUG_CMD_PRINTF:
            SetPrintSwitch(pFrame->data[0]);
            ackData[0]=0;
            SendTestPktAck(DEBUG_CMD_PRINTF, (void*)ackData, 1);
            break;
		default :
			break;
	}
}


