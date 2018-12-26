/*update.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "include.h"


#define AppFlashAddr	(uint32_t)0x2000		//App运行入口地址(110k)
#define AppUpBkpAddr	(uint32_t)0x1D800		//app缓存数据存储基地址(110k)
#define SysCfgInfoAddr	(uint32_t)0x39000		//系统配置信息存储地址(20k)
#define SysUpInfoAddr	(uint32_t)0x3d000		//升级请求消息(1K)
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










