/*TempDetection.c
* 2019-01-08
* Copyright(C) 2017
* quqian@chargerlink.com
*/

#include "includes.h"
#include "TempDetection.h"
#include "server.h"
#include "sc8042.h"



//Rt = R *EXP(B*(1/T1-1/T2))
float ConvertTemp(float Rt)
{
	const float T2 = (273.15+25.0);//T2
	const float Ka = 273.15;
	const float Rp = 100000.0; 	//100K
	const float Bx = 4250;	//B
	float temp;
	
  //like this R=100000, T2=273.15+25,B=3470, RT=100000*EXP(3470*(1/T1-1/(273.15+25))
	temp = log(Rt / Rp);//ln(Rt/Rp)
	temp /= Bx;		//ln(Rt/Rp)/B
	temp += (1 / T2);
	temp = 1 / (temp);
	temp -= Ka;
	
	return temp;
}

//TempWithResistanceTypedef TempWithResistanceValue = {
////	{-40	, 42000},
////	{-35	, 30000},
////	{-30	, 22000},
////	{-25	, 16000},
////	{-20	, 12000},
////	{-15	, 9000},
////	{-10	, 6000},
////	{-5 	, 5000},
////	{0 		, 3800},
//	{5		, 2900},
//	{10		, 2000},
//	{15		, 1600},
//	{20		, 1200},
//	{25		, 1000},
//	{30		, 850},
//	{35		, 650},
//	{40		, 500},
//	{45		, 400},
//	{50		, 350},
//	{55		, 290},
//	{60		, 240},
//	{65		, 200},
//	{70		, 160},
//	{75		, 130},
//	{80		, 110},
//	{85		, 90},
//	{90		, 80},
//	{95		, 60},
//	{100	, 50},
//	{105	, 48},
//	{110	, 40},
//	{115	, 35},
//	{120	, 30}
//};

/**********************************************************************************************************
*                                     CONFIGUE TBS MODULE
*
* 函数说明: 配置TBS子模块
*
* 入口参数: TBS_SubModule    TBS内各个子模块，可以为以下参数或其组合
*                             @arg TBS_TBSCON_TMPEn      ：温度测量模块
*                             @arg TBS_TBSCON_VBATEn     ：电池电压测量模块
*                             @arg TBS_TBSCON_ADC0En     ：ADC0测量模块
*                             @arg TBS_TBSCON_ADC1En     ：ADC1测量模块
*                             @arg TBS_TBSCON_VCCEn      ：电源电压测量模块
*                             @arg TBS_TBSCON_ADC2En     ：ADC2测量模块      (for HT501x, HT502x)
*                             @arg TBS_TBSCON_VREFEn     ：Vref测量模块      (for HT501x, HT502x)
*                             @arg TBS_TBSCON_FILTER     : 针对温度测量的Filter输出配置  (for HT6x2x, HT6x3x, HT502x)
*                             @arg TBS_TBSCON_ADC3En     : ADC3测量模块      (for HT6x2x, HT6x3x)
*                             @arg TBS_TBSCON_ADC4En     : ADC4测量模块      (for HT6x2x, HT6x3x)
*                             @arg TBS_TBSCON_ADC5En     : ADC5测量模块      (for HT6x2x, HT6x3x)
*                             @arg TBS_TBSCON_TBSCLKSEL  : TBS 工作时钟源选择控制位  (for HT6x3x)
*
*           NewState         = ENABLE： 模块使能
*                            = DISABLE：模块关闭
*
* 返回参数: 无
*
* 特殊说明: 1) 由于RTC补偿涉及温度传感器，不建议关闭温度传感器模块。
*           2) 建议在使能电池电压比较之前，先调用函数HT_TBS_BatCmpThresholdSet() 设置好电池电压比较阈值,
*           3) 电池电压比较功能若想正确执行，需使能电池电压测量模块
**********************************************************************************************************/
void HT_TBSConfig(uint32_t TBS_SubModule, FunctionalState NewState)
{
    /*  assert_param  */
    uint32_t tempreg;

    tempreg = HT_TBS->TBSCON & TBS_TBSCON_xEn;

    if(HT_CMU->CLKSTA & CMU_CLKSTA_LFSEL)
    {
        tempreg |= 0x6d60;                       /*!< MEMS TPS 默认配置         */
    }
    else
    {
#if defined HT501x
        tempreg |= 0x0100;
#elif  defined HT502x
        tempreg |= 0x0101;
#else
        tempreg |= 0x6540;                       /*!< OSC TPS 默认配置         */
#endif
    }

    TBS_SubModule &= TBS_TBSCON_xEn;
    if (NewState != DISABLE)
    {
        tempreg |= (uint32_t)TBS_SubModule;      /*!< 使能TBS模块                 */
    }
    else
    {
        tempreg &= ~(uint32_t)TBS_SubModule;     /*!< 关闭TBS模块                 */
    }

    HT_TBS->TBSCON = tempreg;                    /*!< 写寄存器                    */
}



