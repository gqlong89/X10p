#include "relayCtrlTask.h"
#include "gun.h"
#include "includes.h"
#include "emu.h"
#include "server.h"
#include "simuart.h"



extern Gun_GPIO_Port_t Gun_GPIO_PortTable[GUN_NUM_MAX];
extern QueueHandle_t sem;

static __IO uint8_t  ATTFlag = 0; 
static __IO uint16_t OpenlFlag = 0;
static __IO uint16_t CloselFlag = 0;
static __IO uint8_t timerHandle = 0;
static __IO uint8_t operate = 0; //0--�ؼ̵���  1--���̵���



void EXTI5_CallBack(void)
{
	if(READ_ATT_IRQ() == 0)//��ѹ������ж�
	{
        //��⵽������жϣ���Ҫ����6.5ms�Ķ�ʱ�������ڿ��̵�������8.5ms�Ķ�ʱ�������ڹؼ̵�����
        if(OpenlFlag != 0)	//���̵���
        {
            operate = 1;
            StartRelayTimer(300);	//����ʱ����Ҫ���ݼ̵���ʱ�����
        }
        else if(CloselFlag != 0)	//�ؼ̵���
        {
            operate = 0;
            StartRelayTimer(300);	//����ʱ����Ҫ���ݼ̵���ʱ�����
        }
	}
}


void CrtlRelay_ON(void)
{
    uint8_t i = 0;
	
    while(OpenlFlag)
    {
        //���̵���
        if((OpenlFlag >> i) & 0x01)
        {
            GPIO_SetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
            clrbit(OpenlFlag, i);
        }
        i++;
    }
}

void CrtlRelay_OFF(void)
{
    uint8_t i = 0;
	
    while(CloselFlag)
    {
        //�ؼ̵���
        if((CloselFlag >> i) & 0x01)
        {
            GPIO_ResetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
            clrbit(CloselFlag, i);
        }
        i++;
    }
}

void relayOn_direct(uint8_t gunId,uint8_t on)
{
    uint8_t i = 0;
    
    if(gunId == 0)
    {
        for (i = 0; i < GUN_NUM_MAX; i++) 
        {
            if(on)
			{
                GPIO_SetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
               // printf("[ǹͷ %d �ѿ���].\n", i + 1);
            }
			else
			{
                GPIO_ResetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
               // printf("[ǹͷ %d �ѹر�].\n", i + 1);
            }
        }
        #if 1
        if(on)
        {
            printf("ǹͷȫ������.\n");
        }
        else
        {
            printf("ǹͷȫ���ر�.\n");
        }
        #endif
    }
	else
	{
        if(on)
		{
            GPIO_SetBits(Gun_GPIO_PortTable[gunId-1].port, Gun_GPIO_PortTable[gunId-1].pin);
            printf("[ǹͷ %d �ѿ���].\n", gunId);
        }
		else
		{
            GPIO_ResetBits(Gun_GPIO_PortTable[gunId-1].port, Gun_GPIO_PortTable[gunId-1].pin);
            printf("[ǹͷ %d �ѹر�].\n", gunId);
        }
    }
}


void StartRelayTimer(uint16_t period)
{
    if(timerHandle == 0)
    {
        timerHandle = 1;
		timer4_init(period);
    }
}

/*
*********************************************************************************************************
*                                         SET TIMER PERIOD VALUE
*
* ����˵��: �������ڼĴ���ֵ����յ�ǰ����ֵ
*
* ��ڲ���: TMRx               ֻ����HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5��һ��
*
*           TimerPeriod        ��ʱ������ֵ
*
* ���ز���: ��
*
* ����˵��: ��
*********************************************************************************************************
*/
void HT_TMR_PeriodSet(HT_TMR_TypeDef* TMRx, uint16_t TimerPeriod)
{
    /*  assert_param  */
    TMRx->TMRPRD = TimerPeriod;                             /*!< ���ö�ʱ�����ڼĴ���        */
    TMRx->TMRCNT = 0;                                       /*!< ��ն�ʱ�������Ĵ���        */
}

