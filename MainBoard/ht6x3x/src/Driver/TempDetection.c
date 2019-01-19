/*TempDetection.c
* 2019-01-08
* Copyright(C) 2017
* quqian@chargerlink.com
*/

#include "includes.h"
#include "TempDetection.h"
#include "server.h"



const float T2 = (273.15+25.0);//T2
const float Ka = 273.15;

//Rt = R *EXP(B*(1/T1-1/T2))
float ConvertTemp(float Rt)
{
	const float Rp = 100000.0; //100K
	const float Bx = 4250;//B
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
* ����˵��: ����TBS��ģ��
*
* ��ڲ���: TBS_SubModule    TBS�ڸ�����ģ�飬����Ϊ���²����������
*                             @arg TBS_TBSCON_TMPEn      ���¶Ȳ���ģ��
*                             @arg TBS_TBSCON_VBATEn     ����ص�ѹ����ģ��
*                             @arg TBS_TBSCON_ADC0En     ��ADC0����ģ��
*                             @arg TBS_TBSCON_ADC1En     ��ADC1����ģ��
*                             @arg TBS_TBSCON_VCCEn      ����Դ��ѹ����ģ��
*                             @arg TBS_TBSCON_ADC2En     ��ADC2����ģ��      (for HT501x, HT502x)
*                             @arg TBS_TBSCON_VREFEn     ��Vref����ģ��      (for HT501x, HT502x)
*                             @arg TBS_TBSCON_FILTER     : ����¶Ȳ�����Filter�������  (for HT6x2x, HT6x3x, HT502x)
*                             @arg TBS_TBSCON_ADC3En     : ADC3����ģ��      (for HT6x2x, HT6x3x)
*                             @arg TBS_TBSCON_ADC4En     : ADC4����ģ��      (for HT6x2x, HT6x3x)
*                             @arg TBS_TBSCON_ADC5En     : ADC5����ģ��      (for HT6x2x, HT6x3x)
*                             @arg TBS_TBSCON_TBSCLKSEL  : TBS ����ʱ��Դѡ�����λ  (for HT6x3x)
*
*           NewState         = ENABLE�� ģ��ʹ��
*                            = DISABLE��ģ��ر�
*
* ���ز���: ��
*
* ����˵��: 1) ����RTC�����漰�¶ȴ�������������ر��¶ȴ�����ģ�顣
*           2) ������ʹ�ܵ�ص�ѹ�Ƚ�֮ǰ���ȵ��ú���HT_TBS_BatCmpThresholdSet() ���úõ�ص�ѹ�Ƚ���ֵ,
*           3) ��ص�ѹ�ȽϹ���������ȷִ�У���ʹ�ܵ�ص�ѹ����ģ��
**********************************************************************************************************/
void HT_TBSConfig(uint32_t TBS_SubModule, FunctionalState NewState)
{
    /*  assert_param  */
    uint32_t tempreg;

    tempreg = HT_TBS->TBSCON & TBS_TBSCON_xEn;

    if(HT_CMU->CLKSTA & CMU_CLKSTA_LFSEL)
    {
        tempreg |= 0x6d60;                       /*!< MEMS TPS Ĭ������         */
    }
    else
    {
#if defined HT501x
        tempreg |= 0x0100;
#elif  defined HT502x
        tempreg |= 0x0101;
#else
        tempreg |= 0x6540;                       /*!< OSC TPS Ĭ������         */
#endif
    }

    TBS_SubModule &= TBS_TBSCON_xEn;
    if (NewState != DISABLE)
    {
        tempreg |= (uint32_t)TBS_SubModule;      /*!< ʹ��TBSģ��                 */
    }
    else
    {
        tempreg &= ~(uint32_t)TBS_SubModule;     /*!< �ر�TBSģ��                 */
    }

    HT_TBS->TBSCON = tempreg;                    /*!< д�Ĵ���                    */
}