/**********************************************************************************************************
*                                 SET SubModule OPEN PERIOD
*
* 函数说明: 设置子模块打开频率周期
*
* 入口参数: PRDModule    : 子模块周期控制，可以为以下参数
*                            @arg TMPPRD      ：温度测量周期
*                            @arg VBATPRD     ：电池电压测量周期
*                            @arg ADC0PRD     ：ADC0测量周期
*                            @arg ADC1PRD     ：ADC1测量周期
*                            @arg VCCPRD      ：VCC测量周期
*                            @arg ADC2PRD     ：ADC2测量周期    (for HT501x, HT502x)
*                            @arg VREFPRD     ：VREF测量周期    (for HT501x, HT502x)
*                            @arg ADC3PRD     ：ADC3测量周期    (for HT6x2x, HT6x3x)
*                            @arg ADC4PRD     ：ADC4测量周期    (for HT6x2x, HT6x3x)
*                            @arg ADC5PRD     ：ADC5测量周期    (for HT6x2x, HT6x3x)
*
*           Period       : 周期设定，根据不同芯片，可以为以下参数
*                                   HT501x  HT502x                  HT6x1x  HT6x2x  HT6x3x
*                                 TMPPRD:                           TMPPRD:
*                            @arg TBS_TBSPRD_TMPPRD_1S              TBS_TBSPRD_TMPPRD_OSC0P5S_MEM8S
*                            @arg TBS_TBSPRD_TMPPRD_8S              TBS_TBSPRD_TMPPRD_OSC1S_MEM4S
*                            @arg TBS_TBSPRD_TMPPRD_32S             TBS_TBSPRD_TMPPRD_OSC2S_MEM2S
*                            @arg TBS_TBSPRD_TMPPRD_125mS           TBS_TBSPRD_TMPPRD_OSC4S_MEM1S
*                                                                   TBS_TBSPRD_TMPPRD_OSC8S_MEM0P5S
*                                 VBATPRD:                          TBS_TBSPRD_TMPPRD_OSC16S_MEM125mS
*                            @arg TBS_TBSPRD_VBATPRD_1S             TBS_TBSPRD_TMPPRD_OSC32S_MEM31mS
*                            @arg TBS_TBSPRD_VBATPRD_4S             TBS_TBSPRD_TMPPRD_OSC64S_MEM8mS
*                            @arg TBS_TBSPRD_VBATPRD_8S
*                            @arg TBS_TBSPRD_VBATPRD_32S            VBATPRD:
*                                                                   TBS_TBSPRD_VBATPRD_1S
*                                 VCCPRD:                           TBS_TBSPRD_VBATPRD_2S
*                            @arg TBS_TBSPRD_VCCPRD_1S              TBS_TBSPRD_VBATPRD_8S
*                            @arg TBS_TBSPRD_VCCPRD_4S              TBS_TBSPRD_VBATPRD_16S
*                            @arg TBS_TBSPRD_VCCPRD_8S
*                            @arg TBS_TBSPRD_VCCPRD_32S             ADC0PRD:
*                                                                   TBS_TBSPRD_ADC0PRD_1S
*                                 ADC0PRD:                          TBS_TBSPRD_ADC0PRD_2S
*                            @arg TBS_TBSPRD_ADC0PRD_1S             TBS_TBSPRD_ADC0PRD_8S
*                            @arg TBS_TBSPRD_ADC0PRD_4S             TBS_TBSPRD_ADC0PRD_16S
*                            @arg TBS_TBSPRD_ADC0PRD_8S
*                            @arg TBS_TBSPRD_ADC0PRD_32S            ADC1PRD:
*                                                                   TBS_TBSPRD_ADC1PRD_1S
*                                 ADC1PRD:                          TBS_TBSPRD_ADC1PRD_2S
*                            @arg TBS_TBSPRD_ADC1PRD_1S             TBS_TBSPRD_ADC1PRD_8S
*                            @arg TBS_TBSPRD_ADC1PRD_4S             TBS_TBSPRD_ADC1PRD_16S
*                            @arg TBS_TBSPRD_ADC1PRD_8S
*                            @arg TBS_TBSPRD_ADC1PRD_32S            VCCPRD:
*                                                                   TBS_TBSPRD_VCCPRD_1S
*                                 ADC2PRD:                          TBS_TBSPRD_VCCPRD_2S
*                            @arg TBS_TBSPRD_ADC2PRD_1S             TBS_TBSPRD_VCCPRD_8S
*                            @arg TBS_TBSPRD_ADC2PRD_4S             TBS_TBSPRD_VCCPRD_16S
*                            @arg TBS_TBSPRD_ADC2PRD_8S
*                            @arg TBS_TBSPRD_ADC2PRD_32S            ADC3PRD:(for HT6x2x, HT6x3x)
*                                                                   TBS_TBSPRD_ADC3PRD_1S
*                                 VREFPRD:                          TBS_TBSPRD_ADC3PRD_2S
*                            @arg TBS_TBSPRD_VREFPRD_1S             TBS_TBSPRD_ADC3PRD_8S
*                            @arg TBS_TBSPRD_VREFPRD_4S             TBS_TBSPRD_ADC3PRD_16S
*                            @arg TBS_TBSPRD_VREFPRD_8S
*                            @arg TBS_TBSPRD_VREFPRD_32S            ADC4PRD:(for HT6x2x, HT6x3x)
*                                                                   TBS_TBSPRD_ADC4PRD_1S
*                                                                   TBS_TBSPRD_ADC4PRD_2S
*                                                                   TBS_TBSPRD_ADC4PRD_8S
*                                                                   TBS_TBSPRD_ADC4PRD_16S
*
*                                                                   ADC5PRD:(for HT6x2x, HT6x3x)
*                                                                   TBS_TBSPRD_ADC5PRD_1S
*                                                                   TBS_TBSPRD_ADC5PRD_2S
*                                                                   TBS_TBSPRD_ADC5PRD_8S
*                                                                   TBS_TBSPRD_ADC5PRD_16S
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_TBS_PeriodSet(TBS_PeriodTypeDef PRDModule, uint32_t Period)
{
    /*  assert_param  */
     uint32_t tempreg = (HT_TBS->TBSPRD & (~PRDModule));
     tempreg |= Period;
     HT_TBS->TBSPRD = tempreg;                             /*!< 配置测量周期           */
}


