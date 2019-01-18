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
static __IO uint8_t operate = 0; //0--关继电器  1--开继电器



void EXTI5_CallBack(void)
{
	if(READ_ATT_IRQ() == 0)//电压过零点中断
	{
        //检测到过零点中断，需要开启6.5ms的定时器（用于开继电器）和8.5ms的定时器（用于关继电器）
        if(OpenlFlag != 0)	//开继电器
        {
            operate = 1;
            StartRelayTimer(300);	//具体时间需要根据继电器时间调试
        }
        else if(CloselFlag != 0)	//关继电器
        {
            operate = 0;
            StartRelayTimer(300);	//具体时间需要根据继电器时间调试
        }
	}
}


void CrtlRelay_ON(void)
{
    uint8_t i = 0;
	
    while(OpenlFlag)
    {
        //开继电器
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
        //关继电器
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
               // printf("[枪头 %d 已开启].\n", i + 1);
            }
			else
			{
                GPIO_ResetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
               // printf("[枪头 %d 已关闭].\n", i + 1);
            }
        }
        #if 1
        if(on)
        {
            printf("枪头全部开启.\n");
        }
        else
        {
            printf("枪头全部关闭.\n");
        }
        #endif
    }
	else
	{
        if(on)
		{
            GPIO_SetBits(Gun_GPIO_PortTable[gunId-1].port, Gun_GPIO_PortTable[gunId-1].pin);
            printf("[枪头 %d 已开启].\n", gunId);
        }
		else
		{
            GPIO_ResetBits(Gun_GPIO_PortTable[gunId-1].port, Gun_GPIO_PortTable[gunId-1].pin);
            printf("[枪头 %d 已关闭].\n", gunId);
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
* 函数说明: 设置周期寄存器值并清空当前计数值
*
* 入口参数: TMRx               只能是HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5中一个
*
*           TimerPeriod        定时器周期值
*
* 返回参数: 无
*
* 特殊说明: 无
*********************************************************************************************************
*/
void HT_TMR_PeriodSet(HT_TMR_TypeDef* TMRx, uint16_t TimerPeriod)
{
    /*  assert_param  */
    TMRx->TMRPRD = TimerPeriod;                             /*!< 设置定时器周期寄存器        */
    TMRx->TMRCNT = 0;                                       /*!< 清空定时器计数寄存器        */
}

/*
*********************************************************************************************************
*                                 ENABLE OR DISABLE TIMER INTERRUPT
*
* 函数说明: 使能或者关闭TIMER中断
*
* 入口参数: TMRx       只能是HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5中一个
*
*           ITEn       TIMER中断设置位，可以为以下参数或其组合
*                        @arg TMR_TMRIE_PRDIE
*                        @arg TMR_TMRIE_CAPIE
*                        @arg TMR_TMRIE_CMPIE
*                        @arg TMR_TMRIE_ACIE    (only for HT502x, HT6x2x, HT6x3x)
*
*           NewState   = ENABLE： 使能中断
*                      = DISABLE：关闭中断
* 返回参数: 无
*
* 特殊说明: 无
*********************************************************************************************************
*/
void HT_TMR_ITConfig(HT_TMR_TypeDef* TMRx, uint16_t ITEn, FunctionalState NewState)
{
    /*  assert_param  */
    if (NewState != DISABLE)
    {
        TMRx->TMRIE |= (uint32_t)ITEn;               /*!< 使能TIMER中断           */
    }
    else
    {
        TMRx->TMRIE &= ~(uint32_t)ITEn;              /*!< 关闭TIMER中断           */
    }
}

/*
*********************************************************************************************************
*                            GET SPECIFIED TIMER INTERRUPT FLAG STATUS
*
* 函数说明: 获取相应TIMER中断标志状态
*
* 入口参数: TMRx       只能是HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5中一个
*
*           ITFlag     想要检查的某个TIMER中断，可以为以下参数:
*                        @arg TMR_TMRIF_PRDIF
*                        @arg TMR_TMRIF_CAPIF
*                        @arg TMR_TMRIF_CMPIF
*                        @arg TMR_TMRIF_ACIF    (only for HT6x2x)
*
* 返回参数: ITStatus    = SET：  相应中断标志产生
*                       = RESET：相应中断标志未产生
*
* 特殊说明: 无
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
* 函数说明: 清除TIMER中断标志
*
* 入口参数: TMRx       只能是HT_TMR0/HT_TMR1/HT_TMR2/HT_TMR3/HT_TMR4/HT_TMR5中一个
*
*           ITFlag     想要清除的某个TIMER中断标志，可以为以下参数或其组合:
*                        @arg TMR_TMRIF_PRDIF
*                        @arg TMR_TMRIF_CAPIF
*                        @arg TMR_TMRIF_CMPIF
*                        @arg TMR_TMRIF_ACIF    (only for HT6x2x, HT502x)
*
* 返回参数: 无
*
* 特殊说明: 无
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
* 函数说明: 初始化TIMER模块
*
* 入口参数: TMRx               只能是HT_TMR4/HT_TMR5中一个
*
*           TMR_InitStruct     TIMER初始化结构体指针，主要包含4个参数:
*                              1) TimerSource  : 定时器时钟源选择
*                              2) TimerMode    : 定时器工作模式
*                              3) TimerPeriod  : 定时器定时周期设置
*                              4) TimerCompare : 定时器比较寄存器设置
*                              5) TimerPreDiv  : 定时器预分频设置
*
* 返回参数: 无
*
* 特殊说明: 用户在配置TIMER寄存器前应先使能TIMER模块，具体参见HT_CMU_ClkCtrl1Config()函数
*********************************************************************************************************
*/
#if  defined  HT6x2x  ||  defined  HT6x3x
void HT_TMRExt_Init(HT_TMR_TypeDef* TMRx, TMRExt_InitTypeDef* TMR_InitStruct)
{
    /*  assert_param  */
    uint32_t tempreg;

    if ((TMRx == HT_TMR4) || (TMRx == HT_TMR5))
    {
        TMRx->TMRCON &= (~TMR_TMRCON_CNTEN);               /*!< 关闭定时器使能             */
        TMRx->TMRDIV = TMR_InitStruct->TimerPreDiv;        /*!< 设置定时器预分频器          */
       // TMRx->TMRPRD = TMR_InitStruct->TimerPeriod;        /*!< 设置定时器周期寄存器        */
        TMRx->TMRCMP = TMR_InitStruct->TimerCompare;       /*!< 设置定时器比较寄存器        */
        TMRx->TMRCNT = 0;                                  /*!< 清空定时器计数寄存器        */
     //   tempreg = TMR_TMRCON_CNTEN;                        /*!< 定时器使能                 */
        tempreg |= ((uint32_t)TMR_InitStruct->TimerMode|(uint32_t)TMR_InitStruct->TimerSource);
        TMRx->TMRCON = tempreg;                            /*!< 设置定时器工作模式          */
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
	setbit(HT_CMU->CLKCTRL1 ,2);//打开定时器4
					  //向上计数   //MODE周期计数
	HT_TMR2->TMRCON = (0x3<<1) | (0<<0);
	HT_TMR2->TMRDIV = 4400 - 1;  //0.1ms
	HT_TMR2->TMRPRD = 10000;		//1s
	HT_TMR2->TMRIE = 1;
	NVIC_EnableIRQ(TIMER_4_IRQn);
    DisWr_WPREG();
#endif

	TMRExt_InitTypeDef  TMRExt_InitStructure;

    HT_CMU_ClkCtrl1Config(CMU_CLKCTRL1_TMR4EN, ENABLE);            		/*!< 使能TIMER0模块              */

	//TMRExt_InitStructure.TimerSource = TMR_TMRCON_CLKSEL_PLL;            /*!< Timer选择时钟源       */
    TMRExt_InitStructure.TimerMode = TIMExtCycleTiming;                  /*!< Timer设定为定时器功能       */
    TMRExt_InitStructure.TimerPreDiv = 4400 - 1;                          	/*!< Timer时钟等于系统时钟       */
    TMRExt_InitStructure.TimerPeriod = 0x1000;                        	/*!< 定时周期设定                */
    TMRExt_InitStructure.TimerCompare = 0x00;                         	/*!< 比较寄存器设定              */
	HT_TMRExt_Init(HT_TMR4, &TMRExt_InitStructure);
	HT_TMR_PeriodSet(HT_TMR4, period);									//设置周期寄存器值并清空当前计数值
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
     CL_LOG("发送信号量 \n");
	#else
    if(xSemaphoreGive(sem) != pdPASS)
    {
        //信号量激活失败  -- 直接控制继电器
        PRINTF("信号量激活失败  -- 直接控制继电器\n");
        relayOn_direct(gunId,on);
    }
	#endif
}

void RelayCtrl(uint8_t gunId,uint8_t on)
{
    if(EMUIFIFlag == 1)
	{
	//	CL_LOG("走过零点检测 走过零点检测 走过零点检测 \n");
        openRelay_Async(gunId, on);
    }
    else
    {
     //   CL_LOG("走过直接控制继电器 \n");
        //计量芯片未初始化成功  -- 直接控制继电器
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
		
            //继电器开
            if((OpenlFlag != 0) || (CloselFlag != 0))
            {
                ATTFlag = 0;
                //清中断标志
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
                    //最多等待130ms (120ms零点检测+10ms控制延时)
                    if(xTaskGetTickCount() > (uint32_t)(tick + 130))
                    {
                    	PRINTF("mmmmmmmmmmmmmm强制控制继电器mmmmmmmmmmmmmmmm\n");
                        //零点检测失败-强制控制继电器
                        if(operate == 0)
                        {
                            CrtlRelay_OFF();//关
                        }
                        else if(operate == 1)
                        {
                            CrtlRelay_ON();//控制继电器-开
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
		
            //继电器开
            if((OpenlFlag != 0) || (CloselFlag != 0))
            {
                ATTFlag = 0;
                //清中断标志
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
				//最多等待330ms (320ms零点检测+10ms控制延时)
	            if(xTaskGetTickCount() > (uint32_t)(tick + 330))
	            {
	            	PRINTF_LOG("mmmmmmmmmmmmmm强制控制继电器mmmmmmmmmmmmmmmm\n");
	                //零点检测失败-强制控制继电器
	                if(operate == 0)
	                {
	                    CrtlRelay_OFF();//关
	                }
	                else if(operate == 1)
	                {
	                    CrtlRelay_ON();//控制继电器-开
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
	if(SET == HT_TMR_ITFlagStatusGet(HT_TMR2, TMR_TMRIF_PRDIF))                /*!< 周期中断           */
    {
        HT_TMR_ClearITPendingBit(HT_TMR2, TMR_TMRIF_PRDIF);                    /*!< 清除中断标志       */

        HT_TMR_ITConfig(HT_TMR4, TMR_TMRIE_PRDIE, DISABLE);
        NVIC_DisableIRQ(TIMER_4_IRQn);
		TIM4_5_Cmd(HT_TMR4, DISABLE);
		
        if(operate == 0)
        {
            CrtlRelay_OFF();//关
        }
        else if(operate == 1)
        {
            CrtlRelay_ON();//控制继电器-开
        }
        timerHandle = 0;
		ATTFlag = 1;
    }
#if 0
    if(SET == HT_TMR_ITFlagStatusGet(HT_TMR2, TMR_TMRIF_CAPIF))                /*!< 捕获中断           */
    {
		
        HT_TMR_ClearITPendingBit(HT_TMR2, TMR_TMRIF_CAPIF);                    /*!< 清除中断标志       */
    }

    if(SET == HT_TMR_ITFlagStatusGet(HT_TMR2, TMR_TMRIF_CMPIF))                /*!< 比较中断           */
    {
		
        HT_TMR_ClearITPendingBit(HT_TMR2, TMR_TMRIF_CMPIF);                    /*!< 清除中断标志       */
    }
#endif
}

void EXTI5_IRQHandler(void)
{
    if(SET == HT_EXTIRise_ITFlagStatusGet(INT_EXTIF_RIF_INT5))         /*!< INT4上升沿中断           */
    {

        HT_EXTIRise_ClearITPendingBit(INT_EXTIF_RIF_INT5);             /*!< 清除中断标志             */
    }

    if(SET == HT_EXTIFall_ITFlagStatusGet(INT_EXTIF_FIF_INT5))         /*!< INT4下降沿中断           */
    {
        HT_EXTIFall_ClearITPendingBit(INT_EXTIF_FIF_INT5);             /*!< 清除中断标志             */
        
        EXTI5_CallBack();
    }
}



