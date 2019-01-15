/*card.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __CARD_H__
#define __CARD_H__

#include "includes.h"



#define M1_CARD_BLOCK_LENTH             (16)
#define PAYMENT_CARD_KEY_MIYAO_ID       (6)

#define ENTRANCE_GUARD_CARD_SECTOR_OFFSET       (10)    //�Ž�������ƫ��
#define ENTRANCE_GUARD_CARD_CHECK_TIMES         (2)     //��������Ž������ܿ��Ĵ���


enum{
    CMD_CARD_UP = (0x01),       //�ϱ�������
    CMD_CARD_READ = (0x02),     //����
    CMD_CARD_WRITE = (0x03),    //д��
};

enum{
    NODE_BLUE = 0,
    NODE_24G
};


enum{
    OptMainTainResetSys = 1,
    OptMainTainResetBlue,
    OptMainTainOpenBroCase,
};


typedef struct {
	uint8_t SeedKeyA[16];		//KEY_A ������Կ
}SECRET_CARD_INFO;//��Կ����Ϣ

typedef struct{
    uint8_t    index;
    uint8_t  data[UPGRADE_PACKAGE_SIZE];
}CB_DOWN_FW_t;

typedef struct{
    uint8_t result;
    uint32_t timestamp;
}CB_RESULE_ACK_t;

typedef struct{
    uint8_t result;
    uint8_t index;
}CB_DOWN_FW_ACK_t;

typedef struct
{
	uint8_t phycard[4];//������
	/*��ʹ�ñ�� 0x00�C���� 0x01-���� 0x02-���� ��������Ƭ
	��������ʹ�������������У���δ����Ķ���
       ���᣺��Ƭ����ʹ�ã���������ע���ȣ�*/
	uint8_t b0_cardUserFlag;
	uint8_t b0_pileNum[6];//���׮��

	int32_t b0_balance;//�����
	uint8_t b0_unixTime[4];//���ʹ��ʱ��

	uint8_t uCardPWD[16];//
	uint8_t s1_block0[16];//
	uint8_t s1_block1[16];//
	uint8_t s1_block2[16];//
	uint8_t s2_block0[16];//
	uint8_t s2_block1[16];//
	uint8_t s2_block2[16];//

	uint8_t b1_cardAppFlag;//����ͨ���  0x11-֧���� 0x12-��Ȩ�� 0x13-δ��ͨ
	uint8_t b1_ucardNum[8];//�û�����
	uint8_t b1_validUnixTime[4];//��Чʱ��

	uint8_t b1_cashPledgeFlag;//Ѻ���� 0x01-��Ѻ��,0x00-��Ѻ��
	//u8 playFlag;//�ÿ������Ѹ���Ѹ���Ϊ1����1Ϊδ����
	uint8_t b1_pwdFlag;//0x01 ���ã�0x00 ������
	uint8_t ucardkeyA[16];//֧�������Ȩ��KeyA
}PAY_CARD_INFO;//֧������Ϣ

typedef enum{
	CARD_TYPE_PAY = 0x11,
	CARD_TYPE_AUTHENTICATION = 0x12,
	CARD_TYPE_NONACTIVATED = 0x13,
}CARD_TYPE;

typedef enum {
	CKB_FIND_AA=0,
	CKB_FIND_FIVE=1,
	CKB_FIND_LEN=2,
	CKB_FIND_VER=3,
	CKB_FIND_SN=4,
	CKB_FIND_MODULE=5,
	CKB_FIND_CMD=6,
	CKB_FIND_DATA=7,
	CKB_FIND_CHECKSUM=8,
}CKB_FIND;

typedef enum {
	WHOLE_MODULE=0,
	KEY_MODULE,
	CARD_MODULE,
	BT_MODULE,		//
	ENUM_MODUL_UPGRADE		= 4,
}MODULE_ID;

enum {
	//
	ENUM_UPGRADE_REQUEST 	= 0x01,
	ENUM_SEND_UPGRADE_PKT 	= 0x02,
};

