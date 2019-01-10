#ifndef _RTC_C_
#define _RTC_C_
/*********************************************************************************************************************/
#include "ca51f_config.h"
#if (IC_TYPE == IC_TYPE_CA51F2X)
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#elif (IC_TYPE == IC_TYPE_CA51F3X)
#include "ca51f3sfr.h"
#include "ca51f3xsfr.h"
#include "gpiodef_f3.h"
#endif
#include "system.h"
#include "rtc.h"
#include "delay.h"
#include <intrins.h>
/*********************************************************************************************************************/
/***********************************************************************************
函数名：		RTC_init
功能说明： 初始化RTC控制寄存器
输入参数： rtce		RTC功能模块使能控制
						mse		RTC毫秒中断使能控制
						hse		RTC半秒中断使能控制
返回值：	无
***********************************************************************************/																												
void RTC_init(RTCE_TypeDef rtce,MSE_TypeDef mse,HSE_TypeDef hse)
{	
	RTCON = (RTCON & 0x1F) | RTCE(rtce) | MSE(mse) | HSE(hse);
}
/***********************************************************************************
函数名：		RTC_WriteSecond
功能说明： RTC写入秒值
输入参数： second		秒值
返回值：		无
***********************************************************************************/	
void RTC_WriteSecond(unsigned char second) //second = 0~59
{
	RTCON |= RTCWE(1);
	RTCS = second;
	Delay_50us(1);
	RTCON &= ~RTCWE(1);	 	
}

/***********************************************************************************
函数名：		RTC_WriteMinute
功能说明： RTC写入分值
输入参数： minute		分值
返回值：		无
***********************************************************************************/	
void RTC_WriteMinute(unsigned char minute) //minute = 0~59
{
	RTCON |= RTCWE(1);
	RTCM = minute;
	Delay_50us(1);
	RTCON &= ~RTCWE(1);
}


/***********************************************************************************
函数名：		RTC_WriteHour
功能说明： RTC写入小时
输入参数： hour		小时值
返回值：		无
***********************************************************************************/
void RTC_WriteHour(unsigned char hour)	//hour = 0~23
{
	RTCON |= RTCWE(1);
	RTCH = (RTCH&0xE0)|hour;
	Delay_50us(1);
	RTCON &= ~RTCWE(1);	 	
}


/***********************************************************************************
函数名：		RTC_WriteWeek
功能说明： RTC写入星期
输入参数： week		星期值
返回值：		无
***********************************************************************************/
void RTC_WriteWeek(unsigned char week) //hour = 1~7代表星期一~星期天
{
	RTCON |= RTCWE(1);
	RTCH = (RTCH&0x1F)|(week<<5);
	Delay_50us(1);
	RTCON &= ~RTCWE(1);	 	
}


/***********************************************************************************
函数名：		RTC_AlarmCfg
功能说明： 配置RTC ALARM功能寄存器位
输入参数： hce		小时比较使能控制
						mce		分钟比较使能控制
						sce		秒比较使能控制
返回值：		无
***********************************************************************************/
void RTC_AlarmCfg(HCE_TypeDef hce,MCE_TypeDef mce,SCE_TypeDef sce)
{	
	RTCON = (RTCON & 0xE3) | HCE(hce) | MCE(mce) | SCE(sce);
}

/***********************************************************************************
函数名：		RTC_SetAlaramTime
功能说明：  设置RTC ALARM时间
输入参数：  al_hr		闹钟小时值
						al_min		闹钟分钟值
						al_sec		闹钟秒值
返回值：		无
***********************************************************************************/
void RTC_SetAlaramTime(unsigned char al_hr,unsigned char al_min,unsigned char al_sec)
{
	RTAH	=	al_hr;
	RTAM	=	al_min;
	RTAS	=	al_sec;
}
#endif
/***********************************************************************************
函数名：		RTC_SetRTMSS
功能说明：  设置毫秒中断间隔
输入参数：  rtmss		
返回值：		无
***********************************************************************************/
void RTC_SetRTMSS(unsigned char rtmss)
{
	RTMSS = rtmss;
}
/*********************************************************************************************************************/
