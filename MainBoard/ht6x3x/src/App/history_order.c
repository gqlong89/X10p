
#include "includes.h"
#include "history_order.h"
#include "flash.h"
#include "sc8042.h"
#include "server.h"
#include "gun.h"



MUX_SEM_STR gTradeRecordMux = {0,0};
__IO HISTORY_ORDER_HRAD orderHeadFlag;
static uint16_t readTradeIndex = 0;
uint8_t gBuff[FLASH_PAGE_SIZE];


void Clear_RecordOrder(void)
{
	orderHeadFlag.r_index = 0;
	orderHeadFlag.w_index = 0;
    orderHeadFlag.checkSum = GetPktSum((void*)&orderHeadFlag, sizeof(orderHeadFlag)-2);
    E2promWriteDataEx(RecordHeadFlashAddr, (void*)&orderHeadFlag, sizeof(orderHeadFlag));
	readTradeIndex = orderHeadFlag.r_index;
}


int GetOrderCnt(void)
{
    return (orderHeadFlag.w_index + RECORD_MAX_BLOCK_NUM - orderHeadFlag.r_index) % RECORD_MAX_BLOCK_NUM;
}


int CheckOrderBuffStatus(void)
{
    int cnt = GetOrderCnt() + 1;

    if (MAX_RECORD > cnt) {
        return CL_OK;
    }
    return CL_FAIL;
}


int CheckOrderInfo(void)
{
    int sum;

    if ((RECORD_MAX_BLOCK_NUM <= orderHeadFlag.w_index) || (RECORD_MAX_BLOCK_NUM <= orderHeadFlag.r_index) || (MAX_RECORD < (sum = GetOrderCnt()))) {
        CL_LOG("w_index=%d, or r_index=%d,sum=%d,error.\n",orderHeadFlag.w_index,orderHeadFlag.r_index,sum);
        return CL_FAIL;
    }
    return CL_OK;
}


void HistoryOrder_Init(void)
{
    uint16_t sum;

    E2promReadDataEx((uint8_t*)&orderHeadFlag, RecordHeadFlashAddr, sizeof(orderHeadFlag));
    sum = GetPktSum((uint8_t*)&orderHeadFlag, sizeof(orderHeadFlag)-2);
	if (orderHeadFlag.checkSum != sum) 
	{
		Clear_RecordOrder();
		CL_LOG("orderHeadFlag init.\n");
	}
	else
	{
        if (CL_OK != CheckOrderInfo()) 
		{
            CL_LOG("call CheckOrderInfo fail.\n");
            Clear_RecordOrder();
        }
    }
    readTradeIndex = orderHeadFlag.r_index;
}


int Write_HistoryOrder(HISTORY_ORDER_STR *order)
{
	if (CL_OK != CheckOrderBuffStatus()) {
		CL_LOG("HistoryOrder Flash buff is full, error.\n");
		return CL_FAIL;
	}

	MuxSempTake(&gTradeRecordMux);

	uint32_t currPage = orderHeadFlag.w_index/RECORD_BLOCK_NUM_PER_PAGE;

	uint32_t currPos = orderHeadFlag.w_index%RECORD_BLOCK_NUM_PER_PAGE;

	CL_LOG("currPage=%d   currPos=%d.\n",currPage,currPos);
	//新的扇区，需要擦除扇区
	if(currPos == 0){
        CL_LOG("Erase page page=%d \n",currPage);
		HT_Flash_PageErase(TradeRecordAddr+currPage*FLASH_PAGE_SIZE);
	}

	//偏移地址
	uint32_t write_pos = currPage*FLASH_PAGE_SIZE + currPos*RECORD_BLOCK_SIZE_PER_PAGE;
	//写订单到flash
	order->checkSum = GetPktSum((void*)order, sizeof(HISTORY_ORDER_STR)-2);
	HT_Flash_ByteWrite((void*)order, TradeRecordAddr+write_pos, sizeof(HISTORY_ORDER_STR));
	orderHeadFlag.w_index = (orderHeadFlag.w_index+1) % RECORD_MAX_BLOCK_NUM;

	//修改订单头信息
	orderHeadFlag.checkSum = GetPktSum((void*)&orderHeadFlag, sizeof(orderHeadFlag)-2);
    E2promWriteDataEx(RecordHeadFlashAddr, (void*)&orderHeadFlag, sizeof(orderHeadFlag));
	MuxSempGive(&gTradeRecordMux);
	return CL_OK;
}


