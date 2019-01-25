
#include "includes.h"
#include "simuart.h"
#include "usart.h"
#include "FIFO.h"
#include "server.h"




SIM_UART_STR gSimUartCtrl[SIM_UART_CNT];



void ShowSimUartErr(SIM_USART_STATI_STR *pStati)
{
    CL_LOG("recvIrqOk     =%d.\n",pStati->recvIrqOk);
    CL_LOG("sendTimeOut   =%d.\n",pStati->sendTimeOut);
    CL_LOG("byteCheckErr  =%d.\n",pStati->byteCheckErr);
    CL_LOG("recvOk        =%d.\n",pStati->recvOk);
    printf("\n");
}

/*
*********************************************************************************************************
*                            GET SPECIFIED EXTI RISE INTERRUPT FLAG STATUS
*
* 函数说明: 获取相应EXTI上升沿中断标志状态
*
* 入口参数: ITFlag     想要检查的某个EXTI上升沿中断，可以为以下参数:
*                        @arg INT_EXTIF_RIF_INT0
*                        @arg INT_EXTIF_RIF_INT1
*                        @arg INT_EXTIF_RIF_INT2
*                        @arg INT_EXTIF_RIF_INT3
*                        @arg INT_EXTIF_RIF_INT4
*                        @arg INT_EXTIF_RIF_INT5
*                        @arg INT_EXTIF_RIF_INT6
*                        @arg INT_EXTIF2_RIF_INT7   (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_RIF_INT8   (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_RIF_INT9   (for HT6x2x, HT6x3x and HT502x)
*
* 返回参数: ITStatus    = SET：  相应中断标志产生
*                       = RESET：相应中断标志未产生
*
* 特殊说明: 无
*********************************************************************************************************
*/
ITStatus HT_EXTIRise_ITFlagStatusGet(uint32_t ITFlag)
{
    /*  assert_param  */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
    uint32_t tempreg = (ITFlag & INT_EXTIF2_RIF)>>16;
#endif

    if (HT_INT->EXTIF & ITFlag)
    {
        return SET;                        /*!< Interrupt Flag is set   */
    }
    else
    {
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
        if (HT_INT->EXTIF2 & tempreg)
        {
            return SET;                    /*!< Interrupt Flag is set   */
        }
#endif
        return RESET;                      /*!< Interrupt Flag is reset */
    }
}

/**********************************************************************************************************
*                                   CLEAR EXTI RISE INTERRUPT FLAG
*
* 函数说明: 清除EXTI上升沿中断标志
*
* 入口参数: ITFlag     想要清除的某个EXTI上升沿中断标志，可以为以下参数或其组合:
*                        @arg INT_EXTIF_RIF_INT0
*                        @arg INT_EXTIF_RIF_INT1
*                        @arg INT_EXTIF_RIF_INT2
*                        @arg INT_EXTIF_RIF_INT3
*                        @arg INT_EXTIF_RIF_INT4
*                        @arg INT_EXTIF_RIF_INT5
*                        @arg INT_EXTIF_RIF_INT6
*                        @arg INT_EXTIF2_RIF_INT7   (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_RIF_INT8   (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_RIF_INT9   (for HT6x2x, HT6x3x and HT502x)
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_EXTIRise_ClearITPendingBit(uint32_t ITFlag)
{
    /*  assert_param  */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
    uint32_t tempreg = (ITFlag & INT_EXTIF2_RIF)>>16;
#endif
    ITFlag &= INT_EXTIF_RIF;
    HT_INT->EXTIF  &= ~ITFlag;                  /*!< Clear EXTI Rise Edge Interrupt Flag */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
    HT_INT->EXTIF2  &= ~tempreg;
#endif
}

