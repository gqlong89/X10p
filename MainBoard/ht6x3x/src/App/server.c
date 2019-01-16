/*server.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "server.h"
#include "outNetProto.h"
#include "proto.h"
#include "tfs.h"
#include "CpuCfg.h"
#include "ui.h"
#include "emu.h"
#include "rtc.h"
#include "usart.h"
#include "sim800c.h"
#include "sc8042.h"
#include "flash.h"
#include "lcd.h"
#include "gun.h"
#include "ntcad.h"



const uint8_t gZeroArray[64] = {0};
system_info_t system_info;
uint8_t  gPrintBuff[OUT_NET_PKT_LEN];   //最大心跳报文长度
uint16_t gWrite = 0;
CHG_INFO_STR gChgInfo;
uint8_t SendRecordTimers = 0;//订单发送次数


#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
    if (gWrite < sizeof(gPrintBuff)) 
    {
        gPrintBuff[gWrite++] = ch;
        if ('\n' == ch) 
        {
            UsartSend(DEBUG_UART, gPrintBuff, gWrite);
            gWrite = 0;
        }
    }
    else
    {
		UsartSend(DEBUG_UART, gPrintBuff, sizeof(gPrintBuff));
        gWrite = 0;
	}
    
    return ch;
}


void PrintfData(void *pfunc, uint8_t *pdata, int len)
{
	uint32_t i;

    if (0 == system_info.printSwitch) 
	{
    //    return;
    }
	PRINTF_LOG("call by %s,len=%d, pdata:",(char*)pfunc,len);
    Feed_WDT();
	for (i=0; i<len; i++) 
	{
		printf("%02x",pdata[i]);
	}
	printf("\n");
}


//返回 0 是在线
int IsSysOnLine(void)
{
    if (system_info.isRecvStartUpAck && system_info.is_socket_0_ok) {
        return CL_OK;
    }
    return CL_FAIL;
}


int MuxSempTake(MUX_SEM_STR *pSemp)
{
    if (pSemp->status) {
        if (xTaskGetCurrentTaskHandle() == pSemp->pTaskHandle) {
            //CL_LOG("same task take sem.\n");
            pSemp->status++;
            return CL_OK;
        }else{
            while (pSemp->status) {
                Feed_WDT();
                vTaskDelay(20);
                //if (10 < pSemp->semTakeCnt) {
                    //CL_LOG("take sem cnt=%d,warging.\n",pSemp->semTakeCnt);
                //}else{
                    pSemp->semTakeCnt++;
                //}
            }
        }
    }
    pSemp->status++;
    pSemp->semTakeCnt = 0;
    pSemp->pTaskHandle = xTaskGetCurrentTaskHandle();
    return CL_OK;
}


void MuxSempGive(MUX_SEM_STR *pSemp)
{
    if (0 == pSemp->status) {
        CL_LOG("status=%d,error.\n",pSemp->status);
        return;
    }
    pSemp->status--;
}


// 0x30 -> 0; a -> 0x0a; A -> 0x0a
uint8_t Val(uint8_t ch)
{
    uint8_t val = (uint8_t)-1;

    if ((ch >= '0') && (ch <='9')) {
        val = ch - '0';
        return val;
    }
    if ((ch >= 'A') && (ch <='F')) {
        val = ch - 'A' + 10;
        return val;
    }
    if ((ch >= 'a') && (ch <='f')) {
        val = ch - 'a' + 10;
        return val;
    }
    return val;
}


// des  src:  len:为 src的字符串长度
void DeviceBcd2str(char *des, unsigned char *src , int len)
{
    int i;
    unsigned char *p;
    char tmp[4];

    p = src;
    for(i=0; i<len; i++) {
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%02x", *p++);
        strcat(des, tmp);
    }
}


// 0x12  <= {0x31,0x32}   0x1210 <= {0x31,0x32,0x31} 0x1210 <= {0x31,0x32,0x31,0x30}
// srcLen : pbSrc的长度
int StrToHex(uint8_t *pbDest, const char *pbSrc, int srcLen)
{
    int32_t i=0,j=0;
    uint8_t chl,chh;

    memset(pbDest, 0, srcLen/2);
    while (i < srcLen) {
        if ((i+1) == srcLen) {  //最后一个,是奇数
            chl = Val(pbSrc[i]);
            pbDest[j] = chl<<4;
            j ++;
        }else{
            chh = Val(pbSrc[i]);
            chl = Val(pbSrc[i+1]);
            pbDest[j] = (chh << 4) | chl;
            j ++;
        }
        i = i+2;
    }
    return j;
}


void UpdataGunDataSum(void)
{
    int i;

    for (i=1; i<=GUN_NUM_MAX; i++) {
        system_info.gunCheckSum[i-1] = GetPktSum((void*)&gun_info[i-1], sizeof(gun_info_t));
    }
    FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
}


int GetDialValue(void)
{
    #if (1 == IS_THREE_DIAL)
    return (GET_SWITCH3()<<2) | (GET_SWITCH2()<<1) | GET_SWITCH1();
    #else
    return (GET_SWITCH2()<<1) | GET_SWITCH1();
    #endif
}


void ShowCostTemplate(void)
{
    int i;
    multiPower_t *pMultiPower = &system_info.cost_template.Data.powerInfo;
    segment_str  *pSegment = pMultiPower->segmet;
    unify_t *pUnify = &system_info.cost_template.Data.unifyInfo;

    CL_LOG("trafficid=%d.\n",system_info.cost_template.template_id);
    if (COST_POWER == system_info.cost_template.mode) {
        for (i=0; i<pMultiPower->segmentCnt; i++) {
            CL_LOG("power %d.\n",i+1);
            CL_LOG("startPower=%d.\n",pSegment->startPower);
            CL_LOG("endPower  =%d.\n",pSegment->endPower);
            CL_LOG("price     =%d.\n",pSegment->price);
            CL_LOG("duration  =%d.\n",pSegment->duration);
            pSegment++;
        }
    }else if (COST_UNIFY == system_info.cost_template.mode) {
        CL_LOG("unify.\n");
        CL_LOG("price     =%d.\n",pUnify->price);
        CL_LOG("duration  =%d.\n",pUnify->duration);
    }else{
        CL_LOG("no info,err.\n");
    }
}


//1：按功率段计费 2按统一收费 #define COST_POWER 1  #define COST_UNIFY 2
int GetCostMode(void)
{
    return system_info.cost_template.mode;
}


//0. bcd, SN=10, 10 byte  0001123456789
//1. bcd 识别码
//2.
//3.拔枪是否停止充电 1:停止；2:不停止
//4.充满是否停止     1:停止；2:不停止
//5.拔枪停止充电时长，秒
//6.续冲时长，秒
void WriteCfgInfo(uint8_t index, uint32_t para, uint8_t *pData)
{
    CHARGER_STR charger;

    HT_Flash_ByteRead((void*)&charger, CHARGER_INFO_FLASH_ADDR, sizeof(charger));
    charger.validFlag |= (1<<index);
    switch (index) {
        case CFG_CHARGER_SN:
            memcpy(charger.station_id, pData, para);
            break;
        case CFG_CHARGER_DEVICEID:
            memcpy(charger.idCode, pData, para);
            break;
        case CFG_OPERATORNUM:
            memcpy(charger.OperatorNum, pData, para);
            break;
        case CFG_PULL_GUN_STOP:
            charger.pullGunStopCharging = para;
            break;
        case CFG_CHARGING_FULL_STOP:
            charger.chargingFullStop = para;
            break;
        case CFG_PULL_GUN_STOP_TIME:
            charger.pullGunStopTime = para;
            break;
        case CFG_CHARGING_FULL_TIME:
            charger.chargingFullTime = para;
            break;
        case CFG_CHARGE_POWER:
            charger.changePower = para;
            break;
    }
    charger.checkSum = GetNoZeroSum((void*)&charger, sizeof(charger)-sizeof(charger.checkSum));
    HT_Flash_PageErase(CHARGER_INFO_FLASH_ADDR);
    HT_Flash_ByteWrite((void*)&charger, CHARGER_INFO_FLASH_ADDR, sizeof(charger));
}


void SycTimeCount(uint32_t second)
{
    gOutNetStatus.lastRecvTime = second;
    gChgInfo.lastRecvKbMsgTime = second;
    gChgInfo.turnOnLcdTime = second;
}


void ResetSysTem(void)
{
    Sc8042bSpeech(VOIC_DEVICE_REBOOT);
	//OperateMaintain(OptMainTainResetSys, 0);
    vTaskDelay(1000);
    WDT_RESET_SYS();
    NVIC_SystemReset();
}


int CheckChargerSn(uint8_t *pChargerSn, uint8_t len)
{
    int i;

    for (i=0; i<len; i++) {
        if (9 < (pChargerSn[i] & 0x0f)) {
            return CL_FAIL;
        }
        if (9 < (pChargerSn[i] >> 4)) {
            return CL_FAIL;
        }
    }
    return CL_OK;
}


void PrintSysCfgInfo(void)
{
    CL_LOG("cm=%d,pgst=%d,cft=%d,pgs=%d,cfs=%d,csg=%d,dstart=%d,dstopt=%d.\n",
        system_info.chargerMethod,system_info.pullGunStopTime,system_info.chargingFullTime,system_info.pullGunStop,system_info.chargingFullStop,system_info.chargerStartingGold,system_info.disturbingStartTime,system_info.disturbingStopTime);
    PrintfData("运营商编号", system_info.OperatorNum, sizeof(system_info.OperatorNum));
    PrintfData("设备号", system_info.station_id, sizeof(system_info.station_id));
    PrintfData("识别码", system_info.idCode, sizeof(system_info.idCode));
}


void RecoverInfo(CHARGER_STR *pCharger)
{
    int flag = 0;

    if (pCharger->validFlag & (1<<CFG_CHARGER_SN)) 
	{
        if (memcmp(system_info.station_id, pCharger->station_id, sizeof(pCharger->station_id))) 
		{
            memcpy(system_info.station_id, pCharger->station_id, sizeof(pCharger->station_id));
            flag = 1;
            CL_LOG("rcv sn.\n");
        }
    }

    if (pCharger->validFlag & (1<<CFG_CHARGER_DEVICEID)) 
	{
        if (memcmp(system_info.idCode, pCharger->idCode, sizeof(pCharger->idCode))) 
		{
            memcpy(system_info.idCode, pCharger->idCode, sizeof(pCharger->idCode));
            flag = 1;
            CL_LOG("rcv id.\n");
        }
    }

    if (pCharger->validFlag & (1<<CFG_OPERATORNUM)) 
	{
        if (memcmp(system_info.OperatorNum, pCharger->OperatorNum, sizeof(pCharger->OperatorNum))) 
		{
            memcpy(system_info.OperatorNum, pCharger->OperatorNum, sizeof(pCharger->OperatorNum));
            flag = 1;
            CL_LOG("rcv opnum.\n");
        }
    }

    if (pCharger->validFlag & (1<<CFG_PULL_GUN_STOP)) {
        system_info.pullGunStop = pCharger->pullGunStopCharging;
    }
    if (pCharger->validFlag & (1<<CFG_CHARGING_FULL_STOP)) {
        system_info.chargingFullStop = pCharger->chargingFullStop;
    }
    if (pCharger->validFlag & (1<<CFG_PULL_GUN_STOP_TIME)) {
        system_info.pullGunStopTime = pCharger->pullGunStopTime;
        system_info.cfgFlag = 0x55aa;
    }
    if (pCharger->validFlag & (1<<CFG_CHARGING_FULL_TIME)) {
        system_info.chargingFullTime = pCharger->chargingFullTime;
    }
    if (pCharger->validFlag & (1<<CFG_CHARGE_POWER)) {
        system_info.changePower = pCharger->changePower;
    }

    if ((1 != system_info.pullGunStop) && (2 != system_info.pullGunStop)) {
        system_info.pullGunStop = 1;
    }
    if ((1 != system_info.chargingFullStop) && (2 != system_info.chargingFullStop)) {
        system_info.chargingFullStop = 1;
    }

    if (flag) {
        FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
    }

    PrintSysCfgInfo();
}


//判断是否eeprom,flash数据丢失,并尝试进行数据修复
void ChargerInfoProc(void)
{
    CHARGER_STR charger;

    HT_Flash_ByteRead((void*)&charger, CHARGER_INFO_FLASH_ADDR, sizeof(charger));
    if (charger.checkSum != GetNoZeroSum((void*)&charger, sizeof(charger)-sizeof(charger.checkSum))) 
	{
        charger.validFlag = 0x38;
        charger.pullGunStopCharging = 1;
        charger.chargingFullStop = 1;
        charger.pullGunStopTime = PUT_OUT_GUN_TIME;
        charger.chargingFullTime = CHARGING_FULL_TIME;
        charger.checkSum = GetNoZeroSum((void*)&charger, sizeof(charger)-sizeof(charger.checkSum));
        HT_Flash_PageErase(CHARGER_INFO_FLASH_ADDR);
        HT_Flash_ByteWrite((void*)&charger, CHARGER_INFO_FLASH_ADDR, sizeof(charger));
    }
    RecoverInfo(&charger);
}


int GetChargingTime(int gunId, int power)
{
    int i;
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    segment_str *pSegMent = pGunInfo->powerInfo.segmet;

    if (COST_POWER == pGunInfo->cost_mode) {
        for (i=0; i<pGunInfo->powerInfo.segmentCnt; i++) {
            if ((pSegMent->startPower<= (power/10)) && ((power/10) < pSegMent->endPower)) {
                pGunInfo->powerSemenIndex = i;
                pGunInfo->charger_time_plan = pGunInfo->current_usr_money*pSegMent->duration/pSegMent->price;
                if (CHARGING_START == pGunInfo->chargerMethod) {
					//起步金消耗时间
					pGunInfo->startGoldTime = pGunInfo->chargerStartingGold * pSegMent->duration / pSegMent->price;
				}
                CL_LOG("i=%d,um=%d,price=%d,duration=%d,cpt=%d m,sgt=%d m,gun=%d.\n",i,pGunInfo->current_usr_money,pSegMent->price,pSegMent->duration,pGunInfo->charger_time_plan,pGunInfo->startGoldTime,gunId);
				pGunInfo->getPowerFlag++;
                return CL_OK;
            }

            if (i == (pGunInfo->powerInfo.segmentCnt-1)) {
                //如果实时功率超出了计费模版的功率段的最大功率，就按最大那段功率计费
                if (pSegMent->endPower < (power/10)) {
                    pGunInfo->powerSemenIndex = i;
                    pGunInfo->charger_time_plan = pGunInfo->current_usr_money*pSegMent->duration/pSegMent->price;
                    CL_LOG("i=%d,um=%d,price=%d,duration=%d,cpt=%d m,gun=%d.\n",i,pGunInfo->current_usr_money,pSegMent->price,pSegMent->duration,pGunInfo->charger_time_plan,gunId);
                    pGunInfo->getPowerFlag++;
                    return CL_OK;
                }
            }
            pSegMent++;
        }
        CL_LOG("not find power segment,power=%d w,gunid=%d.\n",power/10,gunId);
        OptFailNotice(201);
        return CL_FAIL;
    }else{
        pGunInfo->getPowerFlag++;
    }
	return CL_OK;
}


void ProcFuseBreak(uint16_t maxPower, uint8_t gunId)
{
    if (maxPower < 70) {
        if (2 <= system_info.noLoadCnt[gunId-1]) {
            SendEventNotice(gunId, EVENT_FUSE_BREAK, 0, 0, EVENT_OCCUR,NULL);
        }else{
            system_info.noLoadCnt[gunId-1]++;
            FlashWriteSysInfo((void*)&system_info, sizeof(system_info), 1);
            CL_LOG("noLoadCnt=%d,gunid=%d.\n",system_info.noLoadCnt[gunId-1],gunId);
        }
    }
}


//power 0.1w
void GunChargingProc(uint8_t gunId, uint16_t power, gun_info_t *pGunInfo)
{
    int i;
    uint32_t time;
    uint32_t powerAll;
    uint16_t powerDiff;
    uint32_t rtcTime = GetRtcCount();
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];

    if (100 <= power) {
        pGunInfo->maxPower = 100;
        if (system_info.noLoadCnt[gunId-1]) {
            system_info.noLoadCnt[gunId-1] = 0;
            FlashWriteSysInfo(&system_info, sizeof(system_info), 1);
        }
    }else{
        if (pGunInfo->maxPower < power) {
            pGunInfo->maxPower = power;
        }
    }

    //如果充电功率5分钟平均小于2w，就认为枪头拔出
    if (pGunCharging->loopCnt <= ++pGunCharging->inCnt) {
        pGunCharging->inCnt = 0;
        pGunCharging->power[pGunCharging->powerIndex] = power;
        if (1 == pGunCharging->isFull) {
            powerAll = 0;
            for (i=0; i<POWER_CHECK_CNT; i++) {
                powerAll += pGunCharging->power[i];
            }
            powerAll = powerAll/POWER_CHECK_CNT;
            if (powerAll < 20) {
                if ((pGunCharging->pullGunStopTime+60) > (uint32_t)(rtcTime - pGunInfo->start_time)) { //充电7分钟之内检测到枪头断开都结束订单
                    CL_LOG("gun pull out,stop,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                    pGunInfo->stopReason = STOP_PULL_OUT;
                    pGunInfo->reasonDetail = REASON_AVERAGE_POWER_LOW;
                    ProcFuseBreak(pGunInfo->maxPower, gunId);
                    return;
                }else{  //大于7分钟之后，实时计费、起步金、固定计费打开枪头检测，停止充电
                    if ((CHARGING_ACTUAL == pGunInfo->chargerMethod) || (CHARGING_START == pGunInfo->chargerMethod) ||
                        ((CHARGING_FIXED == pGunInfo->chargerMethod) && (1 == pGunCharging->pullGunStop))) {
                        CL_LOG("gun pull out,stop,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                        pGunInfo->stopReason = STOP_PULL_OUT;
                        pGunInfo->reasonDetail = REASON_AVERAGE_POWER_LOW;
                        ProcFuseBreak(pGunInfo->maxPower, gunId);
                        return;
                    }
                }
            }
        }
        if (POWER_CHECK_CNT <= ++pGunCharging->powerIndex) {
            pGunCharging->powerIndex = 0;
            pGunCharging->isFull = 1;
        }
    }

    if (power <= PUT_OUT_GUN_POWER) {
        if (GUN_CHARGING_GUN_PULL != pGunInfo->is_load_on) {
            pGunInfo->is_load_on = GUN_CHARGING_GUN_PULL;
            pGunCharging->beginTime = rtcTime;
            CL_LOG("pgst=%ds,gun=%d.\n",pGunCharging->pullGunStopTime,gunId);
        }else{
            time = rtcTime - pGunCharging->beginTime;
            if (pGunCharging->pullGunStopTime <= time) {
                if ((pGunCharging->pullGunStopTime+60) > (uint32_t)(rtcTime - pGunInfo->start_time)) { //充电7分钟之内检测到枪头断开都结束订单
                    CL_LOG("gun pull out,stop,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                    pGunInfo->stopReason = STOP_PULL_OUT;
                    ProcFuseBreak(pGunInfo->maxPower, gunId);
                    return;
                }else{  //大于7分钟之后，实时计费、起步金、固定计费打开枪头检测，停止充电
                    if ((CHARGING_ACTUAL == pGunInfo->chargerMethod) || (CHARGING_START == pGunInfo->chargerMethod) ||
                        ((CHARGING_FIXED == pGunInfo->chargerMethod) && (1 == pGunCharging->pullGunStop))) {
                        CL_LOG("gun pull out,stop,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                        pGunInfo->stopReason = STOP_PULL_OUT;
                        ProcFuseBreak(pGunInfo->maxPower, gunId);
                        return;
                    }
                }
            }else if ((0 == pGunCharging->resetEmuChipFlag) && (30 < time)) {
                WritecalparaByGunId(gunId);
                pGunCharging->resetEmuChipFlag = 1;
            }
        }
    }else { //充满或充电中处理
        if (power < pGunCharging->chargingFullPower) {
            if (GUN_CHARGING_FULL != pGunInfo->is_load_on) {
                pGunInfo->is_load_on = GUN_CHARGING_FULL;
                pGunCharging->currentPower = power;
                pGunCharging->beginTime = rtcTime;
                pGunInfo->powerCheckcnt = 90;  //如果是充电开始的时候还没有进入充电中状态就先进来,需要把功率检测时间加长，避免出现未进入充电中状态就开始确定充电功率
                CL_LOG("power=%d,判满,gun=%d,cft=%ds,cm=%d.\n",power,gunId,pGunCharging->chargingFullTime,pGunInfo->chargerMethod);
            }else{
                //固定计费下开启充满自停开关才做充电自停功能，实时计费和起步金模式下支持充满自停功能。
                if (((CHARGING_FIXED == pGunInfo->chargerMethod) && (1 == pGunCharging->chargingFullStop)) ||
                     (CHARGING_ACTUAL == pGunInfo->chargerMethod) ||
                     ((CHARGING_START == pGunInfo->chargerMethod) && ((uint32_t)(rtcTime - pGunInfo->start_time) > (pGunInfo->startGoldTime*60)))) {
					powerDiff = (power > pGunCharging->currentPower) ? power - pGunCharging->currentPower : pGunCharging->currentPower - power;
					if (pGunInfo->changePower < powerDiff) {
						CL_LOG("powerDiff>%d,p=%d,cp=%d,gun=%d.\n",pGunInfo->changePower,power,pGunCharging->currentPower,gunId);
						pGunCharging->currentPower = power;
						pGunCharging->beginTime = rtcTime;
					}else{
						if (pGunCharging->chargingFullTime <= (uint16_t)(rtcTime - pGunCharging->beginTime)) {
							pGunInfo->stopReason = STOP_CHARGING_FULL;
							CL_LOG("cfs,gun=%d.\n",gunId);
                            return;
						}
					}
				}
            }
        }else { //充电中
            pGunInfo->is_load_on = GUN_CHARGING_WORK;
            if (GUN_STATE_WAIT_PLUG_IN == pGunInfo->gun_state) {
                pGunInfo->gun_state = GUN_STATE_ON;
                pGunInfo->powerCheckcnt = 45;
                pGunInfo->getPowerFlag = 0;
            }
        }

		//确定充电功率
        if (0 == pGunInfo->getPowerFlag) {
            if (pGunInfo->powerCheckcnt) {
                pGunInfo->powerCheckcnt--;
                if (pGunInfo->chargingPower < power) {
                    if (4 <= ++pGunCharging->checkPowerCnt) {
                        pGunCharging->checkPowerCnt = 0;
                        pGunInfo->chargingPower = power;
                        if (0 == pGunInfo->powerCheckcnt) { //如果到了限定时间，功率还在增加，就再延长功率确认时间
                            pGunInfo->powerCheckcnt = 4;
                        }
                    }
                }else if (power < pGunInfo->chargingPower) {
                    pGunCharging->checkPowerCnt = 0;
                }

                if ((15*60) < (uint32_t)(rtcTime - pGunInfo->start_time)) { //如果超过15分钟功率还没有确定，就强制确定一次功率
                    pGunInfo->chargingPower = power;
                    GetChargingTime(gunId, power);
                    FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
                    UpdataGunDataSum();
                }
            }else{ //如果在限定时间内，功率不再增加，就开始按该功率计费
                GetChargingTime(gunId, pGunInfo->chargingPower);
                FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
                UpdataGunDataSum();
            }
        }else if (pGunInfo->getPowerFlag < 15) {
            if (power > pGunInfo->chargingPower) {
                if (150 <= (power - pGunInfo->chargingPower)) { //如果功率持续升高大于8分钟，则进行功率调节
                    if (254 <= ++pGunInfo->powerCheckcnt) {
                        pGunInfo->chargingPower = power;
                        GetChargingTime(gunId, pGunInfo->chargingPower);
                        FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
                        UpdataGunDataSum();
                        pGunInfo->powerCheckcnt = 0;
                    }
                }
            }else if (power < pGunInfo->chargingPower) {
                pGunInfo->powerCheckcnt = 0;
            }
        }
    }
}


void ChargingCtrlProc(void)
{
    GUN_STATUS_STR gunStatus;
    gun_info_t *pGunInfo = NULL;
    GUN_CHARGING_STR *pGunCharging = NULL;
    int i;
    uint16_t  chargingPower = 0;

    for (i=1; i<=GUN_NUM_MAX; i++) {
        pGunInfo = &gun_info[i-1];
        pGunCharging = &gChgInfo.gunCharging[i-1];
        GetGunStatus(i, &gunStatus);
        //CL_LOG("gun %d,voltage=%d,currentPower=%d,current=%d.\n",i,gunStatus.voltage,gunStatus.power,gunStatus.current);
        if (pGunInfo->is_load_on) {
            if (gunStatus.status) {
                if (gunStatus.status & 0x01) {
                    pGunInfo->stopReason = STOP_CHARGER_FAULT;
                    pGunInfo->reasonDetail = REASON_COMM_ERR;
                }else {
                    pGunInfo->stopReason = (gunStatus.status & 0x04) ? STOP_OVER_VOLTAGE : STOP_OVER_LOAD;
                    pGunInfo->reasonDetail = pGunInfo->stopReason;
                    Sc8042bSpeech(VOIC_POWER_TOO_LARGE);
                }
                CL_LOG("fault stop,gunid=%d,status=%d.\n",i,gunStatus.status);
                SendEventNotice(i, EVENT_CHIP_FAULT, CHIP_EMU, gunStatus.status, EVENT_OCCUR,NULL);
            }else {
                if (STOP_UNKNOW == pGunInfo->stopReason) {
                    if (gunStatus.elec > pGunInfo->startElec) {
                        pGunInfo->chargingElec += gunStatus.elec - pGunInfo->startElec;
                        pGunInfo->startElec = gunStatus.elec;
                    }else if (gunStatus.elec < pGunInfo->startElec) {
                        //CL_LOG("startElec=%d,elec=%d,error.\n",pGunInfo->startElec,gunStatus.elec);
                        pGunInfo->startElec = gunStatus.elec;
                    }
                    GunChargingProc(i, gunStatus.power, pGunInfo);
                }
            }

            if (STOP_UNKNOW != pGunInfo->stopReason) {
                StopCharging(i);
            }else{
                if (CHARGING_POWER_LIMIT <= (chargingPower + pGunCharging->currentPower/10)) {
                    pGunInfo->stopReason = STOP_POWER_TOO_BIG;
                    pGunInfo->reasonDetail = REASON_TOTAL_POWER_LARGE;
                    StopCharging(i);
                }else{
                    chargingPower += pGunCharging->currentPower/10;
                }
            }

            if (60 <= (uint32_t)(GetRtcCount() - pGunCharging->chargingTime)) {
                pGunCharging->chargingTime = GetRtcCount();
                pGunInfo->realChargingTime++;
        	    CL_LOG("cgp=%d,cp=%d,m=%d,e=%d,ct=%dm,rct=%d,ilo=%d,cflt=%dm,gun=%d.\n",
                    pGunInfo->chargingPower, gunStatus.power, pGunInfo->money, pGunInfo->chargingElec, (GetRtcCount()-pGunInfo->start_time)/60, pGunInfo->realChargingTime,
                    pGunInfo->is_load_on, (pGunCharging->chargingFullTime-(pGunCharging->chargingTime-pGunCharging->beginTime))/60, i);
            }
            vTaskDelay(10);
        }
    }
    gChgInfo.chargingTotalPower = chargingPower;
}


void ChargingProc(void)
{
    uint8_t cnt = 0;
    segment_str *pPowerSement = NULL;
    gun_info_t *pGunInfo = NULL;
    int i;

	for (i=1; i<=GUN_NUM_MAX; i++) {
        pGunInfo = &gun_info[i-1];
		if (pGunInfo->is_load_on) {
            cnt = 1;
            if ((COST_UNIFY == pGunInfo->cost_mode) || ((COST_POWER == pGunInfo->cost_mode) && pGunInfo->getPowerFlag)) {
                pPowerSement = &pGunInfo->powerInfo.segmet[pGunInfo->powerSemenIndex];
    			pGunInfo->money = pGunInfo->realChargingTime * pPowerSement->price / pPowerSement->duration;
				//充电费用大于用户充电金额,停止充电
				if (pGunInfo->money >= pGunInfo->current_usr_money) {
					CL_LOG("money=%d>=usr_money=%d stop,time=%d min,gunId=%d.\n",pGunInfo->money,pGunInfo->current_usr_money,(GetRtcCount()-pGunInfo->start_time)/60,i);
					pGunInfo->money = pGunInfo->current_usr_money;
					pGunInfo->stopReason = STOP_NORMAL;
                    pGunInfo->reasonDetail = REASON_MONEY_ENOUGH;
					StopCharging(i);
                    continue;
				}
            }

            if ((pGunInfo->realChargingTime > CHARGING_MAX_TIME) || ((24*60*60)<(uint32_t)(GetRtcCount()-pGunInfo->start_time))) {//智能充满超过最大充电时长12小时
			    pGunInfo->money = CHARGING_MAX_TIME * pPowerSement->price / pPowerSement->duration;
				pGunInfo->stopReason = STOP_OTHER;
                pGunInfo->reasonDetail = REASON_OVER_23H;
				CL_LOG("over 12h stop,gunId=%d,start_time=%d.\n",i,pGunInfo->start_time);
				StopCharging(i);
			}
		}
	}

    if (cnt) {
        if (20*60 < (uint32_t)(GetRtcCount() - chargingOldTime)) {
            chargingOldTime = GetRtcCount();
            FlashWriteGunInfo(gun_info, sizeof(gun_info), 1);
            UpdataGunDataSum();
        }
    }
}


void SendHistoryOrder(void)
{
	static int last_index = -1;
	int index;
	HISTORY_ORDER_STR order;

	if ((index = Read_HistoryOrder_first(&order)) != CL_FAIL) {
        if ((1 <= order.gun_id) && (order.gun_id <= GUN_NUM_MAX) && (order.checkSum == GetPktSum((void*)&order, sizeof(order)-2))) {
            gChgInfo.sendOrderGunId = order.gun_id;
		    SendTradeRecordNotice(&order);
            if (index != last_index) {
    			last_index = index;
    			SendRecordTimers = 0;
    		}

    		if (SendRecordTimers++ >= STOP_NOTICE_REPEAT_TIMERS) {
    			Remove_RecordOrder_first();
    			SendRecordTimers = 0;
                gChgInfo.sendOrderGunId = 0;
    		}
        }else{
            Remove_RecordOrder_first();
        }
	}
}


//@2018-4-14 update
void ProcNetStatus(void)
{
    gun_info_t *pGunInfo = NULL;
    static int is_online_history = ONLINE;
	int is_online;

    is_online = (CL_OK == IsSysOnLine()) ? ONLINE : OFFLINE;
	if (is_online != is_online_history) {
		if (ONLINE == is_online) {
			LcdDisplayBackStageConnect(LCD_DISPLAY);
            if (30 < (uint32_t)(GetRtcCount() - gChgInfo.lastOpenTime)) {
                SendStartUpNotice(2);//登录
            }
			// re-store gun state to normal
			for (int i=1; i<=GUN_NUM_MAX; i++) {
                pGunInfo = &gun_info[i-1];
				if (pGunInfo->is_load_on) {
                    pGunInfo->isSync = NET_RECOVER;
				}
			}
		}
		is_online_history = is_online;
	}
}


void OpenNetDevice(void)
{
    while (1) 
	{
        system_info.netType = 5;
        if (CL_OK == OutSizeNetCheck()) 
		{
            system_info.netType = gOutNetStatus.connect;
            break;
        }

        #if (0 == X10C_TYPE)
        system_info.netType = 4;
        if (CL_OK == Sim800cReconnect()) 
		{
            system_info.netType = LOCAL_NET;
            break;
        }
        #endif
    }
}


void StartChargingSend(void)
{
    gun_info_t *pGunInfo = NULL;
    int i;
    GUN_CHARGING_STR *pGunCharging = NULL;
    uint32_t old = GetRtcCount();

    if ((uint32_t)(old - SendStartChargingNoticeTime) > 5) 
	{
        SendStartChargingNoticeTime = old;
        for (i = 1; i <= GUN_NUM_MAX; i++) 
		{
            pGunInfo = &gun_info[i-1];
            pGunCharging = &gChgInfo.gunCharging[i-1];
            if (pGunInfo->is_load_on && (pGunInfo->isSync != 0) && (pGunCharging->startChargerTimers < START_NOTICE_REPEAT_TIMERS)) 
			{
				pGunCharging->startChargerTimers++;
				SendStartChargingNotice(i, pGunInfo->start_time, pGunInfo->isSync);
				if (pGunInfo->isSync == FIRST_START_CHARGING)
				{
					OS_DELAY_MS(500);
					SendHeartBeatFun(i);
				}
                OS_DELAY_MS(200);
            }
        }
    }
}


void ProcGunStatus(void)
{
    uint32_t time;
    int i;
    GUN_STATUS_STR gunStatus;
    GUN_CHARGING_STR *pGunCharging = gChgInfo.gunCharging;
    uint8_t  stop;

    for (i=1; i<=GUN_NUM_MAX; i++) {
        if (pGunCharging->isTesting) {
            stop = 0;
            GetGunStatus(i, &gunStatus);
            //CL_LOG("gun %d,voltage=%d,Power=%d,current=%d.\n",i,gunStatus.voltage,gunStatus.power,gunStatus.current);
            if (gunStatus.status) {
                stop = 1;
            }else{
                time = GetRtcCount() - pGunCharging->beginTime;
                if (pGunCharging->timeLimit < time) {
                    if (TEST_OPEN == pGunCharging->isTesting) {
                        stop = 2;
                    }else{
                        if (gunStatus.power < (2*PUT_OUT_GUN_POWER)) {
                            stop = 3;
                        }else{
                            if ((8*60*60) <= time) {
                                stop = 4;
                            }
                        }
                    }
                }
            }
            if (stop) 
            {
                CL_LOG("stop charging,reason=%d,gunid=%d,status=%d.\n",stop,i,gunStatus.status);
                pGunCharging->isTesting = 0;
                GunTurnOff(i);
            }
        }
        pGunCharging++;
    }
}


void LcdStatusCheck(void)
{
    uint8_t day[8];

    GetRtcTime(day);
    if ((30*60) < (uint32_t)(GetRtcCount() - gChgInfo.turnOnLcdTime)) 
	{
        gChgInfo.turnOnLcdTime = GetRtcCount();
        if ((3 < day[4]) && (day[4] < 17)) 
		{ //临晨3点到第二天17点时段才超时关闭屏幕
            LcdTurnOffLed();
        }
		else
		{
            if (0 == gChgInfo.lcdPowerStatus) 
			{
                LcdTurnOnLed();
            }
        }
    }
}


void ServerTask(void)
{
    uint8_t  first = 1;
    uint32_t time;
    uint8_t  pktBuff[SERVER_PKT_MAX_LEN];
    uint8_t  data;
    uint8_t  step;
    uint8_t  len;
    uint8_t  pktLen;
    uint8_t  length;
    uint8_t  sum;
    uint16_t overFlow = 0;

    SycTimeCount(GetRtcCount());
    gChgInfo.lastBlueStatus = 2;
    GprsInit();
    while (1) 
    {
        system_info.is_socket_0_ok = CL_FALSE;
		LcdDisplayBackStageConnect(LCD_CLEAR);
        LcdDisplaySingnal(LCD_CLEAR);
		LcdDisplayNoSingnal(LCD_DISPLAY);
        OpenNetDevice();
        gChgInfo.errCode = 0;
        gSendHearBeatCnt = 0;
        system_info.tcp_tx_error_times = 0;
        if (1 == first) {
            OS_DELAY_MS(1000);
            first = 0;
        }else{ //离线后的网络恢复，随机等待一定时间(2分钟之内)再重新发送数据到后台，避免后台故障恢复后桩端同时密集发送数据到后台，以缓解后台消息处理压力
            step = gChgInfo.second & 0x7f;
            CL_LOG("wait %ds\n", step);
            OS_DELAY_MS(step * 1000);
        }
        system_info.is_socket_0_ok = CL_TRUE;
        LcdDisplaySingnal(LCD_DISPLAY);
		LcdDisplayNoSingnal(LCD_CLEAR);
        step = FIND_AA;
        while (1) {
            OS_DELAY_MS (60);

            if (overFlow != GetUsartOverFlow(GPRS_UART_PORT)) {
                CL_LOG("gprs fifo of.\n");
                overFlow = GetUsartOverFlow(GPRS_UART_PORT);
            }
            if (system_info.is_socket_0_ok == CL_FALSE) {
                CL_LOG("net break.\n");
                break;
            }

            //本来是本地网络的，如果检测到485网络，就切换到485网络
            if ((LOCAL_NET == system_info.netType) && (OUT_485_NET == gOutNetStatus.connect)) 
            {
                break;
            }

            if (FIND_AA != step) 
            {
                if (5 < (uint32_t)(GetRtcCount() - time)) 
                {
                    CL_LOG("no rx data,step=%d,err.\n",step);
                    step = FIND_AA;
                }
            }
            while (CL_OK == FIFO_S_Get(&gSocketPktRxCtrl, &data)) 
            {
                //printf("%02x ",data);
                switch (step) 
                {
                    case FIND_AA:
                        if (data == 0xAA) {
                            time = GetRtcCount();
                            step = FIND_55;
                            pktBuff[0] = 0xAA;
                            pktLen = 1;
                        }
                        break;

                    case FIND_55:
                        if (data == 0x55) {
                            step = FIND_CHARGER_TYPE;
                            pktBuff[1] = 0x55;
                            pktLen++;
                        }else{
                            step = FIND_AA;
                            CL_LOG("can not find 55.\n");
                        }
                        break;

                    case FIND_CHARGER_TYPE:
                        pktBuff[pktLen++] = data;
                        len = 0;
                        step = FIND_CHAGER_SN;
                        break;

                    case FIND_CHAGER_SN:
                        pktBuff[pktLen++] = data;
                        if (CHARGER_SN_LEN == ++len) {
                            len = 0;
                            step = FIND_LEN;
                        }
                        break;

                    case FIND_LEN:
                        pktBuff[pktLen++] = data;
                        if (2 == ++len) {
                            length = (pktBuff[pktLen-1]<<8) | pktBuff[pktLen-2];
                            if (length >= (SERVER_PKT_MAX_LEN-8)) {
                                CL_LOG("length=%d,err.\n",length);
                                step = FIND_AA;
                            }else{
                                sum = 0;
                                step = FIND_VER;
                            }
                        }
                        break;

                    case FIND_VER:
                        pktBuff[pktLen++] = data;
                        len = 0;
                        sum += data;
                        step = FIND_SERNUM;
                        break;

                    case FIND_SERNUM:
                        pktBuff[pktLen++] = data;
                        sum += data;
                        if (2 == ++len) {
                            step = FIND_CMD;
                        }
                        break;

                    case FIND_CMD:
                        pktBuff[pktLen++] = data;
                        sum += data;
                        if (4 > length) {
                            CL_LOG("length=%d,err.\n",length);
                            step = FIND_AA;
                        }else{
                            len = length - 4;
                            step = len ? RX_DATA : FIND_CHK;
                        }
                        break;

                    case RX_DATA:
                        pktBuff[pktLen++] = data;
                        sum += data;
                        if (1 == --len) {
                            step = FIND_CHK;
                        }
                        break;

                    case FIND_CHK:
                        pktBuff[pktLen++] = data;
                        if (data == sum) {
                            RecvServerData((void*)pktBuff, pktLen);
                        }else{
                            CL_LOG("recv err,sum=%#x,pkt sum=%#x.\n",sum,data);
                        }
                        step = FIND_AA;
                        break;

                    default:
                        step = FIND_AA;
                        break;
                }
            }
        }
    }
}


