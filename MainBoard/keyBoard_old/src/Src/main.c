#ifndef _MAIN_C_
#define _MAIN_C_

/*********************************************************************************************************************/
#include "includes.h"
#include "MainHandle.h"

extern SystemStatus system_ststus;

extern SystemInfo system_info;
 
void main(void)
{
    //设置LVD复位电压为2V
	LVDCON = 0xC2;	
    
    //看门狗上电关闭
    WDCON = 0; 
    
    //flash控件划分
    Flash_Init();
    
    //设置系统时钟为PLL-6倍频
    Sys_Clk_Set_PLL(PLL_Multiple);	

    //串口0初始化---打印串口
    Uart0_Initial(115200);

    //打开全局中断
    EA = 1;
    
    Delay_ms(100);
    
    Debug_Log("[MFRS]:X10-KeyBoard\r\n");
    Debug_Log("[VERS]:13\r\n");
    Debug_Log("[DATE]:2019-01-08\r\n");

    memset(&system_info,0,sizeof(SystemInfo));
    memset(&system_ststus,0,sizeof(SystemStatus));
  
    //按键初始化
	TS_init();	
	TSKey_init();
    
    //SPI初始化
    SPI_init();

    //复位FM175模块
    if(FM175XX_HardReset()!=OK){
        system_ststus.card_state = 1;
        Debug_Log("-> FM175XX Reset ERROR.\r\n");
    }
    

    //设置卡操作协议
    Pcd_ConfigISOType(0);
    //设置射频输出-3，TX1，TX2打开输出，TX2为反向输出
    Set_Rf(3);
  
     //看门狗初始化--2s之内需要喂狗
    WDT_init(WDTS_IRCH,WDRE_reset,0x0E0F);
    
    #ifdef EN_BLUETOOTH
        //蓝牙接口初始化
        Bluetooth_Init();
    #else
        system_info.blue_state = 1;
        system_ststus.blue_state = 1;
    #endif
    
    WDT_FeedDog();
    
    //主机通信初始化
    CommTask_Init();
        
    //进入主任务
    Main_Handle();
    
}
#endif