/*
*********************************************************************************************************
*                          GET SPECIFIED EXTI FALL INTERRUPT FLAG STATUS
*
* 函数说明: 获取相应EXTI下降沿中断标志状态
*
* 入口参数: ITFlag     想要检查的某个EXTI下降沿中断，可以为以下参数:
*                        @arg INT_EXTIF_FIF_INT0
*                        @arg INT_EXTIF_FIF_INT1
*                        @arg INT_EXTIF_FIF_INT2
*                        @arg INT_EXTIF_FIF_INT3
*                        @arg INT_EXTIF_FIF_INT4
*                        @arg INT_EXTIF_FIF_INT5
*                        @arg INT_EXTIF_FIF_INT6
*                        @arg INT_EXTIF2_FIF_INT7     (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_FIF_INT8     (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_FIF_INT9     (for HT6x2x, HT6x3x and HT502x)
*
* 返回参数: ITStatus    = SET：  相应中断标志产生
*                       = RESET：相应中断标志未产生
*
* 特殊说明: 无
*********************************************************************************************************
*/
ITStatus HT_EXTIFall_ITFlagStatusGet(uint32_t ITFlag)
{
    /*  assert_param  */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
    uint32_t tempreg = (ITFlag & INT_EXTIF2_FIF)>>16;
#endif
    if (HT_INT->EXTIF & ITFlag)
    {
        return SET;                        /*!< Interrupt Flag is set   */
    }
    else
    {
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
        if (HT_INT->EXTIF2 & tempreg)
        {
            return SET;                    /*!< Interrupt Flag is set   */
        }
#endif
        return RESET;                      /*!< Interrupt Flag is reset */
    }
}

/**********************************************************************************************************
*                                   CLEAR EXTI FALL INTERRUPT FLAG
*
* 函数说明: 清除EXTI下降沿中断标志
*
* 入口参数: ITFlag     想要清除的某个EXTI下降沿中断标志，可以为以下参数或其组合:
*                        @arg INT_EXTIF_FIF_INT0
*                        @arg INT_EXTIF_FIF_INT1
*                        @arg INT_EXTIF_FIF_INT2
*                        @arg INT_EXTIF_FIF_INT3
*                        @arg INT_EXTIF_FIF_INT4
*                        @arg INT_EXTIF_FIF_INT5
*                        @arg INT_EXTIF_FIF_INT6
*                        @arg INT_EXTIF2_FIF_INT7   (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_FIF_INT8   (for HT6x2x, HT6x3x and HT502x)
*                        @arg INT_EXTIF2_FIF_INT9   (for HT6x2x, HT6x3x and HT502x)
*
* 返回参数: 无
*
* 特殊说明: 无
**********************************************************************************************************/
void HT_EXTIFall_ClearITPendingBit(uint32_t ITFlag)
{
    /*  assert_param  */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
    uint32_t tempreg = (ITFlag & INT_EXTIF2_FIF)>>16;
#endif
    ITFlag &= INT_EXTIF_FIF;
    HT_INT->EXTIF  &= ~ITFlag;                /*!< Clear EXTI Fall Edge Interrupt Flag */
#if  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
    HT_INT->EXTIF2  &= ~tempreg;
#endif
}


uint8_t check_sum(unsigned char revbyte)//返回偶校验值
{
    int count = 0;
    int temp = 0;

	for (int i=0; i<8; i++) {
		temp = (revbyte>>i)&(0x01);
		if (temp == 1)
		count++;
	}

	if ((count%2)==1)
		return 1;
	else
	    return 0;
}


