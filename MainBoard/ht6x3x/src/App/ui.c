/*ui.c
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "ui.h"
#include "lcd.h"
#include "card.h"
#include "rtc.h"
#include "sim800c.h"
#include "sc8042.h"
#include "flash.h"
#include "gun.h"
#include "server.h"
#include "emu.h"

ui_info_t ui_Info;
uint32_t ProgressloopCnt = 0;
KEY_VALUES keyVal;
const uint8_t TestCode[4] = {'2','0','1','8'};
extern uint8_t readCardFlg;



//清空键盘接收缓存区
void clearKeyBuf(KEY_VALUES *key)
{
	memset(key,0,sizeof(KEY_VALUES));
	key->inputLen = 0;
}


//写入一个键值
void putCharToKeyBuf(KEY_VALUES *key,uint8_t value)
{
	if (key->inputLen < 16)
			sprintf(&key->inputStr[key->inputLen++],"%d",value);
}


//往后删减一个键值
void delCharFormKeyBuf(KEY_VALUES *key)
{
	if(key->inputLen >0)  {
		key->inputLen--;
		key->inputStr[key->inputLen] = '\0';
	}
}


//清除显示的按键值
void UiClearKeyVal(void)
{
	//清除显示的按键值
	for(uint8_t i=7; i<11; i++) {
		LcdClrData((DATA_POS_t)i);
	}

	for(uint8_t i=1; i<4; i++) {
		LcdDisplayPoint((DATA_POS_t)i, LCD_CLEAR);
	}
}


//lcd相对位置显示数字
void UiDisplay_PosData(uint8_t pos, uint8_t data)
{
    void (*afun[])(DATA_POS_t) = {LcdDisplayZero,LcdDisplayOne,LcdDisplayTwo,LcdDisplayThree,LcdDisplayFour,LcdDisplayFive,LcdDisplaySix,LcdDisplaySeven,LcdDisplayEight,LcdDisplayNine};

	if ((data>9) || (pos>11) || (pos<1)) {
		return;
	}
    afun[data]((DATA_POS_t)pos);
}


//桩号与识别码显示
void UiDisplay_KeyBuf(KEY_VALUES *key)
{
	uint8_t pos = 10;
	uint8_t tmp = 0;

	UiClearKeyVal();
	if(key->inputLen == 0) return;

	if(key->inputLen < 4){
		for(int i=0; i<key->inputLen; i++) {
			tmp = key->inputStr[key->inputLen-i-1] - 0x30;
			UiDisplay_PosData(pos, tmp);
			pos--;
		}
	} else if(key->inputLen >= 4){
		for(int i=key->inputLen; i>(key->inputLen-4); i--) {
			tmp = key->inputStr[i-1] - 0x30;
			UiDisplay_PosData(pos, tmp);
			pos--;
		}
	}
}


//插座编号显示
void UiDisplay_SocketNum(KEY_VALUES *key)
{
	uint8_t tmp = 0;

	UiClearKeyVal();
	if(key->inputLen == 0) return;

	if(key->inputLen == 1) {
		tmp = key->inputStr[0] - 0x30;
		UiDisplay_PosData(10, tmp);
		UiDisplay_PosData(9, 0);
	} else if(key->inputLen == 2) {
		tmp = key->inputStr[0] - 0x30;
		UiDisplay_PosData(9, tmp);
		tmp = key->inputStr[1] - 0x30;
		UiDisplay_PosData(10, tmp);
	}
}


//lcd显示版本号
void UiDisplay_Version(void)
{
	uint16_t tmp = 0;
	UiClearKeyVal();
	tmp = (uint16_t)FW_VERSION/100;
	UiDisplay_PosData(7, tmp);
	tmp = ((uint16_t)FW_VERSION%100)/10;
	UiDisplay_PosData(8, tmp);
	tmp = ((uint16_t)FW_VERSION%100)%10;
	UiDisplay_PosData(9, tmp);
    #if (0 != FW_VERSION_SUB)
    LcdDisplayPoint((DATA_POS_t)3, LCD_DISPLAY);
    UiDisplay_PosData(10, FW_VERSION_SUB);
    #endif
    if (OUT_485_NET == system_info.netType) {
        LcdDisplayPoint((DATA_POS_t)1, LCD_DISPLAY);
    }
}

//lcd显示信号值
void UiDisplay_SignalVal(int signalVal)
{
	uint8_t tmp = 0;
	tmp = signalVal/10;
	UiDisplay_PosData(5, tmp);
	tmp = signalVal%10;
	UiDisplay_PosData(6, tmp);
}


//lcd显示时间
void UiDisplay_Time(uint8_t hour, uint8_t min, uint8_t sec)
{
	uint8_t tmp = 0;

	tmp = hour/10;
	UiDisplay_PosData(1, tmp);
	tmp = hour%10;
	UiDisplay_PosData(2, tmp);
	tmp = min/10;
	UiDisplay_PosData(3, tmp);
	tmp = min%10;
	UiDisplay_PosData(4, tmp);
	tmp = sec/10;
	UiDisplay_PosData(5, tmp);
	tmp = sec%10;
	UiDisplay_PosData(6, tmp);
	LcdDisplayCol((DATA_POS_t)1, LCD_DISPLAY);
	LcdDisplayCol((DATA_POS_t)2, LCD_DISPLAY);
}

//显示月日
void UiDisplay_MonthDay(uint8_t month, uint8_t day)
{
	uint8_t tmp = 0;

	tmp = month/10;
	UiDisplay_PosData(7, tmp);
	tmp = month%10;
	UiDisplay_PosData(8, tmp);
	tmp = day/10;
	UiDisplay_PosData(9, tmp);
	tmp = day%10;
	UiDisplay_PosData(10, tmp);
	LcdDisplayPoint((DATA_POS_t)2, LCD_DISPLAY);
}

//显示故障码
void UiDisplay_ErrCode(int code)
{
	uint8_t tmp = 0;

	tmp = code/100;
	UiDisplay_PosData(8, tmp);
	tmp = code%100/10;
	UiDisplay_PosData(9, tmp);
	tmp = code%10;
	UiDisplay_PosData(10, tmp);
}

//lcd清除时间
//void UiClearTime(void)
//{
//	for (uint8_t i=1; i++; i<7) {
//		LcdClrData((DATA_POS_t)i);
//	}
//	LcdDisplayCol((DATA_POS_t)1, LCD_CLEAR);
//	LcdDisplayCol((DATA_POS_t)2, LCD_CLEAR);
//}


//显示卡片余额
void UiDisplay_CardMoney(int CardMoney)
{
	uint8_t tmp = 0;

	tmp = CardMoney / 10000;
	LcdDisplayPoint((DATA_POS_t)3, LCD_DISPLAY);
	if (tmp != 0) {
		UiDisplay_PosData(7, tmp);
		tmp = (CardMoney/1000)%10;
		UiDisplay_PosData(8, tmp);
		tmp = (CardMoney/100)%10;
		UiDisplay_PosData(9, tmp);
		tmp = (CardMoney/10)%10;
		UiDisplay_PosData(10, tmp);
	} else {
		tmp = CardMoney/1000;
		if (tmp != 0) {
			UiDisplay_PosData(8, tmp);
			tmp = (CardMoney/100)%10;
			UiDisplay_PosData(9, tmp);
			tmp = (CardMoney/10)%10;
			UiDisplay_PosData(10, tmp);
		} else {
			tmp = CardMoney/100;
			UiDisplay_PosData(9, tmp);
			tmp = (CardMoney/10)%10;
			UiDisplay_PosData(10, tmp);
		}
	}
}

//显示剩余时长
void UiDisplay_RemainTime(uint8_t gunId)
{
	uint32_t currentCount = 0;
	uint32_t chargerTime = 0;
	uint32_t remainTime = 0;
	uint8_t tmp=0;
	uint8_t hour = 0;
	uint8_t min = 0;
	uint8_t sec = 0;

	gun_info_t *pGunInfo = &gun_info[gunId-1];

	currentCount = GetRtcCount();
	if (currentCount > pGunInfo->start_time) {
		chargerTime = currentCount - pGunInfo->start_time;
	}

	if (pGunInfo->charger_time_plan*60>=chargerTime) {
		remainTime = pGunInfo->charger_time_plan*60 - chargerTime;
	}

	hour = remainTime/3600;
	min  = (remainTime-hour*3600)/60;
	sec  = remainTime-hour*3600-min*60;

	tmp = hour/10;
	UiDisplay_PosData(1, tmp);
	tmp = hour%10;
	UiDisplay_PosData(2, tmp);
	tmp = min/10;
	UiDisplay_PosData(3, tmp);
	tmp = min%10;
	UiDisplay_PosData(4, tmp);
	tmp = sec/10;
	UiDisplay_PosData(5, tmp);
	tmp = sec%10;
	UiDisplay_PosData(6, tmp);
	LcdDisplayCol((DATA_POS_t)1, LCD_DISPLAY);
	LcdDisplayCol((DATA_POS_t)2, LCD_DISPLAY);
}


//进入新的UI状态并且更新信息
void UpdateNewUiState(UI_STATE state)
{
	ui_Info.Ui_State = state;
	ui_Info.uiStartTime = GetRtcCount();
	CL_LOG("Ui_State = %d, uiStartTime = %d.\n", ui_Info.Ui_State, ui_Info.uiStartTime);
}

void UiDisplayNomalInfo(void)
{
	LcdAllOff();
	 if (system_info.isRecvStartUpAck) {
		LcdDisplayBackStageConnect(LCD_DISPLAY);
    }
	if (system_info.is_socket_0_ok) {
		LcdDisplaySingnal(LCD_DISPLAY);
	} else {
		LcdDisplayNoSingnal(LCD_DISPLAY);
	}
	LcdDisplayClockDial(LCD_DISPLAY);
}

void UiDisplayCurrentSocket(void)
{
	uint8_t tmp = gChgInfo.current_usr_gun_id/10;
	UiDisplay_PosData(9, tmp);
	tmp = gChgInfo.current_usr_gun_id%10;
	UiDisplay_PosData(10, tmp);
}

//上电开机页
void SwitchToUi_PowerOn(void)
{
	LcdAllOff();
	LcdDisplayNoSingnal(LCD_DISPLAY);
	LcdDisplayChgMethod(LCD_DISPLAY);
	ProgressloopCnt = 0;
	UpdateNewUiState(UI_POWERON);
}

//待机页面
void SwitchToUi_Standby(void)
{
	if (system_info.is_socket_0_ok == CL_TRUE) {
		UiDisplayNomalInfo();
	} else {
		LcdAllOff();
		LcdDisplayNoSingnal(LCD_DISPLAY);
		LcdDisplayClockDial(LCD_DISPLAY);
	}
	LcdDisplayChgMethod(LCD_DISPLAY);
	UpdateNewUiState(UI_STANDBY);
}


//读取卡片中页面
void SwitchToUi_ReadingCard(void)
{
	UiDisplayNomalInfo();
	ProgressloopCnt = 0;
	UpdateNewUiState(UI_READING_CRAD);
}

//读卡成功界面
void SwitchToUI_ReadCardSuccess(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplayCardBalance(LCD_DISPLAY);
	LcdDisplayYuan(LCD_DISPLAY);
	//显示卡片余额(分)
	UiDisplay_CardMoney(gChgInfo.user_card_balance);
	UpdateNewUiState(UI_READ_CARD_SUCCESS);
}

/*****************************************************************************
** Function name:       UIGetState
** Descriptions:        //get当前UI界面
** input parameters:    None
** output parameters:   None
** Returned value:	    UI界面
** Author:              quqian
*****************************************************************************/
UI_STATE UIGetState(void)
{
	return ui_Info.Ui_State;
}

