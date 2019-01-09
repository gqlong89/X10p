/*emu.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/


#ifndef __EMU_H__
#define __EMU_H__

#include "includes.h"


#define VPT_VALUE   10	//过滤阈值，3mA一下认为是0，包含负值



#define   RETRYTIME  10 //10次可以保证硬件差异的误码率 速度也不会影响

#define   EMU_1  3
#define   EMU_2  4
#define   EMU_3  5
#define   EMU_4  6
#define   EMU_5  9
#define   EMU_6  7

#define     FIXED_HEAD            0x6A

#define     CHECK_OK              0x54
#define     CHECK_FAILED          0x63


/*********************计量参数寄存器***********************************/
#define			EMU_SPLI1 				    0x00
#define			EMU_SPLI2 				    0x01
#define			EMU_SPLU  				    0x02
#define			EMU_IDc						    0x03
#define 		EMU_UDc						    0x04
#define			EMU_RMSI1	 				    0x06
#define			EMU_RMSI2  					  0x07
#define			EMU_RMSU 	 				    0x08
#define			EMU_FreqU 					  0x09
#define			EMU_PowerP1 				  0x0A
#define			EMU_PowerQ1  				  0x0B
#define			EMU_PowerS	 				  0x0C
#define			EMU_EnergyP					  0x0D
#define			EMU_EnergyQ					  0x0E
#define 		EMU_UdetCNT					  0x0F
#define			EMU_PowerP2 				  0x10
#define			EMU_PowerQ2  				  0x11
#define 		EMU_MAXUWAVE			    0x12

#define 		EMU_CRCChecksum				0x15
#define 		EMU_BackupData				0x16

#define 		EMU_SUMChecksum				0x18
#define 		EMU_EMUSR					    0x19
#define 		EMU_SYSSTA					  0x1A
#define 		EMU_ChipID					  0x1B
#define 		EMU_DeviceID				  0x1C


/*********************校表参数寄存器***********************************/

#define 		VAR_EMUIE						  0x30
#define			VAR_EMUIF						  0x31
#define 		VAR_WPREG						  0x32
#define 		VAR_SRSTREG						0x33

#define 		VAR_EMUCFG						0x40
#define			VAR_FreqCFG						0x41
#define 		VAR_ModuleEn					0x42
#define 		VAR_ANAEN						  0x43

#define 		VAR_IOCFG						  0x45

#define 		VAR_GP1							  0x50
#define 		VAR_GQ1							  0x51
#define 		VAR_GS1							  0x52

#define 		VAR_GP2							  0x54
#define 		VAR_GQ2							  0x55
#define 		VAR_GS2							  0x56

#define 		VAR_QPhsCal						0x58
#define			VAR_ADCCON						0x59

#define 		VAR_I2Gain						0x5B
#define			VAR_I1Off						  0x5C
#define			VAR_I2Off						  0x5D
#define			VAR_UOff						  0x5E
#define			VAR_PQStart						0x5F

#define			VAR_HFConst						0x61
#define			VAR_CHK						  	0x62
#define			VAR_IPTAMP						0x63

#define			VAR_P1OFFSETH					0x65
#define			VAR_P2OFFSETH					0x66
#define			VAR_Q1OFFSETH					0x67
#define			VAR_Q2OFFSETH					0x68
#define			VAR_I1RMSOFFSET				0x69
#define			VAR_I2RMSOFFSET				0x6A
#define			VAR_URMSOFFSET				0x6B
#define			VAR_ZCrossCurrent			0x6C
#define			VAR_GPhs1						  0x6D
#define			VAR_GPhs2						  0x6E
#define			VAR_PFCnt						  0x6F
#define			VAR_QFCnt						  0x70

#define			VAR_ANACON						0x72
#define			VAR_SUMCHECKL					0x73
#define			VAR_SUMCHECKH					0x74
#define			VAR_MODECFG						0x75
#define			VAR_P1OFFSETL					0x76
#define			VAR_P2OFFSETL					0x77
#define			VAR_Q1OFFSETL					0x78
#define			VAR_Q2OFFSETL					0x79
#define			VAR_UPeakLvl					0x7A
#define			VAR_USagLvl						0x7B
#define			VAR_UCycLen						0x7C


typedef struct{
    uint8_t  powerErrCnt;
    uint8_t  readErrCnt;
    uint8_t  overvolcount;
    uint8_t  overcurcount;
    float    elec;
}EMU_CTRL_STR;

typedef struct{
    uint16_t writeSend0Err;
    uint16_t writeSend1Err;
    uint16_t writeSend2Err;
    uint16_t writeOk;
    uint16_t readCheckErr;
    uint16_t readRecv1Err;
    uint16_t readOk;
}EMU_STATIC_STR;


typedef struct{  //计量芯片配置寄存器

	//EMU配置寄存器
	uint16_t EMUCFG_values;
	//脉冲常数
	uint16_t HFConst_value;
	uint16_t I2Gain_value;
	//通道1
	uint16_t GP1_value;
	uint16_t GQ1_value;
	uint16_t GS1_value;
	uint16_t GPhs1_value;
	uint16_t P1OffsetL_vlue;
	uint16_t P1OffsetH_value;
	//通道2
	uint16_t GP2_value;
	uint16_t GQ2_value;
	uint16_t GS2_value;
	uint16_t GPhs2_value;
	uint16_t P2OffsetH_value;
	uint16_t P2OffsetL_vlue;

	//AD增益
	uint16_t ADCCON_value;
	//移动功率设置值
	uint16_t PQStart_value;
	//ADC 开关寄存器
	uint16_t ANAEN_value;
	//EMU配置
	uint16_t ModuleEn_value;
	//i2电流通道1有效值偏置校正寄存器
	uint16_t I1RMSOFFSET;
	uint16_t I2RMSOFFSET;
	//电压系数
	float k_vol;
	//电流系数
	float K_cur;
	//功率系数
	float k_power;

	uint8_t checksum;
}HT7017CALPARA;

typedef struct{
	HT7017CALPARA  setupconfig;
}EMU_INFO;

typedef struct{
    uint8_t  status;            //bit0:通信故障 bit1:是否过流 bit2:是否过压
    uint8_t  temp;              //温度 枪头温度 度 -50度偏移  -50~200
	uint16_t power;             //功率 0.1w
    uint16_t voltage;           //电压 0.1v
	uint16_t current;           //电流 1mA 最大65535mA=65A
	uint32_t elec;              //电量 0.01kwh
}GUN_STATUS_STR;



extern int GetGunStatus(int gunId, GUN_STATUS_STR *pGunStatus);
extern void emuTask(void);
extern int SIMU_USART_Send(int no,uint8_t *buf, uint8_t len);
extern int WritecalparaByGunId(uint8_t gunId);
extern int HT7017_Read(int no,uint8_t RegAdd,uint8_t *Data);


#define EMU_CAL_SIZE		582  	//(2+2+576+2)

typedef struct{
	uint16_t magic_number;	    //0xAA55
	uint16_t autoLearnFlag;	    //每一位表示一路计量是否学习过 0--未学习  1---已学习
	float matrixA[12*12];	    //矩阵系数
	uint16_t checkSum;			//以上3个字段的校验和
}EMU_CAL;

extern EMU_CAL EmuCalation;
extern uint8_t EMUIFIFlag;//寄存器是否初始化

#endif //__EMU_H__