int OpenTimer(uint8_t timer, uint16_t us)
{
	uint32_t tick;

#if (0 == IS_CPU_DOU_FRE)
	tick = 22;
#else
	tick = 44;
#endif
    if (0 == timer) 
	{
    	HT_TMR0->TMRCON = (0x3<<1) | (0<<0); //向上计数   //MODE周期计数
    	HT_TMR0->TMRDIV = tick - 1;  //1us
    	HT_TMR0->TMRPRD = us;		//重装值  208us
    	HT_TMR0->TMRIE = 1;
		
	    return CL_OK;
    }
	else if (1 == timer) 
	{
    	HT_TMR1->TMRCON = (0x3<<1) | (0<<0); //向上计数   //MODE周期计数
        HT_TMR1->TMRDIV = tick - 1;  //1us
    	HT_TMR1->TMRPRD = us;		//重装值52us
    	HT_TMR1->TMRIE = 1;
		
    	return CL_OK;
    }
	else if (3 == timer) 
	{
    	HT_TMR3->TMRCON = (0x3<<1) | (0<<0); //向上计数   //MODE周期计数
    	HT_TMR3->TMRDIV = tick - 1;  //1us
    	HT_TMR3->TMRPRD = us;		//重装值  208us
    	HT_TMR3->TMRIE = 1;
		
	    return CL_OK;
    }
	
    return CL_FAIL;
}


void TIM_Cmd(int timer, int en)
{
	if(timer == 0) 
    {
		if(en)
        {
            setbit(HT_TMR0->TMRCON,0);
        }
		else
        {
            clrbit(HT_TMR0->TMRCON,0);
        }
	}
    else if(timer == 1)
    {
		if(en)
        {
            setbit(HT_TMR1->TMRCON,0);
        }
		else
        {
            clrbit(HT_TMR1->TMRCON,0);
        }
	}
    else if(timer == 3)
    {
		if(en)
        {
            setbit(HT_TMR3->TMRCON,0);
        }
		else
        {
            clrbit(HT_TMR3->TMRCON,0);
        }
	}
}


void IO_USART_TX_PutVal(unsigned char val, uint8_t uartIndex)
{
    if (0 == uartIndex) {
    	if (val){
            GPIO_SetBits(HT_GPIOH, GPIO_Pin_4);
    	}else{
            GPIO_ResetBits(HT_GPIOH, GPIO_Pin_4);
    	}
    }else if (1 == uartIndex) {
        if (val){
            GPIO_SetBits(HT_GPIOA, GPIO_Pin_8);
    	}else{
            GPIO_ResetBits(HT_GPIOA, GPIO_Pin_8);
    	}
    }else if (2 == uartIndex) {
        if (val){
            GPIO_SetBits(HT_GPIOG, GPIO_Pin_6);
    	}else{
            GPIO_ResetBits(HT_GPIOG, GPIO_Pin_6);
    	}
    }
}


void SimUartSendByte(uint8_t sendStat, uint8_t sendData, uint8_t uartIndex)
{
	if (sendStat == COM_START_BIT) {//发送起始位
		IO_USART_TX_PutVal(0, uartIndex);
	}else if(sendStat == COM_CHECKSUM_BIT) {
		IO_USART_TX_PutVal(check_sum(sendData), uartIndex);
	}else if(sendStat == COM_STOP_BIT) {//stop  high
		IO_USART_TX_PutVal(1, uartIndex);
	}else if(sendStat == 11) {//stop  delay
	}else{
		IO_USART_TX_PutVal(sendData & (1<<(sendStat-1)), uartIndex);
	}
}


