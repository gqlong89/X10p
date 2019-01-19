#ifndef __RELAYCTRLTASK_H__
#define __RELAYCTRLTASK_H__

#include <stdint.h>
#include "ht6xxx.h"

typedef enum{
    RELAY_SYNC = 0,     //ͬ��
    RELAY_ASYNC = 1,    //�첽
}RELAY_CTRL_TYPE;

#if defined HT6x2x  ||  defined  HT6x3x
typedef enum
{
    TIMExtPWMCountUpLevelHigh       = (TMR_TMRCON_PWMHL_HIGH|TMR_TMRCON_PWMC_UP|TMR_TMRCON_MODE_PWM),     /*!< PWM���ϼ�����ʽ����ʼ��ƽ��    */
    TIMExtPWMCountDownLevelHigh     = (TMR_TMRCON_PWMHL_HIGH|TMR_TMRCON_PWMC_DOWM|TMR_TMRCON_MODE_PWM),   /*!< PWM���¼�����ʽ����ʼ��ƽ��    */
    TIMExtPWMCenterAlignedLevelHigh = (TMR_TMRCON_PWMHL_HIGH|TMR_TMRCON_PWMC_CENTER|TMR_TMRCON_MODE_PWM), /*!< PWM������������ʽ����ʼ��ƽ��*/
    TIMExtPWMCountUpLevelLow        = (TMR_TMRCON_PWMHL_LOW|TMR_TMRCON_PWMC_UP|TMR_TMRCON_MODE_PWM),      /*!< PWM���ϼ�����ʽ����ʼ��ƽ��    */
    TIMExtPWMCountDownLevelLow      = (TMR_TMRCON_PWMHL_LOW|TMR_TMRCON_PWMC_DOWM|TMR_TMRCON_MODE_PWM),    /*!< PWM���¼�����ʽ����ʼ��ƽ��    */
    TIMExtPWMCenterAlignedLevelLow  = (TMR_TMRCON_PWMHL_LOW|TMR_TMRCON_PWMC_CENTER|TMR_TMRCON_MODE_PWM),  /*!< PWM������������ʽ����ʼ��ƽ��*/
    TIMExtCycleTiming               = TMR_TMRCON_MODE_TIMING,                                             /*!< ��ʱ�����ڶ�ʱ                 */
}TMRExtMode_TypeDef;                             /*!< end of group TMRExtMode_TypeDef*/

typedef enum
{
    TIMExtClk_LRC = TMR_TMRCON_CLKSEL_LRC,       /*!< Timer Clkʱ��ԴΪLRC                */
    TIMExtClk_LF  = TMR_TMRCON_CLKSEL_LF,        /*!< Timer Clkʱ��ԴΪLF                 */
    TIMExtClk_HRC = TMR_TMRCON_CLKSEL_HRC,       /*!< Timer Clkʱ��ԴΪHRC                */
    TIMExtClk_PLL = TMR_TMRCON_CLKSEL_PLL,       /*!< Timer Clkʱ��ԴΪPLL                */
#if  defined  HT6x2x
    TIMExtClk_MEMS= TMR_TMRCON_CLKSEL_MEMS,      /*!< Timer Clkʱ��ԴΪMEMS               */
#endif
}TMRExtClkSource_TypeDef;                        /*!< end of group TMRExtClkSource_TypeDef*/
#endif


typedef struct
{
    TMRExtClkSource_TypeDef TimerSource;         /*!< ��ʱ��ʱ��Դѡ��               */
    TMRExtMode_TypeDef TimerMode;                /*!< ��ʱ������ģʽ                 */
    uint16_t TimerPreDiv;                        /*!< ��ʱ��ʱ��Ԥ��Ƶ����           */
    uint16_t TimerPeriod;                        /*!< ��ʱ����ʱ����                 */
    uint16_t TimerCompare;                       /*!< ��ʱ���ȽϼĴ���               */
}TMRExt_InitTypeDef;                             /*!< end of group TMRExt_InitTypeDef*/


extern void RelayCtrl(uint8_t gunId,uint8_t on);
extern void RelayCtrlTask(void);
extern void StartRelayTimer(uint16_t period);
extern int timer4_init(uint16_t period);

extern const char EMUID[7];		//��1��ʼ

#define READ_ATT_IRQ()		GPIO_ReadInputDataBit(HT_GPIOA, GPIO_Pin_10)

#endif
