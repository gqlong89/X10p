/*proto.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __PROTO_H__
#define __PROTO_H__


#include "includes.h"
#include "tfs.h"
#include "history_order.h"

#define SERVER_PKT_MAX_LEN              200

//1ɨ�� 2���� 3��Ȩ�� 4ʵ���¿� 5�����¿� 6Ǯ����
enum {
    START_TYPE_SCAN_CODE    = 1,
    START_TYPE_BLUE = 2,
    START_TYPE_AUTH_CARD = 3,
    START_TYPE_MONTH_CARD = 4,
    START_TYPE_ELE_MONTH_CARD = 5,
    START_TYPE_WALLET_CARD = 6,
};


#define COST_POWER                      1  //�ֹ���
#define COST_UNIFY                      2   //ͳһ�Ʒ�

#define COST_TEMP_CNT                   4

#define NO_LOAD_TIME                    (45) //����

#define START_NOTICE_REPEAT_TIMERS					4
#define STOP_NOTICE_REPEAT_TIMERS					5

typedef enum {
	WECHAT_GONGZONGHAO_BT = 1,                  //΢�Ź��ں�����
    WECHAT_XIAOCHENGXV_BT = 2,                  //΢��С��������
    ZHIFUBAO_XIAOCHENGXV_BT = 3,                //֧����С��������
    ZHIFUBAO_SHENGHUOHAO_BT = 4,                //֧�������������
    ANDROID_BLUETOOTH = 5,                      //��׿����
    IOS_BLUETOOTH = 6,                          //IOS����
    ORDER_SOURCE_RESERVE1 = 7,
    ORDER_SOURCE_RESERVE2 = 8,
    ORDER_SOURCE_RESERVE3 = 9,
    ORDER_SOURCE_RESERVE4 = 10,
    WECHAT_GONGZONGHAO_REMOTE_START = 11,       //΢�Ź��ں�Զ�̿���
    WECHAT_XIAOCHENGXV_REMOTE_START = 12,       //΢��С����Զ�̿���
    ZHIFUBAO_XIAOCHENGXV_REMOTE_START = 13,     //֧����С����Զ�̿���
    ZHIFUBAO_SHENGHUOHAO_REMOTE_START = 14,     //֧���������Զ�̿���
    ANDROID_REMOTE_START = 15,                  //��׿Զ�̿���
    IOS_REMOTE_START = 16,                      //IOSԶ�̿���


    ORDER_SOURCE_INSERT_COINS = 20,             //Ͷ��
    ORDER_SOURCE_AUTH_CARD = 30,                //��Ȩ��
    ORDER_SOURCE_PAYMENT_CARD = 31,             //Ǯ����
    ORDER_SOURCE_START_OR_STOP_CARD = 32,       //��ͣ��
}ENUM_ORDER_SOURCE_TYPE;

enum {
    REASON_UNKNOW = 0,
    REASON_COMM_ERR = 1,                //1.��¼оƬͨ�Ź���
    REASON_RESET_TIME_ENOUGH,           //2.��λʱ��乻
    REASON_MONEY_ENOUGH,                //3.���乻
    REASON_TIME_ENOUGH,                 //4.ʱ��乻
    REASON_OVER_23H,                    //5.����12Сʱ
    REASON_BLUE,                        //6.��������
    REASON_TOTAL_POWER_LARGE,           //7.��׮���ʹ���
    REASON_AVERAGE_POWER_LOW,           //8.ƽ�����ʹ���
};

enum {
    STOP_NORMAL = 0,                    //0.���շ������ͣ���������磬��ʱ����磬�������ȣ�����ֹͣ���
    STOP_SWIPE_CARD,                    //1.�û�����ˢ��ֹͣ���
    STOP_INPUT_CODE,                    //2.�û���������У����ֹͣ���
    STOP_PHONE,                         //3.�û�Զ�̽���
    STOP_ADMINI,                        //4.����ԱԶ�̽���
    STOP_EMERG_STOP,                    //5.��ͣ���£�ֹͣ���
    STOP_PULL_OUT,                      //6.��⵽ǹͷ�Ͽ���ֹͣ���
    STOP_SYS_REBOOT,                    //7.ϵͳ����ֹͣ���
    STOP_NET_BREAK,                     //8.���׮���ߣ�ֹͣ���
    STOP_POWER_OFF,                     //9.���׮���磬ֹͣ���
    STOP_CHARGER_FAULT,                 //10.���׮���ϣ�ֹͣ���
    STOP_CAR_FAULT,                     //11.�綯�����ϣ�ֹͣ���
    STOP_OTHER,                         //12.����ԭ��ֹͣ��� δ֪ԭ��
    STOP_METER_FAULT,                   //13.������
    STOP_CARD_FAULT,                    //14.ˢ��������
    STOP_LCD_FAULT,                     //15.LCD����
    STOP_CHARGING_FULL,                 //16.soc ����
    STOP_BALANCE_NOT_ENOUG,             //17.����(�ľ�)
    STOP_OVER_LOAD,                     //18.����ֹͣ
    STOP_OVER_VOLTAGE,                  //19.��ѹֹͣ
    STOP_LOW_VOLTAGE,                   //20.Ƿѹֹͣ
    STOP_POWER_TOO_BIG,                 //21.��׮ʵʱ���ʹ���ֹͣ��磻
    STOP_BLUE,                          //22.�ֻ�����ֹͣ
    STOP_GND_FAULT,                     //23.�豸�ӵع���
    STOP_TEMP_HIGHT,                    //24.�豸�¶ȹ���
};

#define STOP_UNKNOW STOP_OTHER

/*
1������
2���ػ�
3������
4���������ӶϿ�
5���������ӶϿ�������
6���������
7���������
8������
9���β�ͷ
10��������ͨ�Ź���
11������˿�۶�
12�����س���
13����ο������ʧ��
14����������Ԥ��
15��оƬ����Ԥ��
16����������
17��У׼�쳣
18: ��������
*/
enum {
	EVENT_START = 1,                        //1.
	EVENT_SHUT_DOWN,                        //2.
	EVENT_POWER_DROP,                       //3.
	EVENT_NET_BREAK,                        //4.
	EVENT_NET_RECONNECT,                    //5.
	EVENT_START_CHARGING,                   //6
	EVENT_STOP_CHARGING,                    //7
	EVENT_CHARGING_FULL,                    //8
	EVENT_PLUG_PULL_OUT,                    //9
	EVENT_CARD_FAULT,                       //10
	EVENT_FUSE_BREAK,                       //11 ����˿�۶�
	EVENT_OVER_LOAD,                        //12 ���س���
    EVENT_MULTI_OPEN_FAIL,                  //13
    EVENT_ENV_TEMP_HIGH,                    //14
    EVENT_CHIP_TEMP_HIGH,                   //15
    EVENT_ORDER_RESEND,                     //16
    EVENT_ADJUST_ERR,                       //17
    EVENT_CHIP_FAULT,                       //18 ��������
    EVENT_GUN_FAULT,                        //0x13 19 �豸�ӵع���
    EVENT_PSW_UPDATE,                       //0x14 20 ��Կ���½�� 0�����³ɹ���1��ID2����ʧ�ܣ�2��Token����3��HashУ��ʧ�ܣ�4���ڲ�����
    EVENT_SMOKE_ALARM,                      //0x15 21 �̸б���
    EVENT_DOOR_F_OPEN,                      //0x16 22 ��բǿ������
};


