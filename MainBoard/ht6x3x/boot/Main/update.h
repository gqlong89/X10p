/*update.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "include.h"


#define AppFlashAddr	    			(uint32_t)0x2000						//App������ڵ�ַ(210k)
#define AppUpBkpAddr	    			(uint32_t)0x34800 + AppFlashAddr		//app�������ݴ洢����ַ(210k)
#define KeyBoardBackAddr 				(uint32_t)0x34800 + AppUpBkpAddr		//�����屸����58k
#define TradeRecordAddr					(uint32_t)0xE800 + KeyBoardBackAddr		//����δ�ϱ���¼�洢����ַ	16k
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










