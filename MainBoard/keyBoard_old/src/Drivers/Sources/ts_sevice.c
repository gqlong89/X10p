#ifndef _TS_SERVICE_C_
#define _TS_SERVICE_C_

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
#include "system_clock.h"		
#include "uart.h"
#include "delay.h"
#include <intrins.h>
/*********************************************************************************************************************/
#include "ts_service.h"		
#include "ts_configuration.h"
#include "ts_def.h"
#include "ts_api.h"
  
/*********************************************************************************************************************/
code unsigned int TS_CH_INFO_SEQ[][2]=
{
#if SUPPORT_KEY
	KEY_SEQ
#endif
#if SUPPORT_WHEEL_SLIDER
#if (WHEEL_SLIDER_COUNT > 0)
	WHEEL_SLIDER0_SEQ
#endif
#if (WHEEL_SLIDER_COUNT > 1)
	WHEEL_SLIDER1_SEQ
#endif
#if (WHEEL_SLIDER_COUNT > 2)
	WHEEL_SLIDER2_SEQ
#endif
#if (WHEEL_SLIDER_COUNT > 3)
	WHEEL_SLIDER3_SEQ
#endif
#endif
#if SUPPORT_WATER_DETECT
	{WATER_REF_CAP_CH,0},
	{WATER_DET_CH,0},
#endif
};



#if SUPPORT_WHEEL_SLIDER
code unsigned char SLIDER_CH_COUNT[]=
{
	WHEEL_SLIDER0_CH_COUNT,
#if (WHEEL_SLIDER_COUNT > 1)
	WHEEL_SLIDER1_CH_COUNT,
#endif
#if (WHEEL_SLIDER_COUNT > 2)
	WHEEL_SLIDER2_CH_COUNT,
#endif	
#if (WHEEL_SLIDER_COUNT > 2)
	WHEEL_SLIDER3_CH_COUNT,
#endif	
};
code unsigned char WHEEL_SLIDER0_MAX_RATE_TAB[]={WHEEL_SLIDER0_CH_MAX_RATE};
#if (WHEEL_SLIDER_COUNT > 1)
code unsigned char WHEEL_SLIDER1_MAX_RATE_TAB[]={WHEEL_SLIDER1_CH_MAX_RATE};
#endif
#if (WHEEL_SLIDER_COUNT > 2)
code unsigned char WHEEL_SLIDER2_MAX_RATE_TAB[]={WHEEL_SLIDER2_CH_MAX_RATE};
#endif
#if (WHEEL_SLIDER_COUNT > 3)
code unsigned char WHEEL_SLIDER3_MAX_RATE_TAB[]={WHEEL_SLIDER2_CH_MAX_RATE};
#endif
code unsigned char *WHEEL_SLIDER_MAX_RATE[]=
{
	WHEEL_SLIDER0_MAX_RATE_TAB,
#if (WHEEL_SLIDER_COUNT > 1)
	WHEEL_SLIDER1_MAX_RATE_TAB,
#endif
#if (WHEEL_SLIDER_COUNT > 2)
	WHEEL_SLIDER2_MAX_RATE_TAB,
#endif
#if (WHEEL_SLIDER_COUNT > 3)
	WHEEL_SLIDER3_MAX_RATE_TAB,
#endif
};


code unsigned char POSITION_CORRECTION_FACTOR[]=
{
	WHEEL_SLIDER0_POSITION_CORRECTION_FACTOR,
#if (WHEEL_SLIDER_COUNT > 1)
	WHEEL_SLIDER1_POSITION_CORRECTION_FACTOR,
#endif
#if (WHEEL_SLIDER_COUNT > 2)
	WHEEL_SLIDER2_POSITION_CORRECTION_FACTOR,
#endif	
#if (WHEEL_SLIDER_COUNT > 3)
	WHEEL_SLIDER3_POSITION_CORRECTION_FACTOR,
#endif	
};
#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//触摸管脚初始化
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void TS_IO_init(void)
{
	unsigned char i;
	code unsigned char *pXSFR[]=
	{
		&P70F,
		&P67F,
		&P66F,
		&P37F,
		&P36F,
		&P33F,
		&P32F,
		&P47F,
		&P46F,
		&P45F,
		&P44F,
		&P43F,
		&P42F,
		&P41F,
		&P40F,
		&P34F,
		&P35F,
		&P56F,
		&P50F,
		&P51F,
		&P52F,
		&P53F,
		&P54F,
		&P55F,
	};

	unsigned char *pPinS;

	GPIO_Init(P57F,P57_CAP_SETTING);

 
	for(i = 0; i < KEY_CH_COUNT; i++)
	{
		pPinS = pXSFR[TS_CH_INFO_SEQ[i][0]&0x7F];
		*pPinS = 4;
	}
}


#if SUPPORT_KEY
code unsigned long int MASK_TAB[24]=
{
	0x000001,
	0x000002,
	0x000004,
	0x000008,
	0x000010,
	0x000020,
	0x000040,
	0x000080,
	0x000100,
	0x000200,
	0x000400,
	0x000800,
	0x001000,
	0x002000,
	0x004000,
	0x008000,
	0x010000,
	0x020000,
	0x040000,
	0x080000,
	0x100000,
	0x200000,
	0x400000,
	0x800000,
};
#endif

