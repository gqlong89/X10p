#ifndef __FIFO_H__
#define __FIFO_H__



typedef struct
{
    volatile unsigned char* pStartAddr;                    //!< FIFO Memory Pool Start Address
    unsigned int buffSize;                               //!< The capacity of FIFO
    volatile unsigned int usedLen;                       //!< The number of elements in FIFO
    volatile unsigned int readIndex;                     //!< FIFO Data Read Index Pointer
    volatile unsigned int writeIndex;                    //!< FIFO Data Write Index Pointer
}xdata FIFO_S_t;



extern  int FIFO_S_Init(FIFO_S_t* pFIFO, void* pBaseAddr, unsigned int UnitCnt) ;

int FIFO_S_Put(FIFO_S_t* pFIFO, unsigned char Element) reentrant;

extern  int FIFO_S_Get(FIFO_S_t* pFIFO, unsigned char *pData) reentrant;

extern  int FIFO_S_IsEmpty(FIFO_S_t* pFIFO) ;

extern int FIFO_S_CountUsed(FIFO_S_t* pFIFO) ;

extern  int FIFO_S_Flush(FIFO_S_t* pFIFO) ;

#endif
