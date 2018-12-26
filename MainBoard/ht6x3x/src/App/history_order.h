#ifndef __HISTORY_ORDER_H__
#define __HISTORY_ORDER_H__

#include <stdint.h>


#pragma pack(1)

//订单信息存储头
typedef struct{
	__IO uint16_t r_index;	                //读偏移量
	__IO uint16_t w_index;	                //写偏移量
	uint16_t rsv;
	uint8_t  rsv1[24];
	uint16_t checkSum;
}HISTORY_ORDER_HRAD;


/*********上报历史订单********/
typedef struct{
	uint8_t  gun_id;
	uint8_t  ordersource;				//订单来源
	uint8_t  stopReason;				//结束原因
	uint8_t  stopDetails;				//结束详情
	uint8_t  addChargingTimeCnt;        //进入判满阶段后，由于功率变化，延长充电时间次数
	uint8_t  ctrlFlag;                  //b0~1:拔枪停止充电(1停止/2不停)；b2~3:充满自停(1停止/2不停)
	uint16_t pullGunStopTime;           //拔枪停止充电时间，秒
	uint8_t  chargerMode;				//充电模式
	uint16_t chargerPararm;				//充电参数
	uint8_t  subsidyType;				//补贴类型
	uint16_t subsidyPararm;				//补贴参数
	uint8_t  fw_version;				//固件版本
	uint8_t  user_account[20];          //用户账号
	uint8_t  order[10];
	uint32_t startTime;
	uint32_t stopTime;
	uint32_t startElec;
	uint32_t stopElec;
	uint32_t cost_price;			    //订单费用
	uint32_t template_id;			    //计费模板id
	uint16_t power;
	uint8_t  sampleTimes;			    //采样次数	X10没有功率曲线 填0，保证格式一致
	uint8_t  sampleCycle;			    //采用周期
	uint32_t realChargingTime;          //实际充电时间 秒
	uint8_t  rsv[49];                   //保证128字节
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

