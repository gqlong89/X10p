#ifndef _TS_API_H
#define _TS_API_H

#ifdef _TK_API_C_
/***************************************************************************************************************/
/***************************************************************************************************************/											

TSState_T 	  xdata TS_State;
#if SUPPORT_KEY
unsigned long int xdata KeysFlagSN;	
#endif

#if SUPPORT_WHEEL_SLIDER
int xdata WheelSliderPosition[WHEEL_SLIDER_COUNT];
#endif

#if SUPPORT_WATER_DETECT
bit WaterFlag;
#endif
#else
/***************************************************************************************************************/
/***************************************************************************************************************/											
extern TSState_T 	  xdata TS_State;
#if SUPPORT_KEY
extern unsigned long int xdata KeysFlagSN;	
#endif
#if SUPPORT_WHEEL_SLIDER
extern int xdata WheelSliderPosition[WHEEL_SLIDER_COUNT];
#endif
#if SUPPORT_WATER_DETECT
extern bit WaterFlag;
#endif
#endif




void TS_init(void);
void TS_Action(void);
void TS_GetKey(void);
		 
#endif