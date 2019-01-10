#include "includes.h"
#include "MainHandle.h"



extern SystemStatus system_ststus;

extern SystemInfo system_info;


//接收缓存区
unsigned char xdata commRXDataBuf[MAX_RECV_BUF];
    
static unsigned char  MSG_ID = 0;
    
FIFO_S_t commFIfO;

void CommTask_Init(void)
{
    Uart2_Initial(115200);
    
    FIFO_S_Init(&commFIfO,commRXDataBuf,MAX_RECV_BUF);
}


unsigned char checkSum(unsigned char *buf,unsigned int len)
{
    unsigned char  checkSum = 0 ;
    unsigned int  i = 0;
    
    for(i = 0;i<len;i++){
        checkSum += buf[i];
    }
    
    return checkSum;
}


int Package_Send(ProtoPackage *st)
{
    unsigned int protoLen = st->msgLen+5;
    
    st->head.head[0] = ((MSG_HEAD >> 8) & 0xFF);
    st->head.head[1] = (MSG_HEAD & 0xFF);
    st->head.len[0] = (protoLen & 0xFF);
    st->head.len[1] = ((protoLen>>8) & 0xFF);
    st->checksum = st->head.version = COMM_VERSION;
    st->head.msgId = MSG_ID++;
    
    st->checksum += st->head.msgId;
    st->checksum += st->head.type;
    st->checksum += st->head.cmd;
    st->checksum += checkSum(st->msg,st->msgLen);
    
    Uart_SendData(UART_2,(unsigned char*)&st->head,sizeof(ProtoPackageHead));
    Uart_SendData(UART_2,st->msg,st->msgLen);
    Uart_SendData(UART_2,&st->checksum,1);
    
    return OK;
}

//同步基本信息
int SyncSystemInfo(void)
{
    ProtoPackage st;
    st.head.cmd = CMD_UP_SYSTEM_INFO;
    st.head.type = MsgType_ALL;
    st.msgLen = sizeof(SystemInfo);
    st.msg = (unsigned char *)&system_info;
    return Package_Send(&st);
}

//状态同步
int SyncSystemState(void)
{
    ProtoPackage st;
    st.head.cmd = CMD_UP_SYSTEM_STATE;
    st.head.type = MsgType_ALL;
    st.msgLen = sizeof(SystemStatus);
    st.msg = (unsigned char *)&system_ststus;
    return Package_Send(&st);
}


//键值上报
int KeyValuesUpLoad(unsigned char values)
{    
    unsigned char xdata buf[1];
    ProtoPackage st;
    st.head.cmd = CMD_KEY;
    st.head.type = MsgType_KEY;
    buf[0] = values;
    st.msgLen = 1;
    st.msg = buf;
    return Package_Send(&st);
}


//卡类型上报
int CardTypeUpLoad(unsigned char cardType,unsigned char *serialNum)
{
    unsigned char xdata buf[64];
    ProtoPackage st;
    st.head.cmd = CMD_CARD_UP;
    st.head.type = MsgType_CARD;
    buf[0] = cardType;
    memcpy(buf+1,serialNum,4);
    st.msgLen = 5;
    st.msg = buf;
    return  Package_Send(&st);
}

/*****************************************************************************
** Function name:       WriteCardBlockSuccessUpLoad
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int WriteCardBlockSuccessUpLoad(unsigned char result)
{
    unsigned char xdata buf[1];
    ProtoPackage st;
    st.head.cmd = CMD_CARD_WRITE;
    st.head.type = MsgType_CARD;
    buf[0] = result;
    st.msgLen = 1;
    st.msg = buf;
    return  Package_Send(&st);
}

//卡号上报
int CardNumUpLoad(unsigned char result,unsigned char *cardNum)
{
    unsigned char xdata buf[17];
    ProtoPackage st;
    st.head.cmd = CMD_CARD_READ;
    st.head.type = MsgType_CARD;
    buf[0] = result;
    memcpy(buf+1,cardNum,16);
    st.msgLen = 17;
    st.msg = buf;
    return  Package_Send(&st);
}

//上报PCB编号
int PCBBumUpLoad(unsigned char *pcb)
{
    ProtoPackage st;
    st.head.type = MsgType_ALL;
    st.head.cmd = CMD_READ_PCB;
    st.msgLen = 8;
    st.msg = pcb;
    return  Package_Send(&st);
}


//发送响应消息
int SendMsg_ACK(MsgType type,unsigned char cmd,unsigned char result)
{
    unsigned char xdata buf[1];
    ProtoPackage st;
    st.head.cmd = cmd;
    st.head.type = type;
    buf[0] = result;
    st.msgLen = 1;
    st.msg = buf;
    return  Package_Send(&st);
}


//转发蓝牙、2.4G数据
int TransBlueData(unsigned char *gBuleRxData,unsigned int datalen)
{
    unsigned char *p = gBuleRxData;
    unsigned char nodeType = gBuleRxData[2];
    unsigned int off = 0;
    ProtoPackage st;
    st.head.cmd = CMD_BLUE_SEND;
    st.head.type = MsgType_BLUE;
    st.msgLen = 1+2+datalen;
    
    if(nodeType == 0){
        off = 2;
    }else if(nodeType == 1){
        off = 8;
    }
    p += off;
    
    *p++ = nodeType;
    *p++ = (datalen & 0xFF);
    *p++ = ((datalen<<8) & 0xFF);
  
    st.msg = gBuleRxData+off;
    
//    Debug_Log("recv blue data:");
//    Debug_Hex(st.msg,st.msgLen);
//    Debug_Log("\r\n");
    
    return  Package_Send(&st);
}



void System_Reset(void)
{
    ((void(code *)(void))0x0000)();
}