typedef enum {
	WHOLE_MESSAGE_CMD_BASIC_INFO=0x01,				//������Ϣ�ϱ�
	WHOLE_MESSAGE_CMD_OPERATION_MAINTENANCE=0x02,	//����ά��
	WHOLE_MESSAGE_CMD_STATU_REPORT=0x03,			//״̬�ϱ�
	WHOLE_MESSAGE_CMD_SET_PCB=0x04,					//����PCB����
	WHOLE_MESSAGE_CMD_GET_PCB=0x05,					//��ȡPCB����
}WHOLE_MESSAGE_CMD;

typedef enum {
	BT_MESSAGE_CMD_SET_NAME=0x01,					//������������cmd
	BT_MESSAGE_CMD_SET_MAC=0x02,					//��������mac��ַcmd
	BT_MESSAGE_CMD_RECIVE=0x03,						//��������cmd
	BT_MESSAGE_CMD_SEND=0x04,						//��������cmd
}BT_MESSAGE_CMD;

#pragma pack(1)
typedef struct {
    uint8_t  aa;                                    //0.
    uint8_t  five;                                  //1.
	uint16_t len;								    //2.����
    uint8_t  ver;                                   //3.�汾��
    uint8_t  sn;                                    //4.������ˮ��
	uint8_t  module;							    //5.ģ��ID	//0 ����   1 ����	2 ˢ��	3 ����
    uint8_t  cmd;                                   //6.�������
}CKB_HEAD_STR;

typedef struct {
    CKB_HEAD_STR head;
    uint8_t  data[OUT_NET_PKT_LEN-sizeof(CKB_HEAD_STR)];
}CKB_STR;

//������Ϣ�ϱ�
typedef struct {
	uint8_t fwVersion;						    //8051�̼��汾
	uint8_t btState;						    //����ģ��״̬: 0���� 1ͨ�Ź���
	uint8_t btVersion[BLUE_VERSION_LEN];        //�����汾
	uint8_t btName[BLUE_NAME_LEN+2];            //��������
	uint8_t btMacAddr[BLUE_GWADDR_LEN];	        //��������mac��ַ
}CKB_BASIC_INFO_REPORT_STR;

//������Ϣ�ϱ�ACK
typedef struct {
	uint8_t result;
}CKB_BASIC_INFO_REPORT_ACK_STR;

//����ά��
typedef struct {
	uint8_t type;				//1��λϵͳ	2��λ���� 3���������㲥����
	uint8_t rsv;				//3���������㲥����(0�رչ㲥 1�򿪹㲥)
	uint8_t rsv1;
	uint8_t rsv2;
}CKB_OPERATION_MAINTENANCE_STR;


//״̬�ϱ�
typedef struct {
	uint8_t  sysStatus;				            //ÿһ�����ش���һ���¼������״̬ 0:���� 1:����
	uint8_t  blueStatus;                        //0������1����
    uint8_t  cardStatus;
    uint8_t  keyStatus;
}CKB_STATU_REPORT_STR;

//������PCB����
typedef struct {
	uint8_t pcb[8];
}CKB_PCB_STR;

//���ð�����PCB�������ϱ�
typedef struct {
	uint8_t result;
}CKB_SET_PCB_REPORT_STR;

//��ֵ�ϱ�
typedef struct {
	uint8_t value;		//����0~9	10����	11ȷ��
}KEY_VAL_REPORT_STR;

//��ֵ�ϱ�ACK
typedef struct {
	uint8_t result;		//0:���ճɹ�	1:����ʧ��
}KEY_VAL_REPORT_ACK_STR;

//����Ϣ�ϱ�
typedef struct {
	uint8_t type;		//1:���뿨	2:�����뿨
	uint8_t serial[4];		//�����뿨��Ч
}CARD_INFO_REPORT_STR;

//����Ϣ�ϱ�ACK
typedef struct {
	uint8_t result;		//0:��ȡ�ɹ�	1:��ȡʧ��
}CARD_INFO_REPORT_ACK_STR;

//��������
typedef struct {
	uint8_t keyA[6];
	uint8_t	sectorNum;
	uint8_t blockNum;
}READ_CARD_REQ_STR;

//�������ϱ�
typedef struct {
	uint8_t result;		//0:��ȡ�ɹ�	1:��ȡʧ��
	uint8_t data[16];	//��ȡ�Ŀ�����
}READ_CARD_REPORT_STR;

//д������
typedef struct {
	uint8_t keyA[6];
	uint8_t	sectorNum;
	uint8_t blockNun;
	uint8_t	data[16];
}WRITE_CARD_REQ_STR;

