/*CpuCfg.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __CPUCFG_H__
#define __CPUCFG_H__

#include "includes.h"

#define EnWr_WPREG()		HT_CMU->WPREG = 0xA55A			//则关闭写保护功能，用户可以写操作被保护的寄存器
#define DisWr_WPREG()		HT_CMU->WPREG = 0x0000			//则开启写保护功能，用户禁止写操作被保护的寄存器

#define Debug_En_WDT()		HT_CMU->CLKCTRL1|=0x4000		//调试模式下使能看门狗功能
#define Debug_Dis_WDT()		HT_CMU->CLKCTRL1&=~0x4000		//调试模式下关闭看门狗功能


#define	Feed_WDT64mS()		HT_WDT->WDTCLR = 0xAA00		    //FeedWDT per 64ms
#define	Feed_WDT()			HT_WDT->WDTCLR = 0xAA7F		    //FeedWDT per 8s: 64ms*(1+SET[0..7])
//#define	Feed_WDT()			HT_WDT->WDTCLR = 0xAA3F		//FeedWDT per 4s: 64ms*(1+SET[0..7])
#define	Feed_WDT16S()		HT_WDT->WDTCLR = 0xAAFF		    //FeedWDT per 16s
#define WDT_RESET_SYS()     HT_WDT->WDTCLR = 0x00           //立即复位


#define AppFlashAddr	    (uint32_t)0x2000		//App运行入口地址(110k)
#define AppUpBkpAddr	    (uint32_t)0x1D800		//app缓存数据存储基地址(110k)
#define TradeRecordAddr		(uint32_t)0x39000		//订单未上报记录存储基地址	16k
#define SysUpInfoAddr	    (uint32_t)0x3D000		//升级请求消息(1K)
#define CHARGER_INFO_FLASH_ADDR 0x3D400             //充电桩重要信息保存地址 1k
//#define RecordHeadFlashAddr	    (uint32_t)0x3D400	//订单头信息(1K)


#define FLASH_PAGE_SIZE					1024
#define	RECORD_BLOCK_SIZE_PER_PAGE		128
#define	RECORD_BLOCK_NUM_PER_PAGE		(FLASH_PAGE_SIZE/RECORD_BLOCK_SIZE_PER_PAGE)        //1024/128=8每个扇区存 8笔订单
#define	RECORD_FLASH_SIZE				(128*128)	//(16K)
#define RECORD_MAX_BLOCK_NUM			(RECORD_FLASH_SIZE/RECORD_BLOCK_SIZE_PER_PAGE)      //最大可存储订单数量 128笔 也是环形队列的深度
#define MAX_RECORD						(RECORD_MAX_BLOCK_NUM-RECORD_BLOCK_NUM_PER_PAGE)    //实际最大存储订单数量 120笔 需要空一个扇区

#define APP_FW_SIZE                     (AppUpBkpAddr-AppFlashAddr)


#define SysInfoEepromAddr	            (uint32_t)0x0000		//系统配置信息eeprom存储基地址  2k
#define GunInfoEepromAddr               (uint32_t)0x0800		//枪头状态信息eeprom存储基地址  2k
#define RecordHeadFlashAddr             (uint32_t)0x1000        //订单信息存储地址  128字节
#define EMUCaliationAddr		        (uint32_t)0x1400		//计量校准参数
#define EEPROM_PAGE_SIZE                32


typedef struct{
	uint16_t updateFlag;
	uint16_t checkSum;
	uint32_t fsize;
}SYS_UPDATE_INFO_t;



void delay_ms(uint16_t ms);
void delay_us(uint32_t us);

int BootSetReqInfo(void);
int BootClrReqInfo(void);

extern SYS_UPDATE_INFO_t updateInfo;

#endif //__CPUCFG_H__