enum {
    CHIP_EMU = 0,                           //0.����оƬ
    CHIP_FLASH,                             //1.flash
    CHIP_E2PROM,                            //2.e2prom 0ϵͳ��Ϣ��1ǹͷ��Ϣ
    CHIP_BLUE,                              //3.����
};


enum {
    FIRST_START_CHARGING = 1,
    NET_RECOVER,
    POWER_RECOVER,
};


enum {
    EVENT_PROMPT = 1,
    EVENT_ALARM,
    EVENT_FAULT,
};

enum {
    EVENT_OCCUR = 1,
    EVENT_RECOVER,
};

enum {
    SWIPE_CARD = 1,
    SCAN_CODE,
    BLUE_START,
};

enum {
	MESSAGE_VER_NOENCRYPT = 1,
	MESSAGE_VER_ENCRYPT = 2,
};

enum {
    MQTT_CMD_REGISTER = 0,
	MQTT_CMD_START_UP = 1,                  //1.
	MQTT_CMD_CARD_ID_REQ,                   //2.
	MQTT_CMD_REMOTE_SET_POWER_ON,           //3.
	MQTT_CMD_REMOTE_SET_POWER_OFF,          //4.
	MQTT_CMD_REPORT_POWER_ON,               //5.��翪ʼ֪ͨ
	MQTT_CMD_REPORT_POWER_OFF,              //6.������֪ͨ
	MQTT_CMD_HEART_BEAT,                    //7
	MQTT_CMD_COST_DOWN,                     //8
	MQTT_CMD_COST_REQ,                      //9
	MQTT_CMD_DFU_DOWN_FW_INFO,              //10
	MQTT_CMD_REMOTE_CTRL,                   //11
	MQTT_CMD_SYS_LOG,                       //12
    MQTT_CMD_EVENT_NOTICE,                  //13
	MQTT_CMD_AES_REQ=0x12,					//�豸������Կ
	MQTT_CMD_UPDATE_AES_NOTICE=0x13,		//��̨������Կ֪ͨ
};