/**********************************************************************************************************
*                                     SET COMPARE THRESHOLD
*
* 函数说明: 设置比较阈值
*
* 入口参数: CmpReg    比较寄存器，可以为以下参数
*                       @arg CMP_VBAT     ：电池电压比较寄存器
*                       @arg CMP_ADC0     ：ADC0比较寄存器      (for HT501x, HT502x, HT6x2x, HT6x3x)
*                       @arg CMP_ADC1     ：ADC1比较寄存器      (for HT501x, HT502x)
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_TBS_CMPThresholdSet(TBS_CMPRegTypeDef CmpReg, int16_t Threshold)
{
    /*  assert_param  */
    if(CmpReg == CMP_VBAT)
        HT_TBS->VBATCMP = Threshold;                /*!< 设置电池电压比较阈值           */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT501x  ||  defined  HT502x
    else if(CmpReg == CMP_ADC0)
        HT_TBS->ADC0CMP = Threshold;                /*!< 设置ADC0比较阈值               */
#endif
#if  defined  HT501x  ||  defined  HT502x
    else if(CmpReg == CMP_ADC1)
        HT_TBS->ADC1CMP = Threshold;                /*!< 设置ADC1比较阈值               */
#endif
}

/**********************************************************************************************************
*                                     GET MEASURED VALUES
*
* 函数说明: 获取测量值
*
* 入口参数: SubModule    : 子模块选择
*
* 返回参数: 被选中子模块测量值，返回值为16位有符号数
*
* 特殊说明: 无
**********************************************************************************************************/
int16_t HT_TBS_ValueRead(TBS_SubModuleTypeDef SubModule)
{
    /*  assert_param  */
    switch(SubModule)
    {
        case TBS_TMP:
            return  (int16_t)HT_TBS->TMPDAT;       /*!< 返回温度测量值           */
        case TBS_VBAT:
            return  (int16_t)HT_TBS->VBATDAT;      /*!< 返回电池电压测量值       */
        case TBS_ADC0:
            return  (int16_t)HT_TBS->ADC0DAT;      /*!< 返回ADC0测量值           */
        case TBS_ADC1:
            return  (int16_t)HT_TBS->ADC1DAT;      /*!< 返回ADC1测量值           */
#if defined HT501x  ||  defined HT502x
        case TBS_ADC2:
            return  (int16_t)HT_TBS->ADC2DAT;      /*!< 返回ADC2测量值           */
#endif
		case TBS_ADC3:
            return  (int16_t)HT_TBS->ADC3DAT;      /*!< 返回ADC3测量值           */
        case TBS_ADC4:
			return  (int16_t)HT_TBS->ADC4DAT;      /*!< 返回ADC4测量值           */
        default:
            return  (int16_t)HT_TBS->VCCDAT;       /*!< 返回电源电压测量值       */
    }
}
/**********************************************************************************************************
*                                 ENABLE OR DISABLE TBS INTERRUPT
*
* 函数说明: 使能或者关闭TBS中断
*
* 入口参数: ITEn       TBS中断设置位，可以为以下参数或其组合
*                        @arg TBS_TBSIE_TMPIE
*                        @arg TBS_TBSIE_VBATIE
*                        @arg TBS_TBSIE_ADC0IE
*                        @arg TBS_TBSIE_ADC1IE
*                        @arg TBS_TBSIE_VBATCMPIE
*                        @arg TBS_TBSIE_VCCIE
*                        @arg TBS_TBSIE_ADC2IE        (for HT501x, HT502x)
*                        @arg TBS_TBSIE_VREFIE        (for HT501x)
*                        @arg TBS_TBSIE_ADC0CMPIE     (for HT6x2x, HT6x3x, HT501x, HT502x)
*                        @arg TBS_TBSIE_ADC1CMPIE     (for HT501x, HT502x)
*                        @arg TBS_TBSIE_ADC3IE        (for HT6x2x, HT6x3x)
*                        @arg TBS_TBSIE_ADC4IE        (for HT6x2x, HT6x3x)
*                        @arg TBS_TBSIE_ADC5IE        (for HT6x2x, HT6x3x)
*
*           NewState   = ENABLE： 使能中断
*                      = DISABLE：关闭中断
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_TBS_ITConfig(uint32_t ITEn, FunctionalState NewState)
{
    /*  assert_param  */
    if (NewState != DISABLE)
    {
        HT_TBS->TBSIE |= (uint32_t)ITEn;            /*!< 使能TBS中断           */
    }
    else
    {
        HT_TBS->TBSIE &= ~(uint32_t)ITEn;           /*!< 关闭TBS中断           */
    }
}

