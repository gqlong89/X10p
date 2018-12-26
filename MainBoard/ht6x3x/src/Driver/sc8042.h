/*sc8042.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __SC8042_H__
#define __SC8042_H__

#include "includes.h"


#define VOIC_DELAY_CNT          100

#define POWER_ON_SC()			GPIO_SetBits(HT_GPIOG,GPIO_Pin_8)
#define POWER_DOWN_SC()			GPIO_ResetBits(HT_GPIOG,GPIO_Pin_8)
#define ENABLE_SC_DATA()		GPIO_SetBits(HT_GPIOH,GPIO_Pin_1)
#define DISABLE_SC_DATA()		GPIO_ResetBits(HT_GPIOH,GPIO_Pin_1)

#define ENABLE_SC_RST()			GPIO_SetBits(HT_GPIOH,GPIO_Pin_0)
#define DISABLE_SC_RST()		GPIO_ResetBits(HT_GPIOH,GPIO_Pin_0)

extern uint32_t voiceUndateTime;
extern uint8_t  powerOnFlag;

typedef enum{
	VOIC_NULL1=1,							//��
	VOIC_WELCOME=2,							//��ӭʹ��
	VOIC_NULL2=3,							//��
	VOIC_SHARE_CHARGE=4,					//������
	VOIC_START_UPGRADE=5,                   //��ʼ����
	VOIC_READING_CARD=6,               		//���ڶ���
	VOIC_DEVICE_EXCEPTION=7,				//�豸����
	VOIC_CARD_BANLANCE=8,					//��Ƭ���
	VOIC_CARD_RECHARGER=9,					//������,���ֵ
	VOIC_CARD_INVALID=10,					//��Ƭ��Ч
	VOIC_CARD_CHARGING=11,					//��İ������ڳ��
	VOIC_INPUT_SOCKET_NUM=12,				//������������,��ȷ�ϼ�
	VOIC_INPUT_CHARGRE_MONEY=13,			//����������,��ȷ�ϼ�.���践����һ��,�밴���ؼ�
	VOIC_SOCKET_OCCUPID=14,					//������ռ��,��ѡ����������
	VOIC_SOCKET_ERROR=15,					//��������,��ѡ����������
	VOIC_SOCKET_NUM_INVALID=16,				//���������Ч,����������
	VOIC_NIN_YI_XUANZE=17,					//����ѡ��
	VOIC_HAO_CHAZUO=18,						//�Ų���
	VOIC_CHARGER_MONEY=19,					//�����
	VOIC_YUAN=20,							//Ԫ
	VOIC_VERIFIED_PLUG=21,					//��ȷ�ϳ������ͳ��˲������Ѳ��
	VOIC_CARD_BANLANCE_INSUFFICIENT=22,		//����,����������
	VOIC_START_CHARGING=23,					//��ʼ���
	VOIC_STOP_CHARGER_TIP=24,				//����������,�ɰε���ͷ
	VOIC_PLUG_IN_PLUG=25,					//�뽫��ͷ����
	VOIC_CARD=26,                          	//�֣�ˢ������
	VOIC_KEY=27,                           	//�⣨���Ҽ���������
	VOIC_BLUETOOTH_ONLINE=28,				//����������
	VOIC_BLUETOOTH_OFFLINE=29,				//�����ѶϿ�
	VOIC_ERROR=30,							//����
	VOIC_0=31,                             	//0 (����0)
    VOIC_1=32,                             	//1.
    VOIC_2=33,                             	//2.
    VOIC_3=34,                             	//3.
    VOIC_4=35,                             	//4.
    VOIC_5=36,                             	//5.
    VOIC_6=37,                             	//6.
    VOIC_7=38,                            	//7.
    VOIC_8=39,                             	//8.
    VOIC_9=40,                             	//9
	VOIC_TEST_TIP=41,						//1��׮��,2��ʶ����,3�̵�������
	VOIC_INPUT_SOCKET_TEST=42,				//������Ű�ȷ������
	VOIC_HUNDRED=43,						//��
	VOIC_TEN=44,							//ʮ
	VOIC_YUAN1=45,							//Ԫ
	VOIC_POINT=46,							//��
	VOIC_DEVICE_REBOOT=47,					//�豸����
	VOIC_SUCCESS=48,						//�ɹ�
	VOIC_A=49,								//a
	VOIC_B=50,								//b
	VOIC_C=51,								//c
	VOIC_D=52,								//d
	VOIC_E=53,								//e
	VOIC_F=54,								//f
	VOIC_TEM=55,							//�¶�
	VOIC_DEGREE=56,							//��
	VOIC_THOUSAND=57,						//ǧ
	VOIC_START_CHARGER_FAIL=58,				//�������ʧ��
	VOIC_POWER_TOO_LARGE=59,                //�豸���ʹ����޷����
	PLEASE_CARD_AGAIN = 60,                 //���ٴ�ˢ���Կ������
    INPUT_PIPLE_AND_ENTER = 61,             //��������Ż�������,����ȷ�ϼ���
    CABINET_DOOR_OPEN = 62,                 //�����Ѵ�
    INPUT_BATTERY = 63,                     //�������
    CHARGING_CLOSE_DOOR = 64,               //�ѿ�ʼ��磬��غù��ţ�
    CONFIRM_TO_END_CHARGING = 65,           //���������磬�밴ȷ�ϼ���
    CARD_TO_PAY_MONEY = 66,                 //��Ϊ��δ��ʱȡ����أ������ٴ�ˢ���Կ۳���ʱ���á�
    TAKE_OUT_BATTERY_CLOSE_DOOR = 67,       //��ȡ����ز��غù���
    CHECK_GUN_STATUS = 68,                  //�Ų����������δ���ӣ���ȷ��
    REMAINDER_CHARGING_CNT = 69,            //��Ƭʣ�������
    NUMBER_OF_TIMES = 70,                   //��
	VOICE_NUM,
}SC8042B_VOICE_TYPE;

int Sc8042b_Init(void);
void Sc8042bSpeech(int index);
void OptFailNotice(int code);
void SpeechCardBalance(uint32_t balance);
void SpeechStartCharge(void);
void SpeechChargeing(uint8_t id);
void OptSuccessNotice(int code);
void TempNotice(uint16_t temp);
void CheckVoiceChipPower(void);
void SoundCode(int code);
extern void StartChargeSpeech(void);
void PointOutPullGun(uint8_t gun);
void PointOutNumber(uint16_t number);


#endif //__SC8042_H__


