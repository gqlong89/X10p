#include "includes.h"
#include "proto.h"



FIFO_S_t BlueFIfO;

extern SystemInfo system_info;
extern SystemStatus system_ststus;
extern bit isBuleConnect;
extern Time_t xdata lastConnectTime;


//开机--蓝牙设备检测
int CheckBlueDevice(void)
{
    char step = 0;
    while(1){
        switch(step){
            case 0://AT 指令测试
                if(BuleCheckReset(3) == OK){
                    step = 1;
                }else{
                    return ERROR;
                }
            break;
            case 1://获取蓝牙设备信息

                if(GetBluInfo(system_info.blue_name,system_info.blue_version) == OK){
                    step = 2;
                }else{
                    return ERROR;
                }

            break;
            case 2://获取网关地址

                if(GetGW_MAC(system_info.gw_mac) == OK){
                    step = 3;
                }else{
                    return ERROR;
                }
            break;
            case 3://设置成终端节点
                if(GetDevtType() == 1){
                    if(SetDevt() == OK){
                     step = 4;
                    }else{
                        return ERROR;
                    }
                }else{
                    step = 4;
                }
            break;
            case 4://设置配对状态
                if(SetBluePair(1) == OK){
                    return OK;
                }else{
                    return ERROR;
                }
            break;
        }
    }
}


void BlueBluetooth_Reset(void)
{
    if(CheckBlueDevice() != OK){
        system_info.blue_state = 1;
        system_ststus.blue_state = 1;
        Debug_Log("blue check error.\n");

    }else{
        Debug_Log("\nblue check success.\n");
    }

    //CD 置高 ，接收数据
    BLUE_CD_HIGHT();

    FIFO_S_Flush(&BlueFIfO);
}



//蓝牙初始化
void Bluetooth_Init(void)
{
    //蓝牙串口接收缓存区
    static unsigned char xdata BlueRXDataBuf[BLUE_RX_BUF];

    BlueGPIO_Conf();

    Uart1_Initial(115200);
    FIFO_S_Init(&BlueFIfO,BlueRXDataBuf,BLUE_RX_BUF);

    BlueBluetooth_Reset();
}




int SendBlueData(unsigned char nodeType, unsigned char *m_data, unsigned int len)
{
    BLUE_PKT_STR  pFrame;
    unsigned char checksum=0;
    Time_t xdata syncSystemStstusTime=getSystemTimer();

    //检测IRQ 2s超时
    while(READ_BT_IRQ() != 0){
        WDT_FeedDog();
        Delay_100us(10);
        if((unsigned int)(getSystemTimer() - syncSystemStstusTime) >= (TIMEOUT_1s*2)){
            Debug_Log("SendBlueData timeout\n");
            break;
            //return ERROR;
        }
    }

    pFrame.head.ab = 0xab;
    pFrame.head.cd = 0xcd;
    memset(pFrame.head.addr,0,6);
    if(nodeType == 1){
        memcpy(pFrame.head.addr,system_info.gw_mac,5);
    }
    pFrame.head.target = nodeType;
    pFrame.head.len[0] = (len & 0xFF);
    pFrame.head.len[1] = (len << 8) & 0xFF;

    checksum = checkSum((unsigned char*)&pFrame, 11);
    checksum += checkSum(m_data,len);

    pFrame.end[0] = checksum;
    pFrame.end[1] = '\r';
    pFrame.end[2] = '\n';

    Uart_SendData(UART_1,(char*)&pFrame,11);
    Uart_SendData(UART_1,(char*)m_data,len);
    Uart_SendData(UART_1,(char*)pFrame.end,3);

    return OK;
}



void HandleBlueMsg(void)
{
    static unsigned char xdata gBuleRxData[BLUE_MAX_SIZE];
    static unsigned int  xdata pktLen = 0;
    static unsigned char step = BLUE_AB;
    static unsigned char  sum = 0;
    static unsigned char *pktBuff = gBuleRxData;
    static unsigned int  xdata len,datalen;
    static unsigned int xdata time;
    unsigned char temp_data;

    if (BLUE_AB != step) {
        if (8 < ((unsigned int)((unsigned int)getSystemTimer() - time))) {
            step = BLUE_AB;
        }
    }

    while(FIFO_S_Get(&BlueFIfO, &temp_data) == 0) {
        switch (step) {
            case BLUE_AB:
                if (temp_data == 0xAB) {
                    time = getSystemTimer();
                    step = BLUE_CD;
                    pktBuff[0] = 0xAB;
                    pktLen = 1;
                    sum = 0xAB;
                }
            break;
        case BLUE_CD:
            if (temp_data == 0xcd) {
                step = BLUE_TARGET;
                pktBuff[1] = 0xcd;
                pktLen++;
                sum += temp_data;
            }else if (temp_data == 0xAB) {
                step = BLUE_CD;
                pktBuff[0] = 0xAB;
                pktLen = 1;
                sum = 0xAB;

            }else{
                step = BLUE_AB;
            }
            break;
            case BLUE_TARGET:
                pktBuff[pktLen++] = temp_data;
                len = 0;
                if(temp_data == 0){
                    step = BLUE_LEN;
                    isBuleConnect = 1;
                    lastConnectTime = getSystemTimer();
                }
                else
                    step = BLUE_ADDR;
                sum += temp_data;
                break;

            case BLUE_ADDR:
                pktBuff[pktLen++] = temp_data;
                sum += temp_data;
                if (6 == ++len) {
                    len = 0;
                    step = BLUE_LEN;
                }
                break;

            case BLUE_LEN:
                pktBuff[pktLen++] = temp_data;
                sum += temp_data;
                if (2 == ++len) {
                    datalen = len = (pktBuff[pktLen-1]<<8) | pktBuff[pktLen-2];
                    if ((512-16) < len) {
                        step = BLUE_AB;
                    }else if (0 == len) {
                        step = BLUE_CHK;
                    }else{
                        step = BLUE_RX_DATA;
                    }
                }
                break;

            case BLUE_RX_DATA:
                pktBuff[pktLen++] = temp_data;
                sum += temp_data;
                if (0 == --len) {
                    step = BLUE_CHK;
                }
                break;

            case BLUE_CHK:
                pktBuff[pktLen++] = temp_data;
                if (temp_data == sum) {
                    step = BLUE_END;
                    len = 0;
                }else{
                    step = BLUE_AB;
                }
                break;

            case BLUE_END:
                pktBuff[pktLen++] = temp_data;
                if (2 == ++len) {
                    //datalen 表示有效数据长度
                    TransBlueData(pktBuff,datalen);//数据透传
                    step = BLUE_AB;
                }
                break;

            default:
                step = BLUE_AB;
                break;
        }
    }
}