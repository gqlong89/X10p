/*sc8042.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __SC8042_H__
#define __SC8042_H__

#include "includes.h"


#define VOIC_DELAY_CNT          100

#define POWER_ON_SC()			GPIO_SetBits(HT_GPIOG,GPIO_Pin_8)
#define POWER_DOWN_SC()			GPIO_ResetBits(HT_GPIOG,GPIO_Pin_8)
#define ENABLE_SC_DATA()		GPIO_SetBits(HT_GPIOH,GPIO_Pin_1)
#define DISABLE_SC_DATA()		GPIO_ResetBits(HT_GPIOH,GPIO_Pin_1)

#define ENABLE_SC_RST()			GPIO_SetBits(HT_GPIOH,GPIO_Pin_0)
#define DISABLE_SC_RST()		GPIO_ResetBits(HT_GPIOH,GPIO_Pin_0)

extern uint32_t voiceUndateTime;
extern uint8_t  powerOnFlag;

typedef enum{
	VOIC_NULL1=1,							//空
	VOIC_WELCOME=2,							//欢迎使用
	VOIC_NULL2=3,							//空
	VOIC_SHARE_CHARGE=4,					//共享充电
	VOIC_START_UPGRADE=5,                   //开始升级
	VOIC_READING_CARD=6,               		//正在读卡
	VOIC_DEVICE_EXCEPTION=7,				//设备故障
	VOIC_CARD_BANLANCE=8,					//卡片余额
	VOIC_CARD_RECHARGER=9,					//卡余额不足,请充值
	VOIC_CARD_INVALID=10,					//卡片无效
	VOIC_CARD_CHARGING=11,					//你的爱车正在充电
	VOIC_INPUT_SOCKET_NUM=12,				//请输入插座编号,按确认键
	VOIC_INPUT_CHARGRE_MONEY=13,			//请输入充电金额,按确认键.如需返回上一级,请按返回键
	VOIC_SOCKET_OCCUPID=14,					//插座被占用,请选用其他插座
	VOIC_SOCKET_ERROR=15,					//插座故障,请选用其他插座
	VOIC_SOCKET_NUM_INVALID=16,				//插座编号无效,请重新输入
	VOIC_NIN_YI_XUANZE=17,					//您已选择
	VOIC_HAO_CHAZUO=18,						//号插座
	VOIC_CHARGER_MONEY=19,					//充电金额
	VOIC_YUAN=20,							//元
	VOIC_VERIFIED_PLUG=21,					//请确认充电插座和车端插座均已插好
	VOIC_CARD_BANLANCE_INSUFFICIENT=22,		//余额不足,请重新输入
	VOIC_START_CHARGING=23,					//开始充电
	VOIC_STOP_CHARGER_TIP=24,				//如需结束充电,可拔掉插头
	VOIC_PLUG_IN_PLUG=25,					//请将插头插入
	VOIC_CARD=26,                          	//嘀（刷卡音）
	VOIC_KEY=27,                           	//噔（左右键按键音）
	VOIC_BLUETOOTH_ONLINE=28,				//蓝牙已连接
	VOIC_BLUETOOTH_OFFLINE=29,				//蓝牙已断开
	VOIC_ERROR=30,							//错误
	VOIC_0=31,                             	//0 (数字0)
    VOIC_1=32,                             	//1.
    VOIC_2=33,                             	//2.
    VOIC_3=34,                             	//3.
    VOIC_4=35,                             	//4.
    VOIC_5=36,                             	//5.
    VOIC_6=37,                             	//6.
    VOIC_7=38,                            	//7.
    VOIC_8=39,                             	//8.
    VOIC_9=40,                             	//9
	VOIC_TEST_TIP=41,						//1输桩号,2输识别码,3继电器测试
	VOIC_INPUT_SOCKET_TEST=42,				//输插座号按确定测试
	VOIC_HUNDRED=43,						//百
	VOIC_TEN=44,							//十
	VOIC_YUAN1=45,							//元
	VOIC_POINT=46,							//点
	VOIC_DEVICE_REBOOT=47,					//设备重启
	VOIC_SUCCESS=48,						//成功
	VOIC_A=49,								//a
	VOIC_B=50,								//b
	VOIC_C=51,								//c
	VOIC_D=52,								//d
	VOIC_E=53,								//e
	VOIC_F=54,								//f
	VOIC_TEM=55,							//温度
	VOIC_DEGREE=56,							//度
	VOIC_THOUSAND=57,						//千
	VOIC_START_CHARGER_FAIL=58,				//开启充电失败
	VOIC_POWER_TOO_LARGE=59,                //设备功率过大，无法充电
	PLEASE_CARD_AGAIN = 60,                 //请再次刷卡以开启充电
    INPUT_PIPLE_AND_ENTER = 61,             //请输入柜门或插座编号,并按确认键；
    CABINET_DOOR_OPEN = 62,                 //柜门已打开
    INPUT_BATTERY = 63,                     //请放入电池
    CHARGING_CLOSE_DOOR = 64,               //已开始充电，请关好柜门；
    CONFIRM_TO_END_CHARGING = 65,           //如需结束充电，请按确认键；
    CARD_TO_PAY_MONEY = 66,                 //因为您未按时取出电池，请您再次刷卡以扣除超时费用。
    TAKE_OUT_BATTERY_CLOSE_DOOR = 67,       //请取出电池并关好柜门
    CHECK_GUN_STATUS = 68,                  //号插座，充电器未连接，请确认
    REMAINDER_CHARGING_CNT = 69,            //卡片剩余充电次数
    NUMBER_OF_TIMES = 70,                   //次
	VOICE_NUM,
}SC8042B_VOICE_TYPE;

int Sc8042b_Init(void);
void Sc8042bSpeech(int index);
void OptFailNotice(int code);
void SpeechCardBalance(uint32_t balance);
void SpeechStartCharge(void);
void SpeechChargeing(uint8_t id);
void OptSuccessNotice(int code);
void TempNotice(uint16_t temp);
void CheckVoiceChipPower(void);
void SoundCode(int code);
extern void StartChargeSpeech(void);
void PointOutPullGun(uint8_t gun);
void PointOutNumber(uint16_t number);


#endif //__SC8042_H__


