
#ifndef __OUTNETPROTO_H__
#define __OUTNETPROTO_H__

#include "includes.h"


#define GET_SWITCH1()		       	            GPIO_ReadInputDataBit(HT_GPIOA, GPIO_Pin_13)
#define GET_SWITCH2()		       	            GPIO_ReadInputDataBit(HT_GPIOA, GPIO_Pin_12)
#define GET_SWITCH3()		       	            GPIO_ReadInputDataBit(HT_GPIOG, GPIO_Pin_4)

enum {
    OUT_NET_START  = 0,                         //����·����ģʽ
	OUT_NET_HEART_BEAT = 1,                     //1.
	TEST_START = 0x99,
};

enum {
	REQ_SEND_PKT = 1,                           //1.�����ͱ���
	SET_SERVER_PARA,                            //2.���ò��Ų���
	SET_UPGRADE,                                //3.����̼�����
	SET_SN_ID,                                  //4.�ն��������ض�ȡ׮���/ʶ����
	REQ_FW,                                     //5.����̼��·�
};

enum {
	HEART_BEAT_UNNORMAL = 0,
    HEART_BEAT_NORMAL = 1,                      //1.
};

enum {
	SERVER_IP_ADDR = 1,                         //1.
	SERVER_PORT,
};

enum {
	REQ_RECV_PKT = 1,                           //1.�����ն˽��ձ���
	REQ_GET_CCID,                               //2.�����ն˶�ȡccid �� �̼���Ϣ
	REQ_DEVICEID,                               //3.�����ն˷���deviceid
	REQ_RECV_FW_INFO,                           //4.������չ̼�
};


/*    0����Զ���ڵȴ����Ų���
1����Զ���ڲ���
2����Զ����ʧ�ܣ��������³���
3����Զ�����쳣��������������
4����Զ���ϣ��޷���ɲ���
5����Զ���ųɹ�����ɽ���
6���ȴ�����׮��/ʶ���� */
enum {
	WAIT_NET_PARA = 0,                          //0.
	DIALING,                                    //1.
	DIAL_FAUIL_DIALING,                         //2.
	NET_FAULT_DIALING,                          //3.
	DIAL_FAULT,                                 //4.
	DIAL_SUCCESS,                               //5.
	WAIT_SN_DEVICEID,
};


typedef struct {
    uint8_t  mode;                              //1:����·����ģʽ 2:����������ģʽ
    uint8_t  connect;                           //0δ���� 2:485  3:����2.4G
    uint8_t  frameSn;
    uint8_t  status;                            //0���޲��� 1���ϴα��Ľ������� 2�����Ľ���ʧ�ܣ������ط�
    uint8_t  rs485Node;                         //485�ڵ��
    uint8_t  blueStatus;                        // 1���յ�
    uint32_t lastSendTime;
    uint32_t lastRecvTime;
}OUT_NET_STATUS_STR;



#pragma pack(1)

typedef struct {
    uint8_t  aa;                                //0.
    uint8_t  five;                              //1.
    uint8_t  src[5];                            //2. ׮��   Bcd
    uint8_t  dest[5];                           //3.
    uint16_t len;                               //4.
    uint8_t  ver;                               //5.
    uint8_t  sn;                                //6.
    uint8_t  cmd;                               //7.
}OUT_NET_HEAD_STR;

typedef struct {
    uint32_t time;                              //0. ʱ��
    uint8_t  status;                            //1. 0���޲�����1�����Ľ���������2�����Ľ���ʧ�ܣ������ط�
    uint8_t  reqOpt;                            //2. 1�� �����ͱ��ģ�2�� ���ò��Ų�����3�� ����̼����أ�4�� ����׮���/ʶ����
    uint16_t len;                               //3.
}OUT_ACK_HEAD_STR;

typedef struct {
    OUT_NET_HEAD_STR head;
    OUT_ACK_HEAD_STR ackHead;
    uint8_t  data[OUT_NET_PKT_LEN];
}OUT_ACK_PKT_STR;


typedef struct {
    OUT_NET_HEAD_STR head;
    uint8_t  data[OUT_NET_PKT_LEN];
}OUT_PKT_STR;


typedef struct {
    char   ip[20];
    uint16_t port;
}IP_PORT_STR;


typedef struct {
    uint8_t  chargerSn[5];
    uint8_t  deviceId[8];
}CHARGER_INFO_STR;


typedef struct {
    uint32_t size;
    uint16_t checkSum;
    uint8_t  ver;
    uint8_t  sum;
}TERMINAL_FW_INFO_STR;


typedef struct {
    uint8_t  netType;                           //2��485��3������2.4G
    uint8_t  heartBeatStatus;                   //0�����������쳣��1��������������
    /*    0����Զ���ڵȴ����Ų���
    1����Զ���ڲ���
    2����Զ����ʧ�ܣ��������³���
    3����Զ�����쳣��������������
    4����Զ���ϣ��޷���ɲ���
    5����Զ���ųɹ�����ɽ���
    6���ȴ�����׮��/ʶ���� */
    uint8_t  localStatus;
    uint8_t  errcode;                           //����ʧ�ܴ���
    uint8_t  simSignel;                         //0~31����Զ���ųɹ�������������Ч
    uint8_t  cmd;                               //0���ޣ�1��������ձ��ģ�2�������ȡICCID��3������ʶ����
    uint8_t  lastOpt;                           //
    uint8_t  lastOptResult;                     //
    uint16_t len;                               //���ձ��ĳ���
    uint8_t  pkt[OUT_NET_PKT_LEN];
}OUT_HEART_BEAT_REQ_STR;


typedef struct {
    OUT_ACK_HEAD_STR ackHead;
    uint8_t  pkt[OUT_NET_PKT_LEN-sizeof(OUT_ACK_HEAD_STR)]; //4. 1�� ���ͱ������ݣ�2�� ���Ų�����3�� �̼��������ӣ�4�� ����׮���/ʶ����
}OUT_HEART_BEAT_ACK_STR;


typedef struct {
    uint8_t  result;
    uint8_t  errCode;
}OUT_REMO_CTRL_ACK_STR;


#pragma pack()

int CheckOutNetStart(void);
int SetServerIp(void);
int SetServerPort(void);
int CheckCreatLink(void);
int OutNetSendPkt(uint8_t *pkt, uint16_t len);
int OutSizeNetCheck(void);
int OutNetRecvPkt(uint8_t *pkt, uint16_t *pLen);
void SysTask(void);
int OutNetInit(void);
void OutNetUpgradeProc(void *pFwInfo);
int SendChargerInfo(uint8_t ifType, OUT_ACK_PKT_STR *pkt);
int PutOutNetPkt(uint8_t *pkt, uint16_t len, uint8_t type);
void CheckDialValue(void);


extern OUT_NET_STATUS_STR gOutNetStatus;

#endif


