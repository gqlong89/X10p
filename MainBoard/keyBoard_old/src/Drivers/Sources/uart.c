

#include "includes.h"

/*********************************************************************************************************************/


//发送标志
bit Uart0TX_Flag=1,Uart1TX_Flag=1,Uart2TX_Flag=1;


//
void UART0_ISR (void) interrupt 4
{
  //RX
	if(RI0)
	{
		RI0 = 0;
	}
  //TX
	if(TI0)
	{
		TI0 = 0;
        Uart0TX_Flag = 1;
	}
}


//蓝牙
void UART1_ISR (void) interrupt 6
{
    //RX
    if(S1CON & BIT0)
    {
        unsigned char RX_Data;
        S1CON = (S1CON&~(BIT0|BIT1))|BIT0;
        RX_Data = S1BUF;
        #ifdef EN_BLUETOOTH
            FIFO_S_Put(&BlueFIfO,RX_Data);
        #endif
    }
    //TX
    if(S1CON & BIT1)
	{
        S1CON = (S1CON&~(BIT0|BIT1))|BIT1;
        Uart1TX_Flag = 1;
    }

}


//主通信
void UART2_TK_ISR (void) interrupt 8
{
    if(TKIF != 0)
	{
		TS_ISR();
	}

    //RX
	if(S2CON & BIT0)
	{
        unsigned char RX_Data;
        S2CON = (S2CON&~(BIT0|BIT1))|BIT0;
        RX_Data = S2BUF;
        FIFO_S_Put(&commFIfO,RX_Data);
	}
    //TX
	if(S2CON & BIT1)
	{
        S2CON = (S2CON&~(BIT0|BIT1))|BIT1;
		Uart2TX_Flag = 1;
	}
}



void Uart0_Initial(unsigned long int baudrate)
{
    unsigned int  value_temp;

    Uart0TX_Flag = 1;

    GPIO_Init(P31F,P31_UART0_RX_SETTING);
    GPIO_Init(P30F,P30_UART0_TX_SETTING);

    /************************************************************************************************************************/
    //TIMER2作为UART0的波特率发生器
    value_temp = 0x10000 - FOSC/(baudrate*32);
    T2CON = 	0x24;
    T2CH  = 	(unsigned char)(value_temp>>8);
    T2CL  = 	(unsigned char)(value_temp);
    TH2   = 	(unsigned char)(value_temp>>8);
    TL2   = 	(unsigned char)(value_temp);;
    TR2   = 	1;
    /************************************************************************************************************************/

    S0CON = 0x50;
    ES0 = 1;
}


void Uart1_Initial(unsigned long int baudrate)
{
    unsigned int  value_temp;

    Uart1TX_Flag = 1;

    GPIO_Init(P67F,P67_UART1_RX_SETTING);
	GPIO_Init(P66F,P66_UART1_TX_SETTING);

    value_temp = 0x400 - FOSC/(baudrate*32);

    S1RELH = (unsigned char)(value_temp>>8);
	S1RELL = (unsigned char)(value_temp);

	S1CON = 0xD0;
	ES1 =	1;
}



void Uart2_Initial(unsigned long int baudrate)
{
	unsigned int value_temp;

    Uart2TX_Flag = 1;

	GPIO_Init(P60F,P60_UART2_RX_SETTING);
	GPIO_Init(P61F,P61_UART2_TX_SETTING);

	value_temp = 0x400 - FOSC/(baudrate*32);
	S2RELH = (unsigned char)(value_temp>>8);
	S2RELL = (unsigned char)(value_temp);

	S2CON = 0xD0;
	INT3EN = 1;
}



void Uart_PutChar(UART_Def num,unsigned char bdat)
{
    if(num == UART_0){
        unsigned int  i = 0;
        while(Uart0TX_Flag != 1){
            i++;
            if(i>2000) return;
        }
        Uart0TX_Flag = 0;
        S0BUF = bdat;
    }
    else if(num == UART_1){
        unsigned int  i = 0;
        while(Uart1TX_Flag != 1){
            i++;
            if(i>2000) return;
        }
        Uart1TX_Flag = 0;
        S1BUF = bdat;
    }
    else if(num == UART_2){
        unsigned int  i = 0;
        while(Uart2TX_Flag != 1){
            i++;
            if(i>2000) return;
        }
        Uart2TX_Flag = 0;
        S2BUF = bdat;
    }
}


void Uart_SendData(UART_Def num,char *bdat,unsigned short len)
{
  unsigned int i = 0;
  for(i = 0;i<len;i++){
    Uart_PutChar(num,bdat[i]);
    WDT_FeedDog();
  }
}



void Debug_Log(char *string)
{
    Uart_SendData(UART_0,string,strlen(string));
}

char putchar (char ch)
{
    Uart_PutChar(UART_0,ch);
    return ch;
}


void Debug_Hex(unsigned char *input_byte,unsigned int input_len)
{

    unsigned char xdata temp_byte;
    unsigned int xdata i;

	for(i=0;i<input_len;i++)
	{
		temp_byte=(*(input_byte+i) & 0xF0)>>4;
		if (temp_byte<0x0A){
            Uart_PutChar(UART_0,0x30+temp_byte);
        }
		else{
            Uart_PutChar(UART_0,0x37+temp_byte);
        }

		temp_byte=(*(input_byte+i) & 0x0F);
		if (temp_byte<0x0A){
            Uart_PutChar(UART_0,0x30+temp_byte);
        }
		else{
            Uart_PutChar(UART_0,0x37+temp_byte);
        }
	}
	return;
}


/*********************************************************************************************************************/
