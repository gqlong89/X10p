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
    B_SHAKE_REQ = 1,					            //握手请求
    B_OPPO_SITE_AUTH = 2,				            //用户鉴权
    B_START_CHARGING = 3,
    B_STOP_CHARGING = 4,
    B_HEART_BEAT = 5,
    B_COST_TEMPLATE_UPLOAD = 6,
    B_REQ_COST_TEMPLATE = 7,
    B_FW_UPGRADE_NOTICE = 8,
    B_FW_DOWN_LOAD = 9,
    B_REQ_BREAK = 0x0A,
	B_DEV_REGISTER	= 0x0B,						    //设备注册
	B_HISTORY_ORDER_UPLOAD = 0x0D,				    //历史订单上传
	B_COST_TEMPLATE_DOWNLOAD = 0x0E,				//计费模板下发
    BT_CMD_SET_CHARGER_SN = 0x0f,
    BT_CMD_SET_DEVICE_ID = 0x10,
    BT_CMD_SET_GW_ADDR = 0x11,                       //0x11
    BT_CMD_SET_SERVER_ADDR = 0x12,                   //0x12
    BT_CMD_SET_SERVER_PORT = 0x13,                   //0x13
    BT_CMD_SET_TERMINAL_INFO = 0x14,				//设置终端编号信息
    BT_CMD_REMO_CTRL = 0x15,
	B_HISTORY_ORDER_ENSURE = 0x16,		            //历史订单上报确认
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
    uint8_t  status;                                //0:蓝牙未连接  1:连接
    uint32_t lastRecvHeartBeat;                     //最近一次接收心跳时间
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
    char     name[4];                               //0. 设备型号
    uint8_t  chargerSn[5];                          //1. 桩编号
    uint8_t  fwVer;                                 //2. 固件版本号
    uint8_t  portCnt;                               //3. 插座数目
    uint8_t  startNo;                               //4. 插座起始编号
}BLUE_SHAKE_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  optType;                               //1. 0：刷卡鉴权，开始充电；1：仅查询余额；2：手机用户反向鉴权
    uint8_t  usrId[11];                             //2. 用户账号（或者卡号）
    uint8_t  mode;                                  //3. 0：智能充满；1：按金额，具体参数见 充电参数；2：按时长；3：按电量
    uint16_t para;                                  //4. 智能充满，为0；按金额，分辨率1分；按时长，分辨率1分钟；按电量，分辨率0.01kwh
}BLUE_OPPO_SITE_AUTH_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:鉴权成功；1:密码错误；2:余额不足/次数用完；3:正在使用；4:账户无效；5:其它原因
    uint8_t  cardType;                              //2. 月卡,  对于手机用户鉴权无效
    uint8_t  rsv;
    uint32_t money;                                 //3. 账户余额，分辨率1分
    uint8_t  order[ORDER_SECTION_LEN];              //4. bcd 订单号
}BLUE_OPPO_SITE_AUTH_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd 订单号
    uint8_t  mode;                                  //2. 0：智能充满；1：按金额，具体参数见 充电参数；2：按时长；3：按电量
    uint16_t para;                                  //3. 智能充满，为0；按金额，分辨率1分；按时长，分辨率1分钟；按电量，分辨率0.01kwh
	uint8_t orderSource;							//4. 订单来源
	uint8_t subsidyType;							//5. 补贴类型
	uint16_t subsidyPararm;							//6. 补贴参数
}BLUE_START_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:启动成功；1: 启动失败
    uint8_t  reason;                                //2. 1：端口故障；2：没有计费模版；3：已经在充电中；4：设备没有校准；5：参数错误
	uint32_t startTime;								//3. 启动时间
	uint16_t startElec;								//4. 启动电量
}BLUE_START_CHARGING_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd 订单号
}BLUE_STOP_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:成功；1: 失败
	uint32_t stopTime;								//2. 结束时间
	uint16_t stopElec;								//3. 结束电量
	uint8_t  reason;                                //4. 结束原因
	uint32_t startTime;                             //5. 开始充电时间
	uint16_t startElec;                             //6. 开始充电电量 分辨率0.01kwh
	uint16_t money;                                 //7. 消费金额 分辨率1分
	uint16_t chargingPower;                         //8. 充电功率 分辨率w
}BLUE_STOP_CHARGING_ACK_STR;


typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  status;                                //1. 充电口状态：0 空闲；1 占用；2 故障；3 离线
    uint8_t  errCode;                               //2. 1~255  当充电口状态为2故障时有效；1：状态异常；2：计量芯片通信故障
}BLUE_GUN_HEART_BEAT_STR;

typedef struct {
    uint8_t  simSignal;                             //0. Sim卡信号
    uint8_t  temp;                                  //1. 环境温度 度 -50度偏移  -50~200
    uint8_t  portCnt;                               //2. 本次报文所包含的充电接口数目
    BLUE_GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_STR;

typedef struct {
    uint32_t time;                                  //0. 系统时间
    uint8_t  result;                                //1. 0接收成功；1接收失败
}BLUE_HEART_BEAT_ACK_STR;

typedef struct {
    uint8_t  optCode;
    uint32_t para;
    uint8_t  result;
}BLUE_REMO_CTRL_ACK_STR;


/**********请求计费模板********/
typedef struct{
	uint8_t gun_id;
	uint32_t template_id;
}BLUE_COST_TEMPLATE_REQ;


/*********上报历史订单********/
typedef struct {
	uint8_t gun_id;
	uint8_t ordersource;				//订单来源
	uint8_t stopReason;					//结束原因
	uint8_t stopDetails;				//结束详情
	uint8_t chargerMode;				//充电模式
	uint16_t chargerPararm;				//充电参数
	uint8_t  subsidyType;				//补贴类型
	uint16_t subsidyPararm;				//补贴参数
	uint8_t fw_version;					//固件版本
	uint8_t phoneSn[16];
	uint8_t order[ORDER_SECTION_LEN];
	uint32_t startTime;
	uint32_t stopTime;
	uint32_t startElec;
	uint32_t stopElec;
	uint32_t cost_price;			//订单费用
	uint32_t template_id;			//计费模板id
	uint16_t power;
	uint8_t sampleTimes;			//采样次数	固定为0
	uint8_t sampleCycle;			//采用周期
}BLUE_UPLOAD_HISTORY_ORDER_REQ_STR;
typedef struct{
	uint8_t gun_id;
	uint8_t result;
}BLUE_UPLOAD_HISTORY_ORDER_ACK_STR;


/**********固件升级********/
typedef struct{
	uint32_t     fw_size;
	uint32_t     package_num;
	uint16_t     checkSum;
    uint8_t      fw_version;
}BLUE_DOWN_FW_REQ_STR;
typedef struct {
	uint8_t result;                             //0: 升级成功  1: 接收失败 2: 校验失败 3: 写入失败
}BLUE_DOWN_FW_ACK_STR;

typedef struct {
	uint8_t data[64];
}BLUE_FW_DOWNLOAD_REQ_STR;
typedef struct {
	uint8_t result;
	uint8_t index;
}BLUE_FW_DOWNLOAD_ACK_STR;

/**********历史订单上报确认*********/
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

/**********设备注册********/
typedef struct {
    uint8_t  device_type[8];                   //1.设备类型
    uint8_t  register_code[16];                 //2.注册码
}BLUE_REGISTER_REQ_STR;
typedef struct {
    uint8_t  result;
    uint8_t  idcode[8];                         //中心平台分配的设备识别码 bcd
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


