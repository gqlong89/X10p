#ifndef __BLUE_API_H__
#define __BLUE_API_H__


#define BLUE_DISENABLE() {P21 = 0;}
#define BLUE_ENABLE()    {P21 = 1;}

#define BLUE_CD_HIGHT()  {P23 = 1;}
#define BLUE_CD_LOW()    {P23 = 0;}

#define READ_BT_IRQ()     P22


typedef struct{
    char *cmd;
    char *res;
    int wait;
    int nwait;
    int (*process)(char ok, unsigned char retry);
}xdata BlUE_INITTAB;


typedef enum{
    BlUE_INS = 0,   //Ö¸Áî      
    BLUE_DATA = 1,  //Êý¾Ý

}BlUE_STEP;

void BlueGPIO_Conf(void);


int StringToBCD(unsigned char *BCD, const char *str) ;
char *BCDToString(char *dest, unsigned char *BCD, int bytes);

int BlueRecovery(void);
int BuleReset_Async(void);
int BlueTest(int retry);
int BuleCheckReset(int retry);
int SetBluePair(int value);
int SetBlueName(char *name);
int SetGW_MacAddr(char *mac);
int GetDevtType(void);
int SetDevt(void);
int GetBluInfo(char *name,char *blue_version);
int GetGW_MAC(unsigned char *mac);

#endif
