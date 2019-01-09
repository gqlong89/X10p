/*relay.c
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "gun.h"
#include "emu.h"
#include "rtc.h"
#include "flash.h"
#include "server.h"
#include "relayCtrlTask.h"

gun_info_t gun_info[GUN_NUM_MAX];

uint32_t chargingOldTime = 0;

const Gun_GPIO_Port_t Gun_GPIO_PortTable[GUN_NUM_MAX] =
{
    {   //1		PG13
        HT_GPIOG,
        GPIO_Pin_13,
    },
    {   //2		PG10
        HT_GPIOG,
        GPIO_Pin_10,
    },
    {   //3		PC8
        HT_GPIOC,
        GPIO_Pin_8,
    },
    {   //4		PG14
        HT_GPIOG,
        GPIO_Pin_14,
    },
    {   //5		PC7
        HT_GPIOC,
        GPIO_Pin_7,
    },
    {   //6		PC6
        HT_GPIOC,
        GPIO_Pin_6,
    },
    {   //7		PC10
        HT_GPIOC,
        GPIO_Pin_10,
    },
    {   //8		PC9
        HT_GPIOC,
        GPIO_Pin_9,
    },
    {   //9		PE8
        HT_GPIOE,
        GPIO_Pin_8,
    },
    {   //10	PE6
        HT_GPIOE,
        GPIO_Pin_6,
    },
	{   //11	PG1
        HT_GPIOG,
        GPIO_Pin_1,
    },
	{   //12	PG0
        HT_GPIOG,
        GPIO_Pin_0,
    },
};



//返回正在充电的枪头数目 0:就是没有在充电的枪头
int GetChargingGunCnt(void)
{
    int i;
    int cnt = 0;

    for (i=0; i<GUN_NUM_MAX; i++) {
        if (gun_info[i].is_load_on) {
            cnt++;
        }
    }
    return cnt;
}


int CheckGunStatu(uint8_t gunId)
{
	GUN_STATUS_STR gunStatus;

	if (CL_OK != GetGunStatus(gunId, &gunStatus)) {
		CL_LOG("call GetGunStatus fail.\n");
		return CL_FAIL;
	}

	if (gunStatus.status) {
        CL_LOG("status=%d,error.\n",gunStatus.status);
		return CL_FAIL;
	} else {
		return CL_OK;
	}
}


int GunTurnOn(uint8_t gunId)
{
    if((1 <= gunId) && (gunId <=GUN_NUM_MAX)){
        GPIO_SetBits(Gun_GPIO_PortTable[gunId-1].port, Gun_GPIO_PortTable[gunId-1].pin);
        CL_LOG("open gunid=%d.\n",gunId);
        return 0;
    }else{
        return -1;
    }
}

int GunTurnOff(uint8_t gunId)
{
	//printf("turn %u gun off\r\n", gunId);

    if((1 <= gunId) && (gunId <= GUN_NUM_MAX))
	{
        GPIO_ResetBits(Gun_GPIO_PortTable[gunId-1].port, Gun_GPIO_PortTable[gunId-1].pin);
        return 0;
    }
	else
	{
        return -1;
    }
}


int TurnOffAllGun(void)
{
//    for (int i=0; i<GUN_NUM_MAX; i++) 
//    {
//        GPIO_ResetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
//    }
	RelayCtrl(0,0);

    return 0;
}

int TurnOnAllGun(void)
{
 //   for (int i=0; i<GUN_NUM_MAX; i++) 
 //   {
 //       GPIO_SetBits(Gun_GPIO_PortTable[i].port, Gun_GPIO_PortTable[i].pin);
 //   }
	RelayCtrl(0,1);
	
    return 0;
}


void RestoreGunStatus(void)
{
    int sum;
    gun_info_t *pGunInfo = NULL;

    for(int i=1; i<=GUN_NUM_MAX; i++) {
        pGunInfo = &gun_info[i-1];
        CL_LOG("gunid=%d,is_load_on=%d,gun_state=%d.\n",i,pGunInfo->is_load_on,pGunInfo->gun_state);
        if (pGunInfo->is_load_on) {
            sum = GetPktSum((void*)&gun_info[i-1], sizeof(gun_info_t));
            if (system_info.gunCheckSum[i-1] != sum) {
                CL_LOG("gun %d data check fail,clac sum=%d,sum=%d.\n",i,sum,system_info.gunCheckSum[i-1]);
                pGunInfo->realChargingTime = (GetRtcCount() - pGunInfo->start_time)/60;
            }
            InitChargingCtrlPara(i);
            if ((pGunInfo->changePower < 30) || (pGunInfo->changePower > 150)) {
                pGunInfo->changePower = 60;
            }
            pGunInfo->is_load_on = GUN_CHARGING_UNKNOW_POWER;   //先强制修改为未检测到功率，避免出现马上停止充电
            CL_LOG("restore gun-%d ON.\n", i);
            GunTurnOn(i);
            pGunInfo->isSync = POWER_RECOVER;
        }
    }
}


int GunGpioInit(void)
{
	//PC6 as ch6, PC7 as ch5, PC8 as ch3, PC9 as ch8, PC10 as ch7
	GPIO_Cfg(HT_GPIOC, GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);

	//PG0 as ch12, PG1 as ch11, PG10 as ch2, PG13 as ch1, PG14 as ch4
	GPIO_Cfg(HT_GPIOG, GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_13|GPIO_Pin_14, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);

	//PE8 as ch9	PE6 as ch10
	GPIO_Cfg(HT_GPIOE, GPIO_Pin_6|GPIO_Pin_8, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
	return 0;
}

//交流电流过零点检测中断引脚
void AttGpioInit(void)
{
	GPIO_Cfg(HT_GPIOA, GPIO_Pin_10, GPIO_Mode_AF1, GPIO_Input_Up, GPIO_Output_PP);
	
	EnWr_WPREG();	
	HT_INT->EXTIF = 0x0000;
	NVIC_ClearPendingIRQ(EXTI5_IRQn);
	NVIC_SetPriority(EXTI5_IRQn, 2);
	HT_INT->EXTIE |= (1 << 5);			//INT5下降沿中断使能
	NVIC_EnableIRQ(EXTI5_IRQn);
    DisWr_WPREG();
}

int GunInit(void)
{
    GunGpioInit();
	AttGpioInit();
    FlashReadGunInfo((void*)gun_info, sizeof(gun_info));
    TurnOffAllGun();
    CL_LOG("init ok.\n");
    return 0;
}