/*
*********************************************************************************************************
*                                 ENABLE OR DISABLE TIMER INTERRUPT
*
* ����˵��: ʹ�ܻ��߹ر�TIMER�ж�
*
* ��ڲ���: TMRx       ֻ����HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5��һ��
*
*           ITEn       TIMER�ж�����λ������Ϊ���²����������
*                        @arg TMR_TMRIE_PRDIE
*                        @arg TMR_TMRIE_CAPIE
*                        @arg TMR_TMRIE_CMPIE
*                        @arg TMR_TMRIE_ACIE    (only for HT502x, HT6x2x, HT6x3x)
*
*           NewState   = ENABLE�� ʹ���ж�
*                      = DISABLE���ر��ж�
* ���ز���: ��
*
* ����˵��: ��
*********************************************************************************************************
*/
void HT_TMR_ITConfig(HT_TMR_TypeDef* TMRx, uint16_t ITEn, FunctionalState NewState)
{
    /*  assert_param  */
    if (NewState != DISABLE)
    {
        TMRx->TMRIE |= (uint32_t)ITEn;               /*!< ʹ��TIMER�ж�           */
    }
    else
    {
        TMRx->TMRIE &= ~(uint32_t)ITEn;              /*!< �ر�TIMER�ж�           */
    }
}

/*
*********************************************************************************************************
*                            GET SPECIFIED TIMER INTERRUPT FLAG STATUS
*
* ����˵��: ��ȡ��ӦTIMER�жϱ�־״̬
*
* ��ڲ���: TMRx       ֻ����HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5��һ��
*
*           ITFlag     ��Ҫ����ĳ��TIMER�жϣ�����Ϊ���²���:
*                        @arg TMR_TMRIF_PRDIF
*                        @arg TMR_TMRIF_CAPIF
*                        @arg TMR_TMRIF_CMPIF
*                        @arg TMR_TMRIF_ACIF    (only for HT6x2x)
*
* ���ز���: ITStatus    = SET��  ��Ӧ�жϱ�־����
*                       = RESET����Ӧ�жϱ�־δ����
*
* ����˵��: ��
*********************************************************************************************************
*/
ITStatus HT_TMR_ITFlagStatusGet(HT_TMR_TypeDef* TMRx, uint16_t ITFlag)
{
    /*  assert_param  */
    if (TMRx->TMRIF & ITFlag)
    {
        return SET;                        /*!< TIMER Interrupt Flag is set   */
    }
    else
    {
        return RESET;                      /*!< TIMER Interrupt Flag is reset */
    }
}

/*
*********************************************************************************************************
*                                   CLEAR TIMER INTERRUPT FLAG
*
* ����˵��: ���TIMER�жϱ�־
*
* ��ڲ���: TMRx       ֻ����HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5��һ��
*
*           ITFlag     ��Ҫ�����ĳ��TIMER�жϱ�־������Ϊ���²����������:
*                        @arg TMR_TMRIF_PRDIF
*                        @arg TMR_TMRIF_CAPIF
*                        @arg TMR_TMRIF_CMPIF
*                        @arg TMR_TMRIF_ACIF    (only for HT6x2x, HT502x)
*
* ���ز���: ��
*
* ����˵��: ��
*********************************************************************************************************
*/
void HT_TMR_ClearITPendingBit(HT_TMR_TypeDef* TMRx, uint16_t ITFlag)
{
    /*  assert_param  */
#if  defined  HT6x1x  ||  defined  HT501x                /*!< HT6x1x, HT501x                  */
    ITFlag |= 0x08;
#endif                                                   /*!< HT6x2x, HT502x                  */
    TMRx->TMRIF  &= ~((uint32_t)ITFlag);                 /*!< Clear TIMER Interrupt Flag      */
}