/*****************************************************************************
** Function name:       SwitchToUI_ShowBalance
** Descriptions:        /显示余额界面
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void SwitchToUI_ShowBalance(uint32_t user_card_balance)
{
    LcdAllOff();
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplayCardBalance(LCD_DISPLAY);
	LcdDisplayYuan(LCD_DISPLAY);
	//显示卡片余额(分)
	UiDisplay_CardMoney(user_card_balance);
	UpdateNewUiState(UI_READ_CARD_SUCCESS);
}

/*****************************************************************************
** Function name:       UiKeyHandle_PaymentCardInputSocketNum
** Descriptions:        //支付卡输入插座编号按键处理
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void UiKeyHandle_PaymentCardInputSocketNum(uint8_t key)
{
	uint8_t gunId = 0;
	if(key <= 9)
    {
		if(keyVal.inputLen == 2)
        {
            return;
        }
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_SocketNum(&keyVal);
	}
    else if(10 == key)  //返回
    {
		if(keyVal.inputLen == 0)
        {
			SwitchToUi_Standby();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_SocketNum(&keyVal);
	}
    else if(11 == key)  //确认
    {
		if((keyVal.inputLen > 2) && (keyVal.inputLen == 0)) return;
		if(keyVal.inputLen == 1)
        {
			gunId = keyVal.inputStr[0]-0x30;
		}
        else if(keyVal.inputLen == 2)
        {
			gunId = (keyVal.inputStr[0]-0x30)*10 + (keyVal.inputStr[1]-0x30);
		}

		//当前输入的插座编号
		gChgInfo.current_usr_gun_id = gunId;
		if ((gunId == 0) || (gunId > GUN_NUM_MAX))
        {
			Sc8042bSpeech(VOIC_SOCKET_NUM_INVALID);
            SwitchToUi_SocketNumNotExsit();
		}
        else {
			gun_info_t *pGunInfo = &gun_info[gunId-1];

            //CL_LOG("pGunInfo.is_load_on = %d.\n", pGunInfo->is_load_on);
			if (pGunInfo->is_load_on)
            {
				Sc8042bSpeech(VOIC_SOCKET_OCCUPID);
                //SwitchToUi_PaymentCardSocketOccupy();
                SwitchToUi_SocketOccupy();
			}
            else {
				if (CL_OK == CheckGunStatu(gChgInfo.current_usr_gun_id))
                {
					//输入插座编号后按确认键进入选择充电金额
                    SwitchToUi_InputChargerMoney();
					clearKeyBuf(&keyVal);
					Sc8042bSpeech(VOIC_INPUT_CHARGRE_MONEY);
					OS_DELAY_MS(500);
				}else{
                    Sc8042bSpeech(VOIC_SOCKET_ERROR);
                    SwitchToUi_SocketException();
                }
			}
		}
	}
}


/*****************************************************************************
** Function name:       SwitchToUi_PaymentCardBanlanceInsufficient
** Descriptions:        //支付卡余额不足页面(输入金额后,金额大于卡余额)
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void SwitchToUi_PaymentCardBanlanceInsufficient(uint8_t switchFlag)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplayYuan(LCD_DISPLAY);
	if (switchFlag == 0) {
		LcdDisplayChargeingMoney(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.money*100);
		UpdateNewUiState(UI_PAYMENT_CARD_MONEY_INSUFFICIENT);
	}
    else {
		LcdDisplayCardBalance(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.PaymentCardMoney);
	}
}

/*****************************************************************************
** Function name:       PaymentCardSpeechStartCharge
** Descriptions:        //您已选择X号插座，充值金额X元，请再次刷卡以开启充电
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void PaymentCardSpeechStartCharge(void)
{
	StartChargeSpeech();
	Sc8042bSpeech(PLEASE_CARD_AGAIN);
	vTaskDelay(2000);
}
/*****************************************************************************
** Function name:       SwitchToUi_PaymentCardStartCharging
** Descriptions:        //支付卡输入开启充电中页面
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void SwitchToUi_PaymentCardStartCharging(uint8_t switchFlag)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	ProgressloopCnt = 0;
	if (switchFlag == 0) {
		LcdDisplayNum(LCD_DISPLAY);
		LcdDisplaySocketNum(LCD_DISPLAY);
		UiDisplayCurrentSocket();
		UpdateNewUiState(UI_START_PAYMENT_CARD_CHARGING);
	} else {
		LcdDisplayYuan(LCD_DISPLAY);
		LcdDisplayChargeingMoney(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.money * 100);
	}
}

/*****************************************************************************
** Function name:       UiKeyHandle_PaymentCardInputChargerMoney
** Descriptions:        //支付卡输入充电金额按键处理
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void UiKeyHandle_PaymentCardInputChargerMoney(uint8_t key)
{
	uint8_t money = 0;
    uint8_t tmp = 0;

	if(key <= 9)
    {
		if(keyVal.inputLen == 1)
        {
            return;
        }
		putCharToKeyBuf(&keyVal, key);
		tmp = keyVal.inputStr[0] - 0x30;
		UiDisplay_PosData(10, tmp);
	}
    else if(10 == key)  //返回
    {
		if(keyVal.inputLen == 0)
        {
			Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
            SwitchToUi_InputSocketNum();
			clearKeyBuf(&keyVal);
			return;
		}
		delCharFormKeyBuf(&keyVal);
		uint8_t tmp = keyVal.inputStr[0];
		UiDisplay_PosData(10, tmp);

	}
    else if(11 == key)  //确认
    {
		if(keyVal.inputLen != 1) return;
		if(keyVal.inputLen == 1){
			money = keyVal.inputStr[0]-0x30;
		}

		if (money == 0)
        {
			CL_LOG("money=%d,err.\n", money);
			return;
		}
		//当前输入的充电金额
		gChgInfo.money = money;
		CL_LOG("m=%d元.\n", gChgInfo.money);
        CL_LOG("cm=%d分.\n", gChgInfo.PaymentCardMoney);

		if (gChgInfo.money*100 > gChgInfo.PaymentCardMoney)
        {
			//卡余额不足页面
            SwitchToUi_PaymentCardBanlanceInsufficient(0);
			Sc8042bSpeech(VOIC_CARD_BANLANCE_INSUFFICIENT);
		}
        else {
            gChgInfo.PayCardReadFlag = 5;
			//您已选择X号插座，充值金额X元，请再次刷卡以开启充电
            PaymentCardSpeechStartCharge();
            SwitchToUi_PaymentCardStartCharging(0);
		}
	}
}

#if 0
//此卡正在充电界面
void SwitchToUI_CardCharging(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplayChargeing(LCD_DISPLAY);
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);
	LcdDisplayRemainingTime(LCD_DISPLAY);
	//显示插座编号, 剩余时长
	UiDisplayCurrentSocket();
	UiDisplay_RemainTime(gChgInfo.current_usr_gun_id);
	ProgressloopCnt = 0;
	UpdateNewUiState(UI_CARD_CHARGING);
}
#endif

//卡鉴权失败页面
void SwitchToUi_VerifyCardFailure(uint8_t statu)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	if (1 == statu) {//卡余额不足
		LcdDisplayYuan(LCD_DISPLAY);
		LcdDisplayCardBalance(LCD_DISPLAY);//卡片金额
		//显示卡片余额(分)
		UiDisplay_CardMoney(gChgInfo.user_card_balance);
	}else if (2 == statu) {//充电卡不能再本机中使用
		LcdDisplayInvalid(LCD_DISPLAY);//无效
	}
	UpdateNewUiState(UI_VERIFY_CARD_FAILURE);
}


void Ui_InputSocketNum(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);
}

//输入插座编号页面
void SwitchToUi_InputSocketNum(void)
{
	Ui_InputSocketNum();
	UpdateNewUiState(UI_INPUT_SOCKET_NUM);
}

void Ui_InputChargerMoney(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplayYuan(LCD_DISPLAY);
	LcdDisplayChargeingMoney(LCD_DISPLAY);
}

//输入金额页面
void SwitchToUi_InputChargerMoney(void)
{
	Ui_InputChargerMoney();
	UpdateNewUiState(UI_INPUT_CHARGER_MONEY);
}

void Ui_SocketOccupy(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);

	LcdDisplaySocketOccupied(LCD_DISPLAY);
	UiDisplayCurrentSocket();
}

//插座被占用页面
void SwitchToUi_SocketOccupy(void)
{
	Ui_SocketOccupy();
	UpdateNewUiState(UI_SOCKET_OCCUPY);
}

void Ui_SocketException(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);

	LcdDisplaySocketFault(LCD_DISPLAY);
	UiDisplayCurrentSocket();
}

//插座异常页面
void SwitchToUi_SocketException(void)
{
	Ui_SocketException();
	UpdateNewUiState(UI_SOCKET_EXCEPTION);
}

void Ui_SocketNumNotExsit(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);

	LcdDisplayInvalid(LCD_DISPLAY);
	UiDisplayCurrentSocket();
}

//插座编号不存在页面
void SwitchToUi_SocketNumNotExsit(void)
{
	Ui_SocketNumNotExsit();
	UpdateNewUiState(UI_SOCKET_NOT_EXSIT);
}


//开启充电中页面
void SwitchToUi_StartCharging(uint8_t switchFlag)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	ProgressloopCnt = 0;
	if (switchFlag == 0) {
		LcdDisplayNum(LCD_DISPLAY);
		LcdDisplaySocketNum(LCD_DISPLAY);
		UiDisplayCurrentSocket();
		UpdateNewUiState(UI_START_CHARGING);
	} else {
		LcdDisplayYuan(LCD_DISPLAY);
		LcdDisplayChargeingMoney(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.money*100);
	}
}

//余额不足页面(输入金额后,金额大于卡余额)
void SwitchToUi_BanlanceInsufficient(uint8_t switchFlag)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplayYuan(LCD_DISPLAY);
	if (switchFlag == 0) {
		LcdDisplayChargeingMoney(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.money*100);
		UpdateNewUiState(UI_CARD_MONEY_INSUFFICIENT);
	} else {
		LcdDisplayCardBalance(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.user_card_balance);
	}
}

//充电中页面
void SwitchToUi_Charging(uint8_t switchFlag)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySuccess(LCD_DISPLAY);
	LcdDisplayChargeing(LCD_DISPLAY);
	ProgressloopCnt = 0;
	if (0 == switchFlag) {
		LcdDisplayNum(LCD_DISPLAY);
		LcdDisplaySocketNum(LCD_DISPLAY);
		UiDisplayCurrentSocket();
		UpdateNewUiState(UI_CHARGING);
	} else {
		LcdDisplayYuan(LCD_DISPLAY);
		LcdDisplayChargeingMoney(LCD_DISPLAY);
		UiDisplay_CardMoney(gChgInfo.money*100);
	}
}

void Ui_StartChargingFailure(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);
	LcdDisplayFailure(LCD_DISPLAY);
	UiDisplayCurrentSocket();
}

//开启充电失败页面
void SwitchToUi_StartChargingFailure(void)
{
	Ui_StartChargingFailure();
	UpdateNewUiState(UI_START_CHARGING_FAILURE);
}

#if 0
//功率过大页面
void SwitchToUi_OverPower(void)
{
	UiDisplayNomalInfo();
	LcdDisplayAllEdge();
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);
	LcdDisplayFailure(LCD_DISPLAY);
	UiDisplayCurrentSocket();
	UpdateNewUiState(UI_OVER_POWER);
}
#endif

//离线或故障页面
void SwitchToUi_EquipOfflineOrException(void)
{
	LcdAllOff();
	LcdDisplayNoSingnal(LCD_DISPLAY);
	LcdDisplaySocketFault(LCD_DISPLAY);
	UpdateNewUiState(UI_EQUIP_OFFLINE_EXCEPTION);
}

//设备升级中页面
void SwitchToUi_EquipUpgrade(void)
{
	UiDisplayNomalInfo();
	UpdateNewUiState(UI_EQUIP_UPGRADE);
}


//管理员测试模式页面
void SwitchToUi_AdministratorTestMode(void)
{
	LcdAllOff();
	 if (system_info.isRecvStartUpAck) {
		LcdDisplayBackStageConnect(LCD_DISPLAY);
    }
	if (system_info.is_socket_0_ok) {
		LcdDisplaySingnal(LCD_DISPLAY);
	} else {
		LcdDisplayNoSingnal(LCD_DISPLAY);
	}

	UiDisplay_Version();
    //显示485拨码值
    UiDisplay_PosData(1, GetDialValue());
	int signalVal = GetNetSignal();
	UiDisplay_SignalVal(signalVal);
	UpdateNewUiState(UI_ADMINISTRATOR_TEST);
	Sc8042bSpeech(VOIC_TEST_TIP);
}


//输入桩号SN页面
void SwitchToUi_InputChargerSn(void)
{
	uint8_t tmp = 0;
	LcdAllOff();
	LcdDisplayBackStageConnect(LCD_DISPLAY);
	LcdDisplaySingnal(LCD_DISPLAY);
	for (uint8_t i=0; i<5; i++) {
		tmp = system_info.station_id[i+3]/16;
		UiDisplay_PosData(2*i+1, tmp);
		tmp = system_info.station_id[i+3]%16;
		UiDisplay_PosData(2*i+2, tmp);
	}
	UpdateNewUiState(UI_INPUT_CHARGER_SN);
}


//输入识别码页面
void SwitchToUi_InputDeviceId(void)
{
	LcdAllOff();
	LcdDisplayBackStageConnect(LCD_DISPLAY);
	LcdDisplaySingnal(LCD_DISPLAY);
	UpdateNewUiState(UI_INPUT_DEVICE_ID);
}


//生产测试页面
void SwitchToUi_ProductTest(void)
{
	LcdAllOff();
	UpdateNewUiState(UI_PRODUCT_TEST);
}


//设置打印开关
void SwitchToUi_SetPrintSwitch(void)
{
	LcdAllOff();
	LcdDisplayBackStageConnect(LCD_DISPLAY);
	LcdDisplaySingnal(LCD_DISPLAY);
	UpdateNewUiState(UI_SET_PRINT_SWITCH);
}


//继电器测试页面
void SwitchToUi_RelayTest(void)
{
	LcdAllOff();
	LcdDisplayBackStageConnect(LCD_DISPLAY);
	LcdDisplaySingnal(LCD_DISPLAY);
	UpdateNewUiState(UI_RELAY_TEST);
}

//事件通知界面测试
void SwitchToUi_EventNoticeTest(void)
{
	LcdAllOff();
	LcdDisplayBackStageConnect(LCD_DISPLAY);
	LcdDisplaySingnal(LCD_DISPLAY);
	LcdDisplayEdge(LCD_DISPLAY);
	LcdDisplayTab2Edge(LCD_DISPLAY);
	LcdDisplaySocketNum(LCD_DISPLAY);
	LcdDisplayNum(LCD_DISPLAY);
	UpdateNewUiState(UI_EVENT_NOTICE_TEST);
}


int ceil10(uint8_t n)
{
	int sum = 1;
	while(n--){
		sum *= 10;
	}
	return sum;
}

//判断是否紧急码
int isEmergencyCode(char *inputStr,int inputLen)
{
	int time ;
	int generaterHour=0;  //应急码生成时间
	int currHour=0;			//当前时间后4为
	char tmp;
	int limateTime;
	int i = 0;
	uint8_t a[6];

	if(inputLen != 7) {
		CL_LOG("len err:%d.\n",inputLen);
		return CL_FAIL;
	}
	//校验码判断
	for(i = 0;i<6;i++){
		a[i] = inputStr[i] - 0x30;
	}
	uint8_t check = (a[0]*a[2]+a[1]*a[3]+a[4]*a[5] + 5) % 10;
	CL_LOG("EmergencyCode check=%d\n",check);
	if(check != (inputStr[6]-0x30)){
		CL_LOG("check err:%d.\n",check);
		return CL_FAIL;
	}

	for(i = 0;i<4;i++){
		tmp = inputStr[i] - 0x30;
		generaterHour += tmp*ceil10(3-i);
	}

	limateTime = (inputStr[4] - 0x30)*10+(inputStr[5] - 0x30);
	limateTime = limateTime*10;
	if(limateTime > 720 || limateTime <=0 ){
		CL_LOG("err time=%dm.\n",limateTime);
		return CL_FAIL;
	}

	time = GetRtcCount()/3600;
	for(i = 0;i<4;i++){
		tmp = time%10;
		currHour += tmp*ceil10(i);
		time = time/10;
	}

	//应急码超出有效时间
	if(currHour > (generaterHour+8)) {
		CL_LOG("time valid:%d %d.\n",generaterHour,currHour);
		return CL_FAIL;
	}
	return limateTime;
}


//待机按键监听处理
void UiKeyHandle_Standby(uint8_t key)
{
	if(key <= 9) {
		putCharToKeyBuf(&keyVal, key);
        UiDisplay_KeyBuf(&keyVal);
        gChgInfo.lastInputTime = GetRtcCount();
        gChgInfo.inputCode = 1;
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
            gChgInfo.inputCode = 0;
			return;
		}
        gChgInfo.lastInputTime = GetRtcCount();
        delCharFormKeyBuf(&keyVal);
        if (keyVal.inputLen == 0) {
            gChgInfo.inputCode = 0;
		}
		UiDisplay_KeyBuf(&keyVal);
	} else if(11 == key) {//确认
		int limateTime;
		if (keyVal.inputLen == 4) {
			if (memcmp(&keyVal.inputStr, TestCode, 4) == 0) {
				SwitchToUi_AdministratorTestMode();
				//CL_LOG("input test code right, enter test mode by input test code.\n");
			}
		}else if ((limateTime = isEmergencyCode(keyVal.inputStr,keyVal.inputLen)) != CL_FAIL) {
			CL_LOG("code err,time=%d.\n",limateTime*60);
			TestOpenGunProc(0, limateTime*60, INPUT_CODE_OPEN);
		}
        gChgInfo.inputCode = 0;
        clearKeyBuf(&keyVal);
	}
}


//刷卡进入测试模式按键处理
void UiKeyHandle_AdministratorTestMode(uint8_t key)
{
    //const uint8_t voiceDelay[] = {0,1,1,1,1,1,1,1,2,1,
    //							  2,3,6,4,4,4,1,1,1,1,
    //							  4,2,1,2,2,1,1,1,1,1,
    //							  1,1,1,1,1,1,1,1,1,1,
    //							  4,3,1,1,1,1,2,1,1,1,
    //							  1,1,1,1,1,1,1,2,3,2,
    //							  };

	UiClearKeyVal();
	if(1 == key) {
		keyVal.inputStr[0] = 1;
		LcdDisplayOne(TEN);
	} else if (2 == key) {
		keyVal.inputStr[0] = 2;
		LcdDisplayTwo(TEN);
	} else if (3 == key) {
		keyVal.inputStr[0] = 3;
		LcdDisplayThree(TEN);
	} else if (4 == key) {
		keyVal.inputStr[0] = 4;
		LcdDisplayFour(TEN);
	} else if (5 == key) {
		keyVal.inputStr[0] = 5;
		LcdDisplayFive(TEN);
	} else if(6 == key) {
		keyVal.inputStr[0] = 6;
		LcdDisplaySix(TEN);
	}else if(7 == key) {
		keyVal.inputStr[0] = 7;
		LcdDisplaySeven(TEN);
	}else if(8 == key) {
		keyVal.inputStr[0] = 8;
		LcdDisplayEight(TEN);
	}else if(10 == key) {//返回键
		clearKeyBuf(&keyVal);
		SwitchToUi_Standby();
	} else if(11 == key) {//确认键
		if(keyVal.inputStr[0] == 1){//输入桩号
			SwitchToUi_InputChargerSn();
		} else if(keyVal.inputStr[0] == 2) {//输入识别码
			SwitchToUi_InputDeviceId();
		} else if(keyVal.inputStr[0] == 3) {//继电器测试
			Sc8042bSpeech(VOIC_INPUT_SOCKET_TEST);
			SwitchToUi_RelayTest();
		} else if(keyVal.inputStr[0] == 4) {//播报语音
			//for(int i=1; i<VOICE_NUM; i++) {
			//	Sc8042bSpeech(i);
			//	vTaskDelay(voiceDelay[i-1]*1000);
			//}
		} else if (keyVal.inputStr[0] == 5) {//液晶屏测试
            LcdDisplayAll();
        } else if (keyVal.inputStr[0] == 6) {//发送事件通知测试
			SwitchToUi_EventNoticeTest();
		} else if (keyVal.inputStr[0] == 7) {//设置打印开关
			SwitchToUi_SetPrintSwitch();
		} else if (keyVal.inputStr[0] == 8) {//重启
			ResetSysTem();
		}
		clearKeyBuf(&keyVal);
	}
}


//输入桩号监听处理
void UiKeyHandle_InputChargerSn(uint8_t key)
{
    uint8_t tmp[10] = {0};

	if(key <= 9) {
		if(keyVal.inputLen == 10) return;
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_KeyBuf(&keyVal);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			SwitchToUi_AdministratorTestMode();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_KeyBuf(&keyVal);
	} else if (11 == key) {//确认
		if (keyVal.inputLen != 10) return;
		memcpy((void*)tmp, keyVal.inputStr, 10);
		//保存桩号
		memset(gun_info, 0, sizeof(gun_info));
		FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
		memset(system_info.station_id, 0, sizeof(system_info.station_id));
		StrToHex(&system_info.station_id[3], (void*)tmp, 10);
        WriteCfgInfo(CFG_CHARGER_SN, CHARGER_SN_LEN, system_info.station_id);
		PrintfData("station_sn:", (void*)system_info.station_id, 8);
        memset(system_info.idCode, 0, sizeof(system_info.idCode));
        WriteCfgInfo(CFG_CHARGER_DEVICEID, CHARGER_SN_LEN, system_info.idCode);
		UpdataGunDataSum();

        EmuCalation.magic_number = 0;
        FlashWritMatrix((void*)&EmuCalation,sizeof(EMU_CAL));
        Clear_RecordOrder();
		ResetSysTem();
	}
}


//输入识别码监听处理
void UiKeyHandle_InputDeviceId(uint8_t key)
{
	if(key <= 9) {
		if(keyVal.inputLen == 12) return;
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_KeyBuf(&keyVal);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			SwitchToUi_AdministratorTestMode();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_KeyBuf(&keyVal);
	} else if(11 == key) {//确认
		if (keyVal.inputLen != 12) return;
		//保存识别码
		memset(system_info.idCode, 0, sizeof(system_info.idCode));
		StrToHex(&system_info.idCode[2], keyVal.inputStr, 12);
        FlashWriteSysInfo((void*)&system_info, sizeof(system_info), 1);

        WriteCfgInfo(CFG_CHARGER_DEVICEID, CHARGER_SN_LEN, system_info.idCode);
		ResetSysTem();
	}
}


void SetPrintSwitch(uint8_t para)
{
    system_info.printSwitch = para;
    system_info.logOpenTime = GetRtcCount();
	FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
}


void UiKeyHandle_SetPrintSwitch(uint8_t key)
{
	if(key <= 9) {
		if(keyVal.inputLen == 1) return;
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_KeyBuf(&keyVal);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			SwitchToUi_AdministratorTestMode();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_KeyBuf(&keyVal);
	} else if(11 == key) {//确认
	    if (0 == keyVal.inputLen) {
            SwitchToUi_AdministratorTestMode();
			return;
        }else {
            SetPrintSwitch(keyVal.inputStr[0]-0x30);
            keyVal.inputLen = 0;
        }
	}
}


//继电器测试按键监听处理
void UiKeyHandle_RelayTest(uint8_t key)
{
	uint8_t gunId = 0;

	if(key <= 9) {
		if(keyVal.inputLen == 2) return;
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_KeyBuf(&keyVal);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			//TurnOffAllGun();
			SwitchToUi_AdministratorTestMode();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_KeyBuf(&keyVal);
	} else if(11 == key) {//确认
		if(keyVal.inputLen == 0) return;
		if(keyVal.inputLen == 1){
			gunId = keyVal.inputStr[0]-0x30;
		}else if(keyVal.inputLen == 2){
			gunId = (keyVal.inputStr[0]-0x30)*10 + (keyVal.inputStr[1]-0x30);
		}
		//CL_LOG("relayTest: input gunId = %d.\n", gunId);
		if ((gunId>=1) && (gunId<=12)) {
            CL_LOG("relay open gunid=%d.\n",gunId);
            OpenGunEmergency(gunId, 3*60, TEST_OPEN);
		} else if (gunId == 13) {
		    CL_LOG("relay open all gun.\n");
			TestOpenGunProc(0, 3*60, TEST_OPEN);
		} else {
			CL_LOG("gunId error\r\n");
		}
		clearKeyBuf(&keyVal);
		UiClearKeyVal();
	}
}


//事件通知测试按键监听
void UiKeyHandle_EventNoticeTest(uint8_t key)
{
	uint8_t eventCode = 0;

	if(key <= 9) {
		if(keyVal.inputLen == 2) return;
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_KeyBuf(&keyVal);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			SwitchToUi_AdministratorTestMode();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_KeyBuf(&keyVal);
	} else if(11 == key) {//确认
		if(keyVal.inputLen == 0) return;
		if(keyVal.inputLen == 1){
			eventCode = keyVal.inputStr[0]-0x30;
		}else if(keyVal.inputLen == 2){
			eventCode = (keyVal.inputStr[0]-0x30)*10 + (keyVal.inputStr[1]-0x30);
		}

		if((eventCode>=1)&&(eventCode<=18)) {
			//发送事件通知
			SendEventNotice(1, eventCode, 0, 0, EVENT_OCCUR, NULL); //默认发1号枪头
			CL_LOG("send event notice,code=%d,gun=%d.\n", eventCode,1);
		} else {
			CL_LOG("code=%d,err.\n", eventCode);
		}
		clearKeyBuf(&keyVal);
		UiClearKeyVal();
	}
}


//输入插座编号按键处理
void UiKeyHandle_InputSocketNum(uint8_t key)
{
	uint8_t gunId = 0;
	if(key <= 9) {
		if(keyVal.inputLen == 2) return;
		putCharToKeyBuf(&keyVal, key);
		UiDisplay_SocketNum(&keyVal);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			SwitchToUi_Standby();
			return;
		}
		delCharFormKeyBuf(&keyVal);
		UiDisplay_SocketNum(&keyVal);
	} else if(11 == key) {//确认
		if((keyVal.inputLen > 2) && (keyVal.inputLen == 0)) return;
		if(keyVal.inputLen == 1){
			gunId = keyVal.inputStr[0]-0x30;
		}else if(keyVal.inputLen == 2){
			gunId = (keyVal.inputStr[0]-0x30)*10 + (keyVal.inputStr[1]-0x30);
		}

		//当前输入的插座编号
		gChgInfo.current_usr_gun_id = gunId;
		if ((gunId == 0) || (gunId > GUN_NUM_MAX)) {
			Sc8042bSpeech(VOIC_SOCKET_NUM_INVALID);
			SwitchToUi_SocketNumNotExsit();
		} else {
			gun_info_t *pGunInfo = &gun_info[gunId-1];
			if (pGunInfo->is_load_on) {
				Sc8042bSpeech(VOIC_SOCKET_OCCUPID);
                if(0x11 == gChgInfo.isPayCard)
                {
                    //SwitchToUi_PaymentCardSocketOccupy();
                    SwitchToUi_SocketOccupy();
                }
                else if(0x12 == gChgInfo.isPayCard)
                {
                    SwitchToUi_SocketOccupy();
                }
			} else {
			    if(0x11 == gChgInfo.isPayCard) {
                    if (CL_OK == CheckGunStatu(gChgInfo.current_usr_gun_id))
                    {
    					//输入插座编号后按确认键进入选择充电金额
                        //SwitchToUi_PaymentCardInputChargerMoney();
                        SwitchToUi_InputChargerMoney();
    					clearKeyBuf(&keyVal);
    					Sc8042bSpeech(VOIC_INPUT_CHARGRE_MONEY);
    					OS_DELAY_MS(500);
    				}else{
                        Sc8042bSpeech(VOIC_SOCKET_ERROR);
                        //SwitchToUi_PaymentCardSocketException();
                        SwitchToUi_SocketException();
                    }
                }else if(0x12 == gChgInfo.isPayCard) {
                    if (CL_OK == CheckGunStatu(gChgInfo.current_usr_gun_id)) {
    					if (gChgInfo.mode == 0) { //智能充满
    					    if (1 < gChgInfo.sendAuthCnt) {
                                CL_LOG("authCnt=%d,error.\n",gChgInfo.sendAuthCnt);
    					    }else{
        						//发送鉴权信息(卡号,订单信息)
        						SendCardAuthReq(CARD_MONTH);
    					    }
    					} else {
    						//输入插座编号后按确认键进入选择充电金额
    						SwitchToUi_InputChargerMoney();
    						clearKeyBuf(&keyVal);
    						Sc8042bSpeech(VOIC_INPUT_CHARGRE_MONEY);
    						OS_DELAY_MS(500);
    					}
    				}else{
                        Sc8042bSpeech(VOIC_SOCKET_ERROR);
    		            SwitchToUi_SocketException();
                    }
                }
			}
		}
	}
}


//输入充电金额按键处理
//******************************************************************
void UiKeyHandle_InputChargerMoney(uint8_t key)
{
	uint8_t money = 0;

	if(key <= 9) {
		if(keyVal.inputLen == 1) return;
		putCharToKeyBuf(&keyVal, key);
		uint8_t tmp = keyVal.inputStr[0] - 0x30;
		UiDisplay_PosData(10, tmp);
	} else if(10 == key) {//返回
		if(keyVal.inputLen == 0) {
			Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
			SwitchToUi_InputSocketNum();
			clearKeyBuf(&keyVal);
			return;
		}
		delCharFormKeyBuf(&keyVal);
		uint8_t tmp = keyVal.inputStr[0];
		UiDisplay_PosData(10, tmp);

	} else if(11 == key) {//确认
		if(keyVal.inputLen != 1) return;
		if(keyVal.inputLen == 1){
			money = keyVal.inputStr[0]-0x30;
		}

		if (money == 0) {
			CL_LOG("money=0,err.\n");
			return;
		}

        if(0x11 == gChgInfo.isPayCard)
        {
            //当前输入的充电金额
    		gChgInfo.money = money;
    		//CL_LOG("当前输入金额 = %d 元.\n", gChgInfo.money);
            //CL_LOG("卡片金额 = %d 分.\n", gChgInfo.PaymentCardMoney);

    		if (gChgInfo.money*100 > gChgInfo.PaymentCardMoney)
            {
    			//卡余额不足页面
                SwitchToUi_PaymentCardBanlanceInsufficient(0);
    			Sc8042bSpeech(VOIC_CARD_BANLANCE_INSUFFICIENT);
    		}
            else {
                gChgInfo.PayCardReadFlag = 5;
    			//您已选择X号插座，充值金额X元，请再次刷卡以开启充电
                PaymentCardSpeechStartCharge();
                SwitchToUi_PaymentCardStartCharging(0);
                //UpdateNewUiState(UI_STANDBY);
    		}
        }else if(0x12 == gChgInfo.isPayCard) {
            //当前输入的充电金额
    		gChgInfo.money = money;
    		if ((gChgInfo.money*100) > gChgInfo.user_card_balance) {
    			//卡余额不足页面
    			SwitchToUi_BanlanceInsufficient(0);
    			Sc8042bSpeech(VOIC_CARD_BANLANCE_INSUFFICIENT);
    		} else {
    		    if (1 < gChgInfo.sendAuthCnt) {
                    CL_LOG("authCnt=%d,error.\n",gChgInfo.sendAuthCnt);
                }else{
                    gChgInfo.sendAuthCnt++;
                    //发送鉴权信息(卡号,订单信息)
        			SpeechStartCharge();
        			SendCardAuthReq(CARD_AUTH);
        			SwitchToUi_StartCharging(0);
                }
    		}
        }
	}
}

void UiProgressLoop(void)
{
	ProgressloopCnt++;
	if (ProgressloopCnt == 1) {
		LcdDisplayProgressBar1(LCD_DISPLAY);
		LcdDisplayProgressBar2(LCD_CLEAR);
		LcdDisplayProgressBar3(LCD_CLEAR);
		LcdDisplayProgressBar4(LCD_CLEAR);
	} else if (ProgressloopCnt == 2) {
		LcdDisplayProgressBar1(LCD_DISPLAY);
		LcdDisplayProgressBar2(LCD_DISPLAY);
		LcdDisplayProgressBar3(LCD_CLEAR);
		LcdDisplayProgressBar4(LCD_CLEAR);
	} else if (ProgressloopCnt == 3) {
		LcdDisplayProgressBar1(LCD_DISPLAY);
		LcdDisplayProgressBar2(LCD_DISPLAY);
		LcdDisplayProgressBar3(LCD_DISPLAY);
		LcdDisplayProgressBar4(LCD_CLEAR);
	} else if (ProgressloopCnt == 4) {
		LcdDisplayProgressBar1(LCD_DISPLAY);
		LcdDisplayProgressBar2(LCD_DISPLAY);
		LcdDisplayProgressBar3(LCD_DISPLAY);
		LcdDisplayProgressBar4(LCD_DISPLAY);
	}

	if (ProgressloopCnt == 4) {
		ProgressloopCnt = 0;
	}
}

//ui状态轮询处理
void UiStateLoop(void)
{
	uint8_t rtcTime[7] = {0};
	uint32_t currentRtcCount = GetRtcCount();
    uint32_t time;

	GetRtcTime(rtcTime);
	if (ui_Info.Ui_State != UI_POWERON && ui_Info.Ui_State != UI_CARD_CHARGING && ui_Info.Ui_State != UI_ADMINISTRATOR_TEST
		&& ui_Info.Ui_State != UI_INPUT_CHARGER_SN && ui_Info.Ui_State != UI_INPUT_DEVICE_ID
		&& ui_Info.Ui_State != UI_SET_PRINT_SWITCH && ui_Info.Ui_State != UI_RELAY_TEST && ui_Info.Ui_State != UI_CARD_CHARGING) {
		UiDisplay_Time(rtcTime[4], rtcTime[5], rtcTime[6]);
	}

	if (ui_Info.Ui_State == UI_STANDBY) {
        if ((0 == gChgInfo.errCode) && (0 == gChgInfo.inputCode)) {
		    UiDisplay_MonthDay(rtcTime[2], rtcTime[3]);
        }
        if (gChgInfo.inputCode && (60 < (uint32_t)(GetRtcCount() - gChgInfo.lastInputTime))) {
            gChgInfo.inputCode = 0;
            clearKeyBuf(&keyVal);
        }
	}

    time = currentRtcCount - ui_Info.uiStartTime;
	if (ui_Info.Ui_State == UI_POWERON) {	//上电
		UiProgressLoop();
	} else if (ui_Info.Ui_State == UI_EQUIP_UPGRADE) {	//升级
		UiProgressLoop();
	} else if (ui_Info.Ui_State == UI_STANDBY) {//待机界面
	} else if(ui_Info.Ui_State == UI_READING_CRAD) {//读卡信息界面
		UiProgressLoop();
		//读卡信息若30s后台没反应,返回待机界面
		if (time > 30) {
			readCardFlg = 0;
            gCardReqFlag = 0;
			SwitchToUi_Standby();
			Sc8042bSpeech(VOIC_START_CHARGER_FAIL);
		}
	} else if (ui_Info.Ui_State == UI_READ_CARD_SUCCESS) {
		if (time >= 5) {
			//读卡查询余额成功,5s后切换输入插座编号页面
			SwitchToUi_InputSocketNum();
			clearKeyBuf(&keyVal);
			Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
		}
	} else if (ui_Info.Ui_State == UI_VERIFY_CARD_FAILURE) {//读卡或订单鉴权失败界面
		if (time >= 5) {
			SwitchToUi_Standby();
		}
	}else if (ui_Info.Ui_State == UI_CARD_CHARGING) {//该卡片有充电中订单界面
		UiProgressLoop();
		if (time >= 5) {//5s过后返回待机页面
			SwitchToUi_Standby();
		}
	} else if (ui_Info.Ui_State == UI_INPUT_SOCKET_NUM) {//输入插座编号界面
		if(time > 30) {
			SwitchToUi_Standby();
		}
	}else if(ui_Info.Ui_State == UI_INPUT_CHARGER_MONEY) {//输入充电金额界面
		if(time > 30) {
			SwitchToUi_Standby();
		}
	} else if(ui_Info.Ui_State == UI_SOCKET_OCCUPY) {//插座被占用
		if(time > 5) {
			Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
			SwitchToUi_InputSocketNum();
			clearKeyBuf(&keyVal);
		}
	}else if(ui_Info.Ui_State == UI_SOCKET_EXCEPTION) {//插座故障
		if(time > 5) {
			Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
			SwitchToUi_InputSocketNum();
			clearKeyBuf(&keyVal);
		}

	} else if(ui_Info.Ui_State == UI_SOCKET_NOT_EXSIT) {//插座编号不存在
		if(time > 5) {
			Sc8042bSpeech(VOIC_INPUT_SOCKET_NUM);
			SwitchToUi_InputSocketNum();
			clearKeyBuf(&keyVal);
		}
	}  else if(ui_Info.Ui_State == UI_START_CHARGING) {//开启充电中界面
		UiProgressLoop();
		if((time >= 3) && (time < 30)) {
			SwitchToUi_StartCharging(1);
		}else if (time >= 30) {//30s后鉴权没结果返回开启充电失败
			SwitchToUi_StartChargingFailure();
		}
	} else if(ui_Info.Ui_State == UI_START_PAYMENT_CARD_CHARGING) {//支付卡开启充电中界面
		UiProgressLoop();
		if((time >= 3) && (time < 10))
        {
			SwitchToUi_PaymentCardStartCharging(1);
		}
        else if (time >= 10)
        {
			SwitchToUi_Standby();
		}
	}
	else if (ui_Info.Ui_State == UI_START_CHARGING_FAILURE) {//开启充电失败界面
		if (time >= 5) {//5s后返回待机页面
			SwitchToUi_Standby();
		}
	}
	else if(ui_Info.Ui_State == UI_CHARGING) {//充电中界面
		UiProgressLoop();
		if ((time >= 4) && (time < 7)) {
			SwitchToUi_Charging(1);
		} else if (time >= 7) {
			SwitchToUi_Standby();
		}
	} else if (ui_Info.Ui_State == UI_CARD_MONEY_INSUFFICIENT) {//卡余额不足
		if((time >= 3) && (time < 6)) {
			SwitchToUi_BanlanceInsufficient(1);
		}else if (time >= 6) {
			Sc8042bSpeech(VOIC_INPUT_CHARGRE_MONEY);
			SwitchToUi_InputChargerMoney();
			clearKeyBuf(&keyVal);
		}
	} else if (ui_Info.Ui_State == UI_PAYMENT_CARD_MONEY_INSUFFICIENT) {//支付卡卡余额不足
		if((time >= 3) && (time < 10)) {
			SwitchToUi_PaymentCardBanlanceInsufficient(1);
		}else if (time >= 10) {
			Sc8042bSpeech(VOIC_INPUT_CHARGRE_MONEY);
            SwitchToUi_InputChargerMoney();
			clearKeyBuf(&keyVal);
		}
	}
	else if(ui_Info.Ui_State == UI_ADMINISTRATOR_TEST) {//管理员测试模式界面
		if (time > 60) {
			SwitchToUi_Standby();
		}
	} else if(ui_Info.Ui_State == UI_INPUT_CHARGER_SN) {//输入桩号界面
		if(time > 60) {
			SwitchToUi_AdministratorTestMode();
		}
	} else if(ui_Info.Ui_State == UI_INPUT_DEVICE_ID) {//输入识别码界面
		if(time > 120) {
			SwitchToUi_AdministratorTestMode();
		}
	 } else if(ui_Info.Ui_State == UI_SET_PRINT_SWITCH) {
		if(time > 120) {
			SwitchToUi_AdministratorTestMode();
		}
	 } else if(ui_Info.Ui_State == UI_RELAY_TEST) {//继电器界面
		if(time > 300) {
			SwitchToUi_AdministratorTestMode();
		}
	}
}


//按键触发UI事件
void KeyToUiProc(uint8_t key)
{
	switch(ui_Info.Ui_State) {
		case UI_POWERON:
            UiKeyHandle_Standby(key);
			break;
		case UI_STANDBY:
			UiKeyHandle_Standby(key);
			break;
		case UI_INPUT_SOCKET_NUM:
			UiKeyHandle_InputSocketNum(key);
            gChgInfo.SecondCardTick = GetRtcCount();
			break;
		case UI_INPUT_CHARGER_MONEY:
			UiKeyHandle_InputChargerMoney(key);
            gChgInfo.SecondCardTick = GetRtcCount();
			break;
		case UI_ADMINISTRATOR_TEST:
			UiKeyHandle_AdministratorTestMode(key);
			break;
		case UI_INPUT_CHARGER_SN:
			UiKeyHandle_InputChargerSn(key);
			break;
		case UI_INPUT_DEVICE_ID:
			UiKeyHandle_InputDeviceId(key);
			break;
		case UI_SET_PRINT_SWITCH:
			UiKeyHandle_SetPrintSwitch(key);
			break;
		case UI_RELAY_TEST:
			UiKeyHandle_RelayTest(key);
			break;
		case UI_EVENT_NOTICE_TEST:
			UiKeyHandle_EventNoticeTest(key);
			break;
        case UI_EQUIP_OFFLINE_EXCEPTION:
			UiKeyHandle_Standby(key);
			break;
		default:
			if ( ui_Info.Ui_State == UI_READING_CRAD
				|| ui_Info.Ui_State == UI_READ_CARD_SUCCESS
				|| ui_Info.Ui_State == UI_START_CHARGING ) {
				if (key == 10) {
					SwitchToUi_Standby();
				}
			}
			break;
	}
}


