
#ifndef __USART_H__
#define __USART_H__

#include "includes.h"
#include "FIFO.h"

#define  PHY_UART                       1   //物理串口
#define  SIMU_UART                      2   //模拟串口

#define  SYS_UART_CNT                   10

#define  GPRS_UART_PORT                 0
#define  UART_485_PORT                  2
#define  DEBUG_UART                     8
#define  CKB_UART						1

#define  UART0_RX_SIZE                  520
#define  UART1_RX_SIZE                  OUT_NET_PKT_LEN
#define  UART2_RX_SIZE                  NET_485_PKT_LEN
#define  UART3_RX_SIZE                  64
#define  UART4_RX_SIZE                  64
#define  UART5_RX_SIZE                  64
#define  UART6_RX_SIZE                  64
#define  UART7_RX_SIZE                  64
#define  UART8_RX_SIZE                  4
#define  UART9_RX_SIZE                  64

typedef struct{
    uint8_t  deviceIndex;               //对应模拟串口数组索引
    uint8_t  buffIndex;
    uint8_t  rsv;
    uint8_t  rsv1;
}SIM_UART_INFO_STR;

typedef struct{
    uint8_t  index;                     //串口系统索引 0~8
    uint8_t  type;                      //串口类型，1硬件串口  2模拟串口
    uint16_t recvCnt;
    uint16_t putCnt;
    __IO uint16_t allCnt;
    __IO uint16_t sendCnt;
    __IO uint8_t  *pSendData;
    FIFO_S_t rxBuffCtrl;                //接收缓存控制信息
    union {
        HT_UART_TypeDef * sysAddr;
        SIM_UART_INFO_STR simUart;
    }Data;
}UART_INFO_STR;


extern void UsartInit(void);
int UsartSend(int portIndex, const uint8_t *pData, uint16_t len);
int UsartGetOneData(int portIndex, uint8_t *pData);
int GetUsartRecvCnt(int portIndex);
int GetUsartRecvData(int portIndex, uint8_t *pBuff, uint16_t len);
void UsartFlush(int portIndex);
int GetUsartPutCnt(int portIndex);
int GetUsartOverFlow(int portIndex);
int UsartPut(int portIndex, uint8_t data);
void Init_Uart0(void);
void Init_Uart1(void);
void Init_Uart2(void);
void Init_Uart3(void);
void Init_Uart4(void);
void Init_Uart5(void);
void Init_Uart6(void);
void Init_Uart7(void);
void Init_Uart8(void);
void Init_Uart9(void);


extern UART_INFO_STR gUartPortAddr[SYS_UART_CNT];
extern void (*gUartInitFun[])(void);

#endif //__USART_H__