/*
*********************************************************************************************************
*                                         INITIAL TIMEREXT MODULE
*
* ����˵��: ��ʼ��TIMERģ��
*
* ��ڲ���: TMRx               ֻ����HT_TMR4/HT_TMR5��һ��
*
*           TMR_InitStruct     TIMER��ʼ���ṹ��ָ�룬��Ҫ����4������:
*                              1) TimerSource  : ��ʱ��ʱ��Դѡ��
*                              2) TimerMode    : ��ʱ������ģʽ
*                              3) TimerPeriod  : ��ʱ����ʱ��������
*                              4) TimerCompare : ��ʱ���ȽϼĴ�������
*                              5) TimerPreDiv  : ��ʱ��Ԥ��Ƶ����
*
* ���ز���: ��
*
* ����˵��: �û�������TIMER�Ĵ���ǰӦ��ʹ��TIMERģ�飬����μ�HT_CMU_ClkCtrl1Config()����
*********************************************************************************************************
*/
#if  defined  HT6x2x  ||  defined  HT6x3x
void HT_TMRExt_Init(HT_TMR_TypeDef* TMRx, TMRExt_InitTypeDef* TMR_InitStruct)
{
    /*  assert_param  */
    uint32_t tempreg;

    if ((TMRx == HT_TMR4) || (TMRx == HT_TMR5))
    {
        TMRx->TMRCON &= (~TMR_TMRCON_CNTEN);               /*!< �رն�ʱ��ʹ��             */
        TMRx->TMRDIV = TMR_InitStruct->TimerPreDiv;        /*!< ���ö�ʱ��Ԥ��Ƶ��          */
       // TMRx->TMRPRD = TMR_InitStruct->TimerPeriod;        /*!< ���ö�ʱ�����ڼĴ���        */
        TMRx->TMRCMP = TMR_InitStruct->TimerCompare;       /*!< ���ö�ʱ���ȽϼĴ���        */
        TMRx->TMRCNT = 0;                                  /*!< ��ն�ʱ�������Ĵ���        */
     //   tempreg = TMR_TMRCON_CNTEN;                        /*!< ��ʱ��ʹ��                 */
        tempreg |= ((uint32_t)TMR_InitStruct->TimerMode|(uint32_t)TMR_InitStruct->TimerSource);
        TMRx->TMRCON = tempreg;                            /*!< ���ö�ʱ������ģʽ          */
    }
}
#endif

void TIM4_5_Cmd(HT_TMR_TypeDef* TMRx, int en)
{
	if(en)
	{
	    setbit(TMRx->TMRCON,0);
	}
	else
	{
	    clrbit(TMRx->TMRCON,0);
	}
}

int timer4_init(uint16_t period) 
{
#if 0
	EnWr_WPREG();
	setbit(HT_CMU->CLKCTRL1 ,2);//�򿪶�ʱ��4
					  //���ϼ���   //MODE���ڼ���
	HT_TMR2->TMRCON = (0x3<<1) | (0<<0);
	HT_TMR2->TMRDIV = 4400 - 1;  //0.1ms
	HT_TMR2->TMRPRD = 10000;		//1s
	HT_TMR2->TMRIE = 1;
	NVIC_EnableIRQ(TIMER_4_IRQn);
    DisWr_WPREG();
#endif

	TMRExt_InitTypeDef  TMRExt_InitStructure;

    HT_CMU_ClkCtrl1Config(CMU_CLKCTRL1_TMR4EN, ENABLE);            		/*!< ʹ��TIMER0ģ��              */

	//TMRExt_InitStructure.TimerSource = TMR_TMRCON_CLKSEL_PLL;            /*!< Timerѡ��ʱ��Դ       */
    TMRExt_InitStructure.TimerMode = TIMExtCycleTiming;                  /*!< Timer�趨Ϊ��ʱ������       */
    TMRExt_InitStructure.TimerPreDiv = 4400 - 1;                          	/*!< Timerʱ�ӵ���ϵͳʱ��       */
    TMRExt_InitStructure.TimerPeriod = 0x1000;                        	/*!< ��ʱ�����趨                */
    TMRExt_InitStructure.TimerCompare = 0x00;                         	/*!< �ȽϼĴ����趨              */
	HT_TMRExt_Init(HT_TMR4, &TMRExt_InitStructure);
	HT_TMR_PeriodSet(HT_TMR4, period);									//�������ڼĴ���ֵ����յ�ǰ����ֵ
	HT_TMR_ITConfig(HT_TMR4, TMR_TMRIE_PRDIE, ENABLE);
	NVIC_EnableIRQ(TIMER_4_IRQn);
	TIM4_5_Cmd(HT_TMR4, ENABLE);
	
	return 0;
}


