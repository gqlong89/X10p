/*flash.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "flash.h"
#include "i2c.h"
#include "server.h"
#include "FIFO.h"
#include "rtc.h"

#define  __HT60XX_FLASH_C

#define NO_USE_ASM_FLASH			0


/*
*********************************************************************************************************
*                                           本地宏/结构体
*********************************************************************************************************
*/

#define M8(adr)     (*((uint8_t * ) (adr)))
#define M16(adr)    (*((uint16_t *) (adr)))
#define M32(adr)    (*((uint32_t *) (adr)))




/*
*********************************************************************************************************
*                                         FLASH BYTE WRITE
*
* 函数说明: Flash字节写
*
* 入口参数: pWriteByte    指向被写数据的首地址
*
*           Address       数据将要写到Flash的地址
*
*           Num           写数据长度，以字节为单位
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*********************************************************************************************************
*/
#if NO_USE_ASM_FLASH
void HT_Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< 保存当前写保护状态     */

    MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash解锁              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                             /*!< Flash写                */

    for(i=0; i<Num; i++)
    {
        M8(Address+i) = pWriteByte[i];                                     /*!< 执行Flash写            */
        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /*!< 等待写完成             */
        Feed_WDT();
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash锁定              */

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
    MASTER_INT_EN();
}
#else
__ASM void Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_FLASH_BWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_FLASH_BWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_FLASH_BWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_FLASH_BWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_FLASH_BWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_FLASH_BWR_END
KEIL_FLASH_BWR_LOOP
    LDRB    R5, [R0]
    NOP
    STRB    R5, [R1]
KEIL_FLASH_BWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_FLASH_BWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #1
    ADDS    R1, R1, #1
    SUBS    R2, R2, #1
    BNE     KEIL_FLASH_BWR_LOOP
KEIL_FLASH_BWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
|KEIL_FLASH_BWR_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_FLASH_BWR_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_FLASH_BWR_CONFIG_UL_VAL|  DCD    0x00007A68
}

void HT_Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
	MASTER_INT_DIS();
    Flash_ByteWrite(pWriteByte, Address, Num);
	MASTER_INT_EN();
}
#endif


/*
*********************************************************************************************************
*                                         FLASH BYTE READ
*
* 函数说明: Flash字节读
*
* 入口参数: pReadByte     指向存储读到的数据的首地址
*
*           Address       读Flash的首地址
*
*           Num           读数据长度，以字节为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，以防溢出
*********************************************************************************************************
*/
void HT_Flash_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num)
{
    uint32_t i;

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */

    for(i = 0; i < Num; i++)
    {
        pReadByte[i] = M8(Address + i);                                      /*!< 执行Flash读            */
    }
}

void FlashReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len)
{
    uint16_t i;

    for (i=0; i<len; i++) 
	{
        HT_Flash_ByteRead(&pBuff[i], addr + i, 1);
    }
}


/*
*********************************************************************************************************
*                                         FLASH HALF WORD WRITE
*
* 函数说明: Flash半字写
*
* 入口参数: pWriteHalfWord    指向被写数据的首地址
*
*           Address           数据将要写到Flash的地址
*
*           Num               写数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*           2)用户应保证传递给函数的地址为至少半字对齐
*********************************************************************************************************
*/
#if NO_USE_ASM_FLASH
void HT_Flash_HalfWordWrite(uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num)
{
    uint32_t i;
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< 保存当前写保护状态     */

    MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash解锁              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                             /*!< Flash写                */

    Address &= 0xFFFFFFFE;                                                 /*!< 保证半字对齐           */
    for(i=0; i<Num; i++)
    {
        M16(Address+i*2) = pWriteHalfWord[i];                              /*!< 执行Flash写            */
        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /*!< 等待写完成             */
        Feed_WDT();
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash锁定              */

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
    MASTER_INT_EN();
}

/*
*********************************************************************************************************
*                                         FLASH HALF WORD READ
*
* 函数说明: Flash半字读
*
* 入口参数: pReadHalfWord 指向存储读到的数据的首地址
*
*           Address       读Flash的首地址
*
*           Num           读数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*           2)用户应保证传递给函数的地址为至少半字对齐
*********************************************************************************************************
*/
void HT_Flash_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    Address &= 0xFFFFFFFE;                                                 /*!< 保证半字对齐           */
    for(i = 0; i < Num; i++)
    {
        pReadHalfWord[i] = M16(Address+i*2);                               /*!< 执行Flash读            */
    }

}