enum {
    START_FAIL_FAULT = 1,
    START_FAIL_NO_TEMPLATE,
    START_FAIL_CHARGING,
    START_FAIL_NO_ADJUST,
    START_FAIL_PARA_ERR,
    START_FAIL_NET_ERR,                     //6.�˿���������
    START_FAIL_POWER_LARGE,                 //7.��׮���ʹ���
    START_FAIL_NO_PULL_GUN,                 //8.δ��ǹ
    START_FAIL_NO_MEMORY,                   //9.���治��
};

//1��ϵͳ�������� 2������ǹͷ 3���ر�ǹͷ 4������ά��״̬���رճ����� 5������������ 6���趨����������
//7: �趨�������ʷ�ֵ	8:�趨����ʱ�䷧ֵ	9: �趨��ǹʱ�䷧ֵ		10: ��ǹ�Ƿ�ֹͣ��綩��	11: ����ˢ���幤��ģʽ
enum {
    SYSTEM_REBOOT = 1,
    CTRL_OPEN_GUN,                                  //0x02
    CTRL_CLOSE_GUN,                                 //0x03
    CTRL_SET_SERVER_STATUS,                         //0x04
    CTRL_SET_OUT_POWER,                             //0x05
    CTRL_SET_FULL_POWER,                            //0x06
    CTRL_SET_FULL_TIME,                             //0x07
    CTRL_SET_PULL_GUN_TIME,                         //0x08
    CTRL_SET_PULL_GUN_STOP,                         //0x09
    CTRL_SET_CARD_WORK,                             //0x0a
    CTRL_CLEAR_DATA,                                //0x0b
    CTRL_SET_PRINT_SWITCH,                          //0x0c
	CTRL_SET_DISTURBING_TIME,						//0x0D
	CTRL_SET_CHARGING_FULL_STOP = 0x11,             //0x11
	CTRL_OPEN_DOOR,
	CTRL_SET_CHARGE_CHANGE_POWER = 0x13,
};

enum {
    CARD_AUTH = 0,
    CARD_CHECK_MONEY,
	CARD_MONTH,
};


#pragma pack(1)

typedef struct {
    uint8_t  aa;                                //0.
    uint8_t  five;                              //1.
    uint8_t  type;                              //2��x9 2��x10
    uint8_t  chargerSn[CHARGER_SN_LEN];         //3.
    uint16_t len;                               //4. ������ �汾�� �� У����� �������ֽ���
    uint8_t  ver;                               //5. b0:Ϊ1���ǲ�֧�ּ���;0֧�ּ��ܣ�b1:�������Ƿ���ܣ�b2~3:�����㷨����0:AES ECB
    uint16_t sn;                                //6.
    uint8_t  cmd;                               //7.
}PKT_HEAD_STR;

typedef struct {
    PKT_HEAD_STR head;
    uint8_t  data[OUT_NET_PKT_LEN];
}PKT_STR;

typedef struct {
	uint16_t dataLen;
	uint32_t time;
	uint8_t  data[OUT_NET_PKT_LEN];
}ENCRYPT_PKT_STR;

typedef struct {
    uint8_t  device_type[20];                   //1.�豸����
    uint8_t  register_code[16];                 //2.ע����
    uint8_t  hwId[20];                          //3.Ӳ���豸ID
}REGISTER_REQ_STR;

typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //����ƽ̨������豸ʶ���� bcd
}REGISTER_ACK_STR;


// start up  �豸��¼
typedef struct {
	uint8_t  device_type[20];                   //1.
    uint8_t  chargerSn[CHARGER_SN_LEN];         //2.׮��� bcd
	uint8_t  fw_version;                        //3. x10�汾��
	uint8_t  fw_1_ver;                          //4. X10������汾��
    uint8_t  sim_iccid[20];                     //5.
    uint8_t  onNetWay;                          //6.������ʽ  1������������2��485·��������3��2.4G·������
    uint8_t  modeType;                          //7. 1��2G��2��4G��3��nbIot-I��4��nbIot-II
	uint8_t  login_reason;						//8. 1:�ϵ�����			2:���߻ָ��ط�
	uint8_t  gun_number;                        //9. ���׮���ó������������128���˿�
	uint8_t  device_status;                     //10. ָʾ�豸����ʱ�������ֹ��Ĵ���״̬
	uint8_t  statistics_info[8];				//11. ͳ����Ϣ [0~2]:���Ŵ��� [3]����ʱ��,���� [4]��ǹ/������ͣ���� [5-6]����ʱ�� [7]������̼��汾
	uint8_t  fwDownProto;                       //12. 0:ftp��1:http
}START_UP_REQ_STR;

typedef struct {
    uint8_t  result;							//1.��¼���
	uint32_t time_utc;							//2.������ʱ��
}START_UP_ACK_STR;


// card identify request   ˢ����Ȩ
typedef struct {
	uint8_t  gun_id;                            //0.
	uint8_t  card_type;                         //1.
    uint8_t  optType;                           //2�� 0��ˢ����Ȩ����ʼ��磻1������ѯ��2���ֻ��û������Ȩ
	uint8_t  card_id[16];                       //3.
	uint8_t  card_psw[8];                       //4.
    uint8_t  mode;                              //5.���ģʽ  0�����ܳ��� 1������� 2����ʱ�� 3��������
	uint32_t chargingPara;                      //6.������  ���ܳ�����Ϊ0  �����ֱ���1��  ��ʱ�����ֱ���1����  ���������ֱ���0.01kwh
}CARD_AUTH_REQ_STR;

typedef struct {
    uint8_t  gun_id;                            //0.
	uint8_t  result;                            //1.��Ȩ���
	uint8_t  cardType;                          //2. 1�¿�
	uint8_t  rsv;								//3
	uint32_t user_momey;                        //4.�˻���� ��;�¿�:ʣ�����
	uint8_t  order[ORDER_SECTION_LEN];          //5.������
}CARD_AUTH_ACK_STR;

// remote control power-on/off   Զ���������
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  ordersource;                       //2.	@2014-4-13 update
	uint8_t  user_account[16];                  //3.�û��˺�
	uint8_t  userAttribute;                     //�û����� 1:�����¿�
	uint8_t  rsv[3];
	uint32_t money;                             //4.�ʺ���� ��λ����
	uint8_t  order[ORDER_SECTION_LEN];          //5.������
    uint8_t  mode;                              //6.���ģʽ  0�����ܳ��� 1������� 2����ʱ�� 3��������
	uint32_t chargingPara;                      //7.������  ���ܳ�����Ϊ0  �����ֱ���1��  ��ʱ�����ֱ���1����  ���������ֱ���0.01kwh
}START_CHARGING_REQ_STR;
typedef struct {
    uint8_t  gun_id;                            //0
	uint8_t  result;                            //1:�����ɹ�  1: ����ʧ��
    uint8_t  failReason;                        //2���˿ڹ��� 2��û�мƷ�ģ�� 3���Ѿ��ڳ���� 4���豸û��У׼ 5����������
}START_CHARGING_ACK_STR;

//Զ�̽������
typedef struct {
	uint8_t  gun_id;
    uint8_t  resv_0; // Note: added by cedar
    uint8_t  resv_1; // Note: added by cedar
    uint8_t  resv_2; // Note: added by cedar
	uint8_t  order[ORDER_SECTION_LEN];          //������
}STOP_CHARGING_REQ_STR;
typedef struct {
    uint8_t  gun_id;
	uint8_t  result;
}STOP_CHARGING_ACK_STR;

