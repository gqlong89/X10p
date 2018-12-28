/*relay.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#ifndef __GUN_H__
#define __GUN_H__

#include "includes.h"
#include "proto.h"



#define PLUG_IDLE               0
#define PLUG_USED_NO_OPEN       1
#define PLUG_USED_OPEN          2
#define PLUG_FAULT              3


enum
{
	GUN_CHARGING_IDLE=0,    		    //空闲
    GUN_CHARGING_UNKNOW_POWER=1,        //充电中功率未检测
	GUN_CHARGING_GUN_PULL=2,		    //功率小于1.5w
	GUN_CHARGING_FULL=3,		        //1.5w<功率<40w
	GUN_CHARGING_WORK,                  //40w<功率
};


enum{
    GUN_STATE_IDLE = 0,
    GUN_STATE_WAIT_PLUG_IN,
    GUN_STATE_ON,
    GUN_STATE_OFF,
};

#pragma pack(1)

typedef struct{
	uint8_t  gun_id;
    uint8_t  gun_state;                 //1等待插枪 2充电中
    uint8_t  is_load_on;                //0:没有启动充电，非0:启动充电状态 1:充电中功率未检测  2:功率小于1.5w 3:1.5w<功率<30w 4:30w<功率
	uint8_t  ordersource;				//订单来源  @2018-4-12 add
	uint8_t  user_account[16];          //卡号 //卡号即用户账号
	uint8_t  chargerMethod;			    //计费方式 1、固定收费  2、实时收费  3、起步金收费
    uint8_t  changePower;               //判满变化续充功率 0.1w
	uint16_t chargerStartingGold;       //起步金额 分
	uint8_t  order[ORDER_SECTION_LEN];
    uint16_t charger_time_plan;         //预计充电时长 分钟  至少3分钟
    uint8_t  cost_mode;                 //计费模式 1分功率  2不分功率 COST_UNIFY
    multiPower_t powerInfo;             //分功率段信息
    uint8_t  getPowerFlag;              //检测功率段次数
    uint32_t costTempId;                //计费模版id
	uint32_t start_time;                //订单开始时间
	uint32_t stop_time;
    uint32_t startElec;
    uint16_t money;                     //分钟计费累计消费金额 分
    uint8_t  startMode;                 //启动模式 1扫码 2蓝牙 3鉴权卡 4月卡 5电子月卡 6钱包卡
	uint16_t current_usr_money;		    //充电金额  分
    uint8_t  stopReason;
    uint8_t  reasonDetail;              //停止充电原因细节
    uint8_t  chargingMode;              //充电模式 0：智能充满 1：按金额 2：按时长 3：按电量
    uint8_t  maxPower;					//充电过程中最大功率 0.1w 大于10w是10w
    uint16_t realChargingTime;          //实际充电时间，不含掉电时间 分钟
	uint16_t chargingPower;             //用于功率段计费的功率 0.1w
    uint8_t  powerCheckcnt;             //功率读取次数
    uint8_t  powerSemenIndex;           //功率段指示
    uint16_t chargingElec;              //0.01kwh 累计充电电量
    uint8_t  subsidyType;				//. 补贴类型 @2018-4-13 add
	uint16_t subsidyPararm;				//. 补贴参数 @2018-4-13 add
    uint8_t  isSync;                    //订单开启同步状态 0-不需要同步 1-第一次启动 2--网络重连 3--重新上电  @2018-4-18 add
	uint16_t startGoldTime;				//起步金时间 分钟
}gun_info_t;
#pragma pack()


typedef struct{
    HT_GPIO_TypeDef* port;
    uint16_t pin;
}Gun_GPIO_Port_t;



extern int CheckGunStatu(uint8_t gunId);
extern int GetChargingGunCnt(void);
extern int GunTurnOff(uint8_t gunId);
extern int GunTurnOn(uint8_t gunId);
extern int TurnOffAllGun(void);
extern int TurnOnAllGun(void);

extern int GunGpioInit(void);
extern int GunInit(void);
extern void RestoreGunStatus(void);


extern gun_info_t gun_info[GUN_NUM_MAX];
extern uint32_t chargingOldTime;

#endif


