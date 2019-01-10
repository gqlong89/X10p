#ifndef __COMMTASK_H__
#define __COMMTASK_H__


extern FIFO_S_t commFIfO;

#define PAYMENT_CARD_ID      5
#define PAYMENT_CARD_KEY_MIYAO_ID      6
#define M1_CARD_BLOCK_LENTH      16
#define M1_CARD_BLOCK5_LENTH      16


typedef struct{
    unsigned char soft_version;
    unsigned char blue_state;
    unsigned char blue_version[20];
    unsigned char blue_name[12];
    unsigned char gw_mac[5];
}xdata SystemInfo;


typedef struct{
    unsigned char system_state;
    unsigned char blue_state;
    unsigned char card_state;
    unsigned char key_state;
}xdata SystemStatus;


#define MAX_RECV_BUF      400
#define MAX_PACKAGE_SIZE  400

//��Ϣͷ
#define MSG_HEAD         (0xAA55)

//Э��汾
#define COMM_VERSION     (0x01)


#define CMD_UP_SYSTEM_INFO  (0x01)  //�ϱ�������Ϣ
#define CMD_SYSTEM_OPERATE  (0x02)  //ϵͳ����
#define CMD_UP_SYSTEM_STATE (0x03)  //״̬�ϱ�
#define CMD_WRITE_PCB       (0x04)  //дPCB����
#define CMD_READ_PCB        (0x05)  //��PCB


//�����ϱ�
#define CMD_KEY          (0x01)


#define CMD_CARD_UP      (0x01) //�ϱ�������
#define CMD_CARD_READ    (0x02) //����
#define CMD_CARD_WRITE   (0x03) //д��

#define CMD_SET_BLUE_NAME   (0x01)  //������������
#define CMD_SET_BLUE_MAC    (0x02)  //�������ص�ַ
#define CMD_BLUE_SEND       (0x03)  //������Ϣ
#define CMD_BLUE_RECV       (0x04)  //������Ϣ




typedef enum{
    MsgType_ALL = 0,
    MsgType_KEY = 1,
    MsgType_CARD = 2,
    MsgType_BLUE = 3,
}MsgType;


//��Ϣͷ
typedef struct{
    unsigned char head[2];
    unsigned char len[2];
    unsigned char version;
    unsigned char msgId;
    unsigned char type;
    unsigned char cmd;
}ProtoPackageHead;

//��Ϣ��
typedef struct{
    ProtoPackageHead head;
    unsigned int msgLen;
    unsigned char *msg;
    unsigned char checksum;
}xdata ProtoPackage;




#define UINT8_TO_STREAM(p, d)   {*(p)++ = (unsigned char)(d);}
#define STREAM_TO_UINT8(d, p)   {d = (unsigned char)(*(p)); (p) += 1;}
#define UINT16_TO_STREAM(p, d) {*(p)++ = (unsigned char)(d); *(p)++ = (unsigned char)((d) >> 8);}
#define STREAM_TO_UINT16(d, p) {d = ((unsigned short)(*(p)) + (((unsigned short)(*((p) + 1))) << 8)); (p) += 2;}


unsigned char checkSum(unsigned char *buf,unsigned int len);


int Package_Send(ProtoPackage *st);


//ͬ��ϵͳ��Ϣ
int SyncSystemInfo(void);
//ͬ��ϵͳ״̬
int SyncSystemState(void);


//��ֵ�ϱ�
int KeyValuesUpLoad(unsigned char values);

//�������ϱ�
int CardTypeUpLoad(unsigned char cardType,unsigned char *serialNum);

//�����ϱ�
int CardNumUpLoad(unsigned char result,unsigned char *cardNum);

/*****************************************************************************
** Function name:       CardBlockEightUpLoad
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int CardBlockEightUpLoad(unsigned char result,unsigned char *cardNum);
/*****************************************************************************
** Function name:       WriteCardBlockSuccessUpLoad
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int WriteCardBlockSuccessUpLoad(unsigned char result);
/*****************************************************************************
** Function name:       MiYaoCardTypeUpLoad
** Descriptions:        //��Կ���������ϱ�
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int MiYaoCardTypeUpLoad(unsigned char cardType, unsigned char *MiYaoCardData);

int PCBBumUpLoad(unsigned char *pcb);

//͸����������
int TransBlueData(unsigned char *gBuleRxData,unsigned int len);


//������Ӧ��Ϣ
int SendMsg_ACK(MsgType type,unsigned char cmd,unsigned char result);



void System_Reset(void);

//��ʼ��
void CommTask_Init(void);


#endif