// report device result power-on/off  ��翪ʼ֪ͨ
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  ordersource;                         //2.
    uint8_t  optType;                           //3. 1���״��������֪ͨ 2����������߻ָ��ط� 3�������ϵͳ����ָ��ط�
	uint8_t  user_account[20];                  //4.�û��˺�
	uint8_t  order[ORDER_SECTION_LEN];          //5. ������
	uint32_t start_time;                        //6.��ʼ���ʱ��
	uint32_t start_power;                       //7.��ʼ������
	uint32_t money;                             //8.Ԥ���ѽ��  ��λ���֣����γ��Ԥ�����ѵĽ����Ϊȫ0xff������
    uint32_t template_id;                       //. �Ʒ�ģ��id@2018-4-18 add
    uint8_t subsidyType;				        //. �������� @2018-4-18 add
	uint16_t subsidyPararm;				        //. �������� @2018-4-18 add
}START_CHARGING_NOTICE_STR;

typedef struct {
    uint8_t  gun_id;
	uint8_t  result;
	uint8_t  order[ORDER_SECTION_LEN];          //��̨���ɵĶ�����
}START_CHARGING_NOTICE_ACK_STR;


//������֪ͨ
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  ordersource;                       //2.	@2018-4-13 update
    uint8_t  stop_reason;                       //3.����ԭ��
    uint8_t  stopDetal;                         //4.������ԭ��ϸ��
	uint8_t  addChargingTimeCnt;                //���������׶κ����ڹ��ʱ仯���ӳ����ʱ�����
	uint8_t  ctrlFlag;                          //b0~1:��ǹֹͣ���(1ֹͣ/2��ͣ)��b2~3:������ͣ(1ֹͣ/2��ͣ)
	uint16_t pullGunStopTime;                   //��ǹֹͣ���ʱ�䣬��
    uint8_t  fwVer;                             //6.�̼��汾
	uint8_t  user_account[20];                  //7.�û��˺�
	uint8_t  order[ORDER_SECTION_LEN];          //8.������
	uint32_t startTime;                         //9.��ʼ���ʱ��
	uint32_t stop_time;                         //10.�������ʱ��
	uint32_t startElec;                         //11.��ʼ������
	uint32_t stop_elec;                         //12.����������
	uint32_t charger_cost;                      //13.��������  ��
	uint32_t template_id;                       //14.����id
	uint16_t chargingPower;                     //15.�����ʶμƷѵĹ��ʣ���λ��w
	uint8_t  subsidyType;                       //16. ��������	@2018-4-13 add
	uint16_t subsidyPararm;				        //17. ��������	@2018-4-13 add
    uint8_t  chargerMode;                       //18. ���ģʽ
    uint16_t chargerPararm;                     //19. ������
    uint32_t chargingTime;                      //20. ʵ�ʳ��ʱ�� ��
}STOP_CHARGING_NOTICE_REQ_STR;

typedef struct {
    uint8_t  gun_id;
	uint8_t  result;
}STOP_CHARGING_NOTICE_ACK_STR;


//����������󳤶� 22*12+7=271
typedef struct {
    uint8_t  gunIndex;                          //1.
    uint8_t  chipTemp;                          //2.
    uint8_t  status;                            //3.����״̬ 0 ���� 1 ռ��δ�򿪻�ر� 2 ռ���Ѵ򿪣������ 3 ����
    uint8_t  faultCode;                         //4. ����ǳ���У��ͱ�ʾʣ����ʱ������λ:����
    uint8_t  voltage;                           //5. ��ѹ 1V /����
    uint8_t  current;                           //6.������� �ֱ��ʣ�0.1A/����
    uint16_t power;                             //7.������ʣ���λ����
    uint16_t elec;                              //8.
    uint16_t money;                             //9.
    uint8_t  order[ORDER_SECTION_LEN];          //10.
}GUN_HEART_BEAT_STR;

