#include "includes.h"
#include "MainHandle.h"
#include "proto.h"

//KEYA
static char code SecretCardKEY_A[6] = {'6', 'f', '7', 'd', '2', 'k'};

//ϵͳ��Ϣ
SystemInfo system_info;

//ϵͳ״̬
SystemStatus system_ststus;

//ϵͳ��Ϣ�Ƿ�ͬ��
bit isSyncSystemInfo = 0;


Time_t xdata cardFlagTime=0;
//Ѱ����־
bit cardFlag = 1;

//�����Ƿ�����
bit isBuleConnect=0;

//�����������ʱ��
Time_t xdata lastConnectTime = 0;

//������������
unsigned char xdata rebootBlueCnt = 0;



void Flash_Init(void)
{
    PADRD = 127;//�������256���ֽ���Ϊ������
}

void Flash_Write(unsigned int sector,unsigned char *buf, size_t wlen)
{
    unsigned int addr = sector*128;
    unsigned char i;

    FSCMD = 0;
    LOCK = 0x2A;
    PTSH = (unsigned char)((sector*0x80)>>8);
    PTSL = (unsigned char)(sector*0x80);
    FSCMD = 3;
    LOCK = 0xAA;

    FSCMD = 0;
    LOCK = 0x2A;
    PTSH = (unsigned char)(addr>>8);
    PTSL = (unsigned char)addr;
    FSCMD = 2;
    for(i = 0;i<wlen;i++){
        FSDAT = buf[i];
    }

    LOCK = 0xAA;
}


size_t Flash_Read(unsigned int sector,unsigned char *buf, size_t rlen)
{
    unsigned int addr = sector*128;
    unsigned char *p = buf;
    unsigned char i;
    FSCMD = 0;
    LOCK = 0x2A;

    PTSH = (unsigned char)(addr>>8);
    PTSL = (unsigned char)addr;
    FSCMD = 1;

    for(i = 0;i<rlen;i++){
        *p++ = FSDAT;

    }
    return (p-buf);
}


//����ʱ���־
Time_t xdata keyFlagTime = 0;
bit keyFlag = 1;



//���������¼�
void key_onclick_callback(unsigned int values)
{
    KeyValuesUpLoad(values);
    keyFlag = 0;
    keyFlagTime = getSystemTimer();

    Debug_Log("key:");
    Debug_Hex((char*)&values,2);
    Debug_Log("\r\n");
}

 int GetPktSum(unsigned char *pData, unsigned int len)
 {
     int i;
     unsigned char sum = 0;

     for (i=0; i<len; i++) {
         sum += pData[i];
     }
     return sum;
 }


 void Check_M1_Card()
 {
    //�����ͱ�־����UID������
    unsigned char xdata PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
    unsigned char result = 0;

    if ((cardFlag == 1))
    {
        if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK) == OK))
        {
            Debug_Log("���� UID1.\n");
            if(PICC_ATQA[0]==0x04)  //M1��
            {
                //Debug_Log("read card ID3:");
                //Debug_Hex(PICC_UID,4);
                //Debug_Log("\r\n");

                //������֤
                //KEY1��֤�ɹ�---��Կ��
                if((Mifare_Auth(0, 2, SecretCardKEY_A,PICC_UID) == OK))
                {
                    result++;
                    Debug_Log("����2 result = ");
                    Debug_Hex(&result,1);
                    Debug_Log("\n");

                    CardTypeUpLoad(1,PICC_UID);
                    //Debug_Log("��Կ������2.\n");
                }
                TypeA_Halt();
                if(0 < result)  //�����Ƿ���Կ��
                {
                    cardFlagTime = getSystemTimer();
                    cardFlag = 0;
                    return;
                }
            }
            if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK) == OK))
            {
                if(PICC_ATQA[0]==0x04)  //M1��
                {
                    if(Mifare_Auth(0, (ENTRANCE_GUARD_CARD_SECTOR_OFFSET + 2), SecretCardKEY_A,PICC_UID) == OK)
                    {
                        result++;
                        Debug_Log("����12 result = ");
                        Debug_Hex(&result,1);
                        Debug_Log("\n");

                        CardTypeUpLoad(1,PICC_UID);
                        //Debug_Log("��Կ������12.\n");
                        cardFlagTime = getSystemTimer();
                        cardFlag = 0;
                    }
                    else if(0 == result)  //�����Ƿ���Կ��
                    {
                        Debug_Log("����Կ��.\n");
                        CardTypeUpLoad(2,PICC_UID);
                        cardFlagTime = getSystemTimer();
                        cardFlag = 0;
                    }
                    TypeA_Halt();
                    return;
                }
            }
        }
    }
}


 //��������
