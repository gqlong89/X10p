/**
  ******************************************************************************
  * @file    main.c
  * @author  liutao
  * @version V1.0
  * @date    17-October-2017
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2017 </center></h2>
  ******************************************************************************
  */

#include "includes.h"
#include "simuart.h"
#include "i2c.h"
#include "productTest.h"
#include "blueProto.h"
#include "ntcad.h"
#include "ui.h"
#include "emu.h"
#include "rtc.h"
#include "usart.h"
#include "sim800c.h"
#include "sc8042.h"
#include "flash.h"
#include "lcd.h"
#include "gun.h"
#include "server.h"
#include "upgrade.h"
#include "ht6xxx.h"


TaskHandle_t MainTaskHandle_t;
TaskHandle_t ServerTaskHandle_t;
TaskHandle_t gEmuTaskHandle_t;
uint32_t SendTradeRecordNoticeTime = 0;
uint32_t SendStartChargingNoticeTime = 0;
uint16_t oneUploadTradeNum = 0;
uint32_t uploadHistoryOrderTimes = 0;
const uint8_t gStationId[8] = {0x00,0x00,0x00,0x14,0x72,0x58,0x36,0x90};
uint32_t gLastResetReason;


//******************************************************************
//! \brief  	delay
//! \param  	none
//! \retval
//! \note
//! \note
//******************************************************************
void delay(volatile int tick)
{
    while(tick--);
}

//******************************************************************
//! \brief  	SystemClockInit
//! \param  	none
//! \retval
//! \note   	初始化系统时钟
//! \note
//******************************************************************
void SystemClockInit(void)
{
	HT_PMU->RSTSTA = 0x0000;
	Feed_WDT();
	EnWr_WPREG();
	HT_CMU->CLKCTRL0 &= ~0x0380;//关闭HRC_DET,PLL_DET,LF_DET
	DisWr_WPREG();
    Feed_WDT();
	SwitchTo_Fpll();
	Close_Hrc();
}


void SysCfgInit(void)
{
    system_info.is_socket_0_ok = CL_FALSE;
    system_info.mqtt_sn  = 0;
    system_info.tcp_tx_error_times = 0;
    system_info.isRecvStartUpAck = 0;
    system_info.netType = 0;

    if (((150*60) < system_info.chargingFullTime) || (system_info.chargingFullTime < (30*60))) {
        system_info.chargingFullTime = CHARGING_FULL_TIME;
    }
    if (0xaa55 != system_info.noLoadFlag) {
        system_info.noLoadFlag = 0xaa55;
        memset(system_info.noLoadCnt, 0, sizeof(system_info.noLoadCnt));
    }
    if (0xaa55 != system_info.voiceFlag) {
        system_info.voiceFlag = 0xaa55;
        system_info.disturbingStartTime = CLOSE_VIOCE_START_IIME;
        system_info.disturbingStopTime = CLOSE_VIOCE_END_IIME;
    }
    if (0x55aa != system_info.cfgFlag) {
        system_info.pullGunStopTime = PUT_OUT_GUN_TIME;
        system_info.cfgFlag = 0x55aa;
    }
    if (0xee != system_info.changePowerFlag) {
        system_info.changePower = 60;
        system_info.changePowerFlag = 0xee;
    }

    CL_LOG("reset = %x.\n", gLastResetReason);
    CL_LOG("fwVer = [%03d], subVer = [%d].\n", FW_VERSION, FW_VERSION_SUB);
    CL_LOG("ip = [%s], port = [%d].\n", NET_SERVER_IP, NET_SERVER_PORT);
    PrintSysCfgInfo();
    CL_LOG("ss=%d, gs=%d.\n",sizeof(system_info), sizeof(gun_info));
}


void LoadSysCfgInfo(void)
{
	FlashReadSysInfo((void*)&system_info, sizeof(system_info));
    if ((MAGIC_NUM_BASE) == system_info.magic_number) 
	{
	//	printf("\n\n\n########################################################################################### \n");
        CL_LOG("second start.\n");
        SysCfgInit();
    }
	else
	{
	//	printf("\n\n\n******************************************************************************************* \n");
        CL_LOG("first start.\n");
        memset((void*)&system_info, 0, sizeof(system_info_t));
        system_info.magic_number = MAGIC_NUM_BASE;
        system_info.noLoadFlag = 0xaa55;
		system_info.disturbingStartTime = CLOSE_VIOCE_START_IIME;
		system_info.disturbingStopTime = CLOSE_VIOCE_END_IIME;
        system_info.voiceFlag = 0xaa55;
        memcpy((void*)&system_info.station_id, gStationId, sizeof(system_info.station_id));
        SysCfgInit();
        FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
        memset(gun_info, 0, sizeof(gun_info));
		FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
        Clear_RecordOrder();
    }
}