/*
*********************************************************************************************************
*                                         FLASH WORD WRITE
*
* 函数说明: Flash字写
*
* 入口参数: pWriteWord    指向被写数据的首地址
*
*           Address       数据将要写到Flash的地址
*
*           Num           写数据长度，以字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*           2)用户应保证传递给函数的地址为至少字对齐
*********************************************************************************************************
*/
void HT_Flash_WordWrite(uint32_t* pWriteWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< 保存当前写保护状态     */

    MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash解锁              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                             /*!< Flash写                */

    Address &= 0xFFFFFFFC;                                                 /*!< 保证字对齐             */
    for(i=0; i<Num; i++)
    {
        M32(Address+i*4) = pWriteWord[i];                                  /*!< 执行Flash写            */
        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /*!< 等待写完成             */
        Feed_WDT();
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash锁定              */

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
    MASTER_INT_EN();
}

/*
*********************************************************************************************************
*                                         FLASH WORD READ
*
* 函数说明: Flash字读
*
* 入口参数: pReadWord     指向存储读到的数据的首地址
*
*           Address       读Flash的首地址
*
*           Num           读数据长度，以字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*           2)用户应保证传递给函数的地址为至少字对齐
*********************************************************************************************************
*/
void HT_Flash_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    Address &= 0xFFFFFFFC;                                                 /*!< 保证字对齐             */
    for(i=0; i<Num; i++)
    {
        pReadWord[i] = M32(Address+i*4);                                   /*!< 执行Flash读            */
    }

}

/*
*********************************************************************************************************
*                                         FLASH CHIP ERASE
*
* 函数说明: Flash全擦除
*
* 入口参数: 无
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*********************************************************************************************************
*/
void HT_Flash_ChipErase(void)
{
    /*  assert_param  */

    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< 保存当前写保护状态     */

	MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash解锁              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_CHIPERASE;                         /*!< Flash页擦除            */

    M32(0x1000) = 0xFF;                                                    /*!< 执行Flash全擦除        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< 等待写完成             */
    Feed_WDT();
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash锁定              */

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
	MASTER_INT_EN();
}
#endif

