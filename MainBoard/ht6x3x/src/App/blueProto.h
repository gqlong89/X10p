/*key.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __BLUE_PROTO_H__
#define __BLUE_PROTO_H__

#include "includes.h"
#include "history_order.h"
#include "FIFO.h"


enum {
    B_SHAKE_REQ = 1,					            //��������
    B_OPPO_SITE_AUTH = 2,				            //�û���Ȩ
    B_START_CHARGING = 3,
    B_STOP_CHARGING = 4,
    B_HEART_BEAT = 5,
    B_COST_TEMPLATE_UPLOAD = 6,
    B_REQ_COST_TEMPLATE = 7,
    B_FW_UPGRADE_NOTICE = 8,
    B_FW_DOWN_LOAD = 9,
    B_REQ_BREAK = 0x0A,
	B_DEV_REGISTER	= 0x0B,						    //�豸ע��
	B_HISTORY_ORDER_UPLOAD = 0x0D,				    //��ʷ�����ϴ�
	B_COST_TEMPLATE_DOWNLOAD = 0x0E,				//�Ʒ�ģ���·�
    BT_CMD_SET_CHARGER_SN = 0x0f,
    BT_CMD_SET_DEVICE_ID = 0x10,
    BT_CMD_SET_GW_ADDR = 0x11,                       //0x11
    BT_CMD_SET_SERVER_ADDR = 0x12,                   //0x12
    BT_CMD_SET_SERVER_PORT = 0x13,                   //0x13
    BT_CMD_SET_TERMINAL_INFO = 0x14,				//�����ն˱����Ϣ
    BT_CMD_REMO_CTRL = 0x15,
	B_HISTORY_ORDER_ENSURE = 0x16,		            //��ʷ�����ϱ�ȷ��
};



enum {
    BT_FIND_EE,                                     //0.
    BT_FIND_CMD,                                    //1.
    BT_FIND_LEN,                                    //2.
    BT_RX_DATA,                                     //3.
    BT_FIND_CHK,
    BT_FIND_END,
};

typedef struct {
    uint8_t  status;                                //0:����δ����  1:����
    uint32_t lastRecvHeartBeat;                     //���һ�ν�������ʱ��
	FIFO_S_t rxBtBuff;
}BLUE_STATUS_STR;

#pragma pack(1)

typedef struct {
    uint8_t  start;
    uint8_t  cmd;
    uint8_t  len;
}BLUE_PROTO_HEAD_STR;

typedef struct {
    BLUE_PROTO_HEAD_STR head;
    uint8_t  data[256];
    //uint8_t  checkSum;
}BLUE_PROTO_STR;



typedef struct {
    uint32_t time;                                  //0.
	char phonesSn[12];                              //1.
}BLUE_SHAKE_REQ_STR;

typedef struct {
    char     name[4];                               //0. �豸�ͺ�
    uint8_t  chargerSn[5];                          //1. ׮���
    uint8_t  fwVer;                                 //2. �̼��汾��
    uint8_t  portCnt;                               //3. ������Ŀ
    uint8_t  startNo;                               //4. ������ʼ���
}BLUE_SHAKE_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  optType;                               //1. 0��ˢ����Ȩ����ʼ��磻1������ѯ��2���ֻ��û������Ȩ
    uint8_t  usrId[11];                             //2. �û��˺ţ����߿��ţ�
    uint8_t  mode;                                  //3. 0�����ܳ�����1��������������� ��������2����ʱ����3��������
    uint16_t para;                                  //4. ���ܳ�����Ϊ0�������ֱ���1�֣���ʱ�����ֱ���1���ӣ����������ֱ���0.01kwh
}BLUE_OPPO_SITE_AUTH_STR;

typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  result;                                //1. 0:��Ȩ�ɹ���1:�������2:����/�������ꣻ3:����ʹ�ã�4:�˻���Ч��5:����ԭ��
    uint8_t  cardType;                              //2. �¿�,  �����ֻ��û���Ȩ��Ч
    uint8_t  rsv;
    uint32_t money;                                 //3. �˻����ֱ���1��
    uint8_t  order[ORDER_SECTION_LEN];              //4. bcd ������
}BLUE_OPPO_SITE_AUTH_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd ������
    uint8_t  mode;                                  //2. 0�����ܳ�����1��������������� ��������2����ʱ����3��������
    uint16_t para;                                  //3. ���ܳ�����Ϊ0�������ֱ���1�֣���ʱ�����ֱ���1���ӣ����������ֱ���0.01kwh
	uint8_t orderSource;							//4. ������Դ
	uint8_t subsidyType;							//5. ��������
	uint16_t subsidyPararm;							//6. ��������
}BLUE_START_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  result;                                //1. 0:�����ɹ���1: ����ʧ��
    uint8_t  reason;                                //2. 1���˿ڹ��ϣ�2��û�мƷ�ģ�棻3���Ѿ��ڳ���У�4���豸û��У׼��5����������
	uint32_t startTime;								//3. ����ʱ��
	uint16_t startElec;								//4. ��������
}BLUE_START_CHARGING_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd ������
}BLUE_STOP_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  result;                                //1. 0:�ɹ���1: ʧ��
	uint32_t stopTime;								//2. ����ʱ��
	uint16_t stopElec;								//3. ��������
	uint8_t  reason;                                //4. ����ԭ��
	uint32_t startTime;                             //5. ��ʼ���ʱ��
	uint16_t startElec;                             //6. ��ʼ������ �ֱ���0.01kwh
	uint16_t money;                                 //7. ���ѽ�� �ֱ���1��
	uint16_t chargingPower;                         //8. ��繦�� �ֱ���w
}BLUE_STOP_CHARGING_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. ǹ�� ��1��ʼ
    uint8_t  status;                                //1. ����״̬��0 ���У�1 ռ�ã�2 ���ϣ�3 ����
    uint8_t  errCode;                               //2. 1~255  ������״̬Ϊ2����ʱ��Ч��1��״̬�쳣��2������оƬͨ�Ź���
}BLUE_GUN_HEART_BEAT_STR;

typedef struct {
    uint8_t  simSignal;                             //0. Sim���ź�
    uint8_t  temp;                                  //1. �����¶� �� -50��ƫ��  -50~200
    uint8_t  portCnt;                               //2. ���α����������ĳ��ӿ���Ŀ
    BLUE_GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_STR;

typedef struct {
    uint32_t time;                                  //0. ϵͳʱ��
    uint8_t  result;                                //1. 0���ճɹ���1����ʧ��
}BLUE_HEART_BEAT_ACK_STR;

typedef struct {
    uint8_t  optCode;
    uint32_t para;
    uint8_t  result;
}BLUE_REMO_CTRL_ACK_STR;


/**********����Ʒ�ģ��********/
typedef struct{
	uint8_t gun_id;
	uint32_t template_id;
}BLUE_COST_TEMPLATE_REQ;