/**********************************************************************************************************
*                                 SET SubModule OPEN PERIOD
*
* ����˵��: ������ģ���Ƶ������
*
* ��ڲ���: PRDModule    : ��ģ�����ڿ��ƣ�����Ϊ���²���
*                            @arg TMPPRD      ���¶Ȳ�������
*                            @arg VBATPRD     ����ص�ѹ��������
*                            @arg ADC0PRD     ��ADC0��������
*                            @arg ADC1PRD     ��ADC1��������
*                            @arg VCCPRD      ��VCC��������
*                            @arg ADC2PRD     ��ADC2��������    (for HT501x, HT502x)
*                            @arg VREFPRD     ��VREF��������    (for HT501x, HT502x)
*                            @arg ADC3PRD     ��ADC3��������    (for HT6x2x, HT6x3x)
*                            @arg ADC4PRD     ��ADC4��������    (for HT6x2x, HT6x3x)
*                            @arg ADC5PRD     ��ADC5��������    (for HT6x2x, HT6x3x)
*
*           Period       : �����趨�����ݲ�ͬоƬ������Ϊ���²���
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
* ���ز���: ��
*
* ����˵��: ��
**********************************************************************************************************/
void HT_TBS_PeriodSet(TBS_PeriodTypeDef PRDModule, uint32_t Period)
{
    /*  assert_param  */
     uint32_t tempreg = (HT_TBS->TBSPRD & (~PRDModule));
     tempreg |= Period;
     HT_TBS->TBSPRD = tempreg;                             /*!< ���ò�������           */
}


/**********************************************************************************************************
*                                     SET COMPARE THRESHOLD
*
* ����˵��: ���ñȽ���ֵ
*
* ��ڲ���: CmpReg    �ȽϼĴ���������Ϊ���²���
*                       @arg CMP_VBAT     ����ص�ѹ�ȽϼĴ���
*                       @arg CMP_ADC0     ��ADC0�ȽϼĴ���      (for HT501x, HT502x, HT6x2x, HT6x3x)
*                       @arg CMP_ADC1     ��ADC1�ȽϼĴ���      (for HT501x, HT502x)
*
* ���ز���: ��
*
* ����˵��: ��
**********************************************************************************************************/
void HT_TBS_CMPThresholdSet(TBS_CMPRegTypeDef CmpReg, int16_t Threshold)
{
    /*  assert_param  */
    if(CmpReg == CMP_VBAT)
        HT_TBS->VBATCMP = Threshold;                /*!< ���õ�ص�ѹ�Ƚ���ֵ           */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT501x  ||  defined  HT502x
    else if(CmpReg == CMP_ADC0)
        HT_TBS->ADC0CMP = Threshold;                /*!< ����ADC0�Ƚ���ֵ               */
#endif
#if  defined  HT501x  ||  defined  HT502x
    else if(CmpReg == CMP_ADC1)
        HT_TBS->ADC1CMP = Threshold;                /*!< ����ADC1�Ƚ���ֵ               */
#endif
}

/**********************************************************************************************************
*                                     GET MEASURED VALUES
*
* ����˵��: ��ȡ����ֵ
*
* ��ڲ���: SubModule    : ��ģ��ѡ��
*
* ���ز���: ��ѡ����ģ�����ֵ������ֵΪ16λ�з�����
*
* ����˵��: ��
**********************************************************************************************************/
int16_t HT_TBS_ValueRead(TBS_SubModuleTypeDef SubModule)
{
    /*  assert_param  */
    switch(SubModule)
    {
        case TBS_TMP:
            return  (int16_t)HT_TBS->TMPDAT;       /*!< �����¶Ȳ���ֵ           */
        case TBS_VBAT:
            return  (int16_t)HT_TBS->VBATDAT;      /*!< ���ص�ص�ѹ����ֵ       */
        case TBS_ADC0:
            return  (int16_t)HT_TBS->ADC0DAT;      /*!< ����ADC0����ֵ           */
        case TBS_ADC1:
            return  (int16_t)HT_TBS->ADC1DAT;      /*!< ����ADC1����ֵ           */
#if defined HT501x  ||  defined HT502x
        case TBS_ADC2:
            return  (int16_t)HT_TBS->ADC2DAT;      /*!< ����ADC2����ֵ           */
#endif
		case TBS_ADC3:
            return  (int16_t)HT_TBS->ADC3DAT;      /*!< ����ADC3����ֵ           */
        case TBS_ADC4:
			return  (int16_t)HT_TBS->ADC4DAT;      /*!< ����ADC4����ֵ           */
        default:
            return  (int16_t)HT_TBS->VCCDAT;       /*!< ���ص�Դ��ѹ����ֵ       */
    }
}
/**********************************************************************************************************
*                                 ENABLE OR DISABLE TBS INTERRUPT
*
* ����˵��: ʹ�ܻ��߹ر�TBS�ж�
*
* ��ڲ���: ITEn       TBS�ж�����λ������Ϊ���²����������
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
*           NewState   = ENABLE�� ʹ���ж�
*                      = DISABLE���ر��ж�
* ���ز���: ��
*
* ����˵��: ��
**********************************************************************************************************/
void HT_TBS_ITConfig(uint32_t ITEn, FunctionalState NewState)
{
    /*  assert_param  */
    if (NewState != DISABLE)
    {
        HT_TBS->TBSIE |= (uint32_t)ITEn;            /*!< ʹ��TBS�ж�           */
    }
    else
    {
        HT_TBS->TBSIE &= ~(uint32_t)ITEn;           /*!< �ر�TBS�ж�           */
    }
}

