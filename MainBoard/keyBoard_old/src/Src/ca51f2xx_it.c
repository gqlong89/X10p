#include "includes.h"

volatile unsigned long SystemTimer = 0;

void (* timeout_callback)(void) = NULL;
unsigned int xdata timeout_cnt = 0;


//�ⲿ�ж�8--RCT�ж�
void INT8_ISR (void) interrupt 13 	 
{
	if(RTCIF & RTC_MF)			//�����ж�	   ʱ��Ϊ (RTMSS+1)*128*Trtc, 
	{
		RTCIF = RTC_MF;
		TS_MS_ISR();
		if(TS_LongKeyTimer)
		{
			TS_LongKeyTimer--;		
		}
	}
	if(RTCIF & RTC_HF)			//�����ж�
	{
		RTCIF = RTC_HF;	
		TS_HS_ISR();
        
        SystemTimer++;
        
        if(timeout_callback != NULL && timeout_cnt-- > 0){
            if(timeout_cnt == 0){
                timeout_callback();
                timeout_callback = NULL;
            }
        }
        
	}
}



Time_t getSystemTimer(void)
{
    return SystemTimer;
}


int StartCallBack(unsigned int timeout,void (* fun)(void))
{
    if(timeout_callback != NULL){
        return ERROR;
    }
    
    timeout_cnt = timeout;
    timeout_callback = fun;
    
    return OK;
}

