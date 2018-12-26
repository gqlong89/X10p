/*sim800c.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __SIM800C_H__
#define __SIM800C_H__

#include "includes.h"
#include "FIFO.h"


#define SIM800_DEBUG                    0

#define SOCKET_ID                       0

#define TX_FAIL_MAX_CNT                 4

#define BUFFER_SIZE                     520

#define ICCID_LEN                       20

#define GPRS_POWER_ENABLE()	  	        GPIO_SetBits(HT_GPIOA,GPIO_Pin_1)
#define GPRS_POWER_DISABLE()	        GPIO_ResetBits(HT_GPIOA,GPIO_Pin_1)

#define GPRS_PWRKEY_HIGH()	  		    GPIO_ResetBits(HT_GPIOA,GPIO_Pin_3)		//pwrkey低有效
#define GPRS_PWRKEY_LOW()	  		    GPIO_SetBits(HT_GPIOA,GPIO_Pin_3)

#define GPRS_SLEEP()	  		        GPIO_SetBits(HT_GPIOA, GPIO_Pin_2)
#define GPRS_WAKEUP()	  		        GPIO_ResetBits(HT_GPIOA, GPIO_Pin_2)

// socket rx state-machine
#define _SOCKET_RX_STATE_INIT           0
#define _SOCKET_RX_STATE_WAIT           1
#define _SOCKET_RX_STATE_RECV_DATA      2
#define _SOCKET_RX_STATE_FINISH         3
#define _SOCKET_RX_STATE_CLOSE          4

enum{
    NET_STATE_SOCKET0_OPEN=0,
    NET_STATE_READY,
    NET_STATE_FAILURE,
    NET_NULL,
};


typedef enum{
    SIM800_RESET = 0,                           //0.
    SIM800_ATE,                                 //1.
	SIM800_ATI,                                 //2.
    SIM800_CSQ,                                 //3.
    SIM800_CPIN,                                //4.
	SIM800_CCID,                                //5.
	SIM800_CSCLK,                               //6.
	SIM800_CIPRXGET,                            //7.
	SIM800_CREG,                                //8.
    SIM800_CGATT,                               //9.
    SIM800_CK_CGATT,                            //10.
	SIM800_CIPSHUT,                             //11.
	SIM800_CIPMUX,                              //12.
	SIM800_CSTT,                                //13.
	SIM800_CIPSTATUS,                           //14.
	SIM800_CIICR,                               //15.建立无线链路,激活移动场景
	SIM800_CIPSTATUS1,                          //16.
	SIM800_CIFSR,                               //17.CIFSR
	#if (1 == SPECIAL_NET_CARD)
	SIM800_SET_CSND,                            //18.AT+CDNSCFG=100.100.2.136,100.100.2.138
	SIM800_QUE_CSND,                            //19.AT+CDNSCFG?
	#endif
    SIM800_IPOPEN,                              //20.
    SIM800_STATE_NUM                            //21.
}ip_state;


typedef struct{
    char *cmd;
    char *res;
    int wait;                               //at指令等待应答时间
    int nwait;                              //重试间隔时间
    int (*process)(char ok, uint8_t retry);
}sim900_inittab;


extern int GprsInit(void);
extern int GprsCheckRes(char *cmd, char *res, uint16_t tmo);
extern int GprsSendCmd(char *cmd, char *ack, uint16_t waittime, int flag);
void GprsReset(void);
int sim800_check_reset(char ok, uint8_t retry);
int sim800_check_ATE(char ok, uint8_t retry);
int sim800_check_ATS(char ok, uint8_t retry);
int sim800_check_ATI(char ok, uint8_t retry);
int sim800_check_cpin(char ok, uint8_t retry);
int sim800_check_ack(char ok, uint8_t retry);
int sim800_check_CCID(char ok, uint8_t retry);
int sim800_check_CSCLK(char ok, uint8_t retry);
int sim800_check_CIPRXGET(char ok, uint8_t retry);
int sim800_check_CSQ(char ok, uint8_t retry);
int sim800_check_CREG(char ok, uint8_t retry);
int sim800_check_CGATT(char ok, uint8_t retry);
int sim800_check_CIPMUX(char ok, uint8_t retry);
int sim800_check_CIPSHUT(char ok, uint8_t retry);
int sim800_check_CSTT(char ok, uint8_t retry);
int sim800_check_CIICR(char ok, uint8_t retry);
int sim800_check_cgreg(char ok, uint8_t retry);
int sim800_ipopen(char ok, uint8_t retry);
int sim800_default(char ok, uint8_t retry);
int sim800_check_CIPMODE(char ok, uint8_t retry);
int SocketSendData(int socket, uint8_t* data, uint16_t len);
int GetNetSignal(void);
int Sim800cReconnect(void);
int sim800_check_CIPQSEND(char ok, uint8_t retry);
int FtpGet(const char* serv, const char* un, const char* pw, const char* file, uint16_t chsum_in);
int trim(char * data);
extern int GprsSendCmdChkNoSpace(char * cmd, char * ack, int waitCnt, int waittime, uint8_t *data);
int GprsSocketStateCheck(void);


extern FIFO_S_t gSocketPktRxCtrl;
extern uint32_t gSimStatus;
extern uint8_t  gprsBuffer[BUFFER_SIZE];
extern uint8_t  gNetSignal;
extern uint16_t gGprsRxLen;
extern MUX_SEM_STR  gGprsSendMux;

#endif //__SIM800C_H__


