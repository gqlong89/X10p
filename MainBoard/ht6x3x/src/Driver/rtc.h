/*rtc.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __RTC_H__
#define __RTC_H__

#include "includes.h"


#define	C_Toff		0x0000									//温度偏置寄存器
#define	C_MCON01	0x2000									//控制系数01
#define	C_MCON23	0x0588									//控制系数23
#define	C_MCON45	0x4488									//控制系数45



extern void SetRtcCount(uint32_t tick);
extern uint32_t GetRtcCount(void);
extern void GetRtcTime(void* pRTCTime);
extern int RtcInit(void);
extern char* GetCurrentTime(void);
extern uint32_t GetTimeStamp(void);
extern int LinuxTickToDay(uint32_t tick, uint8_t *pDay);


extern __IO uint32_t gSysSecond;

#endif //__RTC_H__