void TS_MS_ISR (void)
{
	static unsigned char xdata ms_cnt = 0;
	unsigned char i;
 
	ms_cnt++;
	if(ms_cnt >= 4)
	{
		ms_cnt = 0;
		for(i = 0; i < OPENED_TS_COUNT; i++)
		{
			if(TS_AreaConfirmTimer[i])
			{
				TS_AreaConfirmTimer[i]--;
			}
		}

#if SUPPORT_WHEEL_SLIDER
		for(i = 0; i < WHEEL_SLIDER_COUNT; i++)
		{
			if(TSWheelSlider_TouchConfirmTimer[i])
			{
				TSWheelSlider_TouchConfirmTimer[i]--;
			}
		}	
#endif
	}	
}
void TS_HS_ISR (void)
{
	unsigned char i;
#if SUPPORT_KEY
#if (FINGER_LONG_TOUCH_TIME_LIMIT > 0)
	for(i = 0; i < KEY_CH_COUNT; i++)
	{
		if(TSKey_LongTouchLimitTimer[i])
		{
			TSKey_LongTouchLimitTimer[i]--;
		}
	}
#endif
#endif
	
#if SUPPORT_WHEEL_SLIDER	
#if (WHEEL_SLIDER_LONG_TOUCH_TIME_LIMIT > 0)
	for(i = 0; i < WHEEL_SLIDER_COUNT; i++)
	{
		if(TSWheelSlider_LongTouchLimitTimer[i])
		{
			TSWheelSlider_LongTouchLimitTimer[i]--;
		}
	}
#endif
#endif
	
	if(TS_RefCHBaseLineAdjuTimer)
	{
		TS_RefCHBaseLineAdjuTimer--;
	}	
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
	if(EnterStopScanTimer)
	{
		EnterStopScanTimer--;
	}	
#endif	
}
void TS_ISR (void)
{	
	#define TS_ACQ_TIMES			5
	unsigned char index_copy,i;
	WORD_UNION TS_Data[6];

	index_copy = INDEX;
	for(i = 0; i < 6; i++)
	{
		if(TKIF & (1<<i))
		{
			TKIF = (1<<i);	
			INDEX = i;
			TS_Data[i].bVal[0] = TKMSH;
			TS_Data[i].bVal[1] = TKMSL;
		}
	}
	for(i = 0; i < 6; i++)
	{
		if(TS_Index+i == OPENED_TS_COUNT)
		{
			if(TS_AcqCnt == 0) 	TS_RefChRawData = TS_Data[i].wVal;
			else 								TS_RefChRawData += TS_Data[i].wVal;	
			break;
		}
		if(TS_AcqCnt == 0) 		TS_RawData[TS_Index+i] = TS_Data[i].wVal;
		else 									TS_RawData[TS_Index+i] += TS_Data[i].wVal;	
	}
	if(TS_Index+6 > OPENED_TS_COUNT)
	{
		TS_AcqCnt++;
		if(TS_AcqCnt >= TS_ACQ_TIMES)
		{
			TS_AcqCnt = 0;
			TS_CycleScanDoneFlag = 1;	
		}
		else
		{
			TS_Index = 0;
			TS_ScanStart();
		}
	}
	else 
	{
		TS_Index += 6;
		TS_ScanStart();	
	}
	INDEX = index_copy;
}