// MQTT heart-beat  ң�ż�����
typedef struct {
    uint8_t  netSigle;                          //1. Sim���ź�
	uint8_t  envTemp;                           //2. �����¶� �� -50��ƫ��  -50~200
    uint8_t  status;                            //[0]b0:������״̬���� b1~b6:����оƬ״̬����
    uint8_t  rsv;
    uint16_t learnFlag;                         //ÿ·����·��ѧϰ��־
    uint8_t  gunCnt;                            //3. ���α����������ĳ��ӿ���Ŀ
    GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_REQ_STR;

typedef struct {
    uint32_t time;                              //0
    uint8_t  status;                            //1��	���ճɹ�   1��	����ʧ��
	uint32_t traffiId;                          //2.�Ʒ�ģ��id
}HEART_BEAT_ACK_STR;


typedef struct {
    uint16_t startPower;                        //��ʼ���� ��λw
    uint16_t endPower;                          //�������� ��λw
    uint16_t price;                             //ָ��ʱ�������ã��ֱ��ʷ�
    uint16_t duration;                          //�Ʒ�ʱ��,����  ����=100,ʱ��=120��ʾ1Ԫ��2Сʱ
}segment_str;

typedef struct {
    uint8_t  segmentCnt;                        //���ʶ���Ŀ��1~4
    segment_str segmet[COST_TEMP_CNT];
}multiPower_t;

typedef struct {
    uint16_t  price;                            //ָ��ʱ�������ã��ֱ���:��
    uint16_t  duration;                         //�Ʒ�ʱ��,����  ����=100,ʱ��=120��ʾ1Ԫ��2Сʱ
}unify_t;

// cost template   �Ʒ�ģ������
typedef struct {
    uint8_t  gunId;                             //0
	uint32_t template_id;                       //1.
    uint8_t  mode;                              //2. 1�������ʶμƷ� 2��ͳһ�շ�
    union {
        multiPower_t powerInfo;
        unify_t unifyInfo;
    }Data;
}COST_TEMPLATE_REQ_STR;

//
typedef struct {
    uint8_t  gunId;
	uint32_t template_id;
    uint8_t  mode;                              //1�������ʶμƷ� 2��ͳһ�շ�
	uint8_t Data[256];
}COST_TEMPLATE_HEAD_STR;


typedef struct {
    uint8_t  gunId;
	uint8_t  result;
}COST_TEMPLATE_ACK_STR;

typedef struct{
    uint8_t  gun_id;                            //1.
}REQ_COST_TEMPLATE_STR;


// DFU - down fw info �̼�����
typedef struct{
	char     url[48];                           //������������ַ�����㲹0
	char     usrName[4];                        //��¼�û���
	char     psw[4];
    char     fileName[8];                       //�ļ���
	uint32_t checkSum;                          //�ļ��ֽ��ۼӺ�
}DOWN_FW_REQ_STR;

typedef struct {
	uint8_t result;                             //0: �����ɹ�  1: ����ʧ�� 2: У��ʧ�� 3: д��ʧ��
}DOWN_FW_ACK_STR;


typedef struct {
    uint8_t  optCode;                           //1��ϵͳ�������� 2������ǹͷ 3���ر�ǹͷ 4������ά��״̬���رճ����� 5������������ 6���趨����������
    uint32_t para;                              //���Ʋ���  ������趨���ʣ���λ��kw
}REMO_CTRL_REQ_STR;

typedef struct {
    uint8_t  optCode;
    uint8_t  result;
}REMO_CTRL_ACK_STR;


typedef struct {
    uint8_t  gun_id;
    uint8_t  logType;                           //1��������־ 2��ͳ����Ϣ
    uint8_t  data[OUT_NET_PKT_LEN];
}SYS_LOG_STR;


//�¼�֪ͨ
typedef struct {
    uint8_t  gun_id;                            //0. �����0��ʾ��׮,1~128,�����ӿ�
    uint8_t  code;                              //1. �¼�����
    uint8_t  para1;                             //2.
    uint32_t para2;                             //3.
    uint8_t  status;                            //4.1������  2���ָ�
    uint8_t  level;                             //5.�¼��ȼ� 1��ʾ  2�澯  3���ع���
    char     discrip[EVEN_DISCRI_LEN];          //6.�¼���ϸ����
}EVENT_NOTICE_STR;

typedef struct {
    uint8_t  result;                            //0�����ճɹ���1������ʧ��
    uint8_t  gun_id;                            //0. �����0��ʾ��׮,1~128,�����ӿ�
    uint8_t  code;                              //1. �¼�����
    uint8_t  para1;                             //2.
    uint32_t para2;                             //3.
    uint8_t  status;                            //4.1������  2���ָ�
    uint8_t  level;                             //5.�¼��ȼ� 1��ʾ  2�澯  3���ع���
}EVENT_NOTICE_ACK_STR;


//�豸������Կ
typedef struct {
	uint8_t  id2[24];                           //0. ID2
	uint8_t  authCode[100];                     //1. ��֤��
	uint8_t  extra[8];                          //2. ��չ�ֶ�
	uint8_t  reason;                            //3. 0������������ע��ʱ����2��������Ϣʧ��
	uint8_t  token[16];				            //׮�����ɵ����ֵ
	uint8_t  method;				            //0:MD5		1:SHA256
}DEVICE_AES_REQ_STR;

typedef struct {
	uint8_t  result;                            //0. ���
	uint32_t time_utc;                          //1. ϵͳʱ��
	uint8_t  aesInfo[128];	//����token(16byte) + ��Կ(16byte) + hashֵ(MD5:32byte)/(SHA256:64byte)
}DEVICE_AES_REQ_ACK_STR;


//��̨������Կ֪ͨ
typedef struct {
	uint32_t time_utc;
	uint8_t  reason;
}UPDATE_AES_NOTICE_STR;

typedef struct {
	uint8_t result;
}UPDATE_AES_ACK_STR;
#pragma pack()


extern int RecvServerData(PKT_STR *pFrame, uint16_t len);
extern void StopCharging(uint8_t gunId);
//int SendStopChargingNotice(void *order);
extern int SendTradeRecordNotice(HISTORY_ORDER_STR *order);
extern int SendStartUpNotice(int flag);
extern int SendRegister(void);
extern int SendReqCostTemplate(uint8_t gunId);
extern int SendStartChargingNotice(int gun_id, uint32_t startTime, int flag);
void CostTemplateReq(void);
extern void HeartBeatHandle(void);
extern int SendHeartBeatFun(uint8_t gun_id);
extern int SendEventNotice(uint8_t gunId, uint8_t event, uint8_t para1, uint32_t para2, uint8_t status, char *pDisc);
extern int GetPktSum(uint8_t *pData, uint16_t len);
extern int SendCardAuthReq(int flag);
extern int SendSysLog(uint8_t gunId, uint8_t logType, char *pStr);
extern int CheckStartPara(uint8_t gun_id);
extern int MakeTradeInfo(HISTORY_ORDER_STR *mqtt_report_power_off_req, int gun_id);
extern int SendDeviceAesReq(uint32_t time_utc, uint8_t reason);
extern int TestCloseGunProc(uint8_t gunId);
extern int CostTempCopy(COST_TEMPLATE_HEAD_STR *pcost);
extern void StartCharging(uint8_t startMode, uint32_t money, uint8_t *pCardSn, uint8_t *pOrder, uint8_t ordersource);
extern void StartChargingqqqqqq(uint8_t startMode, uint32_t money, uint8_t *pCardSn, uint8_t *pOrder, uint8_t ordersource);
extern int GetLeftChargingTime(uint8_t gun_id);
extern int CheckCostTemplate(void);
extern int OpenGunEmergency(uint8_t gunId, uint16_t timeLimit, uint8_t openType);
extern int TestOpenGunProc(uint8_t gunId, uint16_t timeLimit, uint8_t openType);
extern void SetPullGunStopTimeFuncy(uint32_t time);
extern void InitChargingCtrlPara(uint8_t  gunId);
extern void SetChargeChangePowerFuncy(uint8_t chargePower);

extern uint8_t req_cnt[GUN_NUM_MAX];
extern uint8_t gCardReqFlag;
extern uint8_t gProtoSendBuff[OUT_NET_PKT_LEN];
extern MUX_SEM_STR gProtoSendMux;
extern uint8_t  gSendHearBeatCnt;
extern uint8_t AesKeyUpdateFlag;
extern uint8_t  gID2[TFS_ID2_LEN+1];
extern uint32_t SendTradeRecordNoticeTime;
extern uint32_t SendStartChargingNoticeTime;

#endif //__PROTO_H__

