#ifndef _TK_API_C_
#define _TK_API_C_

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
#include "system_clock.h"		
#include "uart.h"
#include "rtc.h"
#include "delay.h"
#include <intrins.h>
/*********************************************************************************************************************/
#include "ts_configuration.h"
#include "ts_def.h"
#include "ts_api.h"
#include "ts_service.h"	   
/*********************************************************************************************************************/
void TS_init(void)
{
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
#if (RTC_CLK_SELECT == IRCL)
	unsigned char ilh,ill;	
#endif
#endif
	
#if SUPPORT_WHEEL_SLIDER
	unsigned char i;
#endif
	

#if (RTC_CLK_SELECT == IRCL)
	CKCON |= ILCKE;
	CKSEL |= RTCKS(1);
#elif (RTC_CLK_SELECT == XOSCL)	
	CKCON |= XLCKE;
	while(!(CKCON & XLSTA));
#endif

	RTC_init(RTCE_On,MSE_On,HSE_On);
	RTC_SetRTMSS(0);


	INT8EN = 1;			
	
	TS_Cnt 		= OPENED_TS_COUNT;

	TS_IO_init();
  
	TKCON = TKST(0)|TKIE(1)|TMEN(0)|TWKE(0)|VRS(VRS_VAL);
	TKCFG = TKDIV(TKDIV_VAL)|TKTMS(TKTMS_VAL);		
	INT3EN = 1; 
    //INT2EN = 1;

	TS_CycleScanDoneFlag = 0;
#if SUPPORT_KEY	
	KeysFlagSN = 0;	
#endif
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
	EnterStopScanTimer = ENTER_STOP_MODE_TIME;
#if (RTC_CLK_SELECT == IRCL)
	ilh = ILCFGH;
	ill = ILCFGL;
	Calibration_IL_Targ__IH_Ref(72089);
	ILH_72K = ILCFGH;
	ILL_72K = ILCFGL;		
	ILCFGH = ilh;
	ILCFGL = ill;
#endif	
	TS_GetWakeupChInfo();
#endif
	TS_State = TS_INIT;
	TS_Init_Step = 0;


#if SUPPORT_WHEEL_SLIDER
	for(i = 0; i < WHEEL_SLIDER_COUNT; i++)
	{
		WheelSliderState[i] = WHEEL_SLIDER_NO_TOUCH;
		WheelSliderTouchFlag[i] = 0;
		WheelSliderPosition[i] = -1;		
	}
#endif		
}

void TS_Action(void)
{
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
	bit EnterStopFlag = 0;
#endif
	switch(TS_State)
	{
		case TS_INIT:
			TS_RunInit();			
			break;
		case TS_DEAL:		
			
			if(TS_CycleScanDoneFlag)
			{			
				
				TS_CycleScanDoneFlag = 0;
#if ANTI_SPEAKPHONE
				TS_CheckRefCh();
#endif
				TS_DataFiltering();

#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
				if(EnterStopScanTimer)
#endif
				{
					TS_Index = 0;
					TS_ScanStart();									
				}
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
				else
				{
					EnterStopFlag = 1;
				}
#endif
#if SUPPORT_KEY
				TSKey_DataDeal();
#endif
#if SUPPORT_WHEEL_SLIDER
				TS_WheelSliderDeal();
#endif	

#if SUPPORT_WATER_DETECT
				TS_WaterDetectDeal();
#endif	
				
#if SUPPORT_STOP_MODE_AUTO_SCAN
				if(EnterStopFlag)
				{
					TS_EnterStopModeAutoScan();	
				}
#endif		
#if SUPPORT_STOP_MODE_MANUAL_SCAN
				if(EnterStopFlag)
				{
					TS_EnterStopModeManaulScan();	
				}
#endif										
			}
			break;
		default:
			break;
	}

}

/*********************************************************************************************************************/
#endif