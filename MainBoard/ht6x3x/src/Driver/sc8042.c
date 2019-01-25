/*sc8042.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "sc8042.h"
#include "usart.h"
#include "rtc.h"
#include "server.h"
#include "sim800c.h"
#include "ui.h"


uint32_t voiceUndateTime=0;
uint8_t  powerOnFlag = 0;



void CheckVoiceChipPower(void)
{
    if ((uint32_t)(GetRtcCount() - voiceUndateTime) > 120) 
	{
		if (powerOnFlag == 1) 
		{
			CL_LOG("pd sc8024.\n");
			POWER_DOWN_SC();//语音模块下电
			powerOnFlag = 0;
		}
	}
}


void Sc8042bSpeech(int index)
{
	uint8_t  i;
    uint8_t  rtcTime[8];

	GetRtcTime(rtcTime);
	Feed_WDT();
	if (gOutNetStatus.mode != 2)
	{ //以防工厂晚间生产喇叭不会响
		if (system_info.disturbingStartTime != system_info.disturbingStopTime) 
		{//语音禁播开启时间和结束时间不相等就禁播语音
			if (system_info.disturbingStartTime < system_info.disturbingStopTime) 
			{
				if ((rtcTime[4] >= system_info.disturbingStartTime) && (rtcTime[4] < system_info.disturbingStopTime)) 
				{
					CL_LOG("禁播语音.\n");
					return;
				}
			} 
			else 
			{
				if ((rtcTime[4] >= system_info.disturbingStartTime) || (rtcTime[4] < system_info.disturbingStopTime)) 
				{
					CL_LOG("禁播语音.\n");
					return;
				}
			}
		}
	}

    if (gChgInfo.netStatus & (1<<BLUE_UPGRADE)) 
	{
        CL_LOG("bt ota,not sp.\n");
        return;
    }

	voiceUndateTime = GetRtcCount();
	if (powerOnFlag == 0) 
	{
		CL_LOG("po sc8024.\n");
		POWER_ON_SC();
		powerOnFlag = 1;
	}

    CL_LOG("%d.\n",index);
    MASTER_INT_DIS();
	ENABLE_SC_RST();
    Feed_WDT();
	delay_us(VOIC_DELAY_CNT);
	DISABLE_SC_RST();
	delay_us(VOIC_DELAY_CNT);
	for(i=0; i<index; i++) 
	{
        Feed_WDT();
		//数据脉冲拉高
		ENABLE_SC_DATA();
		delay_us(VOIC_DELAY_CNT);
		//数据脉冲拉低
		DISABLE_SC_DATA();
		delay_us(VOIC_DELAY_CNT);
	}
    Feed_WDT();
    MASTER_INT_EN();
}


void SoundCode(int code)
{
    uint8_t codeVal[3];

    codeVal[0] = code / 100 % 10;
    codeVal[1] = code / 10 % 10;
    codeVal[2] = code / 1 % 10;
    Sc8042bSpeech(codeVal[0]+31);
    vTaskDelay(250);
    Sc8042bSpeech(codeVal[1]+31);
    vTaskDelay(250);
    Sc8042bSpeech(codeVal[2]+31);
    vTaskDelay(250);
}


//提示操作失败及具体代码
void OptFailNotice(int code)
{
    Feed_WDT();
    CL_LOG("errcode=%03d.\n",code);
    if ((code > SIM800_STATE_NUM) && (100 != code) && (101 != code)) 
	{
		SoundCode(code);
	}
	else
	{
		if (code != gChgInfo.errCode) 
		{
            UiDisplay_ErrCode(code);
			SoundCode(code);
			gChgInfo.errCode = code;
		}
	}
}


//提示操作成功及具体代码
void OptSuccessNotice(int code)
{
    Feed_WDT();
    CL_LOG("ok code=%03d.\n",code);
    Sc8042bSpeech(VOIC_SUCCESS);
    vTaskDelay(1000);
    SoundCode(code);
}


void TempNotice(uint16_t temp)
{
    uint8_t codeVal[3];

    Feed_WDT();
    CL_LOG("temp=%d.\n",temp);
    codeVal[0] = temp / 100 % 10;
    codeVal[1] = temp / 10 % 10;
    codeVal[2] = temp / 1 % 10;
    Sc8042bSpeech(VOIC_TEM);
    vTaskDelay(1000);
    Sc8042bSpeech(codeVal[0]+31);
    vTaskDelay(250);
    Sc8042bSpeech(codeVal[1]+31);
    vTaskDelay(250);
    Sc8042bSpeech(codeVal[2]+31);
    Sc8042bSpeech(VOIC_DEGREE);
    vTaskDelay(250);
}


void SoundNumber(uint32_t balance)
{
    uint8_t temp0,temp1,temp2;

    temp0 = balance / 10000;
	temp1 = (balance/1000)%10;
	temp2 = (balance/100)%10;

    if (temp0) {
		Sc8042bSpeech(temp0+31);
		vTaskDelay(250);
		Sc8042bSpeech(VOIC_HUNDRED);
		vTaskDelay(250);
	}

    if (temp1) {
        Sc8042bSpeech(temp1+31);
        vTaskDelay(250);
        Sc8042bSpeech(VOIC_TEN);
        vTaskDelay(250);
    }else if (temp0 && temp2) {
        Sc8042bSpeech(0+31);
        vTaskDelay(250);
    }

    if (temp2) {
        Sc8042bSpeech(temp2+31);
        vTaskDelay(250);
    }

    if ((0 == temp0) && (0 == temp1) && (0 == temp2)) {
        Sc8042bSpeech(0+31);
        vTaskDelay(250);
    }
}


void PointOutNumber(uint16_t number)
{
    Sc8042bSpeech(REMAINDER_CHARGING_CNT);
    vTaskDelay(2000);
    SoundNumber(number);
    Sc8042bSpeech(NUMBER_OF_TIMES);
    vTaskDelay(250);
}


void SpeechCardBalance(uint32_t balance)
{
	uint8_t temp3,temp4;

	if (balance > 99999) 
	{
		return;
	}

	temp3 = (balance/10)%10;
	temp4 = balance%10;

	Sc8042bSpeech(VOIC_CARD_BANLANCE);
	vTaskDelay(1000 / VOICE_DELAY_DIV);
    SoundNumber(balance);
    Sc8042bSpeech(VOIC_POINT);
    vTaskDelay(250 / VOICE_DELAY_DIV);
    Sc8042bSpeech(temp3+31);
    vTaskDelay(250 / VOICE_DELAY_DIV);
    Sc8042bSpeech(temp4+31);
    vTaskDelay(250 / VOICE_DELAY_DIV);
	Sc8042bSpeech(VOIC_YUAN);
}


//播放 1~12 的语音
void SpeechPile(uint8_t gun)
{
    uint8_t tmp;

	tmp = gun / 10;
	if (tmp==0) {
		Sc8042bSpeech(gun+31);
		vTaskDelay(250);
	} else {
		Sc8042bSpeech(VOIC_TEN);
		vTaskDelay(250);
		tmp = gun % 10;
		if (tmp != 0) {
			Sc8042bSpeech(tmp+31);
			vTaskDelay(250);
		}
	}
}


void StartChargeSpeech(void)
{
	Sc8042bSpeech(VOIC_NIN_YI_XUANZE);
	vTaskDelay(800);
    SpeechPile(gChgInfo.current_usr_gun_id);
	Sc8042bSpeech(VOIC_HAO_CHAZUO);
	vTaskDelay(800);
	Sc8042bSpeech(VOIC_CHARGER_MONEY);
	vTaskDelay(800);
	Sc8042bSpeech(gChgInfo.money+31);
	vTaskDelay(250);
	Sc8042bSpeech(VOIC_YUAN);
	vTaskDelay(250);
}

void SpeechStartCharge(void)
{
	StartChargeSpeech();
	Sc8042bSpeech(VOIC_VERIFIED_PLUG);
	vTaskDelay(2000);
}

void SpeechChargeing(uint8_t id)
{
    SpeechPile(id);
	Sc8042bSpeech(VOIC_HAO_CHAZUO);
	vTaskDelay(800);
	Sc8042bSpeech(VOIC_START_CHARGING);
	vTaskDelay(1000);
	Sc8042bSpeech(VOIC_STOP_CHARGER_TIP);
}


void PointOutPullGun(uint8_t gun)
{
    SpeechPile(gun);
    Sc8042bSpeech(CHECK_GUN_STATUS);
}


int Sc8042b_Init(void)
{
	GPIO_Cfg(HT_GPIOH, GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	GPIO_Cfg(HT_GPIOH, GPIO_Pin_2, GPIO_Mode_IOIN, GPIO_Input_Up, GPIO_Output_PP);
	GPIO_Cfg(HT_GPIOG, GPIO_Pin_8, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	POWER_ON_SC();
	DISABLE_SC_DATA();
	DISABLE_SC_RST();
	OS_DELAY_MS(500);
	Sc8042bSpeech(VOIC_WELCOME);
	OS_DELAY_MS(800);
//	Sc8042bSpeech(VOIC_NULL2);
    CL_LOG("init ok.\n");
	return 0;
}