void peraseMsg(unsigned char *buf,unsigned int len)
{
    ProtoPackage st;
    unsigned char *p = buf;
    if(len < 9) return ;

    memcpy((void*)&st.head,p,sizeof(ProtoPackageHead));
    p += sizeof(ProtoPackageHead);

    if(st.head.version != COMM_VERSION){
        return ;
    }
    st.msgLen = (st.head.len[0] | (st.head.len[1]<<8)) - 5;
    st.msg = p;

    switch(st.head.type){

        //������Ϣ�ϱ�
        case MsgType_ALL:
            if(st.head.cmd == CMD_UP_SYSTEM_INFO){
                isSyncSystemInfo = 1;
                Debug_Log("recv system info ack..\n");
            }
            //ϵͳ����
            else if(st.head.cmd == CMD_SYSTEM_OPERATE){

                SendMsg_ACK(MsgType_ALL,CMD_SYSTEM_OPERATE,0);

                //ϵͳ��λ
                if(st.msg[0] == 0x01){
                    Debug_Log("System_Reset req .\n");
                    System_Reset();
                }
                //������λ
                else if(st.msg[0] == 0x02){
                    Debug_Log("BlueBluetooth_Reset req .\n");
                    BlueBluetooth_Reset();
                }
                //�����㲥����
                else if(st.msg[0] == 0x03){
                    SetBluePair(st.msg[1]);
                }
            }
            //дPCB
            else if(st.head.cmd == CMD_WRITE_PCB){
                Flash_Write(0,st.msg,8);
                SendMsg_ACK(MsgType_ALL,CMD_WRITE_PCB,0);
            }
            //��PCB
            else if(st.head.cmd == CMD_READ_PCB){

                unsigned char xdata pcb[8];
                Flash_Read(0,pcb,8);
                PCBBumUpLoad(pcb);
            }
        break;

        //����
        case MsgType_KEY:
            if(st.head.cmd == CMD_KEY){//�����ϴ���Ӧ
                //northing...
            }
        break;

        //��
        case MsgType_CARD:
        {
            char xdata block_data[M1_CARD_BLOCK_LENTH] = {0,};
            //PaymentCard_type *pBlockData = block_data;

            if(st.head.cmd == CMD_CARD_UP){//�ϴ������� -��Ӧ

                //Debug_Log("catd type up ACK.\n");
                if(st.msg[0] != 0){
                    TypeA_Halt();
                }

            }
            else if(st.head.cmd == CMD_CARD_READ)   //��������
            {
                #if 1
                if(st.msgLen >= 8)
                {
                    unsigned char xdata keyA[6];
                    unsigned char sector ,block ,sectorBlock;
                    unsigned char xdata PICC_ATQA1[2],PICC_SAK1[3],PICC_UID1[4];

                    memcpy(keyA,st.msg,6);
                    //Debug_Log("��Կ����Կ\r\n");
                    Debug_Log("keyA :");
                    Debug_Hex(keyA,6);
                    Debug_Log("\r\n");

                    sector = st.msg[6];
                    block = st.msg[7];
                    sectorBlock = sector*4 + block;
                    Debug_Log("Ѱ��\r\n");
                    Debug_Log("sector :");
                    Debug_Hex(&sector, 1);
                    Debug_Log("\r\n");
                    Debug_Log("block :");
                    Debug_Hex(&block, 1);
                    Debug_Log("\r\n");
                    
                    TypeA_Halt();
                    //Ѱ��
                    if (TypeA_CardActivate(PICC_ATQA1,PICC_UID1,PICC_SAK1) != OK)
                    {
                        TypeA_Halt();
                        CardNumUpLoad(1,block_data);
                        cardFlag = 1;
                        Debug_Log("Ѱ��ʧ��.\r\n");
                        return;
                    }
                    //Debug_Log("Ѱ��ͨ��\r\n");
                    //Debug_Log("PICC_ATQA1:");
                    //Debug_Hex(PICC_ATQA1,2);
                    //Debug_Log("\r\n");
                    ////M1��
//                    if(PICC_ATQA1[0]!=0x04)
//                    {
//                        TypeA_Halt();
//                        CardNumUpLoad(1,block_data);
//                        
//                        Debug_Log("Card Author failed2222.\r\n");
//                        return;
//                    }

                    //Debug_Log("������֤\r\n");
                    //������֤
                    if(Mifare_Auth(0, sector, keyA, PICC_UID1) != OK)
                    {
                        TypeA_Halt();
                        CardNumUpLoad(1,block_data);
                        Debug_Log("������֤ʧ��.\r\n");
                        return;
                    }
                    //Debug_Log("������֤ͨ��\r\n");

                    //Debug_Log("����");
                    //Debug_Hex(&sectorBlock, 1);
                    //Debug_Log("����\r\n");
                    //�����ݿ�
                    if(Mifare_Blockread(sector*4+block, block_data) != OK)
                    {
                        TypeA_Halt();
                        CardNumUpLoad(1,block_data);
                        cardFlag = 1;
                        Debug_Log("�����ݿ�ʧ��.\r\n");
                        return;
                    }
                    //Debug_Log("����");
                    //Debug_Hex(&sectorBlock, 1);
                    //Debug_Log("����ͨ��\r\n");

                    //Debug_Log("�����ݿ�:");
                    //Debug_Hex(&sectorBlock, 1);
                    //Debug_Log(":");
                    //Debug_Hex(&block_data[0], 16);
                    //Debug_Log("\r\n");

                    //�ϴ�����
                    CardNumUpLoad(0,&block_data[0]);
                    TypeA_Halt();
                }
                #endif
            }
			else if(st.head.cmd == CMD_CARD_WRITE)      //д������
            {
                #if 1
                unsigned char xdata keyA[6];
                unsigned char sector ,block, sectorBlock;
                unsigned char xdata PICC_ATQA1[2],PICC_SAK1[3],PICC_UID1[4];

                memcpy(keyA,st.msg,6);

                sector = st.msg[6];
                block = st.msg[7];

                sectorBlock = sector*4 + block;
                //Debug_Log("д��keyA :");
                //Debug_Hex(&keyA[0], 6);
                //Debug_Log("\r\n");

                ///Debug_Log("дshuju�������� :");
                //Debug_Hex(&st.msg[0],16+8);
                //Debug_Log("\r\n");

                Debug_Log("д��Ѱ��\r\n");
                TypeA_Halt();
                //Ѱ��
                if (TypeA_CardActivate(PICC_ATQA1,PICC_UID1,PICC_SAK1) != OK)
                {
                    Debug_Log("д��Ѱ��ʧ��.\r\n");
                    TypeA_Halt();
                    WriteCardBlockSuccessUpLoad(WRITE_ERROR);
                    return;
                }
                //Debug_Log("д��Ѱ��ͨ��\r\n");
                //Debug_Log("д��������֤ \r\n");
                //������֤
                if(Mifare_Auth(0, sector, keyA, PICC_UID1) != OK)
                {
                    Debug_Log("д��������֤ʧ��.\r\n");
                    TypeA_Halt();
                    WriteCardBlockSuccessUpLoad(WRITE_ERROR);
                    return;
                }
                //Debug_Log("д��������֤ͨ��\r\n");

               // Debug_Log("д�����ݵ���");
                Debug_Log("sectorBlock: 0x");
                Debug_Hex(&sectorBlock, 1);
                Debug_Log("\r\n");

               // Debug_Log("д��������:");
              //  Debug_Log("kkkkkkkk");
               // Debug_Hex(((WRITE_CARD_REQ_STR*)st.msg)->blockData,16);
               // Debug_Log("\r\n");
                switch(sectorBlock)
                {
                    case 8:
                        Debug_Log("д��8У��\r\n");
                        if(((WRITE_CARD_REQ_STR*)st.msg)->blockData[15] == GetPktSum(&((WRITE_CARD_REQ_STR*)st.msg)->blockData[0], 15))
                        {
                          //  Debug_Log("д����8У��ͨ��\r\n");
                           // Debug_Log("д����8\r\n");
                            if(OK != Mifare_Blockwrite(8, &((WRITE_CARD_REQ_STR*)st.msg)->blockData[0]))
                            {
                                Debug_Log("д����8ʧ��\r\n");
                                WriteCardBlockSuccessUpLoad(WRITE_ERROR);
                            }
                            else
                            {
                              //  Debug_Log("д����8�ɹ�\r\n");
                                WriteCardBlockSuccessUpLoad(WRITE_SUCCESS);
                            }
                        }
                    break;
                    default:
                        if(OK != Mifare_Blockwrite(sectorBlock, &((WRITE_CARD_REQ_STR*)st.msg)->blockData[0]))
                        {
                            Debug_Log("д����xʧ��\r\n");
                            WriteCardBlockSuccessUpLoad(WRITE_ERROR);
                        }
                        else
                        {
                          //  Debug_Log("д����x�ɹ�\r\n");
                            WriteCardBlockSuccessUpLoad(WRITE_SUCCESS);
                        }
                    break;
                }
                TypeA_Halt();
                #endif
            }
        }
        break;
        //����
        case MsgType_BLUE:
        {
            #ifdef EN_BLUETOOTH
            if(system_ststus.blue_state == 0){
                //������������
                if(st.head.cmd == CMD_SET_BLUE_NAME){

                    if(SetBlueName(st.msg) == OK){
                        SendMsg_ACK(MsgType_BLUE,CMD_SET_BLUE_NAME,0);
                    }else{
                        SendMsg_ACK(MsgType_BLUE,CMD_SET_BLUE_NAME,1);
                    }
                }
                //�������ص�ַ
                else if(st.head.cmd == CMD_SET_BLUE_MAC){

                    unsigned char str_mac[11];
                    BCDToString(str_mac,st.msg,5);

                    if(SetGW_MacAddr(str_mac) == OK){
                        SendMsg_ACK(MsgType_BLUE,CMD_SET_BLUE_MAC,0);
                    }else{
                        SendMsg_ACK(MsgType_BLUE,CMD_SET_BLUE_MAC,1);
                    }
                }
                //������Ϣ - ��Ϣת��
                else if(st.head.cmd == CMD_BLUE_RECV){

                    if(st.msgLen >= 3){
                        unsigned char nodeType = st.msg[0];
                        unsigned int  len = (unsigned int)(st.msg[1] | st.msg[2]<<8);

                        //ת�����ݵ�������2.4G
                        if(SendBlueData(nodeType,st.msg+3,len) == OK){
                            //��Ӧ��
                            SendMsg_ACK(MsgType_BLUE,CMD_BLUE_RECV,0);
                        }else{
                            //��Ӧ��
                            SendMsg_ACK(MsgType_BLUE,CMD_BLUE_RECV,1);
                        }
                    }else{
                        SendMsg_ACK(MsgType_BLUE,CMD_BLUE_RECV,1);
                    }
                }
                //������Ϣ����Ӧ
                else if(st.head.cmd == CMD_BLUE_SEND){

                }
            }
            #endif
        }
        break;
    }
}



