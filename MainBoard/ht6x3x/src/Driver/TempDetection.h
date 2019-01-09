
#ifndef __TempDetection_H__
#define __TempDetection_H__

#include "includes.h"



typedef enum
{
    TMPPRD  = TBS_TBSPRD_TMPPRD,         /*!< 温度                                */
    VBATPRD = TBS_TBSPRD_VBATPRD,        /*!< 电池电压                            */
    ADC0PRD = TBS_TBSPRD_ADC0PRD,        /*!< ADC0                                */
    ADC1PRD = TBS_TBSPRD_ADC1PRD,        /*!< ADC1                                */
    VCCPRD  = TBS_TBSPRD_VCCPRD,         /*!< 电源电压                            */
#if  defined  HT501x ||  defined  HT502x
    ADC2PRD = TBS_TBSPRD_ADC2PRD,        /*!< ADC2                                */
#endif
#if  defined  HT6x2x ||  defined  HT6x3x
    ADC3PRD = TBS_TBSPRD_ADC3PRD,        /*!< ADC3                                */
    ADC4PRD = TBS_TBSPRD_ADC4PRD,        /*!< ADC4                                */
    ADC5PRD = TBS_TBSPRD_ADC5PRD,        /*!< ADC5                                */
#endif
#if  defined  HT501x
    VREFPRD = TBS_TBSPRD_VREFPRD,        /*!< Vref                                */
#endif
}TBS_PeriodTypeDef;                      /*!< end of group TBS_PeriodTypeDef      */

typedef enum
{
    TBS_TRI_TMP  = TBS_TRIREQ_TMPTRI,    /*!< 快速触发温度采�?                    */
    TBS_TRI_VBAT = TBS_TRIREQ_VBATTRI,   /*!< 快速触发电池采�?                    */
    TBS_TRI_ADC0 = TBS_TRIREQ_ADC0TRI,   /*!< 快速触发ADC0采样                     */
    TBS_TRI_ADC1 = TBS_TRIREQ_ADC1TRI,   /*!< 快速触发ADC1采样                     */
    TBS_TRI_VCC  = TBS_TRIREQ_VCCTRI,    /*!< 快速触发VCC采样                      */
    TBS_TRI_ADC3 = TBS_TRIREQ_ADC3TRI,   /*!< 快速触发ADC3采样                     */
    TBS_TRI_ADC4 = TBS_TRIREQ_ADC4TRI,   /*!< 快速触发ADC4采样                     */
    TBS_TRI_ADC5 = TBS_TRIREQ_ADC5TRI,   /*!< 快速触发ADC5采样                     */
}TBS_TriChnTypeDef;   

typedef enum
{
    TBS_CLKSEL_SUCC       = 0,           /*!< TBS时钟选择修改成功                   */
    TBS_CLKSEL_PARA_ERR   = -1,          /*!< TBS时钟选择修改失败：输入参数错�?    */
    TBS_CLKSEL_PLL_EN_ERR = -2,          /*!< TBS时钟选择修改失败：PLL未打开        */
}TBS_ClkSelReturnTypeDef;                /*!< end of group TBS_ClkSelReturnTypeDef*/
	
typedef enum
{
    TBS_CLKSEL_OSC    = 0x0000,          /*!< ʱ��ԴѡΪOSC                        */
    TBS_CLKSEL_PLLDIV = 0x0020,          /*!< ʱ��ԴѡΪPLL/168                   */
}TBS_ClkSelectTypeDef;                   /*!< end of group TBS_ClkSelectTypeDef   */

typedef enum
{
    TBS_TMP  = 0,                        /*!< 温度                                */
    TBS_VBAT ,                           /*!< 电池电压                            */
    TBS_ADC0 ,                           /*!< ADC0                                */
    TBS_ADC1 ,                           /*!< ADC1                                */
    TBS_VCC  ,                           /*!< 电源电压                            */
#if  defined  HT501x  ||  defined  HT502x
    TBS_ADC2 ,                           /*!< ADC2                                */
#endif
    TBS_ADC3 ,                           /*!< ADC0                                */
    TBS_ADC4 ,                           /*!< ADC1                                */
}TBS_SubModuleTypeDef;                   /*!< end of group TBS_SubModuleTypeDef   */

typedef enum
{
    CMP_VBAT =0,                         /*!< 电池电压                            */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT501x  ||  defined  HT502x
    CMP_ADC0 ,                           /*!< ADC0                                */
#endif
#if  defined  HT501x  ||  defined  HT502x
    CMP_ADC1 ,                           /*!< ADC1                                */
#endif
}TBS_CMPRegTypeDef;                      /*!< end of group TBS_CMPRegTypeDef      */

typedef enum                             /* 目前只限温度测量 */
{
    TBS_ADCFilterCnt_1 = 0x0000,         /*!< ADC输出 1次滤�?                     */
    TBS_ADCFilterCnt_2 = 0x0100,         /*!< ADC输出 2次滤�?                     */
    TBS_ADCFilterCnt_4 = 0x0200,         /*!< ADC输出 4次滤�?                     */
    TBS_ADCFilterCnt_8 = 0x0300,         /*!< ADC输出 8次滤�?                     */
}TBS_ADCFilterCntTypeDef;                /*!< end of group TBS_ADCFilterCntTypeDef */






extern void ADC_Init(void);
extern int16_t ReadTempDetection(TBS_SubModuleTypeDef SubModule);



#endif











