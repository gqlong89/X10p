#ifndef __INCLUDES_H__
#define __INCLUDES_H__

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
#include "uart.h"
#include "system_clock.h"
#include "ts_configuration.h"
#include "ts_def.h"
#include "ts_api.h"
#include "ts_service.h"	
#include "key.h"
#include "rtc.h"
#include "spi.h"
#include "Delay.h"
#include "fm175xx.h"
#include "type_a.h"
#include "i2c.h"
#include "wdt.h"
#include "fifo.h"
#include "bluetooth.h"
#include "proto.h"
#include "blue_api.h"
#include <intrins.h>
#include <intrins.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <absacc.h>


#define ERROR		1
#define	OK			0
#define True	1
#define False	0

typedef unsigned long  Time_t;

extern Time_t getSystemTimer(void);
extern int StartCallBack(unsigned int timeout,void (* fun)(void));


#define TIMEOUT_1s  2



#endif


