/*sim800c.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "sim800c.h"
#include "rtc.h"
#include "usart.h"
#include "sc8042.h"
#include "flash.h"
#include "server.h"
#include "FIFO.h"
#include "upgrade.h"


MUX_SEM_STR gGprsSendMux = {0,0};
FIFO_S_t gSocketPktRxCtrl;
__IO uint8_t gGprsRxBuff[512] = {0};
uint8_t  gNetSignal = 0;
uint8_t  gprsBuffer[BUFFER_SIZE];
uint16_t gGprsRxLen;
uint32_t gSimStatus = 0;
ip_state istage;

const sim900_inittab ipinit_tab[]={
    {NULL, "OK", 3000, 0, sim800_check_reset},                          //0.
    {"ATE0\r", "OK", 5000, 10, sim800_check_ATE},                       //1.
    {"ATI\r", "OK", 1000, 10, sim800_check_ATI},						//2.显示产品ID信息
    {"AT+CSQ\r", "OK", 1000, 150, sim800_check_CSQ},                    //3.查看信号强度
    {"AT+CPIN?\r", "OK", 500, 10, sim800_check_cpin},			        //4.指示用户是否需要密码
    {"AT+CCID\r", "OK", 500, 10, sim800_check_CCID},                    //5.
	{"AT+CSCLK=0\r", "OK", 1000, 10, sim800_check_CSCLK},  		        //6.打开模块睡眠功能
	{"AT+CIPRXGET=0\r", "OK", 1000, 10, sim800_check_CIPRXGET},		    //7.自动接收GPRS数据
	{"AT+CREG?\r","OK", 1000, 50, sim800_check_CREG},			        //8.检查GSM网络注册状态
    {"AT+CGATT=1\r", "OK", 4000, 50, sim800_check_CGATT},		        //9.MT附着GPRS业务
    {"AT+CGATT?\r", "+CGATT: 1", 1000, 50, sim800_check_CGATT},		    //10.检查MS是否附着GPRS业务
	{"AT+CIPSHUT\r","SHUT OK", 1000, 200, sim800_check_CIPSHUT},        //11.
	{"AT+CIPMUX=1\r", "OK", 100, 10, sim800_check_CIPMUX},		        //12.设置多路连接
	#if (0 == SPECIAL_NET_CARD)
    {"AT+CSTT=\"cmnet\"\r","OK", 1000, 50, sim800_check_CSTT},
	#else
    {"AT+CSTT=\"CMIOTALIS.GD\"\r","OK", 1000, 50, sim800_check_CSTT},          //13.
    #endif
	{"AT+CIPSTATUS\r","OK", 1000, 100, sim800_check_ack},               //14.
	{"AT+CIICR\r","OK", 10000, 50, sim800_check_CIICR},			        //15.建立无线链路,激活移动场景
	{"AT+CIPSTATUS\r","OK", 1000, 100, sim800_check_ack},               //16.
    {"AT+CIFSR\r","OK", 1000, 50, sim800_default},                      //17.CIFSR
    #if (1 == SPECIAL_NET_CARD)
    {"AT+CDNSCFG=100.100.2.136,100.100.2.138\r","OK", 1000, 50, sim800_check_CSTT},//18.
	{"AT+CDNSCFG?\r","OK", 1000, 50, sim800_check_CSTT},                //19.
	#endif
	{NULL, NULL, 0, 0, sim800_ipopen},                                  //20.
};




int GetNetSignal(void)
{
    return gNetSignal;
}


//******************************************************************
//! \brief  	gprs_power_off
//! \param
//! \retval
//! \note   	2g模块关机
//******************************************************************
void gprs_power_off(void)
{
    GPRS_POWER_DISABLE();
    CL_LOG("power off 2G\r\n");
}


//******************************************************************
//! \brief  	gprs_power_on
//! \param
//! \retval
//! \note   	2g模块开机
//******************************************************************
void gprs_power_on(void)
{
	//GPRS_WAKEUP();
	GPRS_POWER_ENABLE();
	vTaskDelay(3000);
	GPRS_PWRKEY_LOW();
	vTaskDelay(6000);
	GPRS_PWRKEY_HIGH();
    CL_LOG("power on 2G\r\n");
}


//******************************************************************
//! \brief  	gprs_reset
//! \param
//! \retval
//! \note   	2g模块复位
//******************************************************************
void GprsReset(void)
{
    gprs_power_off();
    vTaskDelay(5000);
    gprs_power_on();
}


int GetSocketState(char *pStr)
{
    char socketState[16] = {0};
    char *str = strstr(pStr,"+CIPSTATUS");
    char *p = strtok(str,",");

    p = strtok(NULL,",");
    p = strtok(NULL,",");
    p = strtok(NULL,",");
    p = strtok(NULL,",");
    p = strtok(NULL,",");
    if (p) {
        sscanf(p,"\"%[^\"]",socketState);
        //CL_LOG("socketState = %s\n",socketState);
        if ((strncmp(socketState,"CONNECTED",strlen("CONNECTED")) == 0)
            || (strncmp(socketState,"INITIAL",strlen("INITIAL")) == 0)
           ) {
            return CL_OK;
        }
    }
    return CL_FAIL;
}


void GetCcidSn(char *pStr)
{
    int i;
    int flag = 0;

    for (i=0; i<strlen(pStr); i++) {
        switch (flag) {
            case 0:
                if ((0x0a == pStr[i]) || (0x0b == pStr[i])) {
                    flag = 1;
                }
                break;
            case 1:
                if ((0x30 <= pStr[i]) && (pStr[i] <= 0x39)) {
                    memcpy(system_info.iccid, &pStr[i], ICCID_LEN);
                    system_info.iccid[ICCID_LEN] = 0;
                    CL_LOG("ccid=%s.\n", system_info.iccid);
                    return;
                }
                break;
        }
    }
}


int GprsCheckRes(char *cmd, char *res, uint16_t tmo)
{
    char *ret=NULL;
    uint8_t c;
    int cnt = 0;
    int retv = CL_FAIL;

    memset(gprsBuffer, 0, sizeof(gprsBuffer));
    //CL_LOG("gprs_check_res : recv data=");
    for (uint16_t time = 0; time < tmo; time+= 50) {
        vTaskDelay(50);
        Feed_WDT();
        while(UsartGetOneData(GPRS_UART_PORT, &c) == 0) {
            #if (1 == SIM800_DEBUG)
            printf("%02x ",c);
            #endif
            if (c) {
                if (sizeof(gprsBuffer) > cnt) {
                    gprsBuffer[cnt++] = c;
                }else{
                    CL_LOG("cnt=%d,error.\n",cnt);
                }
            }
        }
        ret = strstr((const char*)gprsBuffer, (const char*)res);
        if (ret) {
            #if (1 == SIM800_DEBUG)
            printf("\r\n");
            #endif
            retv = CL_OK;
            if (NULL != cmd) {
                if (strstr(cmd, "AT+CSQ")) {
                    ret = strstr((char*)gprsBuffer, "CSQ: ");//+CSQ: 23,99
                    if (ret != NULL) {
                        gNetSignal = atoi(&ret[5]);
            			c = atoi(&ret[6]);
                        CL_LOG("2G signal value=%d, ber=%d.\n", gNetSignal,c);
                    }
                }else if (strstr(cmd, "AT+CCID")) {
					GetCcidSn((void*)gprsBuffer);
				}else if (strstr(cmd, "AT+CIPSTATUS=")) { //判断网络socket的连接状态
                    retv = GetSocketState((void *)gprsBuffer);
                }
            }
            break;
        }
    }

    #if (1 == SIM800_DEBUG)
    CL_LOG("<<< [%s].\n",gprsBuffer);
    #endif
    return retv;
}


//flag = 1是发>
int GprsSendCmd(char *cmd, char *ack, uint16_t waittime, int flag)
{
	uint8_t res = 1;

    MuxSempTake(&gGprsSendMux);
    UsartSend(GPRS_UART_PORT, (void*)cmd, strlen(cmd));

    #if (1 == SIM800_DEBUG)
	CL_LOG(">>> %s.\n",cmd);
    #endif

    if ((ack==NULL) || (waittime==0)) {
        MuxSempGive(&gGprsSendMux);
        return CL_OK;
    }

    if (CL_OK == GprsCheckRes(cmd, ack, waittime)) {
        res = CL_OK; /*check success, retrun 0*/
    }else{
        res = (1==flag) ? 0 : 1;
    }
	MuxSempGive(&gGprsSendMux);
	return res;
}


