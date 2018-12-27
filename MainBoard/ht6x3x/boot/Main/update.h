/*update.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "include.h"


#define AppFlashAddr	    			(uint32_t)0x2000						//App������ڵ�ַ(220k)
#define AppUpBkpAddr	    			(uint32_t)0x37000 + AppFlashAddr		//app�������ݴ洢����ַ(220k)
#define TradeRecordAddr					(uint32_t)0x37000 + AppUpBkpAddr		//����δ�ϱ���¼�洢����ַ	16k
#define SysUpInfoAddr	    			(uint32_t)0x4000 + TradeRecordAddr		//����������Ϣ(1K)
#define CHARGER_INFO_FLASH_ADDR 		(uint32_t)0x400 + SysUpInfoAddr         //���׮��Ҫ��Ϣ�����ַ 1k
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