/**********************************************************************************************************
*                            GET SPECIFIED TBS INTERRUPT FLAG STATUS
*
* 函数说明: 获取相应TBS中断标志状态
*
* 入口参数: ITFlag     想要检查的某个TBS中断，可以为以下参数:
*                        @arg TBS_TBSIE_TMPIF
*                        @arg TBS_TBSIE_VBATIF
*                        @arg TBS_TBSIE_ADC0IF
*                        @arg TBS_TBSIE_ADC1IF
*                        @arg TBS_TBSIE_VBATCMPIF
*                        @arg TBS_TBSIE_VCCIF
*                        @arg TBS_TBSIE_ADC2IF        (for HT501x, HT502x)
*                        @arg TBS_TBSIE_VREFIF        (for HT501x)
*                        @arg TBS_TBSIE_ADC0CMPIF     (for HT6x2x, HT6x3x, HT501x, HT502x)
*                        @arg TBS_TBSIE_ADC1CMPIF     (for HT501x, HT502x)
*                        @arg TBS_TBSIE_ADC3IF        (for HT6x2x, HT6x3x)
*                        @arg TBS_TBSIE_ADC4IF        (for HT6x2x, HT6x3x)
*                        @arg TBS_TBSIE_ADC5IF        (for HT6x2x, HT6x3x)
*
* 返回参数: ITStatus    = SET：  相应中断标志产生
*                       = RESET：相应中断标志未产生
*
* 特殊说明: 无
**********************************************************************************************************/
ITStatus HT_TBS_ITFlagStatusGet(uint32_t ITFlag)
{
    /*  assert_param  */
    if (HT_TBS->TBSIF & ITFlag)
    {
        return SET;                        /*!< TBS Interrupt Flag is set   */
    }
    else
    {
        return RESET;                      /*!< TBS Interrupt Flag is reset */
    }
}

