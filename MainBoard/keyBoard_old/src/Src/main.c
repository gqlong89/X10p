#ifndef _MAIN_C_
#define _MAIN_C_

/*********************************************************************************************************************/
#include "includes.h"
#include "MainHandle.h"

extern SystemStatus system_ststus;

extern SystemInfo system_info;
 
void main(void)
{
    //����LVD��λ��ѹΪ2V
	LVDCON = 0xC2;	
    
    //���Ź��ϵ�ر�
    WDCON = 0; 
    
    //flash�ؼ�����
    Flash_Init();
    
    //����ϵͳʱ��ΪPLL-6��Ƶ
    Sys_Clk_Set_PLL(PLL_Multiple);	

    //����0��ʼ��---��ӡ����
    Uart0_Initial(115200);

    //��ȫ���ж�
    EA = 1;
    
    Delay_ms(100);
    
    Debug_Log("[MFRS]:X10-KeyBoard\r\n");
    Debug_Log("[VERS]:13\r\n");
    Debug_Log("[DATE]:2019-01-08\r\n");

    memset(&system_info,0,sizeof(SystemInfo));
    memset(&system_ststus,0,sizeof(SystemStatus));
  
    //������ʼ��
	TS_init();	
	TSKey_init();
    
    //SPI��ʼ��
    SPI_init();

    //��λFM175ģ��
    if(FM175XX_HardReset()!=OK){
        system_ststus.card_state = 1;
        Debug_Log("-> FM175XX Reset ERROR.\r\n");
    }
    

    //���ÿ�����Э��
    Pcd_ConfigISOType(0);
    //������Ƶ���-3��TX1��TX2�������TX2Ϊ�������
    Set_Rf(3);
  
     //���Ź���ʼ��--2s֮����Ҫι��
    WDT_init(WDTS_IRCH,WDRE_reset,0x0E0F);
    
    #ifdef EN_BLUETOOTH
        //�����ӿڳ�ʼ��
        Bluetooth_Init();
    #else
        system_info.blue_state = 1;
        system_ststus.blue_state = 1;
    #endif
    
    WDT_FeedDog();
    
    //����ͨ�ų�ʼ��
    CommTask_Init();
        
    //����������
    Main_Handle();
    
}
#endif