//д�����ϱ�
typedef struct {
	uint8_t result;		//0:д��ɹ�	1:д��ʧ��
}WRITE_CARD_REPORT_STR;

typedef struct {
	uint8_t  btName[12]; 		//��������
}BT_SET_NAME_STR;

typedef struct {
	uint8_t  result;
}BT_SET_NAME_REPORT_STR;

typedef struct {
	uint8_t  btMac[5];			                    //����mac��ַ
}BT_SET_MAC_STR;

typedef struct {
	uint8_t  result;
}BT_SET_MAC_REPORT_STR;

typedef struct {
	uint8_t   type;		                            //0: ����	1:2.4G
	uint16_t  len;                                  //1.
	uint8_t   data[OUT_NET_PKT_LEN];	            //2. ָ����,��������������ͷ��β��
}BT_MESSAGE_RECIVE_STR;


typedef struct {
	uint8_t result;		//0�ɹ�	1ʧ��
}BT_MESSAGE_RECIVE_ACK_STR;//

typedef struct {
	uint8_t  type;		//0:����	1:2.4G
	uint16_t len;
	uint8_t  data[];	//ָ����,��������������ͷ��β��
}BT_MESSAGE_SEND_STR;

typedef struct {
	uint8_t result;		//0�ɹ�	1ʧ��
}BT_MESSAGE_SEND_REPORT_STR;

typedef struct {
	uint8_t isPayCard;          //0x11-֧���� 0x12-��Ȩ�� 0x13-δ��ͨ
    uint8_t PayCardID[8];       //�û����ţ���λ��ǰ����λ�ں� BCD ��
    uint8_t isHaveDeposit;      //0x01-��Ѻ��,0x00-��Ѻ��
    uint32_t UnixTime;        //UNIX ʱ���
    uint8_t ifEnaUserPWFlag;    //�Ƿ������û������־             0x01 ���ã� 0x00 ������
    uint8_t CheckSum;           //У���
}BLOCK9_STRUCT_TYPE;

typedef struct {
	uint8_t CardUserFlag;           //0x00�C���� 0x01-���� 0x02-����
                                    //��������Ƭ��������ʹ��
                                    //�����������У���δ����Ķ���
                                    //���᣺��Ƭ����ʹ�ã���������ע���ȣ�
    uint8_t ChargePileNum[6];       //���׮�� ��λ��ǰ����λ�ں� BCD ��
    uint32_t CardBalance;         //����� ��λ����,��λ��ǰ����λ�ں� ���з��� 16 ������
    uint32_t UnixTime;            //���ʹ��ʱ�� UNIX ʱ���
    uint8_t CheckSum;               //У���
}BLOCK8_STRUCT_TYPE;

typedef struct {
    uint8_t OperatorNumb[10];        //��Ӫ�̱��
    uint8_t CellPhoneNumb[6];         //�ֻ���
}BLOCK_STRUCT_TYPE;

typedef struct{
    uint8_t block9[M1_CARD_BLOCK_LENTH];
    uint8_t block8[M1_CARD_BLOCK_LENTH];
    uint8_t block5[M1_CARD_BLOCK_LENTH];
    uint8_t MiyaoPayment[PAYMENT_CARD_KEY_MIYAO_ID];
} PAYMENT_Card_STR_TYPE;



#pragma pack()


extern void CkbTask(void);
extern void CkbPowerOn(void);
extern void CkbPowerOff(void);
extern int SendBlueNetPkt(uint8_t dest, uint8_t *pPkt, uint16_t len);
extern int SetBtMac(uint8_t *mac);
extern int SendCkbPcbSn(uint8_t *pcb);
extern int GetCkbPcbSn(void);
extern void OpenBluetoothRadio(void);
extern void CloseBluetoothRadio(void);
extern int CheckNetMacAddr(uint8_t *pStr);
extern int OperateMaintain(uint8_t type, uint8_t para);
extern int App_CB_SendStartUpgrade(uint32_t fileSize, uint32_t package, uint16_t checkSum, uint8_t verson);
extern void BswSrv_StartCardBoard_UpgradeTask(void);

extern CKB_BASIC_INFO_REPORT_STR gBlueInfo;
extern TaskHandle_t CardUpgradeHandle_t;

#endif //__CARD_H__