void SimuUartRecvByte(uint8_t uartIndex, uint8_t buffIndex)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[uartIndex];

    if (4 <= ++pSimUart->recvCnt) 
	{
        pSimUart->recvCnt = 0;
        pSimUart->recvStat++;
    }

	if (pSimUart->recvStat == COM_STOP_BIT) 
	{
		TIM_Cmd(pSimUart->timerIndex, DISABLE);
		FIFO_S_Put(&gUartPortAddr[buffIndex].rxBuffCtrl, pSimUart->recvData);
        pSimUart->statis.recvOk++;
	}
	else if (pSimUart->recvStat == COM_CHECKSUM_BIT) 
	{ //如果当前是校验位 则检查数据正确性
		if (uartIndex == 0) 
		{
			if (GET_USART_RX() != check_sum(pSimUart->recvData)) 
			{//校验位正确 进入下一步接收停止位
				pSimUart->statis.byteCheckErr++;
			}
		} 
		else if (uartIndex == 2) 
		{
			if (GET_RX9() != check_sum(pSimUart->recvData)) 
			{//校验位正确 进入下一步接收停止位
				pSimUart->statis.byteCheckErr++;
			}
		}

	}
	else if (pSimUart->recvStat) 
	{
		if (uartIndex == 0) 
		{
			if (GET_USART_RX()) 
			{
				pSimUart->recvData |= (1 << (pSimUart->recvStat - 1));
			}
		} 
		else if (uartIndex == 2) 
		{
			if (GET_RX9()) 
			{
				pSimUart->recvData |= (1 << (pSimUart->recvStat - 1));
			}
		}
    }
}


void TIMER_0_IRQHandler(void)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[0];
    UART_INFO_STR *pUart = &gUartPortAddr[7];

    if (2 == pSimUart->recvSend) 
    {
    	if (HT_TMR0->TMRIF != 1) 
        {
    		return;
    	}
        HT_TMR0->TMRIF = 0;

        SimUartSendByte(pSimUart->gSendStat, pSimUart->gSendData, 0);
        pSimUart->gSendStat++;
        if (12 == pSimUart->gSendStat) 
        {
            if (++pUart->sendCnt < pUart->allCnt) 
            {
                pSimUart->gSendData = pUart->pSendData[pUart->sendCnt];
                pSimUart->gSendStat = 0;
            }
            else
            {
                TIM_Cmd(0, DISABLE);
            }
        }
    }
    else if (1 == pSimUart->recvSend) 
    {
        if (HT_TMR0->TMRIF == 1) 
        {
            HT_TMR0->TMRIF = 0;
            SimuUartRecvByte(0, 7);
        }
    }
}


void TIMER_1_IRQHandler(void)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[1];
    UART_INFO_STR *pUart = &gUartPortAddr[8];

    if (2 == pSimUart->recvSend) 
    {
    	if (HT_TMR1->TMRIF != 1) 
        {
    		return;
    	}
        HT_TMR1->TMRIF = 0;

        SimUartSendByte(pSimUart->gSendStat, pSimUart->gSendData, 1);
        pSimUart->gSendStat++;
        if (12 == pSimUart->gSendStat) 
        {
            if (++pUart->sendCnt < pUart->allCnt) 
            {
                pSimUart->gSendData = pUart->pSendData[pUart->sendCnt];
                pSimUart->gSendStat = 0;
            }
            else
            {
                TIM_Cmd(1, DISABLE);
            }
        }
    }
    else if (1 == pSimUart->recvSend) 
    {
        if (HT_TMR1->TMRIF == 1) 
        {
            HT_TMR1->TMRIF = 0;
            SimuUartRecvByte(1, 8);
        }
    }
}

void TIMER_3_IRQHandler(void)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[2];
    UART_INFO_STR *pUart = &gUartPortAddr[9];

    if (2 == pSimUart->recvSend) 
    {
    	if (HT_TMR3->TMRIF != 1) 
        {
    		return;
    	}
        HT_TMR3->TMRIF = 0;

        SimUartSendByte(pSimUart->gSendStat, pSimUart->gSendData, 2);
        pSimUart->gSendStat++;
        if (12 == pSimUart->gSendStat) 
        {
            if (++pUart->sendCnt < pUart->allCnt) 
            {
                pSimUart->gSendData = pUart->pSendData[pUart->sendCnt];
                pSimUart->gSendStat = 0;
            }
            else
            {
                TIM_Cmd(3, DISABLE);
            }
        }
    }
    else if (1 == pSimUart->recvSend) 
    {
        if (HT_TMR3->TMRIF == 1) 
        {
            HT_TMR3->TMRIF = 0;
            SimuUartRecvByte(2, 9);
        }
    }
}

