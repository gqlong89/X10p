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

#define ENTRANCE_GUARD_CARD_SECTOR_OFFSET       (10)    //门禁卡扇区偏移
#define ENTRANCE_GUARD_CARD_CHECK_TIMES         (2)     //检测增加门禁卡功能卡的次数


enum{
    CMD_CARD_UP = (0x01),       //上报卡类型
    CMD_CARD_READ = (0x02),     //读卡
    CMD_CARD_WRITE = (0x03),    //写卡
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
	uint8_t SeedKeyA[16];		//KEY_A 种子密钥
}SECRET_CARD_INFO;//密钥卡信息

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
	uint8_t phycard[4];//物理卡号
	/*卡使用标记 0x00–正常 0x01-锁定 0x02-冻结 正常：卡片
	可以正常使用锁定：锁定中，有未结算的订单
       冻结：卡片不可使用（黑名单、注销等）*/
	uint8_t b0_cardUserFlag;
	uint8_t b0_pileNum[6];//充电桩号

	int32_t b0_balance;//卡余额
	uint8_t b0_unixTime[4];//最近使用时间

	uint8_t uCardPWD[16];//
	uint8_t s1_block0[16];//
	uint8_t s1_block1[16];//
	uint8_t s1_block2[16];//
	uint8_t s2_block0[16];//
	uint8_t s2_block1[16];//
	uint8_t s2_block2[16];//

	uint8_t b1_cardAppFlag;//卡开通标记  0x11-支付卡 0x12-鉴权卡 0x13-未开通
	uint8_t b1_ucardNum[8];//用户卡号
	uint8_t b1_validUnixTime[4];//有效时间

	uint8_t b1_cashPledgeFlag;//押金标记 0x01-有押金,0x00-无押金
	//u8 playFlag;//该卡是滞已付款，已付款为1，非1为未付款
	uint8_t b1_pwdFlag;//0x01 启用，0x00 不启用
	uint8_t ucardkeyA[16];//支付卡或鉴权卡KeyA
}PAY_CARD_INFO;//支付卡信息

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
	WHOLE_MESSAGE_CMD_BASIC_INFO=0x01,				//基本信息上报
	WHOLE_MESSAGE_CMD_OPERATION_MAINTENANCE=0x02,	//操作维护
	WHOLE_MESSAGE_CMD_STATU_REPORT=0x03,			//状态上报
	WHOLE_MESSAGE_CMD_SET_PCB=0x04,					//设置PCB编码
	WHOLE_MESSAGE_CMD_GET_PCB=0x05,					//获取PCB编码
}WHOLE_MESSAGE_CMD;

typedef enum {
	BT_MESSAGE_CMD_SET_NAME=0x01,					//设置蓝牙名称cmd
	BT_MESSAGE_CMD_SET_MAC=0x02,					//设置蓝牙mac地址cmd
	BT_MESSAGE_CMD_RECIVE=0x03,						//蓝牙接收cmd
	BT_MESSAGE_CMD_SEND=0x04,						//蓝牙发送cmd
}BT_MESSAGE_CMD;

#pragma pack(1)
typedef struct {
    uint8_t  aa;                                    //0.
    uint8_t  five;                                  //1.
	uint16_t len;								    //2.长度
    uint8_t  ver;                                   //3.版本号
    uint8_t  sn;                                    //4.报文流水号
	uint8_t  module;							    //5.模块ID	//0 整体   1 按键	2 刷卡	3 蓝牙
    uint8_t  cmd;                                   //6.命令代码
}CKB_HEAD_STR;

typedef struct {
    CKB_HEAD_STR head;
    uint8_t  data[OUT_NET_PKT_LEN-sizeof(CKB_HEAD_STR)];
}CKB_STR;

//基本信息上报
typedef struct {
	uint8_t fwVersion;						    //8051固件版本
	uint8_t btState;						    //蓝牙模块状态: 0正常 1通信故障
	uint8_t btVersion[BLUE_VERSION_LEN];        //蓝牙版本
	uint8_t btName[BLUE_NAME_LEN+2];            //蓝牙名称
	uint8_t btMacAddr[BLUE_GWADDR_LEN];	        //蓝牙网关mac地址
}CKB_BASIC_INFO_REPORT_STR;

//基本信息上报ACK
typedef struct {
	uint8_t result;
}CKB_BASIC_INFO_REPORT_ACK_STR;

//操作维护
typedef struct {
	uint8_t type;				//1复位系统	2复位蓝牙 3设置蓝牙广播开关
	uint8_t rsv;				//3设置蓝牙广播开关(0关闭广播 1打开广播)
	uint8_t rsv1;
	uint8_t rsv2;
}CKB_OPERATION_MAINTENANCE_STR;


