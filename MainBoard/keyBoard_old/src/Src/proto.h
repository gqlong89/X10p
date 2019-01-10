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

//消息头
#define MSG_HEAD         (0xAA55)

//协议版本
#define COMM_VERSION     (0x01)


#define CMD_UP_SYSTEM_INFO  (0x01)  //上报基本信息
#define CMD_SYSTEM_OPERATE  (0x02)  //系统操作
#define CMD_UP_SYSTEM_STATE (0x03)  //状态上报
#define CMD_WRITE_PCB       (0x04)  //写PCB编码
#define CMD_READ_PCB        (0x05)  //读PCB


//按键上报
#define CMD_KEY          (0x01)


#define CMD_CARD_UP      (0x01) //上报卡类型
#define CMD_CARD_READ    (0x02) //读卡
#define CMD_CARD_WRITE   (0x03) //写卡

#define CMD_SET_BLUE_NAME   (0x01)  //设置蓝牙名称
#define CMD_SET_BLUE_MAC    (0x02)  //设置网关地址
#define CMD_BLUE_SEND       (0x03)  //发送消息
#define CMD_BLUE_RECV       (0x04)  //接收消息




typedef enum{
    MsgType_ALL = 0,
    MsgType_KEY = 1,
    MsgType_CARD = 2,
    MsgType_BLUE = 3,
}MsgType;


//消息头
typedef struct{
    unsigned char head[2];
    unsigned char len[2];
    unsigned char version;
    unsigned char msgId;
    unsigned char type;
    unsigned char cmd;
}ProtoPackageHead;

//消息包
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


//同步系统信息
int SyncSystemInfo(void);
//同步系统状态
int SyncSystemState(void);


//键值上报
int KeyValuesUpLoad(unsigned char values);

//卡类型上报
int CardTypeUpLoad(unsigned char cardType,unsigned char *serialNum);

//卡号上报
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
** Descriptions:        //密钥卡卡类型上报
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int MiYaoCardTypeUpLoad(unsigned char cardType, unsigned char *MiYaoCardData);

int PCBBumUpLoad(unsigned char *pcb);

//透传蓝牙数据
int TransBlueData(unsigned char *gBuleRxData,unsigned int len);


//发送响应消息
int SendMsg_ACK(MsgType type,unsigned char cmd,unsigned char result);



void System_Reset(void);

//初始化
void CommTask_Init(void);


#endif