int CheckModeCmd(char ok, uint8_t retry, uint16_t delay)
{
    if (ok==0) {
        istage++;
        return CL_OK;
    }else if (retry > 10) {/*retry 10 times*/
        gSimStatus |= (1<<istage);
        OptFailNotice(istage);
        istage = SIM800_RESET; /* goto reset */
	}
    return CL_FAIL;
}


int sim800_check_ATE(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_ATS(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_ATI(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_cpin(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CCID(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CSCLK(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CIPRXGET(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CIPQSEND(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}


int sim800_check_CSQ(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    if ((ok==0) && (gNetSignal)) {
        istage++;
        return CL_OK;
    }else if (retry > 10) {/*retry 10 times*/
        gSimStatus |= (1<<istage);
        OptFailNotice(istage);
        istage = SIM800_RESET; /* goto reset */
	}
    return CL_FAIL;
}

int sim800_check_CREG(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CGATT(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CIPMODE(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CIPMUX(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CIPSHUT(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CSTT(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}

int sim800_check_CIICR(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    if (ok==0) {
        istage++;
        return CL_OK;
    }else{
        if (retry > 2) {
            //istage = SIM800_RESET; /* goto reset */
			istage = SIM800_CGATT;
            return CL_OK;
        }
    }
    return CL_FAIL;
}


int sim800_check_reset(char ok, uint8_t retry)
{
    CL_LOG("reset 2g module\n");
    GprsReset();
    for (int i=0; i<2; i++) {
        if (GprsSendCmd("AT\r", "OK", ipinit_tab[SIM800_RESET].wait, 0)==0) {
            CL_LOG("2G reset ok \n");
            istage++;
            return CL_OK;
        }
		CL_LOG("2G reset fail.\n");
        OS_DELAY_MS(3000);
    }
    OptFailNotice(istage);
    gSimStatus |= (1<<istage);
    return CL_FAIL;
}


int sim800_check_ack(char ok, uint8_t retry)
{
	CL_LOG("in.\n");
    return CheckModeCmd(ok, retry, 0);
}


int SOCKET_open_set(int socket, char* addr, int port)
{
    char cmd_req[64] = {0};
    char cmd_ack[64] = {0};

    sprintf(cmd_ack, "%d, CONNECT OK", socket);
    sprintf(cmd_req,"AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r", socket, addr, port);

    if(!GprsSendCmd(cmd_req, cmd_ack, 5000, 0)) 
    {
        CL_LOG("link %d open ok,ip=%s,port=%d.\n", socket,addr,port);
        return 0;
    }
    else
    {
        CL_LOG("link %d open err.\n", socket);
        return -1;
    }
}

int sim800_ipopen(char ok, uint8_t retry)
{
    static uint32_t state=0;
	int res = 0;
    int ready = 0;

    CL_LOG("in.\n");
    switch(state) 
    {
        case NET_STATE_SOCKET0_OPEN:
        {
            ready = CL_FALSE;
            for(int i = 0; i < 10; i++)
            {
                res = SOCKET_open_set(SOCKET_ID, NET_SERVER_IP, NET_SERVER_PORT);
                if(!res)
                {
                    ready = CL_TRUE;
                    break;
                }
                OS_DELAY_MS(1000);
            }

            if(CL_TRUE == ready)
            {
                //OS_DELAY_MS(10000);
                state = NET_STATE_READY;
            }
            else
            {
                state = NET_STATE_FAILURE;
            }
        }
        break;

        case NET_STATE_READY:
        {
			state = NET_STATE_SOCKET0_OPEN;
            istage = SIM800_STATE_NUM;
        }
        break;

        case NET_STATE_FAILURE:
        {
            state=0;
            gSimStatus |= (1<<istage);
            OptFailNotice(istage);
            istage = SIM800_RESET;
        }
        break;
    }
    return CL_OK;
}


int sim800_default(char ok, uint8_t retry)
{
    if (istage < SIM800_STATE_NUM) 
	{
        istage++;
    }
    return CL_OK;
}


int Sim800cReconnect(void)
{
	char ok;
    uint8_t retry = 0;

	istage = SIM800_RESET;
	while(1) 
    {
        Feed_WDT();
        OS_DELAY_MS(ipinit_tab[istage].nwait*10);

        if (OUT_485_NET == gOutNetStatus.connect) 
        {
            CL_LOG("485 net check ok,stop try local net.\n");
            return CL_FAIL;
        }

        if (ipinit_tab[istage].cmd) 
        {
            ok = GprsSendCmd(ipinit_tab[istage].cmd,ipinit_tab[istage].res,ipinit_tab[istage].wait, 0);
        }

        if (ipinit_tab[istage].process) 
        {
            if (CL_OK == ipinit_tab[istage].process(ok, retry)) 
            {
                retry = 0;
            }
            else
            {
                retry++;
            }
        }

		if (istage == SIM800_STATE_NUM) 
        {
			CL_LOG("2G init and set socket ok.\n");
			return CL_OK;
		}
        else if (SIM800_RESET == istage) 
        {
            CL_LOG("2G init and set socket fail.\n");
            return CL_FAIL;
        }
	}
}

//返回 0成功
int GprsSendData(char *data, int len, char *ack, uint16_t waittime)
{
	int res = 1;

    UsartSend(GPRS_UART_PORT, (void*)data, len);
    #if (1 == SIM800_DEBUG)
	PrintfData("GprsSendData", (void*)data, len);
    #endif
    if ((ack == NULL) || (waittime == 0)) return CL_OK;
    if (CL_OK == GprsCheckRes(NULL, ack, waittime)) {
        res = 0; /*check success, retrun 0*/
    }
	return res;
}

int SocketSendData(int socket, uint8_t* data, uint16_t len)
{
    char cmd_req[64] = {0};
	char cmd_ack[64] = {0};
    int res;

    if (CL_FALSE == system_info.is_socket_0_ok) {
        CL_LOG("socket 0 is closed.\n");
        return -1;
    }

    MuxSempTake(&gGprsSendMux);
    for (int i=0; i<2; i++) {
		sprintf(cmd_ack, "%d, SEND OK", socket);
		sprintf(cmd_req,"AT+CIPSEND=%d,%d\r", socket, len);
        res = GprsSendCmd(cmd_req, ">", 1000, 1);
        if (0 != res) {
            CL_LOG("GprsSendCmd=%d,err.\n",res);
            OptFailNotice(50);
        }

        if (CL_OK == (res = GprsSendData((char*)data, len, cmd_ack, 10000))) {
            break;
        }else{
            system_info.tcp_tx_error_times++;
            CL_LOG("err,times=%d.\n",system_info.tcp_tx_error_times);
            OS_DELAY_MS (1000);
        }
    }
    MuxSempGive(&gGprsSendMux);

    if (res == 0) {
        system_info.tcp_tx_error_times = 0;
    }else {
        CL_LOG("send err,times=%d.\n",system_info.tcp_tx_error_times);
        if (system_info.tcp_tx_error_times >= TX_FAIL_MAX_CNT) {
            system_info.tcp_tx_error_times = 0;
			CL_LOG("send err,reset net.\n");
			system_info.is_socket_0_ok = CL_FALSE;
            gChgInfo.netStatus |= 0x02;
            OptFailNotice(51);
        }
    }
    return res;
}

int trim(char * data)
{
    int i,j;

    for(i=0,j=0; i<strlen((char *)data); i++) {
        if (data[i] != ' ')
            data[j++] = data[i];
    }
    data[j] = '\0';
    return 0;
}

uint8_t * GprsCmdChkNoSpace(char * ack)
{
    char * strx;
    uint8_t data;
    uint8_t flag = 0;

    while ((UsartGetOneData(GPRS_UART_PORT, (void*)&data)) == CL_OK) {
        //接收到一个字符
        if (gGprsRxLen < BUFFER_SIZE) {
            flag = 1;
            gprsBuffer[gGprsRxLen++] = data;
        }
        OS_DELAY_MS(2);
    }

    if (flag) {
        gprsBuffer[gGprsRxLen] = 0;//添加结束符
        trim((char *)gprsBuffer);
        CL_LOG("buf=%s.\n",gprsBuffer);
        strx = strstr((const char*)gprsBuffer, (const char*)ack);
        return (uint8_t*)strx;
    }
    return NULL;
}


int GprsSendCmdChkNoSpace(char * cmd, char * ack, int waitCnt, int waittime, uint8_t *data)
{
    int i;
    int k;
	char *ret = NULL;

    MuxSempTake(&gGprsSendMux);
    for (i=0; i<3; i++) {
        gGprsRxLen = 0;
        UsartSend(GPRS_UART_PORT, (void *)cmd, strlen(cmd));
        if (ack) {		//需要等待应答
            for (k=0; k<waitCnt; k++) {
                OS_DELAY_MS(waittime);
                if (GprsCmdChkNoSpace(ack)) {
                    //得到有效数据
					if (strstr(cmd, "AT+CTFSGETID")) {
						//+CTFSGETID:"00AAAAAABBBBBB24C857F000"
						ret = strstr((char*)gprsBuffer, (void*)"+CTFSGETID:");
						if (ret != NULL) {
							memcpy(data, &ret[12], TFS_ID2_LEN);
						}
					}
					if (strstr(cmd, "AT+CTFSDECRYPT")) {
						ret = strstr((char*)gprsBuffer, (void*)"+CTFSDECRYPT:");
						if (ret != NULL) {
							memcpy(data, &ret[14], TFS_ID2_AES_LEN);
						}
					}

					if (strstr(cmd, "AT+CTFSAUTH=0")) {
						ret = strstr((char*)gprsBuffer, (void*)"+CTFSAUTH:");
						if (ret != NULL) {
							memcpy(data, &ret[11], TFS_CHALLENGE_AUTH_CODE_LEN);
						}
					}

					if (strstr(cmd, "AT+CTFSAUTH=1")) {
						ret = strstr((char*)gprsBuffer, (void*)"+CTFSAUTH:");
						if (ret != NULL) {
							memcpy(data, &ret[11], TFS_TIMESTAMP_AUTH_CODE_LEN);
						}
					}
                    MuxSempGive(&gGprsSendMux);
                    return CL_OK;
                }
            }
        }else{
            MuxSempGive(&gGprsSendMux);
            return CL_OK;
        }
    }
    MuxSempGive(&gGprsSendMux);
    return CL_FAIL;
}

int restoi(char* buf, char* start, char end)
{
    char i;
    char *p = strstr(buf, start);

    p+=strlen(start);
    if(!p) return 0;
    for(i=0; i<10; i++) {
        if(p[i]==end)
        {
            p[i]='\0';
            break;
        }
    }
    return atoi(p);
}

int GetGPRSBuffer(uint8_t *buf, uint16_t len)
{
    uint8_t c;
    int i;
    int timeOut;

    for (i=0; i<len;) {
        Feed_WDT();
        timeOut = 2000;
        while (timeOut) {
            if (UsartGetOneData(GPRS_UART_PORT, &c) == 0) {
                buf[i++] = c;
                break;
            }else{
                if (--timeOut < 3) {
                    CL_LOG("to=%d,err.\n",timeOut);
                    return i;
                }
            }
        }
    }
    return i;
}

int UsartGetline(uint8_t port, char *pBuff, uint16_t len, uint16_t tmo)
{
    int i = 0;
    uint8_t c;

    if (tmo == 0xffff) {
        while (UsartGetOneData(port, &c) == 0) {
            if (c == '\n')
                goto getline;
            pBuff[i++] = c;
        }
    }else{
        while (--tmo) {
            OS_DELAY_MS(2);
            while (UsartGetOneData(port, &c) == 0) {
                if (c == '\n')
                    goto getline;
                pBuff[i++] = c;
                len--;
                if (len == 1)
                    goto getline;
            }
        }
    }
getline:
    pBuff[i] = '\0';
    if (i>0 && pBuff[i-1]=='\r')
        pBuff[--i]='\0';
    return i;
}


int FtpGet(const char* serv, const char* un, const char* pw, const char* file, uint16_t chsum_in)
{
    char ipconfig[64] = {0};
    int ret=0;
    int fsize;
    int cfize=0;
    uint16_t checksum=0;
    uint8_t  getCnt;
    uint8_t  fwCnt;
    uint8_t  fwVer;
    uint16_t getLen;
    char retry;
    int len,i,j,k;
    char *sp1;
    char tmp[64] = {0};
    uint32_t over_time;
    uint32_t appBackupRecordAddr;
    FW_INFO_STR *pFwInfo = NULL;
    FW_HEAD_STR *pFwHead = NULL;

    Feed_WDT();
	gprsBuffer[0] = 0;
    sprintf(tmp, "AT+CIPCLOSE=%d\r", SOCKET_ID);
    GprsSendCmd(tmp,"CLOSE OK",1000, 0);
    GprsSendCmd("AT+CIPSHUT\r","SHUT OK",1000, 0);
    GprsSendCmd("AT+SAPBR=1,1\r","\r\nOK\r\n",1000, 0);
    if (GprsSendCmd("AT+SAPBR=2,1\r","+SAPBR: 1,1",2000, 0) != 0) 
	{
        CL_LOG("send cmd,err.\n");
        ret = -1;
        goto EXIT1;
    }

    GprsSendCmd("AT+FTPCID=1\r","\r\nOK\r\n",200, 0);
    GprsSendCmd("AT+FTPTYPE=\"I\"\r","\r\nOK\r\n",200, 0);
    GprsSendCmd("AT+FTPMODE=0\r","\r\nOK\r\n",200, 0);
    //set ftp server address and port  ftpget("118.31.246.230/2017/1001", "x5", "x543", "5751.bin", 0xa7b3);
    sp1 = strchr(serv, '/');
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp,serv,sp1-serv);
    sprintf(ipconfig, "AT+FTPSERV=\"%s\"\r", tmp);
    //CL_LOG("serv=%s.\n",ipconfig);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",200, 0);

    memset(ipconfig, 0, sizeof(ipconfig));
    sprintf(ipconfig, "AT+FTPUN=\"%s\"\r", un);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);

    memset(ipconfig, 0, sizeof(ipconfig));
    sprintf(ipconfig, "AT+FTPPW=\"%s\"\r", pw);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);

    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp,sp1+1,strlen(sp1+1));
    tmp[strlen(tmp)] = '/';
    memset(ipconfig, 0, sizeof(ipconfig));
    sprintf(ipconfig, "AT+FTPGETPATH=\"%s\"\r", tmp);
    //CL_LOG("ftp get path=%s.\n",ipconfig);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);

    memset(ipconfig, 0, sizeof(ipconfig));
    sprintf(ipconfig, "AT+FTPGETNAME=\"%s\"\r", file);
    //CL_LOG("ftp get filename=%s.\n",ipconfig);
    GprsSendCmd(ipconfig,"\r\nOK\r\n",100, 0);

    /*ftp get filesize*/
    if (GprsSendCmdChkNoSpace("AT+FTPSIZE\r", "+FTPSIZE:", 10,1000, NULL) == 0) 
	{
		//如此处理是为了防止bin文件中含有相同常量，导致误判
		sprintf(tmp,"+FTPSIZE:1,%d,",0);
        fsize = restoi((char *)gprsBuffer, tmp,'\r');
        if (fsize==0) 
		{
            //文件不存在或网络异常
            CL_LOG("fs=0 fail.\n");
            ret = -2;
            OptFailNotice(53);
            goto EXIT1;
        }
    }
	else 
	{
        CL_LOG("fail.\n");
        ret = -2;
        OptFailNotice(52);
        goto EXIT1;
    }
    CL_LOG("fs=%d.\n",fsize);

    /*Open the ftp get session*/
    if (GprsSendCmdChkNoSpace("AT+FTPGET=1\r", "+FTPGET:1,1", 10, 1000, NULL) == 0) 
    {
        ret = 0;
        /*erase flash and set write pos*/
		appBackupRecordAddr = AppUpBkpAddr;
        over_time = GetRtcCount();
        getLen = 16;
        getCnt = 0;
        while(1) 
        {
            Feed_WDT();
            if ((uint32_t)(GetRtcCount() - over_time) > 90) 
            {
                break;
            }

            memset(tmp, 0, sizeof(tmp));
			sprintf(tmp,"AT+FTPGET=%d,%d\r",2, getLen);
            GprsSendCmdChkNoSpace(tmp, NULL, 1,1000, NULL);
            OS_DELAY_MS(10);
            len = 0;
            retry = 0;
            while (retry++ < 20) 
            {
				Feed_WDT();
                memset(gprsBuffer, 0, sizeof(gprsBuffer));
                UsartGetline(GPRS_UART_PORT, (void*)gprsBuffer, 30, 1000);
                //CL_LOG("%s",gprsBuffer);
                for(j=0,k=0; j<strlen((char *)gprsBuffer); j++) 
                {
                    if (gprsBuffer[j]!=' ')
                    {
                        gprsBuffer[k++]=gprsBuffer[j];
                    }
                }
                gprsBuffer[k] = '\0';
                if (strstr((char *)gprsBuffer,"+FTPGET:2,")) 
                {
                    len = restoi((char *)gprsBuffer, "+FTPGET:2,",'\0');
                    break;
                }
            }
            OS_DELAY_MS(5);
            if (len) 
            {
                len = GetGPRSBuffer(gprsBuffer, len);
                if (getCnt < 2) 
                {
                    if (0 == getCnt) 
                    {
                        pFwHead = (void*)gprsBuffer;
                        if ((0xaa == pFwHead->aa) && (0x55 == pFwHead->five)) 
                        {
                            fwCnt = pFwHead->fwCnt;
                            fwVer = pFwHead->fwVer1;
                            getCnt++;
                            CL_LOG("aa55 ok.\n");
                            if (0 == fwCnt) 
                            {
                                CL_LOG("fwCnt=%d,err.\n",fwCnt);
                                OptFailNotice(57);
                                return CL_FAIL;
                            }
                        }
                        else
                        {
                            CL_LOG("head=%02x,%02x,err.\n",gprsBuffer[0],gprsBuffer[1]);
                            OptFailNotice(58);
                            return CL_FAIL;
                        }
                    }
                    else if (1 == getCnt) 
                    { //读第一个固件信息
                        pFwInfo = (void*)gprsBuffer;
                        fsize = pFwInfo->size;
                        chsum_in = pFwInfo->checkSum;
                        #if (0 == X10C_TYPE)
                        ret = memcmp("X10", pFwInfo->name, 3);
                        #else
                        ret = memcmp("X10C", pFwInfo->name, 4);
                        #endif
                        if (0 == ret) 
                        {
                            if ((system_info.localFwInfo.size == fsize) && (system_info.localFwInfo.checkSum == chsum_in) && (system_info.localFwInfo.ver == fwVer)) 
                            {
                                CL_LOG("fw same,no need upgrade.\n");
                                OptSuccessNotice(804);
                                return CL_OK;
                            }
                            getCnt++;
                            CL_LOG("fw name ok,fs=%d,sum=%d,fwVer=%d.\n",fsize,chsum_in,fwVer);
                            getLen = 512;
                            memset(&system_info.localFwInfo, 0, sizeof(system_info.localFwInfo));
                            FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
                            FlashEraseAppBackup();
                        }
                        else
                        {
                            CL_LOG("fw=%s,err.\n",pFwInfo->name);
                            OptFailNotice(59);
                            return CL_FAIL;
                        }
                    }
                    continue;
                }

                for (i=0; i<len; i++) 
                {
                    checksum += (unsigned)gprsBuffer[i];
                }
                cfize += len;
                CL_LOG("total %d [%d%%].\n",cfize,cfize*100/fsize);
                //PrintfData("ftpget", gprsBuffer, len);
				FlashWriteAppBackup(appBackupRecordAddr, (unsigned char *)(gprsBuffer), len);
				appBackupRecordAddr += len;
            } 
            else 
            {
                if (cfize < fsize)
                {
                    OS_DELAY_MS(500);
                    //CL_LOG("cfize=%d,fsize=%d,error.\n",cfize,fsize);
                } 
                else 
                {
                    break;
                }
            }
        }

        /*file size = rxget file size, write checksum to flash*/
        if (cfize == fsize) 
        {
			if (checksum == chsum_in) 
            {
				CL_LOG("size %d,cs %4X,fw=%d,ftp ok.\n", fsize, checksum,fwVer);
				WriteUpdateInfo(fsize, checksum);
                system_info.localFwInfo.size = cfize;
                system_info.localFwInfo.checkSum = checksum;
                system_info.localFwInfo.ver = fwVer;
                system_info.localFwInfo.sum = GetPktSum((void*)&system_info.localFwInfo, sizeof(system_info.localFwInfo)-1);
                FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
                ret = 0;
                OptSuccessNotice(801);
			}
            else
            {
			    CL_LOG("cs=%d!=cs1=%d,err.\n",checksum,chsum_in);
                ret = CL_FAIL;
                OptFailNotice(55);
            }
            goto EXIT1;
        }
        else
        {
            CL_LOG("fs=%d!=cf=%d,err.\n",fsize,cfize);
            ret = -6;
            OptFailNotice(56);
            goto EXIT1;
        }
    }
    else 
    {
        CL_LOG("fail.\n");
        ret = -5;
        OptFailNotice(54);
        goto EXIT1;
    }
EXIT1:
    CL_LOG("ret=%d.\n",ret);
    return ret;
}


int GprsSocketStateCheck(void)
{
    int res;
    char cmd_req[32] = {0};

    if (gChgInfo.sendPktFlag) 
    {
        return CL_FAIL;
    }

    sprintf(cmd_req, "AT+CIPSTATUS=%d\r", SOCKET_ID);      //查询当前socket的连接状态
    res = GprsSendCmd(cmd_req, "OK", 5000, 0);
    if (res != CL_OK) 
    {  //当前socket连接异常，马上重新建链
        CL_LOG("网络连接异常, 正在重新建链...\n");
        sprintf(cmd_req, "AT+CIPCLOSE=%d\r", SOCKET_ID);
        GprsSendCmd(cmd_req, "CLOSE OK", 1000, 0);
        if (CL_OK == SOCKET_open_set(SOCKET_ID, NET_SERVER_IP, NET_SERVER_PORT)) 
        {
            CL_LOG("reconnect.\n");
            return CL_OK;
        }
        return CL_FAIL;
    }
    return CL_OK;
}


//******************************************************************
//! \brief  	GprsInit
//! \param
//! \retval
//! \note   	2g模块初始化
//******************************************************************
int GprsInit(void)
{
    FIFO_S_Init(&gSocketPktRxCtrl, (void*)gGprsRxBuff, sizeof(gGprsRxBuff));
	//PA1 PA2 PA3 as 2G_EN 2G_SLEEP 2G_PWRKEY SDA
	GPIO_Cfg(HT_GPIOA, GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	GPRS_WAKEUP();
    istage = SIM800_RESET;
    CL_LOG("init ok.\n");
	return CL_OK;
}


