/*
*********************************************************************************************************
*                                 			   HT60XX
*                            			      HT60XX³ÌÐò
*
*                       			Copyright 2013, Hi-Trend Tech, Corp.
*                             			 All Rights Reserved
*
*
* Project      : HT60xx
* File         : ht60xx_it.c
* By           : SocTeam
* Version      : V0.4
* Description  :
*********************************************************************************************************
*/

#define  __HT60XX_IT_C

#include "ht60xx_it.h"
#include "flash.h"
#include "update.h"


#define RESET_VCTOR_ID      1
#define NMI_VCTOR_ID        2
#define HARDFAULT_VCTOR_ID  3
#define SVC_VCTOR_ID        11
#define PENDSV_VCTOR_ID     14
#define SYSTICK_VCTOR_ID    15
#define PMU_VCTOR_ID        16
#define AES_VCTOR_ID        17
#define EXTI0_VCTOR_ID      18
#define EXTI1_VCTOR_ID      19
#define EXTI2_VCTOR_ID      20
#define EXTI3_VCTOR_ID      21
#define EXTI4_VCTOR_ID      22
#define EXTI5_VCTOR_ID      23
#define EXTI6_VCTOR_ID      24
#define UART0_VCTOR_ID      25
#define UART1_VCTOR_ID      26
#define UART2_VCTOR_ID      27
#define UART3_VCTOR_ID      28
#define UART4_VCTOR_ID      29
#define UART5_VCTOR_ID      30
#define TIMER_0_VCTOR_ID    31
#define TIMER_1_VCTOR_ID    32
#define TIMER_2_VCTOR_ID    33
#define TIMER_3_VCTOR_ID    34
#define TBS_VCTOR_ID        35
#define RTC_VCTOR_ID        36
#define I2C_VCTOR_ID        37
#define SPI0_VCTOR_ID        38
#define SPI1_VCTOR_ID        39
#define SelfTestFreq_VCTOR_ID   40
#define TIMER_4_VCTOR_ID        41
#define TIMER_5_VCTOR_ID    42
#define UART6_VCTOR_ID      43
#define EXTI7_VCTOR_ID      44
#define Reserved_VCTOR_ID   45
#define SPI2_VCTOR_ID       46
#define DMA_VCTOR_ID        47

//void Reset_Handler()
//{
//    void (*pFunction)(void);

//    uint32_t address;

//    HT_Flash_WordRead(&address, AppFlashAddr+4*RESET_VCTOR_ID,1);

//	pFunction = (void (*)(void))address;
//	pFunction();//Jump t
//}



void NMI_Handler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*NMI_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void HardFault_Handler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*HARDFAULT_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void SVC_Handler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*SVC_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void PendSV_Handler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*PENDSV_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void SysTick_Handler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*SYSTICK_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void PMU_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*PMU_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void AES_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*AES_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void EXTI0_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI0_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void EXTI1_IRQHandler()
{
     void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI1_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void EXTI2_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI2_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void EXTI3_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI3_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void EXTI4_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI4_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void EXTI5_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI5_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void EXTI6_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI6_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void UART0_IRQHandler()
{

    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART0_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}


void UART1_IRQHandler()
{
    void (*pFunction)(void);
    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART1_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void UART2_IRQHandler()
{
     void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART2_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void UART3_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART3_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void UART4_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART4_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void UART5_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART5_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}


void TIMER_0_IRQHandler()
{

     void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TIMER_0_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void TIMER_1_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TIMER_1_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t


}

void TIMER_2_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TIMER_2_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t


}

void TIMER_3_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TIMER_3_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void TBS_IRQHandler()
{

    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TBS_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t



}

void RTC_IRQHandler()
{
     void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*RTC_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t


}

void I2C_IRQHandler()
{
     void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*I2C_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void SPI0_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*SPI0_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t


}

void SPI1_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*SPI1_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void SelfTestF_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+SelfTestFreq_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void TIMER_4_IRQHandler()
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TIMER_4_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void TIMER_5_IRQHandler(void)
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*TIMER_5_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void UART6_IRQHandler(void)
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*UART6_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}

void EXTI7_IRQHandler(void)
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*EXTI7_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void Reserved_IRQHandler(void)
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*Reserved_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void SPI2_IRQHandler(void)
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*SPI2_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t
}

void DMA_IRQHandler(void)
{
    void (*pFunction)(void);

    uint32_t address;

    HT_Flash_WordRead(&address, AppFlashAddr+4*DMA_VCTOR_ID,1);

	pFunction = (void (*)(void))address;
	pFunction();//Jump t

}