/**********************************************************************************************************
*                                   CLEAR TBS INTERRUPT FLAG
*
* 函数说明: 清除TBS中断标志
*
* 入口参数: ITFlag     想要清除的某个TBS中断标志，可以为以下参数或其组合:
*                        @arg TBS_TBSIE_TMPIF
*                        @arg TBS_TBSIE_VBATIF
*                        @arg TBS_TBSIE_ADC0IF
*                        @arg TBS_TBSIE_ADC1IF
*                        @arg TBS_TBSIE_VBATCMPIF
*                        @arg TBS_TBSIE_VCCIF
*                        @arg TBS_TBSIE_ADC2IF        (for HT501x, HT502x)
*                        @arg TBS_TBSIE_VREFIF        (for HT501x)
*                        @arg TBS_TBSIE_ADC0CMPIF     (for HT6x2x, HT6x3x, HT501x, HT502x)
*                        @arg TBS_TBSIE_ADC1CMPIF     (for HT501x, HT502x)
*                        @arg TBS_TBSIE_ADC3IF        (for HT6x2x, HT6x3x)
*                        @arg TBS_TBSIE_ADC4IF        (for HT6x2x, HT6x3x)
*                        @arg TBS_TBSIE_ADC5IF        (for HT6x2x, HT6x3x)
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_TBS_ClearITPendingBit(uint32_t ITFlag)
{
    /*  assert_param  */
    HT_TBS->TBSIF  &= ~((uint32_t)ITFlag);                  /*!< Clear TBS Interrupt Flag       */
}

/**********************************************************************************************************
*                                 SET TBS FILTER VALUE CNT
*
* 函数说明: 设置TBS滤波次数
*
* 入口参数: FilterVal    : 滤波次数
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_TBS_FilCntConfig(TBS_ADCFilterCntTypeDef FilterVal)
{
    /*  assert_param  */
    HT_TBS->TBSCON &= (~TBS_TBSCON_FILTER);
    HT_TBS->TBSCON |= (FilterVal & TBS_TBSCON_FILTER);
}