int SimuUartSendData(UART_INFO_STR*pUart, const uint8_t *pData, uint16_t len)
{
    int waitCnt = len;
    int cnt = 0;
    SIM_UART_STR *pSimUart = &gSimUartCtrl[pUart->Data.simUart.deviceIndex];

    pUart->allCnt = len;
    pUart->sendCnt = 0;
    pUart->pSendData = (void*)pData;
    pSimUart->gSendData = pData[0];
    pSimUart->gSendStat = 0;
    pSimUart->recvSend = 2;
    OpenTimer(pSimUart->timerIndex, pSimUart->baudRate);
	TIM_Cmd(pSimUart->timerIndex, ENABLE);
    while (pUart->sendCnt < pUart->allCnt) 
	{
        OS_DELAY_MS(10);
        Feed_WDT();
        if (waitCnt < ++cnt) 
		{
            pSimUart->statis.sendTimeOut++;
            TIM_Cmd(pSimUart->timerIndex, DISABLE);
            return CL_FAIL;
        }
    }
    return CL_OK;
}
void EXTI7_IRQHandler(void)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[0];

//	HT_INT->EXTIF2 = 0x0000;
    if(SET == HT_EXTIRise_ITFlagStatusGet(INT_EXTIF2_RIF_INT7))         /*!< INT7上升沿中断           */
    {

        HT_EXTIRise_ClearITPendingBit(INT_EXTIF2_RIF_INT7);             /*!< 清除中断标志             */
    }

    if(SET == HT_EXTIFall_ITFlagStatusGet(INT_EXTIF2_FIF_INT7))         /*!< INT7下降沿中断           */
    {

        HT_EXTIFall_ClearITPendingBit(INT_EXTIF2_FIF_INT7);             /*!< 清除中断标志             */
    }


    if(SET == HT_EXTIRise_ITFlagStatusGet(INT_EXTIF2_RIF_INT8))         /*!< INT8上升沿中断           */
    {

        HT_EXTIRise_ClearITPendingBit(INT_EXTIF2_RIF_INT8);             /*!< 清除中断标志             */
    }

    if(SET == HT_EXTIFall_ITFlagStatusGet(INT_EXTIF2_FIF_INT8))         /*!< INT8下降沿中断           */
    {

        HT_EXTIFall_ClearITPendingBit(INT_EXTIF2_FIF_INT8);             /*!< 清除中断标志             */
    }


    if(SET == HT_EXTIRise_ITFlagStatusGet(INT_EXTIF2_RIF_INT9))         /*!< INT9上升沿中断           */
    {

        HT_EXTIRise_ClearITPendingBit(INT_EXTIF2_RIF_INT9);             /*!< 清除中断标志             */
    }

    if(SET == HT_EXTIFall_ITFlagStatusGet(INT_EXTIF2_FIF_INT9))         /*!< INT9下降沿中断           */
    {

        HT_EXTIFall_ClearITPendingBit(INT_EXTIF2_FIF_INT9);             /*!< 清除中断标志             */
    }
	if (GET_USART_RX() == 0) 
	{
		if (pSimUart->recvStat == COM_STOP_BIT) 
		{
            pSimUart->statis.recvIrqOk++;
            pSimUart->recvCnt = 0;
			pSimUart->recvStat = COM_START_BIT;
            pSimUart->recvData = 0;
            pSimUart->recvSend = 1;
            OpenTimer(0, 52);
			TIM_Cmd(pSimUart->timerIndex, ENABLE);
		}
	}
}

