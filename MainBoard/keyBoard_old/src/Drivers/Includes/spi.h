#ifndef __SPI_H__
#define __SPI_H__

//SPCON位定义
#define SPEN(N)		(N<<7)
#define LSBF(N)		(N<<6)
#define SSIG(N)		(N<<5)
#define MSTR(N)		(N<<4)
#define CPOL(N)		(N<<3)
#define CPHA(N)		(N<<2)
#define CPOS(N)		(N)	    //3:系统时钟选择为定时器2溢出
	    					//2:系统时钟选择为定时器1溢出
							//1:系统时钟选择为1/8系统时钟	
							//0:系统时钟选择为1/4系统时钟	

//SPSTA位定义
#define SPIE		(1<<7)   //SPI中断使能
#define WCOL		(1<<2)   //写入冲突标志
#define MODF		(1<<1)   //故障模式
#define SPIF		(1<<0)   //传输完成


#define SPI_CS_HIGH()   (P6 |= BIT5)
#define SPI_CS_LOW()    (P6 &= ~(1<<5))


#define NPD_HIGH()      (P1 |= BIT1)
#define NPD_LOW()       (P1 &= ~(1<<1))


void SPI_init(void);

unsigned char SPIRead(unsigned char addr);

void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);

void SPIWrite(unsigned char addr,unsigned char wrdata);

void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);


#endif