#if  defined  HT6x3x
/**********************************************************************************************************
*                                 SELECT TBS CLOKK
*
* 函数说明: 设置TBS时钟源
*
* 入口参数: ClkSel    : 时钟源设置
*
* 返回参数: result    : 时钟选择结果
*                       TBS_CLKSEL_SUCC       TBS时钟切换成功
*                       TBS_CLKSEL_PARA_ERR   本函数被调用时输入参数错误
*                       TBS_CLKSEL_PLL_EN_ERR 要求切换PLL时钟，但PLL时钟工作异常
*
* 特殊说明: 建议与HT_TBSConfig一起配置
**********************************************************************************************************/
TBS_ClkSelReturnTypeDef HT_TBS_ClkSelConfig(TBS_ClkSelectTypeDef ClkSel)
{
    TBS_ClkSelReturnTypeDef result;

    /*  assert_param  */
    switch (ClkSel)
    {
        case TBS_CLKSEL_OSC:
        case TBS_CLKSEL_PLLDIV:
            if (ClkSel == TBS_CLKSEL_PLLDIV)
            {
                if (((HT_CMU->CLKCTRL0 & CMU_CLKCTRL0_PLLEN) == CMU_CLKCTRL0_PLLEN)
                    && ((HT_CMU->CLKSTA & CMU_CLKSTA_PLLLOCK) == CMU_CLKSTA_PLLLOCK)
                    && ((HT_CMU->CLKSTA & CMU_CLKSTA_PLLFLAG) != CMU_CLKSTA_PLLFLAG))
                {
                    HT_TBS->TBSCON |= (TBS_CLKSEL_PLLDIV & TBS_TBSCON_TBSCLKSEL);
                    result = TBS_CLKSEL_SUCC;
                }
                else
                {
                    HT_TBS->TBSCON &= (TBS_CLKSEL_OSC | (~TBS_TBSCON_TBSCLKSEL));
                    result = TBS_CLKSEL_PLL_EN_ERR;
                }
            }
            else
            {
                HT_TBS->TBSCON &= (TBS_CLKSEL_OSC | (~TBS_TBSCON_TBSCLKSEL));
                result = TBS_CLKSEL_SUCC;
            }
            break;
        default:
            result = TBS_CLKSEL_PARA_ERR;
            break;
    }

    return result;
}

/**********************************************************************************************************
*                                   快速触发TBS采样(仅一次，自动完成清空)
*
* 函数说明: 快速触发TBS采样
*
* 入口参数: TriChn     触发通道:
*                      TBS_TRI_TMP
*                      TBS_TRI_VBAT
*                      TBS_TRI_ADC0
*                      TBS_TRI_ADC1
*                      TBS_TRI_VCC
*                      TBS_TRI_ADC3
*                      TBS_TRI_ADC4
*                      TBS_TRI_ADC5
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_TBS_ADCTriSample(TBS_TriChnTypeDef TriChn)
{
    /*  assert_param  */
    HT_TBS->TRIREQ |= (TriChn& TBS_TRIREQ);
}
#endif

void ADC_Init(void)
{
	GPIO_Cfg(HT_GPIOG, GPIO_Pin_2, GPIO_Mode_AF2, GPIO_Input_Up, GPIO_Output_PP);
	GPIO_Cfg(HT_GPIOG, GPIO_Pin_3, GPIO_Mode_AF2, GPIO_Input_Up, GPIO_Output_PP);
	HT_TBSConfig(TBS_TBSCON_ADC3En | TBS_TBSCON_ADC4En, ENABLE);
//	HT_TBS->TBSCON |= (0x03 << 15);
	HT_TBS_FilCntConfig(TBS_ADCFilterCnt_1);
	HT_TBS_ClkSelConfig(TBS_CLKSEL_PLLDIV);
	HT_TBS_ITConfig(TBS_TBSIE_ADC3IE, DISABLE);
	HT_TBS_ITConfig(TBS_TBSIE_ADC4IE, DISABLE);
	HT_TBS_PeriodSet(ADC3PRD, TBS_TBSPRD_ADC3PRD_1S);
	HT_TBS_PeriodSet(ADC4PRD, TBS_TBSPRD_ADC4PRD_1S);
	HT_TBS_ADCTriSample(TBS_TRI_ADC3);
	HT_TBS_ADCTriSample(TBS_TRI_ADC4);
}