void EXTI4_IRQHandler(void)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[2];

    if(SET == HT_EXTIRise_ITFlagStatusGet(INT_EXTIF_RIF_INT4))         /*!< INT4上升沿中断           */
    {

        HT_EXTIRise_ClearITPendingBit(INT_EXTIF_RIF_INT4);             /*!< 清除中断标志             */
    }

    if(SET == HT_EXTIFall_ITFlagStatusGet(INT_EXTIF_FIF_INT4))         /*!< INT4下降沿中断           */
    {

        HT_EXTIFall_ClearITPendingBit(INT_EXTIF_FIF_INT4);             /*!< 清除中断标志             */
    }
	if (GET_RX9() == 0) 
	{

		if (pSimUart->recvStat == COM_STOP_BIT) 
		{
            pSimUart->statis.recvIrqOk++;
            pSimUart->recvCnt = 0;
			pSimUart->recvStat = COM_START_BIT;
            pSimUart->recvData = 0;
            pSimUart->recvSend = 1;
            OpenTimer(3, 52);
			TIM_Cmd(pSimUart->timerIndex, ENABLE);
		}
	}
}


void SimuartGpioInit(void)
{
	GPIO_Cfg(HT_GPIOH, GPIO_Pin_4, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);//pH4 tx
	GPIO_SetBits(HT_GPIOH, GPIO_Pin_4);
	GPIO_Cfg(HT_GPIOH, GPIO_Pin_3, GPIO_Mode_AF1, GPIO_Input_Up, GPIO_Output_PP);//PH3 rx

	GPIO_Cfg(HT_GPIOA, GPIO_Pin_8, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	GPIO_SetBits(HT_GPIOA, GPIO_Pin_8);
	GPIO_Cfg(HT_GPIOA, GPIO_Pin_7, GPIO_Mode_AF1, GPIO_Input_Up, GPIO_Output_PP);

	//PG6 TX, PA9 RX
	GPIO_Cfg(HT_GPIOG, GPIO_Pin_6, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	GPIO_SetBits(HT_GPIOG, GPIO_Pin_6);
	GPIO_Cfg(HT_GPIOA, GPIO_Pin_9, GPIO_Mode_AF1, GPIO_Input_Up, GPIO_Output_PP);

	EnWr_WPREG();
	HT_INT->EXTIF2 = 0x0000;
	NVIC_ClearPendingIRQ(EXTI9_IRQn);
	NVIC_SetPriority(EXTI9_IRQn, 2);
	HT_INT->EXTIE2 |= 0x004;		//INT9下降沿中断使能
	NVIC_EnableIRQ(EXTI9_IRQn);

	HT_INT->EXTIF = 0x0000;
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
	NVIC_SetPriority(EXTI4_IRQn, 2);
	HT_INT->EXTIE |= 0x010;		//INT4下降沿中断使能
	NVIC_EnableIRQ(EXTI4_IRQn);
    DisWr_WPREG();
}


int SimuartInit(void)
{
    uint32_t i;

    memset(gSimUartCtrl, 0, sizeof(gSimUartCtrl));
    for (i = 0; i < SIM_UART_CNT; i++) 
	{
        gSimUartCtrl[i].recvStat = COM_STOP_BIT;
    }
    gSimUartCtrl[0].timerIndex = 0;
    gSimUartCtrl[0].baudRate = 208; //波特率4800  计量
    gSimUartCtrl[1].timerIndex = 1;
    gSimUartCtrl[1].baudRate = 52;  //波特率19200 debug
	gSimUartCtrl[2].timerIndex = 3;
    gSimUartCtrl[2].baudRate = 208; //波特率4800  计量

    EnWr_WPREG();
    setbit(HT_CMU->CLKCTRL1 ,0);//打开定时器0
    setbit(HT_CMU->CLKCTRL1 ,1);//打开定时器1
	setbit(HT_CMU->CLKCTRL1 ,3);//打开定时器3
    NVIC_EnableIRQ(TIMER_0_IRQn);
    NVIC_EnableIRQ(TIMER_1_IRQn);
	NVIC_EnableIRQ(TIMER_3_IRQn);
    DisWr_WPREG();

	SimuartGpioInit();
	return CL_OK;
}