void openRelay_Async(uint8_t gunId,uint8_t on)
{
    if(gunId == 0)
    {
        if(on)
		{
            OpenlFlag = 0x0FFF;
        }
		else
		{
            CloselFlag = 0x0FFF;
        }
    }
    else 
    {
        if(on)
		{
            setbit(OpenlFlag,(gunId-1));
        }
		else
		{
            setbit(CloselFlag,(gunId-1));
        }
    }
	#if 1
	setbit(gChgInfo.ZeroDetectFlag, (gunId - 1));
     CL_LOG("�����ź��� \n");
	#else
    if(xSemaphoreGive(sem) != pdPASS)
    {
        //�ź�������ʧ��  -- ֱ�ӿ��Ƽ̵���
        PRINTF("�ź�������ʧ��  -- ֱ�ӿ��Ƽ̵���\n");
        relayOn_direct(gunId,on);
    }
	#endif
}

void RelayCtrl(uint8_t gunId,uint8_t on)
{
    if(EMUIFIFlag == 1)
	{
	//	CL_LOG("�߹������ �߹������ �߹������ \n");
        openRelay_Async(gunId, on);
    }
    else
    {
     //   CL_LOG("�߹�ֱ�ӿ��Ƽ̵��� \n");
        //����оƬδ��ʼ���ɹ�  -- ֱ�ӿ��Ƽ̵���
        relayOn_direct(gunId, on);
    }
}


int ReadEMUIF(int no)
{
	uint8_t data[4];
	
	if(HT7017_Read(no,VAR_EMUIF,data) != CL_FAIL)
	{
		uint32_t reg = (uint32_t)((data[0] << 16)|(data[1] << 8) | data[2]);
		
		return reg;
	}
	
	return CL_FAIL;
}

void RelayCtrlTask(void)
#if 0
{
    uint32_t tick = 0;
    uint32_t i = 0;
	
    while(1)
    {
        vTaskDelay(100);
     	gChgInfo.ZeroDetectFlag = gChgInfo.ZeroDetectFlag & 0x0fff;
     	if(gChgInfo.ZeroDetectFlag)
        {
        //	clrbit(gChgInfo.ZeroDetectFlag, (gunId - 1));
        	gChgInfo.ZeroDetectFlag = 0;
		
            //�̵�����
            if((OpenlFlag != 0) || (CloselFlag != 0))
            {
                ATTFlag = 0;
                //���жϱ�־
				for(i = 0; i < 3; i++)
				{
					if(ReadEMUIF(EMUID[6]) == CL_OK)
					{
						break;
					}
				}
                tick = xTaskGetTickCount();
                while(1)
                {
                    if(ATTFlag)
                    {
                        PRINTF("llllllllllllllllll \n");
                        break;
                    }
                    //���ȴ�130ms (120ms�����+10ms������ʱ)
                    if(xTaskGetTickCount() > (uint32_t)(tick + 130))
                    {
                    	PRINTF("mmmmmmmmmmmmmmǿ�ƿ��Ƽ̵���mmmmmmmmmmmmmmmm\n");
                        //�����ʧ��-ǿ�ƿ��Ƽ̵���
                        if(operate == 0)
                        {
                            CrtlRelay_OFF();//��
                        }
                        else if(operate == 1)
                        {
                            CrtlRelay_ON();//���Ƽ̵���-��
                        }
                        break;
                    }
                    vTaskDelay(10);
                }
            }
        }
    }
}
#else
{
    static uint32_t tick = 0;
    uint32_t i = 0;
	
   // while(1)
    {
     	gChgInfo.ZeroDetectFlag = gChgInfo.ZeroDetectFlag & 0x0fff;
     	if(gChgInfo.ZeroDetectFlag)
        {
        //	clrbit(gChgInfo.ZeroDetectFlag, (gunId - 1));
        	gChgInfo.ZeroDetectFlag = 0;
		
            //�̵�����
            if((OpenlFlag != 0) || (CloselFlag != 0))
            {
                ATTFlag = 0;
                //���жϱ�־
				for(i = 0; i < 3; i++)
				{
					if(ReadEMUIF(EMUID[6]) == CL_OK)
					{
						break;
					}
				}
                tick = xTaskGetTickCount();
                gChgInfo.CloseFlag = 0xa5;
            }
        }

		if((((OpenlFlag & 0x0fff) != 0) || ((CloselFlag & 0x0fff) != 0))/* && (gChgInfo.CloseFlag == 0xa5) */)
        {
            if(ATTFlag)
            {
                PRINTF_LOG("llllllllllllllllll \n");
				tick = xTaskGetTickCount();
				gChgInfo.CloseFlag = 0;
            }
			else
			{
				//���ȴ�330ms (320ms�����+10ms������ʱ)
	            if(xTaskGetTickCount() > (uint32_t)(tick + 330))
	            {
	            	PRINTF_LOG("mmmmmmmmmmmmmmǿ�ƿ��Ƽ̵���mmmmmmmmmmmmmmmm\n");
	                //�����ʧ��-ǿ�ƿ��Ƽ̵���
	                if(operate == 0)
	                {
	                    CrtlRelay_OFF();//��
	                }
	                else if(operate == 1)
	                {
	                    CrtlRelay_ON();//���Ƽ̵���-��
	                }
					gChgInfo.CloseFlag = 0;
	              //  break;
	            }
			}
        }
		else
		{
			tick = xTaskGetTickCount();
		}
		
	//	vTaskDelay(100);
    }
}

