#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__


#define BLUE_RX_BUF     230
#define BLUE_MAX_SIZE   256

typedef struct {
    
    struct{
        unsigned char  ab;                                //0.
        unsigned char  cd;                                //1.
        unsigned char  target;                            //2.
        unsigned char  addr[6];
        unsigned char  len[2];                            //4.
    }head;
    unsigned char *msg;
    unsigned char end[3];
}xdata BLUE_PKT_STR;


enum {
    BLUE_AB,
    BLUE_CD,
    BLUE_TARGET,
    BLUE_ADDR,
    BLUE_LEN,
    BLUE_RX_DATA,
    BLUE_CHK,
    BLUE_END,
};


extern FIFO_S_t BlueFIfO;

void Bluetooth_Init(void);

void BlueBluetooth_Reset(void);

void HandleBlueMsg(void);

int SendBlueData(unsigned char nodeType, unsigned char *m_data, unsigned int len);

#endif