/*******************************************************
函数原型:   void swap(int *a, int *b)    
函数参数:  交换数据的亮变量
函数功能:   交换变量值
返回值  :   无
********************************************************/
void swap(int *a, int *b)    
{  
    int temp;  
  
    temp = *a;  
    *a = *b;  
    *b = temp;  
  
    return ;  
}

/*******************************************************
函数原型:   void quicksort(int array[], int maxlen, int begin, int end)
函数参数:   array  需要排序的数组，maxlen:数组的总大小
            begin  数组的开始位置，end数组的结束位置
函数功能:   快速排序
返回值  :   无
********************************************************/
void quicksort(int array[], int maxlen, int begin, int end)  
{  
    int i, j;  
  
    if(begin < end)  
    {  
        i = begin + 1;  
        j = end;        
            
        while(i < j)  
        {  
            if(array[i] > array[begin])  
            {  
                swap(&array[i], &array[j]);  
                j--;  
            }  
            else  
            {  
                i++;  
            }  
        }  
        if(array[i] >= array[begin])  
        {  
            i--;  
        }  
        swap(&array[begin], &array[i]);  
          
        quicksort(array, maxlen, begin, i);  
        quicksort(array, maxlen, j, end);  
    }  
}

//VADCIN0 = 0.0259*ADC0DATA + 0.4276 (mv)
int32_t ReadTempDetection(TBS_SubModuleTypeDef SubModule)
{
	int32_t VoltageValue = 0;
	uint8_t i = 0;
	int temp[10] = {0};
	
	for (i = 0; i < 10; i++) 
	{
		temp[i] = HT_TBS_ValueRead(SubModule);
		VoltageValue += temp[i];
		OS_DELAY_MS(20);
	}
	quicksort(temp,10,0,9);
	VoltageValue = VoltageValue - temp[0];
    VoltageValue = VoltageValue - temp[9];
	
	VoltageValue = VoltageValue / 8;
	VoltageValue = (VoltageValue * 0.0259)  + 0.4276;
//	CL_LOG("VoltageValue = %d.\n", VoltageValue);
    
    return VoltageValue;
}

float ReadResistanceVoltage(TBS_SubModuleTypeDef SubModule)
{
	float ResistVoltageValue = 0;
	
	ResistVoltageValue = ReadTempDetection(SubModule);
	ResistVoltageValue = ResistVoltageValue * TOTAL_RESISTANCE / 10;
//	CL_LOG("ResistVoltageValue = %f.\n", ResistVoltageValue);

	return ResistVoltageValue;
}

float ReadResistanceValue(TBS_SubModuleTypeDef SubModule)
{
	float ResistVoltageValue = 0;
	float Rt;
	float ntcTemp;
	const float Resist = 25000.0;
		
	ResistVoltageValue = ReadResistanceVoltage(SubModule);
	Rt = (float)(20000 * ResistVoltageValue) / (3300 - ResistVoltageValue);
//	CL_LOG("电阻1 = %f.\n", Rt);
	
	if(Rt >= Resist)
	{
		if(Rt > Resist)
		{
			SoundCode(210);
			OS_DELAY_MS(500);
			Rt = 10000000;
		}
		else
		{
			//TempNotice(ret);
			SoundCode(209);
			OS_DELAY_MS(500);
			Rt = 10000000;
		}
	//	CL_LOG("电阻2 = %f.\n", Rt);
	}
	else
	{
		Rt = 1 / ((1 / Rt) - (1 / Resist));
//	CL_LOG("电阻2 = %f.\n", Rt);
	}
	
	ntcTemp = ConvertTemp(Rt);
    
    return ntcTemp;
}