//������Ϣ
void CommPackageHandle(void)
{
    //״̬����־
    static unsigned char recvstate = 0;
    static unsigned char  temp_count = 0;
    static unsigned char xdata checksum;
    static unsigned int xdata data_len;
    static unsigned int xdata recvlen = 0;
    static unsigned char xdata reavbuf[MAX_PACKAGE_SIZE];
    unsigned char temp_data;
    static unsigned int xdata time;

    if (0 != recvstate) {
        if (8 < ((unsigned int)((unsigned int)getSystemTimer() - time))) {
            recvstate = 0;
        }
    }
    while(FIFO_S_Get(&commFIfO,&temp_data) == 0) {
       //Debug_Hex(&temp_data,1);
        switch(recvstate){
            case 0:
                if(temp_data == 0xAA){
                    time = getSystemTimer();
                    recvlen = 0;
                    memset(reavbuf,0,MAX_PACKAGE_SIZE);
                    reavbuf[recvlen++] = temp_data;
                    recvstate = 1;
                }
                break;
            case 1:
                if(temp_data == 0x55){
                    reavbuf[recvlen++] = temp_data;
                    recvstate = 2;
                    temp_count = 0;
                }else{
                    recvstate = 0;
                }
                break;
            case 2://�����ж�
                reavbuf[recvlen++] = temp_data;
                if (2 <= ++temp_count) {
                    data_len = (reavbuf[recvlen-1]<<8) | reavbuf[recvlen-2];
                    if(data_len >= MAX_PACKAGE_SIZE){
                        recvstate = 0;
                    }else{
                        recvstate = 3;
                        temp_count = 0;
                        checksum = 0;
                    }
                }
                break;
            case 3://��������
                if(recvlen < MAX_PACKAGE_SIZE) reavbuf[recvlen++] = temp_data;
                data_len--;
                if(data_len >= 1){
                    checksum += temp_data;
                }else{
                    //��������
                    if(checksum == temp_data){

                        peraseMsg(reavbuf,recvlen);
                        recvstate = 0;
                    }else{
                        Debug_Log("msg checksum error.\n");
                        recvstate = 0;
                    }
                    //Debug_Log("\r\n");
                }
                break;
        }
    }
}


