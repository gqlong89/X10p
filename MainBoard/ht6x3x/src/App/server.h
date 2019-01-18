/*server.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __SERVER_H__
#define __SERVER_H__

#include "includes.h"
#include "proto.h"
#include "card.h"
#include "outNetProto.h"
#include "aes.h"



#define  CHARGING_MAX_TIME                      (12*60)

#define  POWER_CHECK_CNT                        30

typedef enum{
	OFFLINE = 0,
	ONLINE,
}IS_ONLINE;


enum{
    LOCAL_NET = 1,
	OUT_485_NET,
	BLUE_NET,
};

enum{
    CHARGING_FIXED = 1,                         //�̶��Ʒ�
	CHARGING_ACTUAL,                            //ʵʱ�Ʒ�
	CHARGING_START,                             //�𲽽�
};


enum{
    ORDER_NOT_CARD = 0,                         //0. ���ǿ������Ķ�����
	ORDER_AUTH_CARD,                            //1.��Ȩ��
	ORDER_MONTH_CARD,                           //2.�¿�
};

//b0:�Ƿ�������������ʧ�� b1:�Ƿ��͹����ͱ���ʧ�� b2:�Ƿ�������� b3:�Ƿ����2.4G b4:�Ƿ������������ b5:�Ƿ������Զ����
enum{
    HB_RECV_FAIL = 0,                           //0.
    HB_SEND_FAIL,                               //1.
    BLUE_EXIT,                                  //2.
    BLUE_24G_EXIT,                              //3.
    BLUE_UPGRADE,                               //4.
    OUT_NET_UPGRADE,                            //5.
};


typedef enum {
    FIND_AA,
    FIND_55,
    FIND_CHARGER_TYPE,
    FIND_CHAGER_SN,
    FIND_SRC,
    FIND_DEST,
    FIND_LEN,
    FIND_VER,
    FIND_SERNUM,
    FIND_CMD,
    RX_DATA,
    FIND_CHK,
    FIND_END,
} PROTO_MSG_STR;


enum{
    TEST_OPEN = 1,                              //1:���Դ򿪣�2:Ӧ����򿪣�3:��̨Զ�̿��ƴ�
    INPUT_CODE_OPEN,
    REMOTE_CTRL_OPEN,
};


enum{
    CFG_CHARGER_SN = 0,                         //0. bcd, SN=10, 10 byte  0001123456789
    CFG_CHARGER_DEVICEID,                       //1. bcd ʶ����
    CFG_OPERATORNUM,                            //2. ��Ӫ�̱��
    CFG_PULL_GUN_STOP,                          //3.��ǹ�Ƿ�ֹͣ��� 1:ֹͣ��2:��ֹͣ
    CFG_CHARGING_FULL_STOP,                     //4.�����Ƿ�ֹͣ     1:ֹͣ��2:��ֹͣ
    CFG_PULL_GUN_STOP_TIME,                     //5.��ǹֹͣ���ʱ������
    CFG_CHARGING_FULL_TIME,                     //6.����ʱ������
    CFG_CHARGE_POWER,                           //7.�����仯���书�� w
};


typedef struct {
    uint32_t chargingTime;
    uint32_t beginTime;                         //��ʱ��ʼʱ��
    uint16_t timeLimit;                         //ʱ������ ��
    uint16_t currentPower;                      //ʵʱ��繦�� 0.1w

    uint8_t  isTesting;                         //0:û�в���״̬��1:���Դ򿪣�2:Ӧ����򿪣�3:��̨Զ�̿��ƴ�
    uint8_t  isPullOutGun;                      //�Ƿ����ư�ǹ 0���ǣ�1�ǲ������¼���δ�յ�Ӧ�� 0xff:�����¼����յ�Ӧ��,���߷��ͳ�ʱû��Ӧ��
    uint8_t  checkPowerCnt;                     //��⹦�ʴ���
    uint8_t  inCnt;

    uint8_t  loopCnt;                           //���ݰ�ǹʱ�������鳤�Ⱥͼ��Ƶ�ʼ��������ѭ������
    uint8_t  isFull;
    uint8_t  powerIndex;
    uint8_t  rsv;                               //

    uint16_t power[POWER_CHECK_CNT];

    uint8_t  startChargerTimers;                //��ʼ���֪ͨ���ʹ���
    uint8_t  resetEmuChipFlag;
    uint8_t  tempHightCnt;                      //��⵽���´���
    uint8_t  rsvstopFlag;                       //�����ʱ�رռ̵�����־

    uint32_t stopTime;                          //�رռ̵���ʱ��

    uint16_t chargingFullPower;                 //��ʼ�жϳ����Ĺ���ֵ 0.1w
    uint16_t chargingFullTime;                  //����ʱ�� ��

    uint16_t pullGunStopTime;                   //��ǹֹͣ���ʱ�䣬��
    uint8_t  pullGunStop;                       //��ǹֹͣ����־: 2��ǹ��ֹͣ/1��ǹֹͣ
    uint8_t  chargingFullStop;                  //������ͣ��־: 1����ֹͣ/2������ͣ
//	uint32_t logOpenTime;
}GUN_CHARGING_STR;

typedef struct {
    uint8_t isReadFromElevenSector:4;          //�Ƿ�ӵ�ʮһ������ȡ
    uint8_t CardMisoperationTimes:4;                //���������
    uint32_t ReadFromElevenSector:2;            //��Ϊ�Ž����ӵ�ʮһ������ȡ
    uint32_t KeyBoardModuleFlag:2;
    uint32_t isPayCardFlag:2;
}CARD_FLAG_STR_TYPE;

typedef struct {
    uint32_t turnOnLcdTime;                     //���һ�δ�lcd��ʱ��
    uint32_t lastRecvKbMsgTime;                 //�ϴν��հ�������Ϣʱ��
    uint32_t lastOpenTime;                      //���ü�ʱ���� ǹͷ���ؼ�ʱ����Զ������ʱ
	uint16_t chargingTotalPower;                //����ܹ��� w
    uint8_t  netStatus;                         //�������״̬: b0:�Ƿ�������������ʧ�� b1:�Ƿ��͹����ͱ���ʧ�� b2:�Ƿ�������� b3:����״̬(1����/0�쳣) b4:�Ƿ������������ b5:�Ƿ������Զ���� b6:��ԿЭ�̳ɹ� b7:�յ���Կ����֪ͨ
    uint8_t  inputCode;                         //�������붯��
    uint32_t lastInputTime;
    uint32_t blueCheck;

    uint32_t KeyBoardModuleTick;
    uint32_t PaymentCardMoney;                  //��ǰ֧���������(��λ����)
    uint8_t  PayCardReadFlag;                   //֧����������־
    CARD_FLAG_STR_TYPE DataFlagBit;             //���ݱ�־λ
    uint8_t  isMiYaoCard;
    uint8_t  payCardBlock;
    uint8_t  isPayCard;
    uint8_t  sendAuthCnt;                       //����ˢ����Ȩ�Ĵ���
    PAYMENT_Card_STR_TYPE PayCardBlockBuff;
    uint32_t SecondCardTick;
    uint8_t  cardType;                          //���������Ŀ����ͣ�1��Ȩ����2ʵ���¿�
	uint8_t  current_usr_gun_id;                //1~12
    uint8_t  mode;                              //���ģʽ 0�����ܳ��� 1������� 2����ʱ�� 3��������
    uint8_t  sendOrderGunId;                    //���ڷ���������ǹͷid
    uint8_t  ReqKeyReason;                      //�豸������Կ��ԭ�� 0�����������翪����ע��ʱ����1��������Ϣʧ�ܣ�2���յ���Կ����֪ͨ
    uint8_t  sendCnt;

    uint8_t  lastBlueStatus;                    //�ϴ�����״̬ 0:������1:���ϣ� 2:δȷ��
    uint8_t  sendBlueStatus;                    //��������״̬��־ 0δ����1����״̬��0xff �յ�Ӧ��,���Ͷ��û��Ӧ��ֹͣ����
    uint8_t  sendPktFlag;                       //0:���ͱ��ģ�1:��Զ���������ͱ��� 2:����������������
    uint8_t  lcdPowerStatus;                    //lcd��Դ״̬ 0:�رգ�1:��
	uint8_t  current_usr_card_id[16];

    uint32_t user_card_balance;                 //�û������ ��ˢ����ȨӦ��õ� �ֻ����� ��
	TERMINAL_FW_INFO_STR fwInfo;                //�������ع����Ĺ̼���Ϣ
	uint32_t size;                              //���ñ���:�̼���С������������ʱ
    uint16_t sum;
    uint8_t  statusErr;                         //b0:������״̬���� b1~b6:����оƬ״̬����
    uint8_t  id2DecrypErrCnt;                   //����ʧ�ܴ���

    uint8_t  money;                             //ˢ����ɨ��ѡ��Ľ��  Ԫ
	uint8_t  errCode;                           //�����������
	uint8_t  ckb_sn;
	uint8_t  subsidyType;					    //. �������� @2018-4-12 add
	uint16_t subsidyPararm;						//. �������� @2018-4-12 add
    GUN_CHARGING_STR gunCharging[GUN_NUM_MAX];
    AES_KEY  enAes;
    AES_KEY  deAes;
    uint32_t second;
	uint8_t UpgradeIndex;
	uint8_t  UpgradeRunning;
	uint16_t ZeroDetectFlag;
	uint8_t  CloseFlag;
}CHG_INFO_STR;

#pragma pack(1)

typedef struct{
    CKB_HEAD_STR ckbHead;
    OUT_NET_HEAD_STR outNetHead;
    OUT_ACK_HEAD_STR outNetAckHead;
    PKT_HEAD_STR  protoHead;
    uint8_t  data[OUT_NET_PKT_LEN];
}FRAME_STR;


//�ܹ� 64 �ֽ�
typedef struct{
    uint32_t validFlag;                         //��Ч��ʶ b0:׮�� b1:ʶ���� b2:��Ӫ�̱�� b3:��ǹ��ͣ b4:������ͣ b5:��ǹֹͣ���ʱ�� b6:�����仯���书��
    uint8_t  station_id[8];                     //0. bcd, SN=10, 10 byte  0001123456789
    uint8_t  idCode[8];                         //1. bcd ʶ����
    uint8_t  OperatorNum[10];                   //2.
    uint8_t  pullGunStopCharging;               //3.��ǹ�Ƿ�ֹͣ��� 1:ֹͣ��2:��ֹͣ
    uint8_t  chargingFullStop;                  //4.�����Ƿ�ֹͣ     1:ֹͣ��2:��ֹͣ
    uint16_t pullGunStopTime;                   //5.��ǹֹͣ���ʱ������
    uint16_t chargingFullTime;                  //6.��������ʱ������
    uint8_t  changePower;                       //7.�����仯���书�� w
    uint8_t  rsv[25];
    uint16_t checkSum;                          //У����
}CHARGER_STR;

#pragma pack()

typedef struct{
    uint32_t filesize;
    uint32_t package;
    uint16_t checkSum;
    uint8_t  fw_verson;
}START_UPGRADE_REQUEST_STR, *pSTART_UPGRADE_REQUEST_STR;

typedef struct{
    uint32_t isUpgradeFlag;
}KEY_BOARD_UPGRADE_FLAG_STR, *pKEY_BOARD_UPGRADE_FLAG_STR;


//�ýṹ����ʱ���ܷ��� pack(1) ����
typedef struct{
    uint32_t magic_number;
    uint16_t pullGunStopTime;                   //��ǹֹͣ���ʱ�� ��
    uint8_t  pullGunStop;                       //��ǹֹͣ��� 1:ֹͣ��2:��ֹͣ
    uint8_t  chargingFullStop;                  //������ͣ     1:ֹͣ��2:��ֹͣ
    uint8_t  iccid[22];                         // ASCII
    uint8_t  station_id[8];                     // bcd, SN=10, 10 byte  0001123456789
    uint8_t  idCode[8];                         // bcd ʶ����
	uint8_t  pcb_sn[8];					        //bcd
    uint8_t  printSwitch;                       //��ӡ���� 0�ر�  1��
    uint8_t  changePower;                       //�����仯���书�� 0.1w  Ĭ��6w
    COST_TEMPLATE_REQ_STR cost_template;
    uint32_t mqtt_sn;
	uint8_t  chargerMethod;						//�Ʒѷ�ʽ 1���̶��շ�  2��ʵʱ�շ�  3���𲽽��շ�	@2018-4-12����
    uint8_t  netType;                           //����������� 1:����2g 2:485��Զ 3����2.4G: 4:���Ա��� 5:������Զ
    volatile uint8_t    is_socket_0_ok;
    uint8_t  tcp_tx_error_times;
    uint8_t  isRecvStartUpAck;                  //0:δ��¼��̨ 1:��¼��̨
	SECRET_CARD_INFO SCardInfo;
	uint8_t  fwVersion;						    //8051�̼��汾
	uint8_t  btVersion[BLUE_VERSION_LEN];       //�����汾
	uint8_t  setBtNameFlag;                     //�Ƿ��·�����������    0δ�·� 1�·��ɹ�
	uint8_t  setBtMacFlag;                      //�Ƿ��·�������mac��ַ 0δ�·� 1�·��ɹ�
	uint8_t  gunCheckSum[GUN_NUM_MAX];          //ǹͷ��Ϣ�ۼӺ�
    uint16_t chargingFullTime;                  //��������ʱ����ֵ ��
    uint8_t  changePowerFlag;                   //
    TERMINAL_FW_INFO_STR fwInfo;                //��Ϊ�ն�ʱ�Ĺ̼���Ϣ
    TERMINAL_FW_INFO_STR localFwInfo;           //���ع̼���Ϣ
    uint16_t noLoadFlag;                        //0xaa55
    uint8_t  noLoadCnt[GUN_NUM_MAX];            //��������û�и��صĴ���������4���ϱ�����˿�ն�
	uint16_t chargerStartingGold;				//�𲽽�
	uint8_t  OperatorNum[10];                   //��Ӫ�̱��
	uint8_t  disturbingStartTime;				//������������ʱ��
	uint8_t  disturbingStopTime;				//������������ʱ��
	uint16_t voiceFlag;                         //������ʾ����ħ����
	uint16_t cfgFlag;                           //��ǹ��������ͣ���ؼ�ʱ��ħ����
	uint32_t logOpenTime;
	START_UPGRADE_REQUEST_STR X10KeyBoardFwInfo;
	KEY_BOARD_UPGRADE_FLAG_STR KeyBoard;
	uint8_t  fwType;
    uint8_t  rsv[477];
}system_info_t;


char* GetCurrentTime(void);
#if 0
#if (0 == SPECIAL_NET_CARD)
#define CL_LOG(fmt,args...) do { if (system_info.printSwitch) printf("X10 %s %s:(%d) "fmt,GetCurrentTime(), __func__, __LINE__, ##args); }while(0)
#elif (1 == SPECIAL_NET_CARD)
#define CL_LOG(fmt,args...) do { if (system_info.printSwitch) printf("X10m %s %s:(%d) "fmt,GetCurrentTime(), __func__, __LINE__, ##args); }while(0)
#elif (2 == SPECIAL_NET_CARD)
#define CL_LOG(fmt,args...) do { if (system_info.printSwitch) printf("X10c %s %s:(%d) "fmt,GetCurrentTime(), __func__, __LINE__, ##args); }while(0)
#endif
#else
#define CL_LOG(fmt,args...) do { if (0 == system_info.printSwitch) printf("[X10p %s] %s:(%d) "fmt,GetCurrentTime(), __func__, __LINE__, ##args); }while(0)
#define PRINTF_LOG(fmt,args...) do { \
printf("[X10p %s]"fmt,GetCurrentTime(), ##args); \
}while(0)

#define PRINTF(fmt,args...) do { \
	if (0 == system_info.printSwitch) \
	{\
		printf(""fmt, ##args); \
	}\
}while(0)

#endif

void ServerTask(void);
void PrintfData(void *pfunc, uint8_t *pdata, int len);
int GetCostMode(void);
int StrToHex(uint8_t *pbDest, const char *pbSrc, int srcLen);
void ProcNetStatus(void);
void SendHistoryOrder(void);
void ChargingCtrlProc(void);
void MuxSempGive(MUX_SEM_STR *pSemp);
void DeviceBcd2str(char *des, unsigned char *src, int len);
int IsSysOnLine(void);
void ChargingProc(void);
void ProcSysEven(void);
void ProcGunStatus(void);
void LcdStatusCheck(void);
int GetDialValue(void);
void SycTimeCount(uint32_t second);
void ResetSysTem(void);
int MuxSempTake(MUX_SEM_STR *pSemp);
void UpdataGunDataSum(void);
void StartChargingSend(void);
int CheckChargerSn(uint8_t *pChargerSn, uint8_t len);
void ChargerInfoProc(void);
int GetNoZeroSum(uint8_t *pData, uint16_t len);
void ShowCostTemplate(void);
void WriteCfgInfo(uint8_t index, uint32_t para, uint8_t *pData);
void PrintSysCfgInfo(void);


extern const uint8_t gZeroArray[64];
extern system_info_t system_info;
extern CHG_INFO_STR gChgInfo;
extern uint8_t SendRecordTimers;

#endif //__SERVER_H__


