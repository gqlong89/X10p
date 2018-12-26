
#ifndef __UPGRADE_H__
#define __UPGRADE_H__

#include <stdint.h>


typedef struct{
	uint32_t size;				//�̼���С	(������ͷ��Ϣ)
	uint16_t checkSum;			//����		(������ͷ��Ϣ)
	uint16_t package_num;		//�̼��������������ݰ�
	uint8_t fw_version;			//�汾��
	uint16_t current_package; 	//��ǰ���չ̼����ݰ�
	uint32_t write_size;
    uint8_t upgradeFlag;        //������־
	uint8_t lastIndex;
}UPGRADE_STR;


void Upgrade_Init(uint32_t package_num,uint8_t fw_version);
uint8_t Upgrade_Write(uint8_t *pData,uint16_t len);

extern UPGRADE_STR upgradeInfo;

#endif

