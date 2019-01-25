/*usart.c
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#include "includes.h"

static __IO uint32_t TimingDelay=0;


//******************************************************************
//! \brief  	SysTick_Init
//! \param  	tickFlag=1, 22m; tickFlag=0, 44m
//! \retval
//! \note
//! \note
//******************************************************************
void SysTick_Init(void)
{
    uint32_t cpuf;
	
#if (0 == IS_CPU_DOU_FRE)
	cpuf = (44040192/2/1000);
#else
	cpuf = (44040192/1000);
#endif
    if (SysTick_Config (cpuf)) 
	{
		while (1);
	}
}