void TS_ScanStart(void)
{	
	unsigned char i,j;	   
	for(i = 0; i < 6; i++)					
	{
		if(TS_Index + i >= OPENED_TS_COUNT)
		{
			INDEX = i;
#if  (IC_TYPE == IC_TYPE_CA51F2X)
			TKCHS = POL(0) | NPOL(1) | TKCHS(25);
#elif  (IC_TYPE == IC_TYPE_CA51F3X)	
			TKCHS = POL(0) | NPOL(1) | TKCHS(21);
#endif	
			for(j = i+1; j < 6; j++)
			{
				INDEX = j;
				TKCHS = POL(0) | NPOL(1) | TKCHS(0);	
			}
			break;
		}
		INDEX = i;
		TKCHS = POL(0) | NPOL(1) | TKCHS(((unsigned char)TS_CH_INFO_SEQ[TS_Index+i][0]&0x7F)+1);	
		
	}
	TKCON |= TKST(1);
}
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
void TS_GetWakeupChInfo(void)
{
	unsigned char i;
	StopScanChCnt = 0;
	for(i = 0; i < OPENED_TS_COUNT; i++)	
	{
		if(TS_CH_INFO_SEQ[i][0] & WAKEUP)
		{
			StopScanCh[StopScanChCnt] = (TS_CH_INFO_SEQ[i][0]&0x7F)+1;
			WakeUp_DeltaVal[StopScanChCnt] = (unsigned char)((TS_CH_INFO_SEQ[i][1]*WAKEUP_THRESHOLD_RATE)/100);
			StopScanChCnt++;	
		}				
	}	
}
#endif
#if SUPPORT_STOP_MODE_AUTO_SCAN
void TS_EnterStopModeAutoScan(void)
{
	unsigned char i;
	unsigned int ts_data;
	unsigned char Count = 0;
#if (RTC_CLK_SELECT == IRCL)
	unsigned char ilh,ill;
#endif
	EA = 0;
	for(i = 0; i < 6; i++)	
	{
		INDEX = i;
		if(i < StopScanChCnt)
		{
			TKCHS = POL(0) | NPOL(1) | TKCHS(StopScanCh[i]);	
		}
		else
		{
			TKCHS = POL(0) | NPOL(1) | 0;	
		}
	}

	RTCON &=  ~(MSE(1) | HSE(1));
#if (RTC_CLK_SELECT == IRCL)	
	TKMTS = (SAVE_MODE_SCAN_INTERVAL_TIME*11)/20;
#elif (RTC_CLK_SELECT == XOSCL)
	TKMTS = SAVE_MODE_SCAN_INTERVAL_TIME;
#endif	
// 	TKCON |= TMEN(1)|TWKE(1);
	TKCON = (TKCON&0xF8) | VRS(STOP_VRS_VAL);
	TKCON |= TMEN(1);
	TKCFG = TKDIV(STOP_TKDIV_VAL)|TKTMS(STOP_TKTMS_VAL);			

#if (RTC_CLK_SELECT == IRCL)	
	ilh = ILCFGH;
	ill = ILCFGL;
	
	ILCFGH	= ILH_72K ;
	ILCFGL	= ILL_72K;	
#endif
#if (SYSCLK_SRC == PLL)
	Sys_Clk_Set_IRCH();
	PLLCON = 0;
#endif
#ifdef LVD_RST_ENABLE
	LVDCON = 0;	
#endif
	I2CCON = 0;
	CKCON = 0;
		
	MECON |= BIT6;
	
	PWCON = PWCON&(~0x08);	
	TKIF = 0x3F;
	TKCON |= TKST(1);
	while(1)
	{
		if(!(PCON & BIT2))	// 在仿真模式不进入STOP模式
		{
			PCON |= (1<<1);
		}
		_nop_();
		_nop_();
		if(TKIF != 0)
		{
			if(Count<0xff) Count++;
			if(Count < 10)
			{
				TKIF = 0x3F;
			}
			else if(Count == 10)
			{
				for(i = 0; i < 6; i++)
				{
					if(TKIF & (1<<i))
					{
						INDEX = i;
						ts_data = TKMSH*256 + TKMSL;
						
						WakeUp_ThdVal[i] = ts_data - (unsigned int)WakeUp_DeltaVal[i];
						ATKCH = (unsigned char)(WakeUp_ThdVal[i]>>8);	
						ATKCL = (unsigned char)(WakeUp_ThdVal[i]);				
						WakeUp_ThdVal[i] = ts_data + (unsigned int)WakeUp_DeltaVal[i];
						ATKNH = (unsigned char)(WakeUp_ThdVal[i]>>8);	
						ATKNL = (unsigned char)(WakeUp_ThdVal[i]);				
					}
				}
				TKCON |= TWKE(1);
				TKIF = 0x3F;
				TKCON |= TKST(1);
			}
			else if(Count > 10)
			{
				PWCON |= 0x08;	
				TKIF = 0x3F;
	#if (SYSCLK_SRC == PLL)
				Sys_Clk_Set_PLL(PLL_Multiple);	
	#endif
#if (RTC_CLK_SELECT == IRCL)
				ILCFGH	= 	ilh ;
				ILCFGL	= 	ill;
#endif

				TKCON = TKST(0)|TKIE(1)|TMEN(0)|TWKE(0)|VRS(VRS_VAL);
				TKCFG = TKDIV(TKDIV_VAL)|TKTMS(TKTMS_VAL);	

				RTCON |=  (MSE(1) | HSE(1));
				TS_Index = 0;
				TS_ScanStart();
				EnterStopScanTimer = ENTER_STOP_MODE_TIME;	
#ifdef LVD_RST_ENABLE
				LVDCON = 0xC2;	//设置LVD复位电压为2V
#endif				
				EA = 1;
				break;				
			}			
		}	
	}
}
#endif
#if SUPPORT_STOP_MODE_MANUAL_SCAN
void TS_SetStopScanCh(unsigned char idx)
{
	unsigned char i;
	for(i = 0; i < 6; i++)	
	{
		INDEX = i;
		if(i+idx < StopScanChCnt)
		{
			TKCHS = POL(0) | NPOL(1) | TKCHS(StopScanCh[idx+i]);				
		}
		else
		{
			TKCHS = POL(0) | NPOL(1) | 0;		
		}			
	}	
}
void TS_EnterStopModeManaulScan(void)
{
	unsigned char i,ts_idx;
	unsigned char Count = 0;
	unsigned int ts_data;
#if (RTC_CLK_SELECT == IRCL)
	unsigned char ilh,ill;
#endif
	EA = 0;
	
	RTCON &=  ~(MSE(1) | HSE(1));
	RTCIF = RTC_MF | RTC_HF;		
	
	TKCON = (TKCON&0xF8) | VRS(STOP_VRS_VAL);
	TKCON |= TMEN(1);
	TKCFG = TKDIV(STOP_TKDIV_VAL)|TKTMS(STOP_TKTMS_VAL);		
#if (RTC_CLK_SELECT == IRCL)	
	TKMTS = (SAVE_MODE_SCAN_INTERVAL_TIME*11)/20;
	ilh = ILCFGH;
	ill = ILCFGL;
	
	ILCFGH	= ILH_72K ;
	ILCFGL	= ILL_72K;
#elif (RTC_CLK_SELECT == XOSCL)
	TKMTS = SAVE_MODE_SCAN_INTERVAL_TIME;
#endif

#if (SYSCLK_SRC == PLL)
	Sys_Clk_Set_IRCH();
	PLLCON = 0;
#endif
#ifdef LVD_RST_ENABLE
	LVDCON = 0;	
#endif		
	I2CCON = 0;
	CKCON = 0;
		
	MECON |= BIT6;
	
	TKIF = 0x3F;
	PWCON = PWCON&(~0x08);	
	ts_idx = 0;
	TS_SetStopScanCh(ts_idx);
	TKCON |= TKST(1);
	while(1)
	{
		if(!(PCON & BIT2))	// 在仿真模式不进入STOP模式
		{
			PCON |= (1<<1);
		}
		
		_nop_();
		_nop_();
		if(TKIF != 0)
		{
			for(i = 0; i < 6; i++)
			{
				if(TKIF & (1<<i))
				{
					INDEX = i;
					ts_data = TKMSH*256 + TKMSL;
					
					if(Count == 2)
					{
						WakeUp_ThdVal[ts_idx + i] = ts_data - (unsigned int)WakeUp_DeltaVal[ts_idx + i];
					}
					else if(Count > 2)
					{
						if(ts_data <= WakeUp_ThdVal[ts_idx + i])
						{
							
							PWCON |= 0x08;	
							TKIF = 0x3F;		
#if (SYSCLK_SRC == PLL)
							Sys_Clk_Set_PLL(PLL_Multiple);	
#endif
#if (RTC_CLK_SELECT == IRCL)	
							ILCFGH	= 	ilh ;
							ILCFGL	= 	ill;
#endif
							TKCON = TKST(0)|TKIE(1)|TMEN(0)|TWKE(0)|VRS(VRS_VAL);
							TKCFG = TKDIV(TKDIV_VAL)|TKTMS(TKTMS_VAL);	
						
							RTCON |= HSE(1)|MSE(1);
						
							TS_Index = 0;
							TS_ScanStart();
							EnterStopScanTimer = ENTER_STOP_MODE_TIME;
#ifdef LVD_RST_ENABLE
							LVDCON = 0xC2;	//设置LVD复位电压为2V
#endif									
							EA = 1;
							return;				
						}
					}
				}
			}	
			if((ts_idx + 6) >= StopScanChCnt)
			{
				ts_idx = 0;
				if(Count<0xff) Count++;
			}
			else
			{
				ts_idx += 6;
			}			
			TS_SetStopScanCh(ts_idx);	
			TKIF = 0x3F;
		}
	}

}
#endif
void TS_RunInit(void)
{
	unsigned char i;

	static unsigned char TS_ScanTimes=0;
	
	if(TS_Init_Step == 0)
	{
		TS_AcqCnt = 0;
		TS_Index = 0;
		TS_ScanStart();	
		TS_Init_Step = 1;		
		TS_ScanTimes = 50;	
	}
	else if(TS_Init_Step == 1)
	{
		if(TS_CycleScanDoneFlag)
		{
			TS_CycleScanDoneFlag = 0;
			if(--TS_ScanTimes)
			{
				TS_Index = 0;
				TS_ScanStart();						
				return;
			}
			TS_RefCHBaseLineAdjuTimer = 0;
			for(i = 0; i < OPENED_TS_COUNT; i++)
			{	
				TS_FilterBuf[i] = TS_RawData[i]*FILTER_COUNT;
#if (SUPPORT_KEY || SUPPORT_WHEEL_SLIDER)				
				TS_BaseLineData[i] = TS_RawData[i];			
				TS_FirstAreaData[i] = TS_RawData[i]; 
				TS_AreaConfirmTimerSet(i,AREA_PNOISE);
#endif
			}	
#if ANTI_SPEAKPHONE
			TS_RefChBaseLineData  = TS_RefChRawData;	
			TS_RefFilterBuf = TS_RefChRawData*FILTER_COUNT;
#endif
			
#if SUPPORT_WATER_DETECT
			WaterDetCapRateFilterCnt = 0;
			WaterDetCapRateFilterBuf = 0;
			WaterFlag = 0;
#endif
			
			TS_State = TS_DEAL;	
#if SUPPORT_KEY
			TouchMetalFlag = 0;
#endif
			TS_Index = 0;
			TS_ScanStart();							
		}
	}
}
#if (SUPPORT_KEY || SUPPORT_WHEEL_SLIDER)
void TS_TouchMetalProc(unsigned char Operation,unsigned char Ch)
{
	static unsigned char State = 0;
	unsigned char i;
	switch(Operation)
	{
		case 0:
			TouchMetalFlag = 1;
			State = 1;
		  if(KeysFlagSN != 0)
			{
				for(i = 0; i < KEY_CH_COUNT; i++)
				{
					if(KeysFlagSN & MASK_TAB[i])
					{
						if(i == Ch) continue;
						KeysFlagSN &= ~MASK_TAB[i];
						TS_AreaConfirmTimerSet(i,AREA_PNOISE);	
					}
				}				
			}
			break;
		case 1:
			if(TouchMetalFlag)
			{
				if(State == 1)
				{
					if(KeysFlagSN != 0)
					{
						State = 2;
					}
				}
				else if(State == 2)
				{
					if(KeysFlagSN == 0)
					{
						State = 0;
						TouchMetalFlag = 0;
					}
				}
			}
			break;
	}
}
unsigned char TS_GetDataArea(unsigned char ts_ch,unsigned int ts_data)
{
	int Delta;
	int NoiseThd,FingerThd;
	
	Delta 	= (int)TS_BaseLineData[ts_ch] - (int)ts_data;
	
	if(ts_ch < KEY_CH_COUNT)
	{
		if(!TouchMetalFlag)
		{
			if(Delta > (int)TS_BaseLineData[ts_ch]/5)
			{
				TS_TouchMetalProc(0,ts_ch);

				return AREA_FINGER_TOUCH; 
			}	
		}
		else
		{
			FingerThd =  (int)TS_BaseLineData[ts_ch]/5;
			goto SET_NOISE_THD;
		}
		if(KeysFlagSN & MASK_TAB[ts_ch])
		{
			FingerThd = (TS_CH_INFO_SEQ[ts_ch][1]*(FINGER_TOUCH_THRESHOLD_RATE-10))/100;	
			goto SET_NOISE_THD;
		}
	}
	FingerThd = (TS_CH_INFO_SEQ[ts_ch][1]*FINGER_TOUCH_THRESHOLD_RATE)/100;	
SET_NOISE_THD:	
	NoiseThd 	= 	(TS_CH_INFO_SEQ[ts_ch][1]*NOISE_THRESHOLD_RATE)/100;	

	if(Delta >= FingerThd)	return AREA_FINGER_TOUCH; 
	else if(Delta > NoiseThd)	return AREA_PNOISE_PLUS; 
	else if(Delta >= 0)			return AREA_PNOISE;
	else if(Delta >= -NoiseThd) return AREA_NNOISE;
	else 						return AREA_OVER_LOW;

}
unsigned char TS_AreaDeviateDetection(	unsigned char ts_ch,unsigned int cmp_data,unsigned int real_data,unsigned char area)
{	
	int Delta,permit_devia;
	unsigned char rate;
	switch(area)
	{
		case AREA_PNOISE_PLUS:
			rate = AREA_PNOISE_PLUS_PERMITTED_DEVIATION_RATE;
			break;
		case AREA_PNOISE:
			rate = AREA_PNOISE_PERMITTED_DEVIATION_RATE;
			break;
		case AREA_NNOISE:
			rate = AREA_NNOISE_PERMITTED_DEVIATION_RATE;
			break;
		case AREA_OVER_LOW:
			rate = AREA_OVERLOW_PERMITTED_DEVIATION_RATE;
			break;
		default:
			break;
	}
	permit_devia = (TS_CH_INFO_SEQ[ts_ch][1]*rate)/100;

	Delta 	= (int)cmp_data - (int)real_data;

	if((Delta <= permit_devia) && (Delta >= -permit_devia))
	{
		return 1;
	}
	else
	{
		return 0;	
	}		
}
void TS_AreaConfirmTimerSet(unsigned char ts_ch, unsigned char area)
{
	unsigned char set_value;

	switch(area)
	{
		case AREA_FINGER_TOUCH:
#if SUPPORT_KEY
			set_value = FINGER_TOUCH_CONFIRM_TIME;
#endif
			break;
		case AREA_PNOISE_PLUS:
			set_value = AREA_PNOISE_PLUS_CONFIRM_TIME;
			break;
		case AREA_PNOISE:
			set_value = AREA_PNOISE_CONFIRM_TIME;
			break;
		case AREA_NNOISE:
			set_value = AREA_NNOISE_CONFIRM_TIME;
			break;
		case AREA_OVER_LOW:
			set_value = AREA_OVERLOW_CONFIRM_TIME;
			break;
		default:
			break;
	}
	TS_AreaConfirmTimer[ts_ch] = set_value;	
}
#endif

