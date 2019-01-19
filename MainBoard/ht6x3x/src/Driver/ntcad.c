#include "includes.h"
#include "ntcad.h"
#include "gpio.h"
#include "TempDetection.h"


int InitTmp(void)//adcin3 G2 AF2
{
    EnWr_WPREG();
    GPIO_Cfg(HT_GPIOG, GPIO_Pin_2, GPIO_Mode_AF2, GPIO_Input_Up, GPIO_Output_PP);
    HT_TBS->TBSCON = (0x01<<15) |(0x19<<10) | (3<<8) | (2<<5) | (1);//adc disable default  filter采样次数8  default
    HT_TBS->TBSIE =  0x00;
    HT_TBS->TBSPRD = 0;//采样周期
    DisWr_WPREG();
    return 0;
}


int GetCpuTemp(void)
{
	int16_t temp;
//	temp = 12.9852 - (int16_t)(HT_TBS->TMPDAT+1500) * 0.0028;
	temp = (int16_t)ReadResistanceValue(TBS_ADC4);
	printf("temp==%d.\n", temp);
	
	return temp;
}


