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
    CHARGING_FIXED = 1,                         //固定计费
	CHARGING_ACTUAL,                            //实时计费
	CHARGING_START,                             //起步金
};


enum{
    ORDER_NOT_CARD = 0,                         //0. 不是卡开启的订单，
	ORDER_AUTH_CARD,                            //1.鉴权卡
	ORDER_MONTH_CARD,                           //2.月卡
};

//b0:是否发生过心跳接收失败 b1:是否发送过发送报文失败 b2:是否带有蓝牙 b3:是否带有2.4G b4:是否进行蓝牙升级 b5:是否进行拉远升级
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
    TEST_OPEN = 1,                              //1:测试打开；2:应急码打开；3:后台远程控制打开
    INPUT_CODE_OPEN,
    REMOTE_CTRL_OPEN,
};


enum{
    CFG_CHARGER_SN = 0,                         //0. bcd, SN=10, 10 byte  0001123456789
    CFG_CHARGER_DEVICEID,                       //1. bcd 识别码
    CFG_OPERATORNUM,                            //2. 运营商编号
    CFG_PULL_GUN_STOP,                          //3.拔枪是否停止充电 1:停止；2:不停止
    CFG_CHARGING_FULL_STOP,                     //4.充满是否停止     1:停止；2:不停止
    CFG_PULL_GUN_STOP_TIME,                     //5.拔枪停止充电时长，秒
    CFG_CHARGING_FULL_TIME,                     //6.续冲时长，秒
    CFG_CHARGE_POWER,                           //7.判满变化续充功率 w
};


typedef struct {
    uint32_t chargingTime;
    uint32_t beginTime;                         //计时开始时间
    uint16_t timeLimit;                         //时间限制 秒
    uint16_t currentPower;                      //实时充电功率 0.1w

    uint8_t  isTesting;                         //0:没有测试状态；1:测试打开；2:应急码打开；3:后台远程控制打开
    uint8_t  isPullOutGun;                      //是否疑似拔枪 0不是；1是并发送事件，未收到应答； 0xff:发送事件并收到应答,或者发送超时没有应答
    uint8_t  checkPowerCnt;                     //检测功率次数
    uint16_t powerCheckcnt;                     //导致功率段切换的功率变化检测次数
    uint8_t  inCnt;

    uint8_t  loopCnt;                           //根据拔枪时长、数组长度和检测频率计算出来的循环次数
    uint8_t  isFull;
    uint8_t  powerIndex;
    uint8_t  rsv;                               //

    uint16_t power[POWER_CHECK_CNT];

    uint8_t  startChargerTimers;                //开始充电通知发送次数
    uint8_t  resetEmuChipFlag;
    uint8_t  tempHightCnt;                      //检测到高温次数
    uint8_t  rsvstopFlag;                       //诊断临时关闭继电器标志

    uint32_t stopTime;                          //关闭继电器时间

    uint16_t chargingFullPower;                 //开始判断充满的功率值 0.1w
    uint16_t chargingFullTime;                  //续冲时长 秒

    uint16_t pullGunStopTime;                   //拔枪停止充电时间，秒
    uint8_t  pullGunStop;                       //拔枪停止充电标志: 2拔枪不停止/1拔枪停止
    uint8_t  chargingFullStop;                  //充满自停标志: 1充满停止/2充满不停
//	uint32_t logOpenTime;
}GUN_CHARGING_STR;

typedef struct {
    uint8_t isReadFromElevenSector:4;          //是否从第十一扇区读取
    uint8_t CardMisoperationTimes:4;                //误操作次数
    uint32_t ReadFromElevenSector:2;            //作为门禁卡从第十一扇区读取
    uint32_t KeyBoardModuleFlag:2;
    uint32_t isPayCardFlag:2;
}CARD_FLAG_STR_TYPE;

