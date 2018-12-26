
#ifndef __OUTNETPROTO_H__
#define __OUTNETPROTO_H__

#include "includes.h"


#define GET_SWITCH1()		       	            GPIO_ReadInputDataBit(HT_GPIOA, GPIO_Pin_13)
#define GET_SWITCH2()		       	            GPIO_ReadInputDataBit(HT_GPIOA, GPIO_Pin_12)
#define GET_SWITCH3()		       	            GPIO_ReadInputDataBit(HT_GPIOG, GPIO_Pin_4)

enum {
    OUT_NET_START  = 0,                         //进入路由器模式
	OUT_NET_HEART_BEAT = 1,                     //1.
	TEST_START = 0x99,
};

enum {
	REQ_SEND_PKT = 1,                           //1.请求发送报文
	SET_SERVER_PARA,                            //2.设置拨号参数
	SET_UPGRADE,                                //3.请求固件下载
	SET_SN_ID,                                  //4.终端请求网关读取桩编号/识别码
	REQ_FW,                                     //5.请求固件下发
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
	REQ_RECV_PKT = 1,                           //1.请求终端接收报文
	REQ_GET_CCID,                               //2.请求终端读取ccid 和 固件信息
	REQ_DEVICEID,                               //3.请求终端发送deviceid
	REQ_RECV_FW_INFO,                           //4.请求接收固件
};


/*    0：拉远正在等待拨号参数
1：拉远正在拨号
2：拉远拨号失败，正在重新尝试
3：拉远网络异常，正在重启拨号
4：拉远故障，无法完成拨号
5：拉远拨号成功，完成建链
6：等待设置桩号/识别码 */
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
    uint8_t  mode;                              //1:进入路由器模式 2:进入产测软件模式
    uint8_t  connect;                           //0未连接 2:485  3:蓝牙2.4G
    uint8_t  frameSn;
    uint8_t  status;                            //0、无操作 1、上次报文接收正常 2、报文接收失败，请求重发
    uint8_t  rs485Node;                         //485节点号
    uint8_t  blueStatus;                        // 1、收到
    uint32_t lastSendTime;
    uint32_t lastRecvTime;
}OUT_NET_STATUS_STR;



#pragma pack(1)

typedef struct {
    uint8_t  aa;                                //0.
    uint8_t  five;                              //1.
    uint8_t  src[5];                            //2. 桩号   Bcd
    uint8_t  dest[5];                           //3.
    uint16_t len;                               //4.
    uint8_t  ver;                               //5.
    uint8_t  sn;                                //6.
    uint8_t  cmd;                               //7.
}OUT_NET_HEAD_STR;

typedef struct {
    uint32_t time;                              //0. 时间
    uint8_t  status;                            //1. 0、无操作；1、报文接收正常；2、报文接收失败，请求重发
    uint8_t  reqOpt;                            //2. 1、 请求发送报文；2、 设置拨号参数；3、 请求固件下载；4、 设置桩编号/识别码
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
    uint8_t  netType;                           //2：485；3：蓝牙2.4G
    uint8_t  heartBeatStatus;                   //0：心跳接收异常；1：心跳接收正常
    /*    0：拉远正在等待拨号参数
    1：拉远正在拨号
    2：拉远拨号失败，正在重新尝试
    3：拉远网络异常，正在重启拨号
    4：拉远故障，无法完成拨号
    5：拉远拨号成功，完成建链
    6：等待设置桩号/识别码 */
    uint8_t  localStatus;
    uint8_t  errcode;                           //拨号失败代码
    uint8_t  simSignel;                         //0~31，拉远拨号成功，网络正常有效
    uint8_t  cmd;                               //0、无；1、请求接收报文；2、请求读取ICCID；3、请求识别码
    uint8_t  lastOpt;                           //
    uint8_t  lastOptResult;                     //
    uint16_t len;                               //接收报文长度
    uint8_t  pkt[OUT_NET_PKT_LEN];
}OUT_HEART_BEAT_REQ_STR;


typedef struct {
    OUT_ACK_HEAD_STR ackHead;
    uint8_t  pkt[OUT_NET_PKT_LEN-sizeof(OUT_ACK_HEAD_STR)]; //4. 1、 发送报文内容；2、 拨号参数；3、 固件下载链接；4、 设置桩编号/识别码
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


