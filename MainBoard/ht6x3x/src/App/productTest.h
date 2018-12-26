#ifndef __PRODUCT_TEST_H__
#define __PRODUCT_TEST_H__

#include "includes.h"
#include "outNetProto.h"



enum
{
	TEST_CMD_KEY = 1,			//��������
	TEST_CMD_CARD = 2,			//ˢ������
	TEST_CMD_LCD = 3,			//LCD����
	TEST_CMD_2G = 4,			//2Gģ�����
	TEST_CMD_VOICE = 5,			//���Ȳ���
	TEST_CMD_RELAY = 6,			//�̵���
	TEST_CMD_PCB_CODE = 7,			//PCB����
	TEST_CMD_SET_DEVICE_ID = 8,	//����׮��
	TEST_CMD_BLE, //BLE����     //9.
	TEST_CMD_FW_VERSION,        //10.
	TEST_CMD_READ_SN,           //11.
	DEBUG_CMD_DEV_TYPE  = 0x0d,                 //0x0d
    DEBUG_CMD_GW_ADDR,                          //0x0e
    DEBUG_CMD_TER_MINAL,                        //0x0f
    DEBUG_CMD_SERVER_ADDR,                      //0x10
    DEBUG_CMD_SERVER_PORT,                      //0x11
    DEBUG_CMD_PRINTF,                           //0x12
    DEBUG_CMD_REBOOT,                           //0x13
    DEBUG_CMD_DEVICE_ID,                        //0x14
    DEBUG_CMD_SET_RTC,                          //0x15 ����RTCʱ��
    DEBUG_CMD_RED_RTC,                          //0x16 ��ȡRTCʱ��
    DEBUG_CMD_FLASH,                            //0x17 Flash��д����
    DEBUG_CMD_SET_LISTEN_MODE,                  //0x18 ����ģ���Զ�����ģʽ
    DEBUG_CMD_QUE_LISTEN_MODE,                  //0x19 ��ѯģ���Զ�����ģʽ
    DEBUG_CMD_LED,                              //0x1a Ledָʾ�Ʋ���
    DEBUG_CMD_485,                              //0x1b 485�ӿڲ���
    DEBUG_CMD_NET_CONNECT,                      //0x1c ��̨�ԽӲ���
    DEBUG_CMD_4G_TO_2G,                         //0x1d ����4G����2G
    DEBUG_CMD_GET_CHARGER_SN,                   //0x1e ��ȡ׮���
    DEBUG_CMD_GET_DEVICE_ID,                    //0x1f ��ȡʶ����
	TEST_CMD_UART485 = 0x99,
};


void Uart485Task(void);
void SendTestPktAck(uint8_t cmd, uint8_t *pkt, uint8_t len);
void TestDataHandle(OUT_PKT_STR *pFrame, uint8_t len);


extern uint8_t TestKeyFlag;
extern uint8_t TestCardFlag;


#endif //__PRODUCT_TEST_H__