//状态上报
typedef struct {
	uint8_t  sysStatus;				            //每一个比特代表一个事件或故障状态 0:正常 1:产生
	uint8_t  blueStatus;                        //0正常；1故障
    uint8_t  cardStatus;
    uint8_t  keyStatus;
}CKB_STATU_REPORT_STR;

//按键板PCB编码
typedef struct {
	uint8_t pcb[8];
}CKB_PCB_STR;

//设置按键板PCB编码结果上报
typedef struct {
	uint8_t result;
}CKB_SET_PCB_REPORT_STR;

//键值上报
typedef struct {
	uint8_t value;		//数字0~9	10返回	11确认
}KEY_VAL_REPORT_STR;

//键值上报ACK
typedef struct {
	uint8_t result;		//0:接收成功	1:接收失败
}KEY_VAL_REPORT_ACK_STR;

//卡信息上报
typedef struct {
	uint8_t type;		//1:密码卡	2:非密码卡
	uint8_t serial[4];		//非密码卡有效
}CARD_INFO_REPORT_STR;

//卡信息上报ACK
typedef struct {
	uint8_t result;		//0:读取成功	1:读取失败
}CARD_INFO_REPORT_ACK_STR;

//读卡请求
typedef struct {
	uint8_t keyA[6];
	uint8_t	sectorNum;
	uint8_t blockNum;
}READ_CARD_REQ_STR;

//卡数据上报
typedef struct {
	uint8_t result;		//0:读取成功	1:读取失败
	uint8_t data[16];	//读取的卡数据
}READ_CARD_REPORT_STR;

//写卡请求
typedef struct {
	uint8_t keyA[6];
	uint8_t	sectorNum;
	uint8_t blockNun;
	uint8_t	data[16];
}WRITE_CARD_REQ_STR;

//写入结果上报
typedef struct {
	uint8_t result;		//0:写入成功	1:写入失败
}WRITE_CARD_REPORT_STR;

typedef struct {
	uint8_t  btName[12]; 		//蓝牙名称
}BT_SET_NAME_STR;

typedef struct {
	uint8_t  result;
}BT_SET_NAME_REPORT_STR;

typedef struct {
	uint8_t  btMac[5];			                    //蓝牙mac地址
}BT_SET_MAC_STR;

typedef struct {
	uint8_t  result;
}BT_SET_MAC_REPORT_STR;

typedef struct {
	uint8_t   type;		                            //0: 蓝牙	1:2.4G
	uint16_t  len;                                  //1.
	uint8_t   data[OUT_NET_PKT_LEN];	            //2. 指数据,不包含蓝牙报文头和尾部
}BT_MESSAGE_RECIVE_STR;


typedef struct {
	uint8_t result;		//0成功	1失败
}BT_MESSAGE_RECIVE_ACK_STR;//

typedef struct {
	uint8_t  type;		//0:蓝牙	1:2.4G
	uint16_t len;
	uint8_t  data[];	//指数据,不包含蓝牙报文头和尾部
}BT_MESSAGE_SEND_STR;

typedef struct {
	uint8_t result;		//0成功	1失败
}BT_MESSAGE_SEND_REPORT_STR;

typedef struct {
	uint8_t isPayCard;          //0x11-支付卡 0x12-鉴权卡 0x13-未开通
    uint8_t PayCardID[8];       //用户卡号，低位在前，高位在后 BCD 码
    uint8_t isHaveDeposit;      //0x01-有押金,0x00-无押金
    uint32_t UnixTime;        //UNIX 时间戳
    uint8_t ifEnaUserPWFlag;    //是否启用用户密码标志             0x01 启用， 0x00 不启用
    uint8_t CheckSum;           //校验和
}BLOCK9_STRUCT_TYPE;

typedef struct {
	uint8_t CardUserFlag;           //0x00–正常 0x01-锁定 0x02-冻结
                                    //正常：卡片可以正常使用
                                    //锁定：锁定中，有未结算的订单
                                    //冻结：卡片不可使用（黑名单、注销等）
    uint8_t ChargePileNum[6];       //充电桩号 低位在前，高位在后 BCD 码
    uint32_t CardBalance;         //卡余额 单位：分,低位在前，高位在后 ，有符号 16 进制数
    uint32_t UnixTime;            //最近使用时间 UNIX 时间戳
    uint8_t CheckSum;               //校验和
}BLOCK8_STRUCT_TYPE;

typedef struct {
    uint8_t OperatorNumb[10];        //运营商编号
    uint8_t CellPhoneNumb[6];         //手机号
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