/*
*********************************************************************************************************
*                                         FLASH PAGE ERASE
*
* 函数说明: Flash页擦除
*
* 入口参数: EraseAddress    页擦除地址
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变，1K bytes/page
*********************************************************************************************************
*/
#if NO_USE_ASM_FLASH
void HT_Flash_PageErase(uint32_t EraseAddress)
{
    /*  assert_param  */

    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< 保存当前写保护状态     */

	MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash解锁              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_PAGEERASE;                         /*!< Flash页擦除            */

    EraseAddress &= 0xFFFFFFFC;                                            /*!< 保证字对齐             */
    M32(EraseAddress) = 0xFF;                                              /*!< 执行Flash页擦除        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< 等待写完成             */
    Feed_WDT();
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash锁定              */

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
	MASTER_INT_EN();
}
#else
__ASM void HT_Flash_PageErase(uint32_t EraseAddress)
{
    PUSH    {R4}
    LDR     R3, |KEIL_FLASH_PGES_WPREG_UL_VAL|
    LDR     R1, |KEIL_FLASH_PGES_WPREG_REG_ADDR|
    LDR     R2, [R1]
    LSLS    R2, R2, #31
    BPL     KEIL_FLASH_PGES_CLEAR
    MOVS    R2, R3
    B       KEIL_FLASH_PGES_CONFIG_REG_SET
KEIL_FLASH_PGES_CLEAR
    MOVS    R2, #0
KEIL_FLASH_PGES_CONFIG_REG_SET
    STR     R3, [R1]
    LDR     R3, |KEIL_FLASH_PGES_CONFIG_UL_VAL|
    STR     R3, [R1, #0x38]
    MOVS    R3, #2
    STR     R3, [R1, #0x34]
    MOVS    R3, #255
    MOVS    R4, #3
    BICS    R0, R0, R4
    NOP
    STR     R3, [R0]
KEIL_FLASH_PGES_FLAG_CHK_LOOP
    LDR     R0, [R1, #0x34]
    LSLS    R0, R0, #29
    BMI     KEIL_FLASH_PGES_FLAG_CHK_LOOP
    MOVS    R0, #0
    STR     R0, [R1, #0x34]
    STR     R0, [R1, #0x38]
    STR     R2, [R1]
    POP     {R4}
    BX      LR
|KEIL_FLASH_PGES_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_FLASH_PGES_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_FLASH_PGES_CONFIG_UL_VAL|  DCD    0x00007A68
}
#endif

void FlashWriteAppBackup(uint32_t app_backup_record_addr, uint8_t* buffer, uint16_t len)
{
	HT_Flash_ByteWrite((void*)buffer, app_backup_record_addr, len);
}


void FlashEraseAppBackup(void)
{
    uint32_t i;

    for (i = 0; i < (APP_FW_SIZE / 1024); i++) 
	{
        HT_Flash_PageErase(AppUpBkpAddr+i*1024);
    }
}

void FlashEraseKeyBoardBackup(void)
{
    uint32_t i;

    for (i = 0; i < (KEY_BOARD_FW_SIZE / 1024); i++) 
	{
        HT_Flash_PageErase(KeyBoardBackAddr+i*1024);
    }
}


void WriteUpdateInfo(uint32_t fsize, uint32_t checkSum)
{
    updateInfo.updateFlag = 0xaa55;
    updateInfo.fsize = fsize;
    updateInfo.checkSum = checkSum;
    BootSetReqInfo();
}


//readBack: 写完是否回读 0不回读  1回读
//返回: 0写成功  其他:回读失败
#if 0
int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack)
{
    int i;
    uint8_t  data;

    HT_Flash_PageErase(SysCfgInfoAddr);
    HT_Flash_PageErase(SysCfgInfoAddr+1024);
	HT_Flash_ByteWrite((void*)pSysInfo, SysCfgInfoAddr, size);

    if (readBack) {
        for (i=0; i<size; i++) {
            HT_Flash_ByteRead(&data, SysCfgInfoAddr+i, 1);
            if (data != *((uint8_t*)pSysInfo + i)) {
                CL_LOG("write falsh fail.\n");
                return CL_FAIL;
            }
        }
    }
    CL_LOG("write falsh ok.\n");
    return CL_OK;
}


//readBack: 写完是否回读 0不回读  1回读
//返回: 0写成功  其他:回读失败
int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack)
{
	int i;
    uint8_t  data;

    for (i=0; i<4; i++) {
	    HT_Flash_PageErase(GunInfoAddr+i*1024);
    }
	HT_Flash_ByteWrite((void*)pGunInfo, GunInfoAddr, size);

    if (readBack) {
        for (i=0; i<size; i++) {
            HT_Flash_ByteRead(&data, GunInfoAddr+i, 1);
            if (data != *((uint8_t*)pGunInfo + i)) {
                CL_LOG("write falsh fail.\n");
                return CL_FAIL;
            }
        }
    }
    CL_LOG("write falsh ok.\n");
    return CL_OK;
}


void FlashReadSysInfo(void *pInfo, uint16_t size)
{
	HT_Flash_ByteRead((void*)pInfo, SysCfgInfoAddr, size);
}


void FlashReadGunInfo(void *pGunInfo, uint16_t size)
{
    HT_Flash_ByteRead((void*)pGunInfo, GunInfoAddr, size);
}

#else


//EEPROM读取数据
int at24c64_eepromRead(uint32_t addr, void *pInfo, uint16_t size)
{
	I2C_ReadNByte(0xA0, 2, addr, (void*)pInfo, size);
	return CL_OK;
}


//EEPROM页数据写入
int at24c64_eepromWrite(uint32_t addr, void *pInfo, uint16_t size)
{
	uint8_t  *p = NULL;
	uint8_t	 RemainingBytes = 0;
	uint16_t page = 0;
	uint16_t pCnt = 0;
	uint16_t wCheckSum=0;
	uint16_t rCheckSum=0;
	uint8_t  readBuf[EEPROM_PAGE_SIZE+1];

	page = size / EEPROM_PAGE_SIZE;
	RemainingBytes = size % EEPROM_PAGE_SIZE;
	p = pInfo;
	for (uint16_t i=0; i<size; i++) {
		wCheckSum += *(p+i);
	}
	while(1) {
		if (0 == page) {
			if (0 != RemainingBytes) {
				I2C_WriteNByte(0xA0, 2, addr+EEPROM_PAGE_SIZE*pCnt, (void*)p, RemainingBytes);
			}
			break;
		} else {
			I2C_WriteNByte(0xA0, 2, addr+EEPROM_PAGE_SIZE*pCnt, (void*)p, EEPROM_PAGE_SIZE);
			OS_DELAY_MS(15);
			p += EEPROM_PAGE_SIZE;
			page--;
			pCnt++;
		}
	}

    OS_DELAY_MS(10);
	//校验
	page = size / EEPROM_PAGE_SIZE;
	RemainingBytes = size % EEPROM_PAGE_SIZE;
	pCnt=0;
	while(1) {
		if (0 == page) {
			if (RemainingBytes) {
				at24c64_eepromRead(addr+EEPROM_PAGE_SIZE*pCnt, (void*)&readBuf, RemainingBytes);
				for (uint8_t i=0; i<RemainingBytes; i++) {
					rCheckSum += readBuf[i];
				}
			}
			break;
		} else {
			at24c64_eepromRead(addr+EEPROM_PAGE_SIZE*pCnt, (void*)&readBuf, EEPROM_PAGE_SIZE);
			OS_DELAY_MS(15);
			page--;
			pCnt++;
			for (uint8_t i=0; i<EEPROM_PAGE_SIZE; i++) {
				rCheckSum += readBuf[i];
			}
		}
	}

	if(wCheckSum == rCheckSum) {
		CL_LOG("eeprom write ok, wCheckSum = %d, rCheckSum = %d.\n", wCheckSum, rCheckSum);
		return CL_OK;
	} else {
		CL_LOG("eeprom write err, wCheckSum = %d, rCheckSum = %d.\n", wCheckSum, rCheckSum);
		return CL_FAIL;
	}
}


int EepromWriteData(uint32_t addr, void *pInfo, uint16_t size)
{
    int i;

    for (i=0; i<4; i++) 
	{
    	if (CL_OK == at24c64_eepromWrite(addr, pInfo, size)) 
		{
            return CL_OK;
        }
        vTaskDelay(1000);
    }
    return CL_FAIL;
}


//写系统信息
int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack)
{
    if (CL_OK != EepromWriteData(SysInfoEepromAddr, pSysInfo, size)) 
	{
        SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_E2PROM, 0, EVENT_OCCUR, NULL);
        return CL_FAIL;
    }
    return CL_OK;
}


//读系统信息
int FlashReadSysInfo(uint8_t *pInfo, uint16_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++) 
	{
	    at24c64_eepromRead(SysInfoEepromAddr+i, &pInfo[i], 1);
    }
	return CL_OK;
}