/**********************************************************************************************************
*                            GET SPECIFIED TBS INTERRUPT FLAG STATUS
*
* ����˵��: ��ȡ��ӦTBS�жϱ�־״̬
*
* ��ڲ���: ITFlag     ��Ҫ����ĳ��TBS�жϣ�����Ϊ���²���:
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
* ���ز���: ITStatus    = SET��  ��Ӧ�жϱ�־����
*                       = RESET����Ӧ�жϱ�־δ����
*
* ����˵��: ��
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
* ����˵��: ���TBS�жϱ�־
*
* ��ڲ���: ITFlag     ��Ҫ�����ĳ��TBS�жϱ�־������Ϊ���²����������:
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
* ���ز���: ��
*
* ����˵��: ��
**********************************************************************************************************/
void HT_TBS_ClearITPendingBit(uint32_t ITFlag)
{
    /*  assert_param  */
    HT_TBS->TBSIF  &= ~((uint32_t)ITFlag);                  /*!< Clear TBS Interrupt Flag       */
}

/**********************************************************************************************************
*                                 SET TBS FILTER VALUE CNT
*
* ����˵��: ����TBS�˲�����
*
* ��ڲ���: FilterVal    : �˲�����
*
* ���ز���: ��
*
* ����˵��: ��
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
* ����˵��: ����TBSʱ��Դ
*
* ��ڲ���: ClkSel    : ʱ��Դ����
*
* ���ز���: result    : ʱ��ѡ����
*                       TBS_CLKSEL_SUCC       TBSʱ���л��ɹ�
*                       TBS_CLKSEL_PARA_ERR   ������������ʱ�����������
*                       TBS_CLKSEL_PLL_EN_ERR Ҫ���л�PLLʱ�ӣ���PLLʱ�ӹ����쳣
*
* ����˵��: ������HT_TBSConfigһ������
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
*                                   ���ٴ���TBS����(��һ�Σ��Զ�������)
*
* ����˵��: ���ٴ���TBS����
*
* ��ڲ���: TriChn     ����ͨ��:
*                      TBS_TRI_TMP
*                      TBS_TRI_VBAT
*                      TBS_TRI_ADC0
*                      TBS_TRI_ADC1
*                      TBS_TRI_VCC
*                      TBS_TRI_ADC3
*                      TBS_TRI_ADC4
*                      TBS_TRI_ADC5
*
* ���ز���: ��
*
* ����˵��: ��
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
����ԭ��:   void swap(int *a, int *b)    
��������:  �������ݵ�������
��������:   ��������ֵ
����ֵ  :   ��
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
����ԭ��:   void quicksort(int array[], int maxlen, int begin, int end)
��������:   array  ��Ҫ��������飬maxlen:������ܴ�С
            begin  ����Ŀ�ʼλ�ã�end����Ľ���λ��
��������:   ��������
����ֵ  :   ��
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
		OS_DELAY_MS(50);
	}
	quicksort(temp,10,0,9);
	VoltageValue = VoltageValue - temp[0];
    VoltageValue = VoltageValue - temp[9];
	
	VoltageValue = VoltageValue / 8;
	VoltageValue = (VoltageValue * 0.0259)  + 0.4276;
	CL_LOG("VoltageValue = %d.\n", VoltageValue);
    
    return VoltageValue;
}

float ReadResistanceVoltage(TBS_SubModuleTypeDef SubModule)
{
	float ResistVoltageValue = 0;
	
	ResistVoltageValue = ReadTempDetection(SubModule);
	ResistVoltageValue = ResistVoltageValue * TOTAL_RESISTANCE / 10;
	CL_LOG("ResistVoltageValue = %f.\n", ResistVoltageValue);

	return ResistVoltageValue;
}

float ReadResistanceValue(TBS_SubModuleTypeDef SubModule)
{
	float ResistVoltageValue = 0;
	float Rt;
	float ntcTemp;
	
	ResistVoltageValue = ReadResistanceVoltage(SubModule);
	Rt = (float)(20000 * ResistVoltageValue) / (3300 - ResistVoltageValue);
	CL_LOG("���� = %f.\n", Rt);
	ntcTemp = ConvertTemp(Rt);
	CL_LOG("�¶�ntcTemp = %f.\n", ntcTemp);
    
    return ntcTemp;
}



