/*CpuCfg.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __CPUCFG_H__
#define __CPUCFG_H__

#include "includes.h"

#define EnWr_WPREG()		HT_CMU->WPREG = 0xA55A			//��ر�д�������ܣ��û�����д�����������ļĴ���
#define DisWr_WPREG()		HT_CMU->WPREG = 0x0000			//����д�������ܣ��û���ֹд�����������ļĴ���

#define Debug_En_WDT()		HT_CMU->CLKCTRL1|=0x4000		//����ģʽ��ʹ�ܿ��Ź�����
#define Debug_Dis_WDT()		HT_CMU->CLKCTRL1&=~0x4000		//����ģʽ�¹رտ��Ź�����


#define	Feed_WDT64mS()		HT_WDT->WDTCLR = 0xAA00		    //FeedWDT per 64ms
#define	Feed_WDT()			HT_WDT->WDTCLR = 0xAA7F		    //FeedWDT per 8s: 64ms*(1+SET[0..7])
//#define	Feed_WDT()			HT_WDT->WDTCLR = 0xAA3F		//FeedWDT per 4s: 64ms*(1+SET[0..7])
#define	Feed_WDT16S()		HT_WDT->WDTCLR = 0xAAFF		    //FeedWDT per 16s
#define WDT_RESET_SYS()     HT_WDT->WDTCLR = 0x00           //������λ


#define AppFlashAddr	    (uint32_t)0x2000		//App������ڵ�ַ(110k)
#define AppUpBkpAddr	    (uint32_t)0x1D800		//app�������ݴ洢����ַ(110k)
#define TradeRecordAddr		(uint32_t)0x39000		//����δ�ϱ���¼�洢����ַ	16k
#define SysUpInfoAddr	    (uint32_t)0x3D000		//����������Ϣ(1K)
#define CHARGER_INFO_FLASH_ADDR 0x3D400             //���׮��Ҫ��Ϣ�����ַ 1k
//#define RecordHeadFlashAddr	    (uint32_t)0x3D400	//����ͷ��Ϣ(1K)


#define FLASH_PAGE_SIZE					1024
#define	RECORD_BLOCK_SIZE_PER_PAGE		128
#define	RECORD_BLOCK_NUM_PER_PAGE		(FLASH_PAGE_SIZE/RECORD_BLOCK_SIZE_PER_PAGE)        //1024/128=8ÿ�������� 8�ʶ���
#define	RECORD_FLASH_SIZE				(128*128)	//(16K)
#define RECORD_MAX_BLOCK_NUM			(RECORD_FLASH_SIZE/RECORD_BLOCK_SIZE_PER_PAGE)      //���ɴ洢�������� 128�� Ҳ�ǻ��ζ��е����
#define MAX_RECORD						(RECORD_MAX_BLOCK_NUM-RECORD_BLOCK_NUM_PER_PAGE)    //ʵ�����洢�������� 120�� ��Ҫ��һ������

#define APP_FW_SIZE                     (AppUpBkpAddr-AppFlashAddr)


#define SysInfoEepromAddr	            (uint32_t)0x0000		//ϵͳ������Ϣeeprom�洢����ַ  2k
#define GunInfoEepromAddr               (uint32_t)0x0800		//ǹͷ״̬��Ϣeeprom�洢����ַ  2k
#define RecordHeadFlashAddr             (uint32_t)0x1000        //������Ϣ�洢��ַ  128�ֽ�
#define EMUCaliationAddr		        (uint32_t)0x1400		//����У׼����
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