typedef struct {
    uint32_t turnOnLcdTime;                     //最近一次打开lcd的时间
    uint32_t lastRecvKbMsgTime;                 //上次接收按键板消息时间
    uint32_t lastOpenTime;                      //复用计时变量 枪头开关计时；拉远升级计时
	uint16_t chargingTotalPower;                //充电总功率 w
    uint8_t  netStatus;                         //网络操作状态: b0:是否发生过心跳接收失败 b1:是否发送过发送报文失败 b2:是否带有蓝牙 b3:心跳状态(1正常/0异常) b4:是否进行蓝牙升级 b5:是否进行拉远升级 b6:密钥协商成功 b7:收到密钥更新通知
    uint8_t  inputCode;                         //按键输入动作
    uint32_t lastInputTime;
    uint32_t blueCheck;

    uint32_t KeyBoardModuleTick;
    uint32_t PaymentCardMoney;                  //当前支付卡的余额(单位：分)
    uint8_t  PayCardReadFlag;                   //支付卡读卡标志
    CARD_FLAG_STR_TYPE DataFlagBit;             //数据标志位
    uint8_t  isMiYaoCard;
    uint8_t  payCardBlock;
    uint8_t  isPayCard;
    uint8_t  sendAuthCnt;                       //发送刷卡鉴权的次数
    PAYMENT_Card_STR_TYPE PayCardBlockBuff;
    uint32_t SecondCardTick;
    uint8_t  cardType;                          //开启订单的卡类型，1鉴权卡，2实体月卡
	uint8_t  current_usr_gun_id;                //1~12
    uint8_t  mode;                              //充电模式 0：智能充满 1：按金额 2：按时长 3：按电量
    uint8_t  sendOrderGunId;                    //正在发生订单的枪头id
    uint8_t  ReqKeyReason;                      //设备请求密钥的原因 0、主动请求（如开机、注册时）；1、解密消息失败；2、收到密钥更新通知
    uint8_t  sendCnt;

    uint8_t  lastBlueStatus;                    //上次蓝牙状态 0:正常；1:故障； 2:未确定
    uint8_t  sendBlueStatus;                    //发送蓝牙状态标志 0未发；1发送状态；0xff 收到应答,或发送多次没有应答，停止发送
    uint8_t  sendPktFlag;                       //0:发送报文；1:拉远升级不发送报文 2:本地升级不发报文
    uint8_t  lcdPowerStatus;                    //lcd电源状态 0:关闭；1:打开
	uint8_t  current_usr_card_id[16];

    uint32_t user_card_balance;                 //用户卡余额 从刷卡鉴权应答得到 手机启动 分
	TERMINAL_FW_INFO_STR fwInfo;                //缓存网关过来的固件信息
	uint32_t size;                              //复用变量:固件大小；蓝牙升级计时
    uint16_t sum;
    uint8_t  statusErr;                         //b0:按键板状态错误 b1~b6:计量芯片状态错误
    uint8_t  id2DecrypErrCnt;                   //解密失败次数

    uint8_t  money;                             //刷卡或扫码选择的金额  元
	uint8_t  errCode;                           //语音错误代码
	uint8_t  ckb_sn;
	uint8_t  subsidyType;					    //. 补贴类型 @2018-4-12 add
	uint16_t subsidyPararm;						//. 补贴参数 @2018-4-12 add
    GUN_CHARGING_STR gunCharging[GUN_NUM_MAX];
    AES_KEY  enAes;
    AES_KEY  deAes;
    uint32_t second;
	uint8_t UpgradeIndex;
	uint8_t  UpgradeRunning;
	uint16_t ZeroDetectFlag;
	uint8_t  CloseFlag;
	uint32_t KeyBoardUpgradeTick;
}CHG_INFO_STR;

#pragma pack(1)

typedef struct{
    CKB_HEAD_STR ckbHead;
    OUT_NET_HEAD_STR outNetHead;
    OUT_ACK_HEAD_STR outNetAckHead;
    PKT_HEAD_STR  protoHead;
    uint8_t  data[OUT_NET_PKT_LEN];
}FRAME_STR;


