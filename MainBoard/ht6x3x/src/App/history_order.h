#ifndef __HISTORY_ORDER_H__
#define __HISTORY_ORDER_H__

#include <stdint.h>


#pragma pack(1)

//������Ϣ�洢ͷ
typedef struct{
	__IO uint16_t r_index;	                //��ƫ����
	__IO uint16_t w_index;	                //дƫ����
	uint16_t rsv;
	uint8_t  rsv1[24];
	uint16_t checkSum;
}HISTORY_ORDER_HRAD;


/*********�ϱ���ʷ����********/
typedef struct{
	uint8_t  gun_id;
	uint8_t  ordersource;				//������Դ
	uint8_t  stopReason;				//����ԭ��
	uint8_t  stopDetails;				//��������
	uint8_t  addChargingTimeCnt;        //���������׶κ����ڹ��ʱ仯���ӳ����ʱ�����
	uint8_t  ctrlFlag;                  //b0~1:��ǹֹͣ���(1ֹͣ/2��ͣ)��b2~3:������ͣ(1ֹͣ/2��ͣ)
	uint16_t pullGunStopTime;           //��ǹֹͣ���ʱ�䣬��
	uint8_t  chargerMode;				//���ģʽ
	uint16_t chargerPararm;				//������
	uint8_t  subsidyType;				//��������
	uint16_t subsidyPararm;				//��������
	uint8_t  fw_version;				//�̼��汾
	uint8_t  user_account[20];          //�û��˺�
	uint8_t  order[10];
	uint32_t startTime;
	uint32_t stopTime;
	uint32_t startElec;
	uint32_t stopElec;
	uint32_t cost_price;			    //��������
	uint32_t template_id;			    //�Ʒ�ģ��id
	uint16_t power;
	uint8_t  sampleTimes;			    //��������	X10û�й������� ��0����֤��ʽһ��
	uint8_t  sampleCycle;			    //��������
	uint32_t realChargingTime;          //ʵ�ʳ��ʱ�� ��
	uint8_t  rsv[49];                   //��֤128�ֽ�
	uint16_t checkSum;
}HISTORY_ORDER_STR;


#pragma pack()


void HistoryOrder_Init(void);

void Clear_RecordOrder(void);
void TradeIndex_Clear(void);
int Write_HistoryOrder(HISTORY_ORDER_STR *order);
int TradeIndex_Add(void);
int Read_HistoryOrder_first(HISTORY_ORDER_STR *order);
int Read_HistoryOrder_t(uint16_t index,HISTORY_ORDER_STR *order);
int isTradeIndexTrue(uint16_t readTradeIndex);
int Read_HistoryOrder_next(HISTORY_ORDER_STR *order);
int Remove_RecordOrder(uint8_t gun_id,uint32_t startTime,uint32_t stopTime);
int Remove_RecordOrder_first(void);
int GetOrderCnt(void);
int MakeTradeInfo(HISTORY_ORDER_STR *order,int gun_id);
int CheckOrderInfo(void);
int CheckOrderBuffStatus(void);

#endif