//������
void Main_Handle(void)
{
    unsigned char test = 0;
    Time_t xdata feedDogTime=0;
    Time_t xdata syncSystemInfoTime=0;
    Time_t xdata syncSystemStstusTime=0;
    Time_t xdata blueTestTime = 0;

    system_info.soft_version = FW_VERSION;

    WDT_FeedDog();

    while(1){

        if(keyFlag == 1){
            //�������
            TS_Action();
            TS_GetKey();
        }

        #ifdef EN_BLUETOOTH
            //����
            HandleBlueMsg();
        #endif

        //����ͨ������
        CommPackageHandle();

         //������
        if(system_ststus.card_state == 0){
            Check_M1_Card();
        }

         //����Ѱ����־ʱ��
        if(getSystemTimer() - cardFlagTime >= TIMEOUT_1s*5){
            cardFlagTime = getSystemTimer();
            cardFlag = 1;
        }

        //��������
        if(keyFlag == 0 && (getSystemTimer() - keyFlagTime) >= 1){
            keyFlag = 1;
        }

        //ι�� 500ms
        if(getSystemTimer() - feedDogTime >= 1){
            feedDogTime = getSystemTimer();
            WDT_FeedDog();
        }

         //ͬ��ϵͳ��Ϣ��X10
        if(isSyncSystemInfo == 0 && (getSystemTimer() - syncSystemInfoTime >= TIMEOUT_1s*5)){

            syncSystemInfoTime = getSystemTimer();

            SyncSystemInfo();
        }

        //ͬ��ϵͳ״̬��X10
        if((getSystemTimer() - syncSystemStstusTime) >= TIMEOUT_1s*30){
            syncSystemStstusTime = getSystemTimer();

            SyncSystemState();
        }

        #ifdef EN_BLUETOOTH

            //���û���������ӣ����ڼ��CKģ���Ƿ���������
            if(system_ststus.blue_state == 0                            //��Ϊ��ǰ����״̬��OK��
                && isBuleConnect == 0                                   //û��APP���ӵ�����
                && (getSystemTimer() - blueTestTime >= TIMEOUT_1s*60))  //1���Ӽ��һ��
            {
                blueTestTime = getSystemTimer();
                if(BlueTest(1) != OK){
                    Debug_Log("blue Test ERROR.\n");
                    system_ststus.blue_state = 1;
                    rebootBlueCnt++;
                }else{
                    system_ststus.blue_state = 0;
                    rebootBlueCnt = 0;
                    Debug_Log("blue Test OK.\n");
                }
            }

             //��������쳣�����������豸���������3��
            if(system_info.blue_state ==0               //�������豸
                && rebootBlueCnt <= 5                   //��������
                && system_ststus.blue_state == 1){      //��ǰ��ȡ����������Ϣ

               if(BuleReset_Async() == OK){
                    Debug_Log("blue is resetting[r].\n");
                    blueTestTime = getSystemTimer() - (TIMEOUT_1s*55);
               }
            }

            //�ж������Ƿ�����  2����û���յ����ݾ���Ϊ����
            if(isBuleConnect == 1 && (getSystemTimer() - lastConnectTime > TIMEOUT_1s*120)){

                lastConnectTime = getSystemTimer();
                isBuleConnect = 0;
                Debug_Log("blue disconnected.\n");
            }
        #endif
    }
}