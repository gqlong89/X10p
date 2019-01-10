#ifndef __SPI_H__
#define __SPI_H__

//SPCONλ����
#define SPEN(N)		(N<<7)
#define LSBF(N)		(N<<6)
#define SSIG(N)		(N<<5)
#define MSTR(N)		(N<<4)
#define CPOL(N)		(N<<3)
#define CPHA(N)		(N<<2)
#define CPOS(N)		(N)	    //3:ϵͳʱ��ѡ��Ϊ��ʱ��2���
	    					//2:ϵͳʱ��ѡ��Ϊ��ʱ��1���
							//1:ϵͳʱ��ѡ��Ϊ1/8ϵͳʱ��	
							//0:ϵͳʱ��ѡ��Ϊ1/4ϵͳʱ��	

//SPSTAλ����
#define SPIE		(1<<7)   //SPI�ж�ʹ��
#define WCOL		(1<<2)   //д���ͻ��־
#define MODF		(1<<1)   //����ģʽ
#define SPIF		(1<<0)   //�������


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
