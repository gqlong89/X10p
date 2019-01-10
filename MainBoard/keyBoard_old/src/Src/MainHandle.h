#ifndef __MAINHANDLE_H__
#define __MAINHANDLE_H__

#define WRITE_SUCCESS   (0x00) //
#define WRITE_ERROR   (0x01)

#define ENTRANCE_GUARD_CARD_SECTOR_OFFSET       (10)  //ÃÅ½û¿¨ÉÈÇøÆ«ÒÆ

//Ð´¿¨ÇëÇó
typedef struct {
	unsigned char keyA[6];
    unsigned char sectorNum;
	unsigned char blockNun;
    unsigned char blockData[16];
}WRITE_CARD_REQ_STR;

void Flash_Init(void);


void Flash_Write(unsigned int sector,unsigned char *buf, size_t wlen);
size_t Flash_Read(unsigned int sector,unsigned char *buf, size_t rlen);

void Main_Handle(void);


#endif