void TempProc(void)
{
    static uint8_t tempHight = 0;
    int ret = GetCpuTemp();

    if (65 < ret) {
        TempNotice(ret);
        if (0 == tempHight) {
            tempHight = 1;
            SendEventNotice(0, EVENT_ENV_TEMP_HIGH, ret, 0, EVENT_OCCUR, NULL);
        }
    }else if (ret < 55) {
        if (1 == tempHight) {
            tempHight = 0;
            SendEventNotice(0, EVENT_ENV_TEMP_HIGH, ret, 0, EVENT_RECOVER, NULL);
        }
    }
}


//处理离线订单
void ProcTradeRecord(void)
{
	if (((uint32_t)(GetRtcCount() - uploadHistoryOrderTimes) > 8) && (oneUploadTradeNum < GetOrderCnt())) {
		uploadHistoryOrderTimes = GetRtcCount();

		HISTORY_ORDER_STR order;
		if(Read_HistoryOrder_next(&order) == CL_OK){
			BlueUpLoadHistoryOrder(&order);
		}
	}
}


void SystemResetFlag(void)
{
	if(gLastResetReason & 0x00000100)
	{
		CL_LOG("BOR复位标志位.\n");
	}
	else if(gLastResetReason & 0x00000080)
	{
		CL_LOG("调试复位复位标志位.\n");
	}
    else if(gLastResetReason & 0x00000004)
	{
		CL_LOG("Watch Dog复位标志位.\n");
	}
	else if(gLastResetReason & 0x00000040)
	{
		CL_LOG("软复位复位标志位.\n");
	}
	else if(gLastResetReason & 0x00000020)
	{
		CL_LOG("外部RST复位标志位.\n");
	}
	else if(gLastResetReason & 0x00000008)
	{
		CL_LOG("唤醒复位复位标志位.\n");
	}
	else if(gLastResetReason & 0x00000002)
	{
		CL_LOG("LBOR 复位标志位.\n");
	}
	else if(gLastResetReason & 0x00000001)
	{
		CL_LOG("POR复位标志位.\n");
	}
}

void BspInit(void)
{
	SystemClockInit();
	SysTick_Init();
	UsartInit();
	
	SimuartInit();
    OS_DELAY_MS(500);
	printf("\n\n\n########################################################################################### \n");
    SystemResetFlag();
	Lcd_Init();
	LcdEnterInitStu();
    InitTmp();
    RtcInit();
	I2C_Init();
	LoadSysCfgInfo();
    ChargerInfoProc();
    GunInit();
	Sc8042b_Init();
	OutNetInit();
	HistoryOrder_Init();	//订单初始化
	
	CL_LOG("BspInit OK.\n");
	printf("\n########################################################################################### \n");
        
	return;
}

