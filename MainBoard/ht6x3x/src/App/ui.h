/*ui.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __UI_H__
#define __UI_H__

#include "includes.h"




typedef enum{
	UI_POWERON=0,					//上电
	UI_STANDBY,						//待机页
	UI_READING_CRAD,				//读卡中页面
	UI_READ_CARD_SUCCESS,			//读卡成功
	UI_VERIFY_CARD_FAILURE,			//卡片状态页面
	UI_CARD_CHARGING,				//该卡片有充电中订单
	UI_INPUT_SOCKET_NUM,			//输入插座编号页面
	UI_INPUT_CHARGER_MONEY,			//输入金额页面
	UI_SOCKET_OCCUPY,				//插座被占用页面
	UI_SOCKET_EXCEPTION,			//插座故障页面
	UI_SOCKET_NOT_EXSIT,			//插足编号不存在页面
	UI_START_CHARGING,				//开启充电中页面
	UI_START_PAYMENT_CARD_CHARGING,				//支付卡开启充电中界面
	UI_CARD_MONEY_INSUFFICIENT,		//卡余额不足页面
	UI_PAYMENT_CARD_MONEY_INSUFFICIENT,		//zfk???????
	UI_CHARGING,					//充电中页面
	UI_START_CHARGING_FAILURE,		//开启充电失败页面
	UI_OVER_POWER,					//功率过大
	UI_EQUIP_OFFLINE_EXCEPTION,		//设备离线或故障
	UI_EQUIP_UPGRADE,				//设备升级
	UI_ADMINISTRATOR_TEST,			//管理员测试模式页面
	UI_INPUT_CHARGER_SN,			//输入桩号页面
	UI_INPUT_DEVICE_ID,				//输入识别码页面
	UI_SET_PRINT_SWITCH,            //设置蓝牙mac地址
	UI_RELAY_TEST,					//继电器测试
	UI_EVENT_NOTICE_TEST,			//事件通知上报测试
	UI_VOICE_BROADCAST_TEST,		//语音播报测试
	UI_SHOW_SIGNAL,					//信号值显示
	UI_LCD_TEST,					//液晶屏测试
	UI_PRODUCT_TEST,				//生产测试
	UI_MAX,
}UI_STATE;

typedef struct key_values{
	char inputStr[16];
	int inputLen ;
}KEY_VALUES;


typedef struct{
	UI_STATE Ui_State;
	uint32_t uiStartTime;
}ui_info_t;




void clearKeyBuf(KEY_VALUES *key);
void putCharToKeyBuf(KEY_VALUES *key,uint8_t value);
void delCharFormKeyBuf(KEY_VALUES *key);
void UiClearKeyVal(void);
void UpdateNewUiState(UI_STATE state);
void UiStateLoop(void);
void UiDisplay_PosData(uint8_t pos, uint8_t data);
void UiDisplay_KeyBuf(KEY_VALUES *key);
void UiDisplay_Version(void);
void UiDisplay_SignalVal(int signalVal);
void UiDisplay_Time(uint8_t hour, uint8_t min, uint8_t sec);
void UiDisplay_MonthDay(uint8_t month, uint8_t day);
void UiClearTime(void);
void UiKeyHandle_AdministratorTestMode(uint8_t key);
void UiKeyHandle_InputChargerSn(uint8_t key);
void UiKeyHandle_InputDeviceId(uint8_t key);
void UiKeyHandle_RelayTest(uint8_t key);
void UiKeyHandle_InputSocketNum(uint8_t key);
void KeyToUiProc(uint8_t key);
void UiDisplay_ErrCode(int code);
void UiDisplay_CardMoney(int CardMoney);
void SwitchToUi_EquipOfflineOrException(void);
void SwitchToUi_EquipUpgrade(void);
void SwitchToUi_PowerOn(void);
void SwitchToUi_Standby(void);
void SwitchToUi_ReadingCard(void);
void SwitchToUI_ReadCardSuccess(void);
void SwitchToUI_CardCharging(void);
void SwitchToUi_VerifyCardFailure(uint8_t statu);
void SwitchToUi_InputSocketNum(void);
void SwitchToUi_InputChargerMoney(void);
void SwitchToUi_SocketOccupy(void);
void SwitchToUi_SocketException(void);
void SwitchToUi_SocketNumNotExsit(void);
void SwitchToUi_StartCharging(uint8_t switchFlag);
void SwitchToUi_BanlanceInsufficient(uint8_t switchFlag);
void SwitchToUi_Charging(uint8_t switchFlag);
void SwitchToUi_StartChargingFailure(void);
void SwitchToUi_OverPower(void);

void SwitchToUi_AdministratorTestMode(void);
void SwitchToUi_InputChargerSn(void);
void SwitchToUi_InputDeviceId(void);
void SwitchToUi_InputBtMacAddr(void);
void SwitchToUi_RelayTest(void);
void SwitchToUi_ProductTest(void);
extern UI_STATE UIGetState(void);
extern void SwitchToUI_ShowBalance(uint32_t user_card_balance);
extern void SwitchToUi_PaymentCardBanlanceInsufficient(uint8_t switchFlag);

extern void Ui_InputSocketNum(void);
extern void Ui_SocketNumNotExsit(void);
extern void SwitchToUi_SocketOccupy(void);
extern void Ui_SocketOccupy(void);
extern void Ui_SocketException(void);
extern void Ui_InputChargerMoney(void);
extern void Ui_StartChargingFailure(void);
void SetPrintSwitch(uint8_t para);


#endif //__UI_H__