void TS_DataFiltering(void)
{
	unsigned char i;
	for(i = 0; i < OPENED_TS_COUNT; i++)
	{
		TS_FilterBuf[i] -= TS_FilterBuf[i]/FILTER_COUNT;
		TS_FilterBuf[i] += TS_RawData[i];		
		TS_PostData[i]  = TS_FilterBuf[i]/FILTER_COUNT;							  
 	}		
}
#if SUPPORT_KEY
void TSKey_DataDeal(void)
{
	unsigned char i;
	unsigned char  TouchArea,NowArea;		
	int  Delta;	
#if PRINT_TS_KEY_DATA_EN
	Uart_PutChar(UART_0,0xaa);
#endif
	for(i = 0; i < KEY_CH_COUNT; i++)
	{
		Delta 	= (int)TS_BaseLineData[i] - (int)TS_PostData[i];
		
#if PRINT_TS_KEY_DATA_EN
		if((i==PRINT_CH)||(PRINT_CH==0xFF))
		{
			//printf("TSKey_DataDeal=%X\n",Delta);
				Uart_PutChar(UART_0,i);
				Uart_PutChar(UART_0,TS_PostData[i]>>8);
				Uart_PutChar(UART_0,TS_PostData[i]);			
				Uart_PutChar(UART_0,TS_BaseLineData[i]>>8);
				Uart_PutChar(UART_0,TS_BaseLineData[i]);	
				if(Delta >= 0)
				{
					Uart_PutChar(UART_0,Delta>>8);
					Uart_PutChar(UART_0,Delta);						
				}
				else
				{
					Uart_PutChar(UART_0,0);
					Uart_PutChar(UART_0,0);
				}
				

		}
#endif
	}				

	for(i = 0; i < KEY_CH_COUNT; i++)
	{								
		TouchArea = TS_GetDataArea(i,TS_FirstAreaData[i]);
		NowArea   = TS_GetDataArea(i,TS_PostData[i]);

#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
		if((NowArea != AREA_PNOISE)&&(NowArea != AREA_NNOISE))
		{
			EnterStopScanTimer = ENTER_STOP_MODE_TIME;
		}
#endif
		if(TouchArea == AREA_FINGER_TOUCH)
		{
			if(NowArea == AREA_FINGER_TOUCH)
			{
				if(!(KeysFlagSN & MASK_TAB[i]))
				{
					if(!TS_AreaConfirmTimer[i])
					{		
						KeysFlagSN |= MASK_TAB[i];		
#if (FINGER_LONG_TOUCH_TIME_LIMIT > 0)
						TSKey_LongTouchLimitTimer[i] = FINGER_LONG_TOUCH_TIME_LIMIT;
#endif
						TS_AreaConfirmTimer[i] = FINGER_TOUCH_RELEASE_CONFIRM_TIME;	
					}
				}
				else
				{
					TS_AreaConfirmTimer[i] = FINGER_TOUCH_RELEASE_CONFIRM_TIME;
#if (FINGER_LONG_TOUCH_TIME_LIMIT > 0)
					if(!TSKey_LongTouchLimitTimer[i])
					{
						KeysFlagSN &= ~MASK_TAB[i];
						TS_FirstAreaData[i] = TS_PostData[i];
						TS_BaseLineData[i] 	= TS_PostData[i];
						TS_AreaConfirmTimerSet(i,AREA_PNOISE);			
					}						
#endif
				}				
			}
			else
			{
				if(KeysFlagSN & MASK_TAB[i]) 
				{
					if(!TS_AreaConfirmTimer[i])
					{
						KeysFlagSN &= ~MASK_TAB[i];
						TS_FirstAreaData[i] = TS_PostData[i];
						TS_AreaConfirmTimerSet(i,NowArea);
					}					
				}			
				else
				{
					TS_FirstAreaData[i] = TS_PostData[i];
					TS_AreaConfirmTimerSet(i,NowArea);
				}	
			}	
			continue;						
		}	
		else
		{
			if(NowArea == AREA_FINGER_TOUCH)
			{
				TS_FirstAreaData[i] = TS_PostData[i];
				TS_AreaConfirmTimerSet(i,AREA_FINGER_TOUCH);
				continue;		
			}	
		}
					
		if(TS_AreaDeviateDetection(i,TS_FirstAreaData[i],TS_PostData[i],TouchArea))	   //判断偏差是否在允许范围内
		{	
			if(!TS_AreaConfirmTimer[i])
			{
				TS_FirstAreaData[i] = TS_PostData[i];
				TS_BaseLineData[i] 	= TS_PostData[i];
				TS_AreaConfirmTimerSet(i,AREA_PNOISE);				
			}		
		}
		else
		{
			TS_FirstAreaData[i] = TS_PostData[i];	
			TS_AreaConfirmTimerSet(i,NowArea);				
		}
	}	 	
	TS_TouchMetalProc(1,0xff);
}
#endif
#if SUPPORT_WHEEL_SLIDER
void TSWheelSliderGetTouchToggleInfo(unsigned char num)
{
	unsigned char i;
	unsigned int  Rate;
	unsigned int  RateMin,RateSec;
	int  Delta;

	RateMin = 0xFFFF; 			
	RateSec = 0xFFFF;
	for(i = 0; i < SLIDER_CH_COUNT[num]; i++)	
	{
		Delta 	= (int)TS_BaseLineData[TS_BaseCh + i] - (int)TS_PostData[TS_BaseCh + i];	
		if(Delta <= 0) Delta = 1;
		Rate = TS_BaseLineData[TS_BaseCh + i]/Delta;	//基准值与差值的比率
					
		if(Rate < RateMin) 	//查找基准值与差值的比率最小的触摸通道
		{
			RateSec = RateMin; 
			RateMin = Rate;
			Toggleinfo.SecMinChIdx = Toggleinfo.MinChIdx;	
			
			Toggleinfo.Delta2 = Toggleinfo.Delta1;	
			Toggleinfo.Delta1 = Delta;	//记录比率最小通道的Delta
			Toggleinfo.MinChIdx = i;		//记录比率最小通道的索引值
		}
		else if(Rate < RateSec)//查找基准值与差值的比率次小的触摸通道
		{
			RateSec = Rate;
			Toggleinfo.SecMinChIdx = i;	//记录比率次小通道的索引值
			Toggleinfo.Delta2 = Delta;	//记录比率次小通道的Delta
		}
	}
}


