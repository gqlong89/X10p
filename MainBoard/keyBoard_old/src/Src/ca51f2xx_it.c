#include "includes.h"

volatile unsigned long SystemTimer = 0;

void (* timeout_callback)(void) = NULL;
unsigned int xdata timeout_cnt = 0;


//外部中断8--RCT中断
void INT8_ISR (void) interrupt 13 	 
{
	if(RTCIF & RTC_MF)			//毫秒中断	   时间为 (RTMSS+1)*128*Trtc, 
	{
		RTCIF = RTC_MF;
		TS_MS_ISR();
		if(TS_LongKeyTimer)
		{
			TS_LongKeyTimer--;		
		}
	}
	if(RTCIF & RTC_HF)			//半秒中断
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

