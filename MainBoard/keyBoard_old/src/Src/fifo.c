#ifndef __FIFO_C__
#define __FIFO_C__

#include "fifo.h"
#include <stdio.h>
#include <string.h>



int FIFO_S_Init(FIFO_S_t* pFIFO, void* pBaseAddr, unsigned int UnitCnt)
{
    memset(pFIFO, 0, sizeof(FIFO_S_t));
    pFIFO->pStartAddr = pBaseAddr;
    pFIFO->buffSize = UnitCnt;
    return 0;
}

int FIFO_S_Put(FIFO_S_t* pFIFO, unsigned char Element) reentrant
{
    unsigned int datalen;

    if (pFIFO->writeIndex >= pFIFO->readIndex) {
        datalen = pFIFO->writeIndex - pFIFO->readIndex+1;
    }else{
        datalen = pFIFO->writeIndex + pFIFO->buffSize - pFIFO->readIndex+1;
    }

    if (pFIFO->buffSize > datalen) {
        pFIFO->pStartAddr[pFIFO->writeIndex++] = Element;
        pFIFO->usedLen++;
        if (pFIFO->buffSize <= pFIFO->writeIndex) {
            pFIFO->writeIndex = 0;
        }
        return 0;
    }
    return 1;
}


int FIFO_S_Get(FIFO_S_t* pFIFO, unsigned char *pData) reentrant
{
    if (pFIFO->writeIndex != pFIFO->readIndex) {
        *pData = pFIFO->pStartAddr[pFIFO->readIndex++];
        if (pFIFO->buffSize <= pFIFO->readIndex) {
            pFIFO->readIndex = 0;
        }
        pFIFO->usedLen--;
        return 0;
    }
    return 1;
}



int FIFO_S_IsEmpty(FIFO_S_t* pFIFO)
{
    return (pFIFO->writeIndex == pFIFO->readIndex) ? 0 : 1;
}



int FIFO_S_CountUsed(FIFO_S_t* pFIFO)
{
    return (int)(pFIFO->usedLen);
}


int FIFO_S_Flush(FIFO_S_t* pFIFO)
{
    pFIFO->usedLen = 0;
    pFIFO->readIndex = 0;
    pFIFO->writeIndex = 0;
    return 0;
}



#endif