#endif

void TIMER_4_IRQHandler(void)
{
	if(SET == HT_TMR_ITFlagStatusGet(HT_TMR2, TMR_TMRIF_PRDIF))                /*!< �����ж�           */
    {
        HT_TMR_ClearITPendingBit(HT_TMR2, TMR_TMRIF_PRDIF);                    /*!< ����жϱ�־       */

        HT_TMR_ITConfig(HT_TMR4, TMR_TMRIE_PRDIE, DISABLE);
        NVIC_DisableIRQ(TIMER_4_IRQn);
		TIM4_5_Cmd(HT_TMR4, DISABLE);
		
        if(operate == 0)
        {
            CrtlRelay_OFF();//��
        }
        else if(operate == 1)
        {
            CrtlRelay_ON();//���Ƽ̵���-��
        }
        timerHandle = 0;
		ATTFlag = 1;
    }
#if 0
    if(SET == HT_TMR_ITFlagStatusGet(HT_TMR2, TMR_TMRIF_CAPIF))                /*!< �����ж�           */
    {
		
        HT_TMR_ClearITPendingBit(HT_TMR2, TMR_TMRIF_CAPIF);                    /*!< ����жϱ�־       */
    }

    if(SET == HT_TMR_ITFlagStatusGet(HT_TMR2, TMR_TMRIF_CMPIF))                /*!< �Ƚ��ж�           */
    {
		
        HT_TMR_ClearITPendingBit(HT_TMR2, TMR_TMRIF_CMPIF);                    /*!< ����жϱ�־       */
    }
#endif
}

void EXTI5_IRQHandler(void)
{
    if(SET == HT_EXTIRise_ITFlagStatusGet(INT_EXTIF_RIF_INT5))         /*!< INT4�������ж�           */
    {

        HT_EXTIRise_ClearITPendingBit(INT_EXTIF_RIF_INT5);             /*!< ����жϱ�־             */
    }

    if(SET == HT_EXTIFall_ITFlagStatusGet(INT_EXTIF_FIF_INT5))         /*!< INT4�½����ж�           */
    {
        HT_EXTIFall_ClearITPendingBit(INT_EXTIF_FIF_INT5);             /*!< ����жϱ�־             */
        
        EXTI5_CallBack();
    }
}



