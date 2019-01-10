#include "includes.h"

extern SystemStatus system_ststus;

void BlueGPIO_Conf(void)
{
    //BT_EN
    GPIO_Init(P21F,OUTPUT);
    
    //BT_CD
    GPIO_Init(P23F,OUTPUT);
    
    //BT_IRQ
    GPIO_Init(P22F,INPUT);
    
    //BT_SLP
    GPIO_Init(P20F,OUTPUT);
    
}

void BuleReset(void)
{
    Debug_Log("blue is resetting.\n");
    BLUE_DISENABLE();
    WDT_FeedDog();
    Delay_ms(1000);
    WDT_FeedDog();
    Delay_ms(1000);
    WDT_FeedDog();
    BLUE_ENABLE();
    Delay_ms(1000);
    WDT_FeedDog();
}

int BlueCheckRes(char *res, unsigned int tmo, unsigned char *pbuff, unsigned int size)
{
    char *ret=NULL;
    char c;
    int cnt = 0;
    int retv = ERROR;
    unsigned int time = 0;
    memset(pbuff, 0, size);
    
    for(time = 0; time < tmo; time+= 50) {
        Delay_ms(50);
        WDT_FeedDog();
         while(FIFO_S_Get(&BlueFIfO, &c) == 0) {
            if (c) {
                if (size > cnt) {
                    pbuff[cnt++] = c;
                }else{
                }
            }
         }
         
        ret = strstr((const char*)pbuff, (const char*)res);
        if (ret) {
            retv = OK;
            break;
        }
    }

    return retv;
}



int BlueSendCmd(char *cmd, char *ack, unsigned int waittime,unsigned char *gBlueRecvData,unsigned int size)
{
	unsigned char res = 1;

    BLUE_CD_LOW();
    Delay_ms(1);
    FIFO_S_Flush(&BlueFIfO);
    Uart_SendData(UART_1,cmd, strlen(cmd));

    if ((ack==NULL) || (waittime==0)) {
        BLUE_CD_HIGHT();
        return OK;
    }

    if (OK == BlueCheckRes(ack, waittime, gBlueRecvData, size)) {
        
        res = OK; /*check success, retrun 0*/
    }
    BLUE_CD_HIGHT();
	return res;
}



unsigned char Asc2Int(char ch) {
    unsigned char val = 0;

    if ((ch >= '0') && (ch <= '9')) {
        val = ch - '0';
    } else if ((ch >= 'A') && (ch <= 'F')) {
        val = ch - 'A' + 10;
    } else if ((ch >= 'a') && (ch <= 'f')) {
        val = ch - 'a' + 10;
    }
    return val;
}

int StringToBCD(unsigned char *BCD, const char *str) 
{
    unsigned char chh, chl;
    int length = strlen(str);
    int index = 0;

    for (index = 0; index < length; index += 2) {
        chh = Asc2Int(str[index]);
        chl = Asc2Int(str[index + 1]);
        if (chh == -1 || chl == -1)
            return -1;
        BCD[index / 2] = (chh << 4) | chl;
    }
    return (length / 2);
}

char *BCDToString(char *dest, unsigned char *BCD, int bytes) {
    char xdata temp[] = "0123456789ABCDEF";
    int index = 0;
    int length = 0;
    if (BCD == NULL || bytes <= 0)
        return NULL;
    
    for (index = 0; index < bytes; index++) {
        dest[length++] = temp[(BCD[index] >> 4) & 0x0F];
        dest[length++] = temp[BCD[index] & 0x0F];
    }
    dest[length] = '\0';
    return dest;
}


int BlueTest(int retry)
{
    unsigned char i = 0;
    unsigned char xdata gBlueRecvData[20];
    
    for (i=0; i<retry; i++) {
        if (BlueSendCmd("AT\r\n", "OK", 1000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            return OK;
        }
        if(retry > 1){
            WDT_FeedDog();
            Delay_ms(1000);
        }
    }
    return ERROR;
}

int BuleCheckReset(int retry)
{
    unsigned char i = 0;
    
    for (i=0; i<retry; i++) {
        
        BuleReset();
   
        if(BlueTest(2) == OK)
        {
            return OK;
        }
    }
    return ERROR;
}


int BlueRecovery(void)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
    for (i=0; i<2; i++) {
        if (BlueSendCmd("AT+RSTF\r\n", "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            Debug_Log("BlueRecovery OK.\n");
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    Debug_Log("BlueRecovery error.\n");
    
    return ERROR;

}

static void EnableBule(void)
{
    BLUE_ENABLE();
    system_ststus.blue_state = 0;
}

int BuleReset_Async(void)
{
    if(StartCallBack(TIMEOUT_1s*2,EnableBule) == OK){
        BLUE_DISENABLE();
        return OK;
    }
    
    return ERROR;
}


int SetBlueName(char *name)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
    char xdata buf[32];
    memset(buf,0,32);
    sprintf(buf,"AT+NAME=%s\r\n",name);
    for (i=0; i<2; i++) {
        if (BlueSendCmd(buf, "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    return ERROR;
}





//mac为字符串
int SetGW_MacAddr(char *mac)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
    char xdata buf[32];
    memset(buf,0,32);
    sprintf(buf,"AT+GWID=%s\r\n",mac);
    for (i=0; i<2; i++) {
        if (BlueSendCmd(buf, "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    return ERROR;
}

int SetBluePair(int value)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
    char xdata buf[32];
    memset(buf,0,32);
    sprintf(buf,"AT+PAIR=%d\r\n",value);
    for (i=0; i<3; i++) {
        
        if (BlueSendCmd(buf, "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }

    return ERROR;
}

int GetDevtType(void)
{
    int ret = -1;
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
        for (i=0; i<2; i++) {
        if (BlueSendCmd("AT+DEVT?\r\n", "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            Debug_Log(gBlueRecvData);
            sscanf(gBlueRecvData,"+DEVT: %d",&ret);
            Debug_Hex((unsigned char*)&ret,2);
            return ret;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    return -1;

}

int SetDevt(void)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
        for (i=0; i<3; i++) {
        if (BlueSendCmd("AT+DEVT=0\r\n", "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    return ERROR;
}

int GetGW_MAC(unsigned char *mac)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[32];
    for (i=0; i<2; i++) {
        if (BlueSendCmd("AT+GWID?\r\n", "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            char xdata temp[20];
            memset(temp,0,20);
            sscanf(gBlueRecvData,"+GWID: %s",temp);
            StringToBCD(mac,temp);
            
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    return ERROR;
}

int GetBluInfo(char *name,char *blue_version)
{
    int i = 0;
    unsigned char xdata gBlueRecvData[200];
    for (i=0; i<3; i++) {
        if (BlueSendCmd("AT+INFO?\r\n", "OK", 2000,gBlueRecvData,sizeof(gBlueRecvData))==OK) {
            
            char xdata temp[32];
            char *s;
            Debug_Log(gBlueRecvData);
            //获取蓝牙名
            memset(temp,0,32);
            s = strstr(gBlueRecvData,"[NAME]");
            sscanf(s,"[NAME]: %s",temp);
            if(strlen(temp) >= 12)
                memcpy(name,temp,12);
            else
                strcpy(name,temp);
            
            //获取版本号
            memset(temp,0,32);
            s = strstr(gBlueRecvData,"[VERS]");
            sscanf(s,"[VERS]: %s",temp);//
            if(strlen(temp) >= 20)
                memcpy(blue_version,temp,20);
            else
                strcpy(blue_version,temp);
            
            return OK;
        }
        WDT_FeedDog();
        Delay_ms(1000);
    }
    return ERROR;
}

