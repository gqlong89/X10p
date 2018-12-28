/*relay.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#ifndef __GUN_H__
#define __GUN_H__

#include "includes.h"
#include "proto.h"



#define PLUG_IDLE               0
#define PLUG_USED_NO_OPEN       1
#define PLUG_USED_OPEN          2
#define PLUG_FAULT              3


enum
{
	GUN_CHARGING_IDLE=0,    		    //����
    GUN_CHARGING_UNKNOW_POWER=1,        //����й���δ���
	GUN_CHARGING_GUN_PULL=2,		    //����С��1.5w
	GUN_CHARGING_FULL=3,		        //1.5w<����<40w
	GUN_CHARGING_WORK,                  //40w<����
};


enum{
    GUN_STATE_IDLE = 0,
    GUN_STATE_WAIT_PLUG_IN,
    GUN_STATE_ON,
    GUN_STATE_OFF,
};

#pragma pack(1)

typedef struct{
	uint8_t  gun_id;
    uint8_t  gun_state;                 //1�ȴ���ǹ 2�����
    uint8_t  is_load_on;                //0:û��������磬��0:�������״̬ 1:����й���δ���  2:����С��1.5w 3:1.5w<����<30w 4:30w<����
	uint8_t  ordersource;				//������Դ  @2018-4-12 add
	uint8_t  user_account[16];          //���� //���ż��û��˺�
	uint8_t  chargerMethod;			    //�Ʒѷ�ʽ 1���̶��շ�  2��ʵʱ�շ�  3���𲽽��շ�
    uint8_t  changePower;               //�����仯���书�� 0.1w
	uint16_t chargerStartingGold;       //�𲽽�� ��
	uint8_t  order[ORDER_SECTION_LEN];
    uint16_t charger_time_plan;         //Ԥ�Ƴ��ʱ�� ����  ����3����
    uint8_t  cost_mode;                 //�Ʒ�ģʽ 1�ֹ���  2���ֹ��� COST_UNIFY
    multiPower_t powerInfo;             //�ֹ��ʶ���Ϣ
    uint8_t  getPowerFlag;              //��⹦�ʶδ���
    uint32_t costTempId;                //�Ʒ�ģ��id
	uint32_t start_time;                //������ʼʱ��
	uint32_t stop_time;
    uint32_t startElec;
    uint16_t money;                     //���ӼƷ��ۼ����ѽ�� ��
    uint8_t  startMode;                 //����ģʽ 1ɨ�� 2���� 3��Ȩ�� 4�¿� 5�����¿� 6Ǯ����
	uint16_t current_usr_money;		    //�����  ��
    uint8_t  stopReason;
    uint8_t  reasonDetail;              //ֹͣ���ԭ��ϸ��
    uint8_t  chargingMode;              //���ģʽ 0�����ܳ��� 1������� 2����ʱ�� 3��������
    uint8_t  maxPower;					//������������� 0.1w ����10w��10w
    uint16_t realChargingTime;          //ʵ�ʳ��ʱ�䣬��������ʱ�� ����
	uint16_t chargingPower;             //���ڹ��ʶμƷѵĹ��� 0.1w
    uint8_t  powerCheckcnt;             //���ʶ�ȡ����
    uint8_t  powerSemenIndex;           //���ʶ�ָʾ
    uint16_t chargingElec;              //0.01kwh �ۼƳ�����
    uint8_t  subsidyType;				//. �������� @2018-4-13 add
	uint16_t subsidyPararm;				//. �������� @2018-4-13 add
    uint8_t  isSync;                    //��������ͬ��״̬ 0-����Ҫͬ�� 1-��һ������ 2--�������� 3--�����ϵ�  @2018-4-18 add
	uint16_t startGoldTime;				//�𲽽�ʱ�� ����
}gun_info_t;
#pragma pack()


typedef struct{
    HT_GPIO_TypeDef* port;
    uint16_t pin;
}Gun_GPIO_Port_t;



extern int CheckGunStatu(uint8_t gunId);
extern int GetChargingGunCnt(void);
extern int GunTurnOff(uint8_t gunId);
extern int GunTurnOn(uint8_t gunId);
extern int TurnOffAllGun(void);
extern int TurnOnAllGun(void);

extern int GunGpioInit(void);
extern int GunInit(void);
extern void RestoreGunStatus(void);


extern gun_info_t gun_info[GUN_NUM_MAX];
extern uint32_t chargingOldTime;

#endif


