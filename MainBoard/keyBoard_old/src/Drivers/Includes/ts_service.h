#ifndef _TS_SERVICE_H_
#define _TS_SERVICE_H_

#include "ts_configuration.h"

#ifdef _TS_SERVICE_C_
bit TS_CycleScanDoneFlag;
unsigned char xdata TS_Cnt;				
unsigned int  xdata TS_RefChBaseLineData;
unsigned int  xdata TS_RefChRawData;					  
unsigned int 	xdata TS_RawData[OPENED_TS_COUNT];

unsigned int xdata TS_BaseLineData[OPENED_TS_COUNT];
unsigned int xdata TS_FirstAreaData[OPENED_TS_COUNT];
unsigned char xdata TS_AreaConfirmTimer[OPENED_TS_COUNT];
unsigned int xdata TS_PostData[OPENED_TS_COUNT];
unsigned char TS_BaseCh;

#if SUPPORT_KEY
#if (FINGER_LONG_TOUCH_TIME_LIMIT > 0)
unsigned char xdata TSKey_LongTouchLimitTimer[KEY_CH_COUNT];
#endif
#endif

#if SUPPORT_WHEEL_SLIDER
unsigned char xdata TSWheelSlider_TouchConfirmTimer[WHEEL_SLIDER_COUNT];
unsigned char xdata TSWheelSlider_LongTouchLimitTimer[WHEEL_SLIDER_COUNT];


unsigned char xdata WheelSliderState[WHEEL_SLIDER_COUNT];
unsigned char xdata WheelSliderTouchFlag[WHEEL_SLIDER_COUNT];
bit WheelSliderValidTouchFlag;
int idata WheelSliderPositionTemp;

TouchToggleInfo_TypeDef Toggleinfo;
#endif

#if SUPPORT_WATER_DETECT
unsigned int WaterDetCapRateFilterBuf;
unsigned char WaterDetCapRateFilterCnt;
#endif
/***************************************************************************************************************/
/***************************************************************************************************************/
unsigned int xdata TS_FilterBuf[OPENED_TS_COUNT];	
unsigned int xdata TS_RefFilterBuf;												
unsigned char xdata TS_Init_Step;
unsigned char idata TS_RefCHBaseLineAdjuTimer;
unsigned char TS_Index;

unsigned char xdata TS_AcqCnt;


unsigned char xdata TS_CH[OPENED_TS_COUNT];	
#if SUPPORT_KEY
bit TouchMetalFlag;
#endif
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
unsigned char xdata EnterStopScanTimer;
#if (RTC_CLK_SELECT == IRCL)
unsigned char xdata ILH_72K,ILL_72K;
#endif
#if (IC_TYPE == IC_TYPE_CA51F2X)
#define TS_CH_MAX		24
#elif (IC_TYPE == IC_TYPE_CA51F3X) 
#define TS_CH_MAX		20
#endif

unsigned char xdata StopScanCh[TS_CH_MAX];
unsigned char xdata StopScanChCnt;
unsigned int xdata WakeUp_ThdVal[TS_CH_MAX];
unsigned char xdata WakeUp_DeltaVal[TS_CH_MAX];
#endif	   

#else
extern bit TS_CycleScanDoneFlag;
extern unsigned char xdata TS_Cnt;				
extern unsigned int  xdata TS_RefChBaseLineData;
extern unsigned int  xdata TS_RefChRawData;

#if SUPPORT_WHEEL_SLIDER

extern unsigned char xdata WheelSliderState[WHEEL_SLIDER_COUNT];
extern unsigned char xdata WheelSliderTouchFlag[WHEEL_SLIDER_COUNT];
#endif
/***************************************************************************************************************/
/***************************************************************************************************************/
extern unsigned int xdata TS_FilterBuf[OPENED_TS_COUNT];												   
extern unsigned char xdata TS_Init_Step;
extern unsigned char  TS_RefCHBaseConfirmTimer;
extern unsigned char TS_Index;
extern  unsigned char xdata TS_CH[OPENED_TS_COUNT];	
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
extern unsigned char xdata EnterStopScanTimer;
#if (RTC_CLK_SELECT == IRCL)
extern unsigned char xdata ILH_72K,ILL_72K;
#endif
#endif	 	   
extern code unsigned long int MASK_TAB[24];

#endif



void TS_IO_init(void);
void TS_ScanStart(void);
void TSKey_DataDeal(void);
void TS_RunInit(void);
void TS_CheckRefCh(void);
void TS_WheelSliderDeal(void);
void TS_AreaConfirmTimerSet(unsigned char ts_ch, unsigned char area);
void TS_DataFiltering(void);
void TS_EnterStopModeAutoScan(void);
void TS_EnterStopModeManaulScan(void);
void TS_MS_ISR(void);
void TS_HS_ISR(void);
void TS_ISR(void);
#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
void TS_GetWakeupChInfo(void);
#endif
#if SUPPORT_WATER_DETECT
void TS_WaterDetectDeal(void);
#endif
#endif
