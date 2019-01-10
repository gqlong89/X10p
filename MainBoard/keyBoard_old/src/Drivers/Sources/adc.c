#ifndef _ADC_C_
#define _ADC_C_

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
#include "adc.h"
#include <intrins.h>
/*********************************************************************************************************************/
/***********************************************************************************
函数名:		ADC_init										 	
功能描述：初始化ADC功能
输入参数：adc_ref 	ADC参考电压设置
					adc_int		ADC中断设置
					amp_sw		是否开启运放功能
					gain			运放增益
返回值：  无
***********************************************************************************/
void ADC_init(ADC_RefVolt_TypeDef adc_ref,InterruptSW_TypeDef adc_int,AmpSW_TypeDef amp_sw,AMP_Gain_TypeDef gain)							
{
	ADCON = AST(0) | ADIE(adc_int) | HTME(7) | VSEL(adc_ref);	
	ADCFGL = ACKD(7) | ADCALE(1) | ADCHS(0);	
#if (IC_TYPE == IC_TYPE_CA51F2X)
	ADCFGH = AOVE(0) | VTRIM(35);
	if(amp_sw == ENABLE)
	{
		OPACON = OPAM(2) | OPAS(gain);			
	}
#elif (IC_TYPE == IC_TYPE_CA51F3X)
	if(amp_sw == ENABLE)
	{
		ADCFGH = AOPS(gain) | VTRIM(35);
	}
	else
	{
		ADCFGH = AOPS(0) | VTRIM(35);
	}
#endif
	if(adc_int == ENABLE)
	{
		INT2EN = 1; 
	}
}
/***********************************************************************************
函数名:		ADC_ConfigChannel										 	
功能描述：设置ADC输入通道
输入参数：channel 	ADC通道
返回值：  无
***********************************************************************************/
void ADC_ConfigChannel(ADC_Channel_TypeDef channel)
{
	ADCFGL = (ADCFGL&0xF0) | ADCHS(channel);
}
/***********************************************************************************
函数名:		ADC_StartConversion										 	
功能描述：启动ADC转换
输入参数：无
返回值：  无
***********************************************************************************/
void ADC_StartConversion(void)
{
  ADCON |= AST(1);
}

/***********************************************************************************
函数名:		GetAdcValue										 	
功能描述：获取ADC转换数值
输入参数：无
返回值：  ADC值
***********************************************************************************/
unsigned int GetAdcValue(void)
{
	unsigned int ADC_Value;
	while(!(ADCON & ADIF));
	ADCON |= ADIF;
	ADC_Value = ADCDH*256 + ADCDL;
	ADC_Value >>= 4;
	return ADC_Value;
}

/*********************************************************************************************************************/
#endif