/*********�ϱ���ʷ����********/
typedef struct {
	uint8_t gun_id;
	uint8_t ordersource;				//������Դ
	uint8_t stopReason;					//����ԭ��
	uint8_t stopDetails;				//��������
	uint8_t chargerMode;				//���ģʽ
	uint16_t chargerPararm;				//������
	uint8_t  subsidyType;				//��������
	uint16_t subsidyPararm;				//��������
	uint8_t fw_version;					//�̼��汾
	uint8_t phoneSn[16];
	uint8_t order[ORDER_SECTION_LEN];
	uint32_t startTime;
	uint32_t stopTime;
	uint32_t startElec;
	uint32_t stopElec;
	uint32_t cost_price;			//��������
	uint32_t template_id;			//�Ʒ�ģ��id
	uint16_t power;
	uint8_t sampleTimes;			//��������	�̶�Ϊ0
	uint8_t sampleCycle;			//��������
}BLUE_UPLOAD_HISTORY_ORDER_REQ_STR;
typedef struct{
	uint8_t gun_id;
	uint8_t result;
}BLUE_UPLOAD_HISTORY_ORDER_ACK_STR;


/**********�̼�����********/
typedef struct{
	uint32_t     fw_size;
	uint32_t     package_num;
	uint16_t     checkSum;
    uint8_t      fw_version;
}BLUE_DOWN_FW_REQ_STR;
typedef struct {
	uint8_t result;                             //0: �����ɹ�  1: ����ʧ�� 2: У��ʧ�� 3: д��ʧ��
}BLUE_DOWN_FW_ACK_STR;

typedef struct {
	uint8_t data[64];
}BLUE_FW_DOWNLOAD_REQ_STR;
typedef struct {
	uint8_t result;
	uint8_t index;
}BLUE_FW_DOWNLOAD_ACK_STR;

/**********��ʷ�����ϱ�ȷ��*********/
typedef struct{
	uint8_t 	gun_id;
	uint32_t 	startTime;
	uint32_t 	stopTime;
}BLUE_Record_ENSURE_REQ_STR;
typedef struct{
	uint8_t 	gun_id;
	uint32_t 	startTime;
	uint8_t 	result;
}BLUE_Record_ENSURE_ACK_STR;

/**********�豸ע��********/
typedef struct {
    uint8_t  device_type[8];                   //1.�豸����
    uint8_t  register_code[16];                 //2.ע����
}BLUE_REGISTER_REQ_STR;
typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //����ƽ̨������豸ʶ���� bcd
}BLUE_REGISTER_ACK_STR;


#pragma pack()


void BlueProtoProc(BLUE_PROTO_STR *pMsg, uint16_t len);
void BlueSendHeartBeat(void);
void BlueUpLoadHistoryOrder(HISTORY_ORDER_STR *order);
void ProcBtHeartBeat(void);
void BlueCostTemplateUpload(uint8_t gunid);
void BlueCostTemplateReq(uint8_t gunid);
void BlueRegister(void);
void CheckBlueUpgrade(void);
void BlueTimingOpen(void);

extern BLUE_STATUS_STR gBlueStatus;

#endif //__BLUE_PROTO_H__