//总共 64 字节
typedef struct{
    uint32_t validFlag;                         //有效标识 b0:桩号 b1:识别码 b2:运营商标号 b3:拔枪自停 b4:充满自停 b5:拔枪停止充电时长 b6:判满变化续充功率
    uint8_t  station_id[8];                     //0. bcd, SN=10, 10 byte  0001123456789
    uint8_t  idCode[8];                         //1. bcd 识别码
    uint8_t  OperatorNum[10];                   //2.
    uint8_t  pullGunStopCharging;               //3.拔枪是否停止充电 1:停止；2:不停止
    uint8_t  chargingFullStop;                  //4.充满是否停止     1:停止；2:不停止
    uint16_t pullGunStopTime;                   //5.拔枪停止充电时长，秒
    uint16_t chargingFullTime;                  //6.充满续充时长，秒
    uint8_t  changePower;                       //7.判满变化续充功率 w
    uint8_t  rsv[25];
    uint16_t checkSum;                          //校验码
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


//该结构体暂时不能放在 pack(1) 里面
typedef struct{
    uint32_t magic_number;
    uint16_t pullGunStopTime;                   //拔枪停止充电时长 秒
    uint8_t  pullGunStop;                       //拔枪停止充电 1:停止；2:不停止
    uint8_t  chargingFullStop;                  //充满自停     1:停止；2:不停止
    uint8_t  iccid[22];                         // ASCII
    uint8_t  station_id[8];                     // bcd, SN=10, 10 byte  0001123456789
    uint8_t  idCode[8];                         // bcd 识别码
	uint8_t  pcb_sn[8];					        //bcd
    uint8_t  printSwitch;                       //打印开关 0关闭  1打开
    uint8_t  changePower;                       //判满变化续充功率 0.1w  默认6w
    COST_TEMPLATE_REQ_STR cost_template;
    uint32_t mqtt_sn;
	uint8_t  chargerMethod;						//计费方式 1、固定收费  2、实时收费  3、起步金收费	@2018-4-12新增
    uint8_t  netType;                           //网络接入类型 1:本地2g 2:485拉远 3蓝牙2.4G: 4:尝试本地 5:尝试拉远
    volatile uint8_t    is_socket_0_ok;
    uint8_t  tcp_tx_error_times;
    uint8_t  isRecvStartUpAck;                  //0:未登录后台 1:登录后台
	SECRET_CARD_INFO SCardInfo;
	uint8_t  fwVersion;						    //8051固件版本
	uint8_t  btVersion[BLUE_VERSION_LEN];       //蓝牙版本
	uint8_t  setBtNameFlag;                     //是否下发了蓝牙名称    0未下发 1下发成功
	uint8_t  setBtMacFlag;                      //是否下发了网关mac地址 0未下发 1下发成功
	uint8_t  gunCheckSum[GUN_NUM_MAX];          //枪头信息累加和
    uint16_t chargingFullTime;                  //充满续冲时间阈值 秒
    uint8_t  changePowerFlag;                   //
    TERMINAL_FW_INFO_STR fwInfo;                //作为终端时的固件信息
    TERMINAL_FW_INFO_STR localFwInfo;           //本地固件信息
    uint16_t noLoadFlag;                        //0xaa55
    uint8_t  noLoadCnt[GUN_NUM_MAX];            //连续开启没有负载的次数，超过4次上报保险丝烧断
	uint16_t chargerStartingGold;				//起步金
	uint8_t  OperatorNum[10];                   //运营商编号
	uint8_t  disturbingStartTime;				//语音禁播开启时间
	uint8_t  disturbingStopTime;				//语音禁播结束时间
	uint16_t voiceFlag;                         //语音提示开关魔术字
	uint16_t cfgFlag;                           //拔枪、充满自停开关及时间魔术字
	uint32_t logOpenTime;
	START_UPGRADE_REQUEST_STR X10KeyBoardFwInfo;
	KEY_BOARD_UPGRADE_FLAG_STR KeyBoard;
	uint8_t  fwType;
    uint8_t  rsv[477];
}system_info_t;


char* GetCurrentTime(void);
#if 1
    #define CL_LOG(fmt,args...) do { 	\
        { \
            printf("[X10p %s] %s:(%d) "fmt,GetCurrentTime(), __func__, __LINE__, ##args); \
        } \
    }while(0)

    #define PRINTF_LOG(fmt,args...) do { \
        printf("[X10p %s]"fmt,GetCurrentTime(), ##args); \
    }while(0)

    #define PRINTF(fmt,args...) do { \
        {\
            printf(""fmt, ##args); \
        }\
    }while(0)
#else
    #define CL_LOG(fmt,args...) do { 	\
        if (LOG_OPEN == system_info.printSwitch) \
        { \
            printf("[X10p %s] %s:(%d) "fmt,GetCurrentTime(), __func__, __LINE__, ##args); \
        } \
    }while(0)

    #define PRINTF_LOG(fmt,args...) do { \
        printf("[X10p %s]"fmt,GetCurrentTime(), ##args); \
    }while(0)

    #define PRINTF(fmt,args...) do { \
        if (LOG_OPEN == system_info.printSwitch) \
        {\
            printf(""fmt, ##args); \
        }\
    }while(0)
#endif

void ServerTask(void);
void PrintfData(void *pfunc, uint8_t *pdata, int len);
void PrintfChar(void *pfunc, uint8_t *pdata, int len);
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