int TradeIndex_Add(void)
{
	readTradeIndex = (readTradeIndex+1)%RECORD_MAX_BLOCK_NUM;
	if((readTradeIndex) == orderHeadFlag.w_index){
		TradeIndex_Clear();
		return CL_FAIL;
	}
	return CL_OK;
}

int Read_HistoryOrder_first(HISTORY_ORDER_STR *order)
{
	if (GetOrderCnt()) {
        CL_LOG("orderNum=%d.\n",GetOrderCnt());
		uint32_t currPage = orderHeadFlag.r_index/RECORD_BLOCK_NUM_PER_PAGE;
		uint32_t currPos  = orderHeadFlag.r_index%RECORD_BLOCK_NUM_PER_PAGE;
		uint32_t read_pos = currPage*FLASH_PAGE_SIZE + currPos*RECORD_BLOCK_SIZE_PER_PAGE;
        FlashReadDataEx((void*)order, TradeRecordAddr+read_pos, sizeof(HISTORY_ORDER_STR));
		return orderHeadFlag.r_index;
	}
	return CL_FAIL;
}


int Read_HistoryOrder_t(uint16_t index, HISTORY_ORDER_STR *order)
{
	if(index == orderHeadFlag.w_index){
		return CL_FAIL;
	}
	MuxSempTake(&gTradeRecordMux);
	uint32_t currPage = index/RECORD_BLOCK_NUM_PER_PAGE;
	uint32_t currPos  = index%RECORD_BLOCK_NUM_PER_PAGE;
	uint32_t read_pos = currPage*FLASH_PAGE_SIZE + currPos*RECORD_BLOCK_SIZE_PER_PAGE;
    FlashReadDataEx((void*)order, TradeRecordAddr+read_pos, sizeof(HISTORY_ORDER_STR));
	MuxSempGive(&gTradeRecordMux);
	return CL_OK;
}


int isTradeIndexTrue(uint16_t readTradeIndex)
{
	if(orderHeadFlag.w_index > orderHeadFlag.r_index){
		if(readTradeIndex >=orderHeadFlag.r_index && readTradeIndex<orderHeadFlag.w_index){
			return CL_OK;
		}
	}

	if(orderHeadFlag.w_index < orderHeadFlag.r_index){
		if(readTradeIndex>=orderHeadFlag.w_index && readTradeIndex<orderHeadFlag.r_index){
			return CL_FAIL;
		}else{
			return CL_OK;
		}
	}
	return CL_FAIL;
}


int Read_HistoryOrder_next(HISTORY_ORDER_STR *order)
{
	//readTradeIndex 纠正
	if(isTradeIndexTrue(readTradeIndex) != CL_OK){
		readTradeIndex = orderHeadFlag.r_index;
	}
	CL_LOG("readTradeIndex=%d \n",readTradeIndex);
	return Read_HistoryOrder_t(readTradeIndex,order);
}


void TradeIndex_Clear(void)
{
	readTradeIndex = orderHeadFlag.r_index;
}


