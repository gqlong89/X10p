#ifndef __RELAYCTRLTASK_H__
#define __RELAYCTRLTASK_H__

#include <stdint.h>
#include "ht6xxx.h"

typedef enum{
    RELAY_SYNC = 0,     //同步
    RELAY_ASYNC = 1,    //异步
}RELAY_CTRL_TYPE;

#if defined HT6x2x  ||  defined  HT6x3x
typedef enum
{
    TIMExtPWMCountUpLevelHigh       = (TMR_TMRCON_PWMHL_HIGH|TMR_TMRCON_PWMC_UP|TMR_TMRCON_MODE_PWM),     /*!< PWM向上计数方式，初始电平高    */
    TIMExtPWMCountDownLevelHigh     = (TMR_TMRCON_PWMHL_HIGH|TMR_TMRCON_PWMC_DOWM|TMR_TMRCON_MODE_PWM),   /*!< PWM向下计数方式，初始电平高    */
    TIMExtPWMCenterAlignedLevelHigh = (TMR_TMRCON_PWMHL_HIGH|TMR_TMRCON_PWMC_CENTER|TMR_TMRCON_MODE_PWM), /*!< PWM中央对齐计数方式，初始电平高*/
    TIMExtPWMCountUpLevelLow        = (TMR_TMRCON_PWMHL_LOW|TMR_TMRCON_PWMC_UP|TMR_TMRCON_MODE_PWM),      /*!< PWM向上计数方式，初始电平低    */
    TIMExtPWMCountDownLevelLow      = (TMR_TMRCON_PWMHL_LOW|TMR_TMRCON_PWMC_DOWM|TMR_TMRCON_MODE_PWM),    /*!< PWM向下计数方式，初始电平低    */
    TIMExtPWMCenterAlignedLevelLow  = (TMR_TMRCON_PWMHL_LOW|TMR_TMRCON_PWMC_CENTER|TMR_TMRCON_MODE_PWM),  /*!< PWM中央对齐计数方式，初始电平低*/
    TIMExtCycleTiming               = TMR_TMRCON_MODE_TIMING,                                             /*!< 定时器周期定时                 */
}TMRExtMode_TypeDef;                             /*!< end of group TMRExtMode_TypeDef*/

typedef enum
{
    TIMExtClk_LRC = TMR_TMRCON_CLKSEL_LRC,       /*!< Timer Clk时钟源为LRC                */
    TIMExtClk_LF  = TMR_TMRCON_CLKSEL_LF,        /*!< Timer Clk时钟源为LF                 */
    TIMExtClk_HRC = TMR_TMRCON_CLKSEL_HRC,       /*!< Timer Clk时钟源为HRC                */
    TIMExtClk_PLL = TMR_TMRCON_CLKSEL_PLL,       /*!< Timer Clk时钟源为PLL                */
#if  defined  HT6x2x
    TIMExtClk_MEMS= TMR_TMRCON_CLKSEL_MEMS,      /*!< Timer Clk时钟源为MEMS               */
#endif
}TMRExtClkSource_TypeDef;                        /*!< end of group TMRExtClkSource_TypeDef*/
#endif


typedef struct
{
    TMRExtClkSource_TypeDef TimerSource;         /*!< 定时器时钟源选择               */
    TMRExtMode_TypeDef TimerMode;                /*!< 定时器工作模式                 */
    uint16_t TimerPreDiv;                        /*!< 定时器时钟预分频设置           */
    uint16_t TimerPeriod;                        /*!< 定时器定时周期                 */
    uint16_t TimerCompare;                       /*!< 定时器比较寄存器               */
}TMRExt_InitTypeDef;                             /*!< end of group TMRExt_InitTypeDef*/


extern void RelayCtrl(uint8_t gunId,uint8_t on);
extern void RelayCtrlTask(void);
extern void StartRelayTimer(uint16_t period);
extern int timer4_init(uint16_t period);

extern const char EMUID[7];		//从1开始

#define READ_ATT_IRQ()		GPIO_ReadInputDataBit(HT_GPIOA, GPIO_Pin_10)

#endif
