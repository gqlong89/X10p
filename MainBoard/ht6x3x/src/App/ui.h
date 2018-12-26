/*ui.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __UI_H__
#define __UI_H__

#include "includes.h"




typedef enum{
	UI_POWERON=0,					//�ϵ�
	UI_STANDBY,						//����ҳ
	UI_READING_CRAD,				//������ҳ��
	UI_READ_CARD_SUCCESS,			//�����ɹ�
	UI_VERIFY_CARD_FAILURE,			//��Ƭ״̬ҳ��
	UI_CARD_CHARGING,				//�ÿ�Ƭ�г���ж���
	UI_INPUT_SOCKET_NUM,			//����������ҳ��
	UI_INPUT_CHARGER_MONEY,			//������ҳ��
	UI_SOCKET_OCCUPY,				//������ռ��ҳ��
	UI_SOCKET_EXCEPTION,			//��������ҳ��
	UI_SOCKET_NOT_EXSIT,			//�����Ų�����ҳ��
	UI_START_CHARGING,				//���������ҳ��
	UI_START_PAYMENT_CARD_CHARGING,				//֧������������н���
	UI_CARD_MONEY_INSUFFICIENT,		//������ҳ��
	UI_PAYMENT_CARD_MONEY_INSUFFICIENT,		//zfk???????
	UI_CHARGING,					//�����ҳ��
	UI_START_CHARGING_FAILURE,		//�������ʧ��ҳ��
	UI_OVER_POWER,					//���ʹ���
	UI_EQUIP_OFFLINE_EXCEPTION,		//�豸���߻����
	UI_EQUIP_UPGRADE,				//�豸����
	UI_ADMINISTRATOR_TEST,			//����Ա����ģʽҳ��
	UI_INPUT_CHARGER_SN,			//����׮��ҳ��
	UI_INPUT_DEVICE_ID,				//����ʶ����ҳ��
	UI_SET_PRINT_SWITCH,            //��������mac��ַ
	UI_RELAY_TEST,					//�̵�������
	UI_EVENT_NOTICE_TEST,			//�¼�֪ͨ�ϱ�����
	UI_VOICE_BROADCAST_TEST,		//������������
	UI_SHOW_SIGNAL,					//�ź�ֵ��ʾ
	UI_LCD_TEST,					//Һ��������
	UI_PRODUCT_TEST,				//��������
	UI_MAX,
}UI_STATE;

typedef struct key_values{
	char inputStr[16];
	int inputLen ;
}KEY_VALUES;


typedef struct{
	UI_STATE Ui_State;
	uint32_t uiStartTime;
}ui_info_t;




void clearKeyBuf(KEY_VALUES *key);
void putCharToKeyBuf(KEY_VALUES *key,uint8_t value);
void delCharFormKeyBuf(KEY_VALUES *key);
void UiClearKeyVal(void);
void UpdateNewUiState(UI_STATE state);
void UiStateLoop(void);
void UiDisplay_PosData(uint8_t pos, uint8_t data);
void UiDisplay_KeyBuf(KEY_VALUES *key);
void UiDisplay_Version(void);
void UiDisplay_SignalVal(int signalVal);
void UiDisplay_Time(uint8_t hour, uint8_t min, uint8_t sec);
void UiDisplay_MonthDay(uint8_t month, uint8_t day);
void UiClearTime(void);
void UiKeyHandle_AdministratorTestMode(uint8_t key);
void UiKeyHandle_InputChargerSn(uint8_t key);
void UiKeyHandle_InputDeviceId(uint8_t key);
void UiKeyHandle_RelayTest(uint8_t key);
void UiKeyHandle_InputSocketNum(uint8_t key);
void KeyToUiProc(uint8_t key);
void UiDisplay_ErrCode(int code);
void UiDisplay_CardMoney(int CardMoney);
void SwitchToUi_EquipOfflineOrException(void);
void SwitchToUi_EquipUpgrade(void);
void SwitchToUi_PowerOn(void);
void SwitchToUi_Standby(void);
void SwitchToUi_ReadingCard(void);
void SwitchToUI_ReadCardSuccess(void);
void SwitchToUI_CardCharging(void);
void SwitchToUi_VerifyCardFailure(uint8_t statu);
void SwitchToUi_InputSocketNum(void);
void SwitchToUi_InputChargerMoney(void);
void SwitchToUi_SocketOccupy(void);
void SwitchToUi_SocketException(void);
void SwitchToUi_SocketNumNotExsit(void);
void SwitchToUi_StartCharging(uint8_t switchFlag);
void SwitchToUi_BanlanceInsufficient(uint8_t switchFlag);
void SwitchToUi_Charging(uint8_t switchFlag);
void SwitchToUi_StartChargingFailure(void);
void SwitchToUi_OverPower(void);

void SwitchToUi_AdministratorTestMode(void);
void SwitchToUi_InputChargerSn(void);
void SwitchToUi_InputDeviceId(void);
void SwitchToUi_InputBtMacAddr(void);
void SwitchToUi_RelayTest(void);
void SwitchToUi_ProductTest(void);
extern UI_STATE UIGetState(void);
extern void SwitchToUI_ShowBalance(uint32_t user_card_balance);
extern void SwitchToUi_PaymentCardBanlanceInsufficient(uint8_t switchFlag);

extern void Ui_InputSocketNum(void);
extern void Ui_SocketNumNotExsit(void);
extern void SwitchToUi_SocketOccupy(void);
extern void Ui_SocketOccupy(void);
extern void Ui_SocketException(void);
extern void Ui_InputChargerMoney(void);
extern void Ui_StartChargingFailure(void);
void SetPrintSwitch(uint8_t para);


#endif //__UI_H__