//写枪头信息
int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack)
{
    if (CL_OK != EepromWriteData(GunInfoEepromAddr, pGunInfo, size)) {
        SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_E2PROM, 1, EVENT_OCCUR, NULL);
        return CL_FAIL;
    }
    return CL_OK;
}


//读枪头信息
int FlashReadGunInfo(uint8_t *pGunInfo, uint16_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++) 
	{
	    at24c64_eepromRead(GunInfoEepromAddr+i, &pGunInfo[i], 1);
    }
	return CL_OK;
}


//写计量干扰矩阵系数
int FlashWritMatrix(uint8_t *data, uint16_t size)
{
    if (CL_OK != EepromWriteData(EMUCaliationAddr, data, size)) 
	{
        return CL_FAIL;
    }
	
    return CL_OK;
}

//读计量干扰矩阵系数
int FlashReadMatrix(uint8_t *data, uint16_t size)
{
	uint32_t i;

    for (i = 0; i < size; i++) 
	{
	    at24c64_eepromRead(EMUCaliationAddr+i, &data[i], 1);
    }
	return CL_OK;
}


void E2promReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++) 
	{
        at24c64_eepromRead(addr+i, (void*)&pBuff[i], 1);
    }
}


int E2promWriteDataEx(uint32_t addr, void* pBuff, uint16_t len)
{
    return at24c64_eepromWrite(addr, pBuff, len);
}

#endif