int Remove_RecordOrder(uint8_t gun_id,uint32_t startTime,uint32_t stopTime)
{
	uint16_t index = 0;
	HISTORY_ORDER_STR order;
	uint16_t i = 0;
	uint16_t orderNum = GetOrderCnt();

	CL_LOG("orderNum=%d \n",orderNum);
	//遍历所有的历史订单，查找匹配的记录
	for(i =0; i < orderNum;i++){
		index = (orderHeadFlag.r_index+i)%RECORD_MAX_BLOCK_NUM;
		if(Read_HistoryOrder_t(index,&order) == CL_OK){

			if(order.gun_id == gun_id && order.startTime == startTime && order.stopTime == stopTime){
                CL_LOG("find record.\n");
				break;
			}
		}else{
			CL_LOG("Read_HistoryOrder_t error. \n");
		}
	}
	//没有找到
	if(i == orderNum){
		CL_LOG("not find record.\n");
		return CL_FAIL;
	}

	MuxSempTake(&gTradeRecordMux);
	//需要删除的订单不是第一个，需要将删除的单元和第一个单元交换
	if (index != orderHeadFlag.r_index) {
		HISTORY_ORDER_STR firstOrder;
		Read_HistoryOrder_first(&firstOrder);
		uint32_t currPage = index/RECORD_BLOCK_NUM_PER_PAGE;	//要删除的订单所在扇区
		uint32_t currPos  = index%RECORD_BLOCK_NUM_PER_PAGE;	//扇区内的偏移量
		//uint8_t puff[FLASH_PAGE_SIZE];
        FlashReadDataEx(gBuff, TradeRecordAddr+currPage*FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
		//将第一个订单和和要删除的订单调换
		memcpy(gBuff+currPos*RECORD_BLOCK_SIZE_PER_PAGE,&firstOrder,sizeof(HISTORY_ORDER_STR));
		HT_Flash_PageErase(TradeRecordAddr+currPage*FLASH_PAGE_SIZE);
		HT_Flash_ByteWrite(gBuff,TradeRecordAddr+currPage*FLASH_PAGE_SIZE,FLASH_PAGE_SIZE);
	}

	orderHeadFlag.r_index = (orderHeadFlag.r_index+1)%RECORD_MAX_BLOCK_NUM;
	//修改订单头信息
	orderHeadFlag.checkSum = GetPktSum((void*)&orderHeadFlag, sizeof(orderHeadFlag)-2);
    E2promWriteDataEx(RecordHeadFlashAddr, (void*)&orderHeadFlag, sizeof(orderHeadFlag));
	MuxSempGive(&gTradeRecordMux);
	return CL_OK;
}


int Remove_RecordOrder_first(void)
{
	uint16_t orderNum = GetOrderCnt();

	CL_LOG("orderNum=%d.\n",orderNum);
    if (orderNum) {
    	MuxSempTake(&gTradeRecordMux);
    	orderHeadFlag.r_index = (orderHeadFlag.r_index+1)%RECORD_MAX_BLOCK_NUM;
    	//修改订单头信息
    	orderHeadFlag.checkSum = GetPktSum((void*)&orderHeadFlag, sizeof(orderHeadFlag)-2);
        E2promWriteDataEx(RecordHeadFlashAddr, (void*)&orderHeadFlag, sizeof(orderHeadFlag));
    	MuxSempGive(&gTradeRecordMux);
    }
	return CL_OK;
}


int MakeTradeInfo(HISTORY_ORDER_STR *order,int gun_id)
{
	gun_info_t *pGunInfo = &gun_info[gun_id-1];
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gun_id-1];
	order->gun_id = pGunInfo->gun_id;
	order->chargerMode = pGunInfo->chargingMode;
	order->chargerPararm = pGunInfo->current_usr_money;

	//起步金收费模式下,结束充电时的费用处理
	if (CHARGING_START == pGunInfo->chargerMethod) {
        if (pGunInfo->money < pGunInfo->chargerStartingGold) {
            pGunInfo->money = pGunInfo->chargerStartingGold;
        }
	}
    order->cost_price = pGunInfo->money;
	order->addChargingTimeCnt = pGunInfo->changePower;
    order->ctrlFlag = pGunCharging->pullGunStop & 0x03;
    order->ctrlFlag |= ((pGunCharging->chargingFullStop & 0x03) << 2);
    order->pullGunStopTime = pGunCharging->pullGunStopTime;
	order->fw_version = FW_VERSION;
	memcpy(order->order, (void*)pGunInfo->order, ORDER_SECTION_LEN);
	memcpy(order->user_account,pGunInfo->user_account,sizeof(pGunInfo->user_account));
	order->ordersource = pGunInfo->ordersource;
	order->power = pGunInfo->chargingPower/10;
	order->startTime = pGunInfo->start_time;
	order->stopTime = pGunInfo->stop_time;
	order->stopReason = pGunInfo->stopReason;
	order->startElec = 0;
	order->stopElec = pGunInfo->chargingElec;
	order->subsidyPararm = pGunInfo->subsidyPararm;
	order->subsidyType = pGunInfo->subsidyType;
	order->template_id = pGunInfo->costTempId;
	order->sampleCycle = 0;
	order->sampleTimes = 0;
    order->realChargingTime = pGunInfo->realChargingTime;
	return CL_OK;
}


