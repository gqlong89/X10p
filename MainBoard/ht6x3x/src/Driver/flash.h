/*flash.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __FLASH_H__
#define __FLASH_H__

#include "includes.h"


void HT_Flash_ByteWrite(uint8_t* pWriteByte, uint32_t Address, uint32_t Num);
void HT_Flash_HalfWordWrite(uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num);
void HT_Flash_WordWrite(uint32_t* pWriteWord, uint32_t Address, uint32_t Num);
void HT_Flash_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num);
void HT_Flash_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num);
void HT_Flash_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num);
void HT_Flash_PageErase(uint32_t EraseAddress);
void HT_Flash_ChipErase(void);

int FlashReadMatrix(uint8_t *data, uint16_t size);
int FlashWritMatrix(uint8_t *data, uint16_t size);
int FlashReadGunInfo(uint8_t *pGunInfo, uint16_t size);
int FlashReadSysInfo(uint8_t *pInfo, uint16_t size);
int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack);
int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack);
void FlashWriteAppBackup(uint32_t app_backup_record_addr, uint8_t* buffer, uint16_t len);
void FlashEraseAppBackup(void);
void FlashTestTask(void);
void WriteUpdateInfo(uint32_t fsize, uint32_t checkSum);
int FlashWriteTradeRecord(void *pTradeRecord, uint16_t size);
//int ProcTradeRecord(void);
void FlashReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len);
void E2promReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len);
int E2promWriteDataEx(uint32_t addr, void* pBuff, uint16_t len);

//extern uint8_t TradeRecordGunId;
#endif // __FLASH_H__