unsigned char IsValidCh(unsigned char minchidx,unsigned char secchidx,unsigned char ch_size)
{
	unsigned char Temp;
	if(minchidx > secchidx) 
	{
		Temp = secchidx;
	}
	else
	{
		Temp = minchidx;
	}
	if(Temp != (minchidx+secchidx)/2)
	{
		if((minchidx*secchidx != 0) && (minchidx + secchidx+1) != ch_size)
		{
			return 0;
		}
	}
	return 1;
}

void TSWheelSliderTogChDataCal(void)
{
	unsigned long int CalcTemp1,CalcTemp2;
	unsigned int Temp;
	unsigned char minidx,secidx;
	
	minidx = Toggleinfo.MinChIdx + TS_BaseCh;
	secidx = Toggleinfo.SecMinChIdx + TS_BaseCh;
	
	CalcTemp1 = (unsigned long int)(TS_BaseLineData[minidx]) * (unsigned long int)(TS_BaseLineData[secidx]);
	CalcTemp2 = CalcTemp1;
	CalcTemp1 >>= 6;

	CalcTemp1 *= (unsigned long int)TS_PostData[secidx];
		
	CalcTemp2 += ((unsigned long int)TS_BaseLineData[secidx] - (unsigned long int)TS_BaseLineData[minidx]) * (unsigned long int)TS_PostData[secidx];
	CalcTemp2 >>= 6;
	Toggleinfo.Delta2 = TS_BaseLineData[minidx] - (unsigned int)(CalcTemp1/CalcTemp2);	

	if(Toggleinfo.Delta2 > Toggleinfo.Delta1)
	{
		Temp = Toggleinfo.Delta2;
		Toggleinfo.Delta2 = Toggleinfo.Delta1;
		Toggleinfo.Delta1 = Temp;
		Temp = (unsigned int)Toggleinfo.SecMinChIdx;
		Toggleinfo.SecMinChIdx = Toggleinfo.MinChIdx;
		Toggleinfo.MinChIdx = (unsigned char)Temp;
	}
}

