/*flash.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __FLASH_H__
#define __FLASH_H__

#include "includes.h"


extern void HT_Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num);
extern void HT_Flash_HalfWordWrite(uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num);
extern void HT_Flash_WordWrite(uint32_t* pWriteWord, uint32_t Address, uint32_t Num);
extern void HT_Flash_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num);
extern void HT_Flash_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num);
extern void HT_Flash_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num);
extern void HT_Flash_PageErase(uint32_t EraseAddress);
extern void HT_Flash_ChipErase(void);

extern int FlashReadMatrix(uint8_t *data, uint16_t size);
extern int FlashWritMatrix(uint8_t *data, uint16_t size);
extern int FlashReadGunInfo(uint8_t *pGunInfo, uint16_t size);
extern int FlashReadSysInfo(uint8_t *pInfo, uint16_t size);
extern int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack);
extern int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack);
extern void FlashWriteAppBackup(uint32_t app_backup_record_addr, uint8_t* buffer, uint16_t len);
extern void FlashEraseAppBackup(void);
extern void FlashTestTask(void);
extern void WriteUpdateInfo(uint32_t fsize, uint32_t checkSum);
extern int FlashWriteTradeRecord(void *pTradeRecord, uint16_t size);
//int ProcTradeRecord(void);
extern void FlashReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len);
extern void E2promReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len);
extern int E2promWriteDataEx(uint32_t addr, void* pBuff, uint16_t len);
extern void FlashEraseKeyBoardBackup(void);

//extern uint8_t TradeRecordGunId;
#endif // __FLASH_H__


