#ifndef _SPI_C_
#define _SPI_C_

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
#include "uart.h"
#include "spi.h"
#include <intrins.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <absacc.h>
/*********************************************************************************************************************/


void SPI_init(void)
{
  //SPI 引脚设置
  //P6.2/P6.3/P6.4 SPI
	GPIO_Init(P64F,P64_SPI_SCK_SETTING);
	GPIO_Init(P62F,P62_SPI_MISO_SETTING);
	GPIO_Init(P63F,P63_SPI_MOSI_SETTING);
  //P6.5 输出模式
	GPIO_Init(P65F,OUTPUT);	//

  //CS 高电平
  SPI_CS_HIGH();
  
  //高位先发, SSB有效，主机, 1/4系统时钟
	SPCON =	SPEN(1) | LSBF(0) | SSIG(1) | MSTR(1) | CPOL(0) | CPHA(0) |CPOS(1);	     

  //清除SPIF和WCOL标志
  SPSTA = SPIF|WCOL;
  
  
  //CPDN
  GPIO_Init(P11F,OUTPUT);	//
  
  NPD_HIGH();
  
  
  //  //SPI 中断使能
//	INT5EN = 1;
//  SPSTA = SPIE;
}


void wait_busy(void)
{
  unsigned int i = 0;
  while(!(SPSTA&SPIF)){//等待传输完成
    i++;
    if(i>5000){
      return;
    } 
  }  
}



unsigned char SPIRead(unsigned char addr)
{
  unsigned char data reg_value,send_data;
  
  SPI_CS_LOW();
  send_data=(addr<<1)|0x80;
    
	SPSTA = SPIF|WCOL;//清除SPIF和WCOL标志
	SPDAT = send_data;
	wait_busy();

	SPSTA = SPIF|WCOL;
	SPDAT = 0x00; 
	wait_busy();
	reg_value = SPDAT;
  
  SPI_CS_HIGH();
  
  return(reg_value);
}


void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value)
{
 unsigned char data i,send_data;
 	if (sequence_length==0)
		return;

	SPI_CS_LOW();
  send_data=(addr<<1)|0x80;
  
  SPSTA = SPIF|WCOL;//清除SPIF和WCOL标志
	SPDAT = send_data;
  wait_busy();

  
  for(i = 0;i<sequence_length;i++){
  
    SPSTA = SPIF|WCOL;//清除SPIF和WCOL标志
    if (i==sequence_length-1)
			SPDAT=0x00;
		else
			SPDAT=send_data;

		wait_busy();

		*(reg_value+i)=SPDAT;
  }
  
  SPI_CS_HIGH();
  
  return;
}


void SPIWrite(unsigned char addr,unsigned char wrdata)
{
  unsigned char data send_data;
  SPI_CS_LOW();
  send_data=(addr<<1)&0x7E;
  SPSTA = SPIF|WCOL;//清除SPIF和WCOL标志
  SPDAT = send_data;
  wait_busy();
  
  SPSTA = SPIF|WCOL;
  SPDAT = wrdata;
  wait_busy();
  
  SPI_CS_HIGH();
  
  return;

}


void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value)
{
  unsigned char data send_data,i;
	if(sequence_length==0)
		return;

	SPI_CS_LOW();

	send_data=(addr<<1)&0x7E;
  SPSTA = SPIF|WCOL;//清除SPIF和WCOL标志
  SPDAT = send_data;
  wait_busy();
  
  for(i = 0;i<sequence_length;i++){
    SPSTA = SPIF|WCOL;//清除SPIF和WCOL标志
    SPDAT = *(reg_value+i);
		wait_busy();
  }
  

  SPI_CS_HIGH();
  
  return;
}



/*********************************************************************************************************************/
#endif