unsigned char RateFilter(unsigned char rate,unsigned int *filterBuf,unsigned char touchFlag)
{
	if(!touchFlag)
	{
		*filterBuf = rate*4;
	}
	else
	{
		*filterBuf = *filterBuf -	*filterBuf/4 + rate;
	}
	return (*filterBuf/4);	
}
unsigned int DeltaRateSumFilter(unsigned int rate,unsigned int *filterBuf,unsigned char touchFlag)
{
	if(!touchFlag)
	{
		*filterBuf = rate*5;
	}
	else
	{
		*filterBuf = *filterBuf -	*filterBuf/5 + rate;
	}
	return (*filterBuf/5);		
}
unsigned char WheelGetOffsetDirection(unsigned char minidx,unsigned char secidx)
{
	
	if(minidx > secidx)
	{
		if(minidx != secidx+1)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if(secidx != minidx+1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}
int WheelSliderPositionCorrection(int position, int factor)
{
	position += factor;
	if(position > 255)
	{
		position -= 256;
	}
	else if(position < 0)
	{
		position += 256;
	}
	return position;
}

unsigned char WheelSliderTouchJudge(unsigned char num)
{
	unsigned int  DeltaRateSum,Rate;

	unsigned int  gpTemp;
	unsigned int  Offset;
	unsigned char	*pRateMax;
	static unsigned int xdata FilterBuf[WHEEL_SLIDER_COUNT];
	static unsigned int xdata DeltaRateSumFilerBuf[WHEEL_SLIDER_COUNT];
	
	WheelSliderValidTouchFlag = 0;	
	pRateMax = WHEEL_SLIDER_MAX_RATE[num];
	
	TSWheelSliderGetTouchToggleInfo(num);


	DeltaRateSum = (unsigned int)(((unsigned long int)Toggleinfo.Delta1*10000)/(TS_CH_INFO_SEQ[TS_BaseCh+Toggleinfo.MinChIdx][1]*FINGER_TOUCH_THRESHOLD_RATE) 
	+ ((unsigned long int)Toggleinfo.Delta2*10000)/(TS_CH_INFO_SEQ[TS_BaseCh+Toggleinfo.SecMinChIdx][1]*FINGER_TOUCH_THRESHOLD_RATE));
	
	if(DeltaRateSum < 100) //比率最小和次小的通道的Delta值和各自门限值百分比之和大于100表示有触摸事件
	{		
		return 0;
	}
/**********************************************************************************************************/
//判断此两通道是否为相邻通道
	if(!IsValidCh(Toggleinfo.MinChIdx,Toggleinfo.SecMinChIdx,SLIDER_CH_COUNT[num]))
	{
		return 0;
	}
/**********************************************************************************************************/	

	//数据校正
	TSWheelSliderTogChDataCal();

	Rate = (Toggleinfo.Delta1*100)/(Toggleinfo.Delta1+Toggleinfo.Delta2);	//计算Delta1占比

	Rate = RateFilter(Rate,&FilterBuf[num],WheelSliderTouchFlag[num]);	//rate滤波，消除抖动


	gpTemp = DeltaRateSumFilter(DeltaRateSum,&DeltaRateSumFilerBuf[num],WheelSliderTouchFlag[num]);
	
	
	if(DeltaRateSum >= (gpTemp*3)/4)
	{
		WheelSliderValidTouchFlag = 1;		
	}
	
  WheelSliderPositionTemp = Toggleinfo.MinChIdx*(256/SLIDER_CH_COUNT[num])+(128/SLIDER_CH_COUNT[num]);

	if(Rate >= pRateMax[Toggleinfo.MinChIdx])
	{
		Offset = 0;
	}
	else
	{
		Offset = ((pRateMax[Toggleinfo.MinChIdx] - Rate)*(128/SLIDER_CH_COUNT[num]))/(pRateMax[Toggleinfo.MinChIdx]-50);
	}
#if PRINT_WHEEL_SLIDER_MAX_RATE_EN
	if(num == PRINT_WHEEL_SLIDER_NUM)
	{

		Uart0_PutChar(Toggleinfo.MinChIdx + 0x30);	
		Uart0_PutChar(' ');
		
		Uart0_PutChar(Rate/100+0x30);		
		Uart0_PutChar((Rate%100)/10+0x30);		
		Uart0_PutChar((Rate%10)+0x30);	
		Uart0_PutChar('\r');	
		Uart0_PutChar('\n');	
	}	
#endif
	if(WheelGetOffsetDirection(Toggleinfo.MinChIdx,Toggleinfo.SecMinChIdx))
	{
		WheelSliderPositionTemp += Offset;
	}
	else
	{
		WheelSliderPositionTemp -= Offset;
	}
	
	WheelSliderPositionTemp = WheelSliderPositionCorrection(WheelSliderPositionTemp,POSITION_CORRECTION_FACTOR[num]);
	if(WheelSliderPositionTemp > 255) WheelSliderPositionTemp = 255;

	return 1;

}
void TS_WheelSliderDeal(void)
{
	unsigned char num,i;
	unsigned char  TouchArea,NowArea;
	int   Delta;
	
	TS_BaseCh = KEY_CH_COUNT; 
	for(num = 0; num < WHEEL_SLIDER_COUNT; num++)
	{
#if PRINT_WHEEL_SLIDER_DATA_EN
		if(num == PRINT_WHEEL_SLIDER_NUM)
		{
			Uart0_PutChar(0xaa);
			for(i = TS_BaseCh; i < TS_BaseCh + SLIDER_CH_COUNT[num]; i++)
			{						
				Delta 	= (int)TS_BaseLineData[i] - (int)TS_PostData[i];			
				Uart0_PutChar(i);
				Uart0_PutChar(TS_PostData[i]>>8);
				Uart0_PutChar(TS_PostData[i]);			
				Uart0_PutChar(TS_BaseLineData[i]>>8);
				Uart0_PutChar(TS_BaseLineData[i]);	
				if(Delta < 0)
				{
					Uart0_PutChar(0);
					Uart0_PutChar(0);
				}
				else
				{
					Uart0_PutChar(Delta>>8);	
					Uart0_PutChar(Delta);					
				}					
			}				
		}
#endif
		if(WheelSliderState[num] == WHEEL_SLIDER_NO_TOUCH)
		{
			if(!WheelSliderTouchFlag[num])
			{
				if(WheelSliderTouchJudge(num))
				{
					WheelSliderTouchFlag[num] = 1;	
					TSWheelSlider_TouchConfirmTimer[num] = WHEEL_SLIDER_TOUCH_CONFIRM_TIME;				
				}
				else
				{
					for(i = TS_BaseCh; i < TS_BaseCh + SLIDER_CH_COUNT[num]; i++)
					{						
						Delta 	= (int)TS_BaseLineData[i] - (int)TS_PostData[i];
						TouchArea = TS_GetDataArea(i,TS_FirstAreaData[i]);
						NowArea   = TS_GetDataArea(i,TS_PostData[i]);
						
						if(TS_AreaDeviateDetection(i,TS_FirstAreaData[i],TS_PostData[i],TouchArea))	   //判断偏差是否在允许范围内
						{
							if(!TS_AreaConfirmTimer[i])
							{
								TS_FirstAreaData[i] = TS_PostData[i];
								TS_BaseLineData[i] 	= TS_PostData[i];
								TS_AreaConfirmTimerSet(i,AREA_PNOISE);	
							}		
						}
						else
						{
							TS_FirstAreaData[i] = TS_PostData[i];
							TS_AreaConfirmTimerSet(i,AREA_PNOISE);	
						}
					}
				}			
			}	
			else
			{	
				if(WheelSliderTouchJudge(num))	
				{
					if(!TSWheelSlider_TouchConfirmTimer[num])
					{
						WheelSliderState[num] = WHEEL_SLIDER_TOUCH;	
#if (WHEEL_SLIDER_LONG_TOUCH_TIME_LIMIT > 0)
						TSWheelSlider_LongTouchLimitTimer[num] = WHEEL_SLIDER_LONG_TOUCH_TIME_LIMIT;
#endif
					}	
				}
				else
				{
					WheelSliderTouchFlag[num] = 0;
					for(i = TS_BaseCh; i < TS_BaseCh + SLIDER_CH_COUNT[num]; i++)
					{
						TS_AreaConfirmTimerSet(i,AREA_PNOISE);	
					}
				}
			}				
		}
		else if(WheelSliderState[num] == WHEEL_SLIDER_TOUCH)
		{		
#if (WHEEL_SLIDER_LONG_TOUCH_TIME_LIMIT > 0)
			if(!TSWheelSlider_LongTouchLimitTimer[num])
			{
				for(i = TS_BaseCh; i < TS_BaseCh + SLIDER_CH_COUNT[num]; i++)
				{
					TS_FirstAreaData[i] = TS_PostData[i];
					TS_BaseLineData[i] 	= TS_PostData[i];
				}
				WheelSliderTouchFlag[num] = 0;
				goto WHEEL_SLIDER_RELEASE;
			}
#endif
			if(WheelSliderTouchFlag[num])
			{
				if(!WheelSliderTouchJudge(num))	
				{
					WheelSliderTouchFlag[num] = 0;
					TSWheelSlider_TouchConfirmTimer[num] = WHEEL_SLIDER_TOUCH_RELEASE_CONFIRM_TIME;		
				}
			}
			else						 
			{
				if(!WheelSliderTouchJudge(num))	
				{
					if(!TSWheelSlider_TouchConfirmTimer[num])
					{
WHEEL_SLIDER_RELEASE:
						WheelSliderState[num] = WHEEL_SLIDER_NO_TOUCH;	
						WheelSliderPosition[num] = -1;
						for(i = TS_BaseCh; i < TS_BaseCh + SLIDER_CH_COUNT[num]; i++)
						{
							TS_AreaConfirmTimerSet(i,AREA_PNOISE);	
						}
					}
				}
				else
				{
					WheelSliderTouchFlag[num] = 1;				
				}
			}		
		}		
		if(WheelSliderState[num] == WHEEL_SLIDER_TOUCH)
		{
			if(WheelSliderValidTouchFlag)
			{
				WheelSliderPosition[num] = WheelSliderPositionTemp;		
#if PRINT_WHEEL_SLIDER_POSITION_EN
				Uart0_PutChar(num + 0x30);	
				Uart0_PutChar(' ');
				
				Uart0_PutChar(WheelSliderPosition[num]/100+0x30);		
				Uart0_PutChar((WheelSliderPosition[num]%100)/10+0x30);		
				Uart0_PutChar((WheelSliderPosition[num]%10)+0x30);	

				Uart0_PutChar('\r');	
				Uart0_PutChar('\n');	
#endif
			}
		}
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
		if(WheelSliderTouchFlag[num])
		{
			EnterStopScanTimer = ENTER_STOP_MODE_TIME;
		}
#endif
		TS_BaseCh += SLIDER_CH_COUNT[num];
	}
}
#endif
#if SUPPORT_WATER_DETECT
void TS_WaterDetectDeal(void)
{
	unsigned char CapRate;
	TS_BaseCh = OPENED_TS_COUNT-2;
	CapRate = (unsigned char)(((unsigned long int)TS_PostData[TS_BaseCh+1]*100)/(unsigned long int)TS_PostData[TS_BaseCh]);

	if(WaterDetCapRateFilterCnt < WATER_DET_FILTER_CNT)
	{
		WaterDetCapRateFilterCnt++;
		WaterDetCapRateFilterBuf += CapRate;
	}
	else
	{
		WaterDetCapRateFilterBuf -= WaterDetCapRateFilterBuf/WATER_DET_FILTER_CNT;
		WaterDetCapRateFilterBuf += CapRate;
		
		CapRate = (unsigned char)(WaterDetCapRateFilterBuf/WATER_DET_FILTER_CNT);
#if  PRINT_WATER_DET_CAP_RATE_EN
		Uart0_PutChar(CapRate/100+0x30);		
		Uart0_PutChar((CapRate%100)/10+0x30);		
		Uart0_PutChar((CapRate%10)+0x30);	
		Uart0_PutChar('\r');	
		Uart0_PutChar('\n');			
#endif
		if(!WaterFlag)
		{
			if(CapRate < WATER_RATE_THD)
			{
				WaterFlag = 1;
			}
		}
		else
		{
			if(CapRate > NO_WATER_RATE_THD)
			{
				WaterFlag = 0;
			}
		}
	}
	
}
#endif	
#if ANTI_SPEAKPHONE
void TS_RawDataCalibrate(unsigned int data1, unsigned int data2)
{
	unsigned char i;

	for(i = 0; i < OPENED_TS_COUNT; i++)
	{
		TS_RawData[i] = ((unsigned long int)TS_RawData[i]*(unsigned long int)data2)/data1;			
	}	
}
void TS_CheckRefCh(void)
{
	int  Delta;
	unsigned int TS_RefPostData;
	TS_RefFilterBuf -= TS_RefFilterBuf/FILTER_COUNT;
	TS_RefFilterBuf += TS_RefChRawData;		
	TS_RefPostData  = TS_RefFilterBuf/FILTER_COUNT;		
	
	if(!TS_RefCHBaseLineAdjuTimer)
	{
		TS_RefCHBaseLineAdjuTimer = 2;
		Delta = (int)TS_RefPostData - (int)TS_RefChBaseLineData;

		if(Delta > 0)
		{
			TS_RefChBaseLineData++;	
		}
		else if(Delta < 0)
		{
			TS_RefChBaseLineData--;	
		}				
	}
	TS_RawDataCalibrate(TS_RefPostData,TS_RefChBaseLineData);
}
#endif
/*********************************************************************************************************************/
#endif
