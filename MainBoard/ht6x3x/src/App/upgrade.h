
#ifndef __UPGRADE_H__
#define __UPGRADE_H__

#include <stdint.h>


typedef struct{
	uint32_t size;				//固件大小	(不包含头信息)
	uint16_t checkSum;			//检验		(不包含头信息)
	uint16_t package_num;		//固件升级接收总数据包
	uint8_t fw_version;			//版本号
	uint16_t current_package; 	//当前接收固件数据包
	uint32_t write_size;
    uint8_t upgradeFlag;        //升级标志
	uint8_t lastIndex;
}UPGRADE_STR;


void Upgrade_Init(uint32_t package_num,uint8_t fw_version);
uint8_t Upgrade_Write(uint8_t *pData,uint16_t len);

extern UPGRADE_STR upgradeInfo;

#endif