void MainTask(void)
{
    uint32_t old;
    uint32_t secondOk;
    int ret = 0;
    uint8_t  flag = 0;

   	BspInit();
    
	memset(&gChgInfo, 0, sizeof(gChgInfo));

    ret  = xTaskCreate((TaskFunction_t)ServerTask,"ServerTask",512,NULL,1,&ServerTaskHandle_t);
    OS_DELAY_MS(500);
	ret |= xTaskCreate((TaskFunction_t)SysTask,"SysTask",400,NULL,1,NULL);
    OS_DELAY_MS(500);
	ret |= xTaskCreate((TaskFunction_t)CkbTask,"CkbTask",512+128,NULL,1,NULL);
    OS_DELAY_MS(500);
    ret |= xTaskCreate((TaskFunction_t)emuTask,"emuTask",256+128,NULL,1,&gEmuTaskHandle_t);
    CL_LOG("task init, ret=%d.\n",ret);

    ShowCostTemplate();
    RestoreGunStatus();
    secondOk = 0;
	while(1) 
    {
        OS_DELAY_MS(300);
        if (GetRtcCount() != old) {
            old = GetRtcCount();
            Feed_WDT();

            //socket已经建立连接
            if ((system_info.is_socket_0_ok == CL_TRUE) && ((LOCAL_NET == system_info.netType) || (OUT_485_NET == system_info.netType))) {
                if (system_info.isRecvStartUpAck == 0) { //未登录，首次上电，需要处理id2
                    #if (1 == ID2)
                    if (LOCAL_NET == system_info.netType) { //本地sim上网才进行id2加密处理
                        if (0 == memcmp(gZeroArray, gID2, TFS_ID2_LEN)) {
                            if (CL_OK == TfsGetId2(gID2)) {
                                SendDeviceAesReq(GetRtcCount(), gChgInfo.ReqKeyReason);
                                gChgInfo.lastOpenTime = GetRtcCount();
                            }
                        }else{
                            if (0 == AesKeyUpdateFlag) {
                    			if ((gChgInfo.sendCnt ? 64 : 6) < (uint32_t)(GetRtcCount() - gChgInfo.lastOpenTime)) {
                                    SendDeviceAesReq(GetRtcCount(), gChgInfo.ReqKeyReason);
                                    gChgInfo.sendCnt++;
                                    gChgInfo.lastOpenTime = GetRtcCount();
                                }
                            }
                        }
                    }else{
                        AesKeyUpdateFlag = 1;
                    }
                    #else
                    AesKeyUpdateFlag = 1;
                    #endif

                    if (1 == AesKeyUpdateFlag) {
                        if (gChgInfo.netStatus & (1<<6)) {
                            gChgInfo.netStatus &= ~(1<<6);
                            secondOk = 0;
                        }
                        if (0 == (secondOk & 0x3f)) {
                            if (memcmp(gZeroArray, system_info.idCode, sizeof(system_info.idCode))) {
                                SendStartUpNotice(1);//登录
                                gChgInfo.lastOpenTime = GetRtcCount();
                                flag = 1;
                            }else{
                                SendRegister();//注册
                            }
                        }
                    }
                }else{//已经登录
                    if (1 == flag) {//第一次登陆成功
                        flag = 2;
                        SwitchToUi_Standby();
                        //SendReqCostTemplate(0); //每次登录都向后台请求一次计费模版
                    }

                    #if (1 == ID2)
                    if ((LOCAL_NET == system_info.netType) && (0 == AesKeyUpdateFlag)) {
                        if (gChgInfo.netStatus & (1<<7)) {
                            gChgInfo.netStatus &= ~(1<<7);
                            secondOk = 0;
                        }
                        if (0 == (secondOk & 0x3f)) {
                            SendDeviceAesReq(GetRtcCount(), gChgInfo.ReqKeyReason);
                        }
                    }
                    #endif

                    StartChargingSend();

                    //发送充电完成通知
                    if ((uint32_t)(old - SendTradeRecordNoticeTime) > 30) {
                        SendTradeRecordNoticeTime = old;
                        SendHistoryOrder();
                    }

                    if (LOCAL_NET == system_info.netType) {
                        if (0 == (secondOk & 0x1f)) {
                            GprsSocketStateCheck();
                        }
                        if (0 == (secondOk & 0xff)) {
                            GprsSendCmd("AT+CSQ\r","\r\nOK\r\n",100, 0);
                        }
                    }

                    //心跳
                    if (0 == (secondOk % 90)) {
                        if (0 == gChgInfo.sendPktFlag) {
                            HeartBeatHandle();
                        }
                        if (system_info.printSwitch) 
						{
                            if ((4*24*60*60) < (GetRtcCount() - system_info.logOpenTime)) 
							{
                                CL_LOG("log close.\n");
                                SetPrintSwitch(0);
                            }
                        }
                    }

                    if (0 == (secondOk & 0x3ff)) {
                        CL_LOG("running...,second=%d.\n",old);
                        //CostTemplateReq();
                        TempProc();
                    }

                    if ((86400/2) < secondOk) {
                        secondOk = 0;
                        CostTemplateReq();
                        //SendReqCostTemplate(0);
                    }
                }
                secondOk++;
            }
			else
			{
                secondOk = 0;
            }

            if (10 < gChgInfo.second) { //为了避免上电时间不够，功率还没有完全上升就开始检测功率，导致误停止，先等一段时间再进行充电控制处理
                //充电处理
                if (gChgInfo.second & 1) {
                    ChargingCtrlProc();
                }
                if (0 == (gChgInfo.second & 0x07)) {
                    ProcNetStatus();
                    ChargingProc();
                    CheckDialValue();
                }
            }

            if (0 == (gChgInfo.second & 0xf)) {
                CheckVoiceChipPower();
                ProcGunStatus();
                BlueTimingOpen();
			}

            //蓝牙已经连接
			if (gBlueStatus.status && (upgradeInfo.upgradeFlag == 0)) {
				//发送心跳
				if (0 == (gChgInfo.second % 10)) {
					BlueSendHeartBeat();
					//CL_LOG("BlueSendHeartBeat.\n");
				}
				if(memcmp(system_info.idCode,gZeroArray,sizeof(system_info.idCode)) != 0) {
					//判断蓝牙离线
					ProcBtHeartBeat();
					//发送历史订单
					ProcTradeRecord();
				}else{//设备注册
					if (0 == (gChgInfo.second & 0x07)) {
						BlueRegister();
						CL_LOG("blue register.\n");
					}
				}
            }
            gChgInfo.second++;
        }
	}
}


//******************************************************************
//! \brief  	main
//! \param
//! \retval
//! \note   	主函数 初始化的函数不要进行打印
//******************************************************************
int main(void)
{
    gLastResetReason = HT_PMU->RSTSTA;
	delay(0x1FFF);
    Feed_WDT();
	//SystemClockInit();
	//SysTick_Init();
	//UsartInit();
    //由于系统资源限制，目前不能启动大于5个线程
    xTaskCreate((TaskFunction_t)MainTask,"MainTask",512,NULL,1,&MainTaskHandle_t);
    vTaskStartScheduler();
	while(1) 
	{
		Feed_WDT();
		vTaskDelay(1000);
	}
}


