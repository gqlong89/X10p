#ifndef _UART_H_
#define _UART_H_

typedef enum{
  UART_0 = 0,
  UART_1 = 1,
  UART_2 = 2,
}UART_Def;



void Uart0_Initial(unsigned long int baudrate);
void Uart1_Initial(unsigned long int baudrate);
void Uart2_Initial(unsigned long int baudrate);



void Uart_PutChar(UART_Def num,unsigned char bdat);
void Uart_SendData(UART_Def num,char *bdat,unsigned short len);

void Debug_Log(char *string);
void Debug_Hex(unsigned char *input_byte,unsigned int input_len);

#endif
