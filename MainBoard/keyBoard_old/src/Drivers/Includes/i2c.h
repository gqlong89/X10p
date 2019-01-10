#ifndef __I2C_H__
#define __I2C_H__

//I2CCON定义
#define I2CE(N)		(N<<7)
#define I2CIE(N)	(N<<6)
#define STA(N)		(N<<5)
#define STP(N)		(N<<4)
#define CKHD(N)		(N<<3)
#define AAK(N)		(N<<2)
#define CBSE(N)		(N<<1)
#define STFE(N)		(N<<0)

//I2CADR定义
#define   GCE(N)		(N<<7) //N = 0~1

#define   SPFE(N)		(N<<7) //N = 0~1

//I2CFLG定义
#define I2CF		(1<<0)

//IIC从设备地址
#define I2C_ADDR	          0x53



#define IIC_INT()   {P52 = ~P52;}

#define IIC_INT_HIGH()   (P5 |= BIT2)
#define IIC_INT_LOW()    (P5 &= ~(1<<2))


void I2C_init(unsigned char i2cadr);

void IIC_Write(unsigned char *buf,unsigned int len);
void IIC_Putc(unsigned char bdat);



#endif

