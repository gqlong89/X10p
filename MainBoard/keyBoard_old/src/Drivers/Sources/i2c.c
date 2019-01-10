#ifndef _I2C_C_
#define _I2C_C_

/*********************************************************************************************************************/
#include "ca51f_config.h"
#if (IC_TYPE == IC_TYPE_CA51F2X)
  #include "ca51f2sfr.h"
  #include "ca51f2xsfr.h"
  #include "gpiodef_f2.h"
#elif (IC_TYPE == IC_TYPE_CA51F3X)
  #include "ca51f3sfr.h"
  #include "ca51f3xsfr.h"
  #include "gpiodef_f3.h"
#endif
#include "system.h"
#include "i2c.h"
#include "uart.h"
#include <intrins.h>
/*********************************************************************************************************************/


unsigned char xdata rxBuf[20];
unsigned int xdata rxLen = 0;

static bit SendFlag = 1;

void INT6_ISR(void) interrupt 11 
{
    unsigned char Sta_Temp;
    unsigned char Cmd;
    
    if(I2CFLG & I2CF){//IIC�ж�
        
        I2CFLG  |= I2CF;//��ձ�־
        
        Sta_Temp = I2CSTA;
        
        if(Sta_Temp == 0x60){//���յ��ӻ���ַ+дλ ������ACK�ź�---��������
            I2CCON |= AAK(1);
            //rxBuf[rxLen++] = I2CDAT;
            Cmd = I2CDAT;
            
           // Uart_PutChar(UART_0,Cmd);
        }
        else if(Sta_Temp == 0xA8){//���յ��ӻ���ַ+��λ������ACK�ź�---��������
            I2CCON |= AAK(1);
            Cmd = I2CDAT;
            
            Uart_PutChar(UART_0,Cmd);
        }
        else if(Sta_Temp == 0x80){//���ͻ��߽��ܵ����ݣ���⵽Ӧ���ź�
            I2CCON |= AAK(1);
            SendFlag = 1;
            Cmd = I2CDAT;
            
            Uart_PutChar(UART_0,Cmd);
        }
        
    }
}





//IIC��ʼ��--���豸
void I2C_init(unsigned char i2cadr)
{
    //GPIO��ʼ��
    GPIO_Init(P61F,P61_I2C_SDA_SETTING | PU_EN);
    GPIO_Init(P60F,P60_I2C_SCL_SETTING | PU_EN);
    
    I2CIOS = 3;
    
    //ʹ��IICģ�顢�ж�ʹ�ܡ�
	I2CCON = I2CE(1) | I2CIE(1) | STA(0) | STP(0)| CKHD(1) | AAK(1)| CBSE(0) | STFE(0);		
	I2CADR = GCE(0)|(i2cadr);
	I2CCR = 40;
    
    //�ж�ʹ��
    INT6EN = 1;
    
    //P5.2 
    GPIO_Init(P52F,OUTPUT);
    
    IIC_INT_HIGH();
}


void IIC_Putc(unsigned char bdat)
{
    unsigned int  i = 0;

    while(SendFlag != 1){
        i++;
        if(i>5000) return;
    }
   
    I2CDAT =  bdat;
    SendFlag = 0;
  
}


void IIC_Write(unsigned char *buf,unsigned int len)
{
    unsigned int i = 0;
   
    for(i = 0;i<len;i++){
        IIC_Putc(buf[i]);
    }
    
}




#endif