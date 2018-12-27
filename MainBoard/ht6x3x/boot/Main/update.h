/*update.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "include.h"


#define AppFlashAddr	    			(uint32_t)0x2000						//App运行入口地址(220k)
#define AppUpBkpAddr	    			(uint32_t)0x37000 + AppFlashAddr		//app缓存数据存储基地址(220k)
#define TradeRecordAddr					(uint32_t)0x37000 + AppUpBkpAddr		//订单未上报记录存储基地址	16k
#define SysUpInfoAddr	    			(uint32_t)0x4000 + TradeRecordAddr		//升级请求消息(1K)
#define CHARGER_INFO_FLASH_ADDR 		(uint32_t)0x400 + SysUpInfoAddr         //充电桩重要信息保存地址 1k
#define FLASH_PAGE_SIZE		1024



typedef struct{
	uint16_t updateFlag;
	uint16_t checkSum;
	uint32_t fsize;
}SYS_UPDATE_INFO_t;




int UpdateFromAppBkp(uint32_t fsize, uint16_t checksum);
int FlashClrSysUpInfo(void);
int FlashWriteSysUpInfo(void);
int FlashReadSysUpInfo(void);




#endif // __UPDATE_H__










