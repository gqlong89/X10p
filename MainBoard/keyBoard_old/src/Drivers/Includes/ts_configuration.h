#ifndef _TS_CFG_H
#define _TS_CFG_H
/******************************************************************************************************
注意事项：
1. 触摸外挂电容(即TK_CAP引脚连接电容)建议值：20nF（即203）
2. 触摸引脚串联电阻建议值：1K欧姆
3. 此触摸库直接触碰弹簧或金属触点时，只支持单按键触发，如要支持复合按键，须盖上触摸面板调试
4. 触摸键应用中，触摸库对外的数据接口为KeysFlagSN，KeysFlagSN有效位最大为24，每位表示一个触摸键是否被触摸
5. 宏定义DEBUG为预留功能，目前不可使用
********************************************************************************************************/



#define SUPPORT_KEY							1				//1:支持触摸按键;	0:不支持触摸按键
#define SUPPORT_WHEEL_SLIDER		0				//1:支持转圈或滑条 0：不支持   注意：转圈和滑条在程序处理上无区别

#define SUPPORT_WATER_DETECT		0				//1:支持触摸检测水位功能； 0：不支持

#define ANTI_SPEAKPHONE				0					//1：打开抗对讲机干扰功能； 0：关闭抗对讲机干扰功能； 
/*******************************************************************************************************************/
#define SUPPORT_STOP_MODE_AUTO_SCAN				0	  //1：支持  0：不支持  
//该模式在STOP模式下自动扫描触摸通道，扫描过程中芯片一直处于STOP状态，当触摸数据超出所设置阈值时会产生触摸中断并唤醒CPU
// 注意： 1 该模式只支持6个触摸通道唤醒CPU，当唤醒CPU的触摸通道超过6个时，该模式不适用
//				2 该模式使能后，需同时在需要唤醒CPU的触摸通道定义中或上WAKEUP(|WAKEUP)
//				3 该模式下功耗与开启通道的数量、自动扫描间隔时间、触摸外挂电容都有关系
/*******************************************************************************************************************/
#define SUPPORT_STOP_MODE_MANUAL_SCAN			0		//1：支持  0：不支持  
//该模式在STOP模式下手动分组扫描触摸通道，当扫描完一组后产生触摸中断并唤醒CPU，软件判断触摸数据是否超出阈值
//如果超出则退出STOP模式，否则切换到下一组通道并进入STOP模式。
// 注意： 1 该模式不限制唤醒CPU的通道数量，但是功耗会比前一种大，在不大于6个通道时，建议使用前一种模式
//				2 该模式使能后，需同时在需要唤醒CPU的触摸通道定义中或上WAKEUP(|WAKEUP)
//				3 该模式下功耗与开启通道的数量、扫描间隔时间、触摸外挂电容都有关系
/*******************************************************************************************************************/
#if (SUPPORT_STOP_MODE_MANUAL_SCAN || SUPPORT_STOP_MODE_AUTO_SCAN)
	#define WAKEUP	0x80
#else
	#define WAKEUP	0x00
#endif
/*********************************************************************************************************************/
#if SUPPORT_KEY

/*******打印开关定义，用于触摸按键调试**********************************************************/
	#define PRINT_TS_KEY_EN			1			//打印触摸按键值开关
	#define PRINT_TS_KEY_DATA_EN	0		//打印触摸数据开关
	#if PRINT_TS_KEY_DATA_EN
		#define PRINT_CH			6				//打印通道定义,如果定义为0xFF，则打印所有触摸键通道
	#endif
/***********************************************************************************************/

/********触摸按键通道及触发门限值定义***********************************************************/
	#define KEY_CH_COUNT			12			//触摸按键数量，必须正确填写
	#define KEY_SEQ						\
	{TS_CH3|WAKEUP,0x20},		\     
	{TS_CH4|WAKEUP,0x18},    	\
	{TS_CH5|WAKEUP,0x18},    	\
	{TS_CH6|WAKEUP,0x20},    	\
	{TS_CH9|WAKEUP,0x18},    	\
	{TS_CH10|WAKEUP,0x18},    \
	{TS_CH11|WAKEUP,0x20},   \
	{TS_CH12|WAKEUP,0x18},    \
	{TS_CH13|WAKEUP,0x20},    \
	{TS_CH14|WAKEUP,0x18},    \
	{TS_CH18|WAKEUP,0x18},    \
	{TS_CH19|WAKEUP,0x20},    \
/*	|							|
		|							|	
		|							------> 对应触摸通道手指正常触摸时数据变化量，依次填写。注意：按键触发门限值、噪声门限值等都是在此数值基础上取一定比例
		--------------------> 按键功能触摸通道序号，依次填写。如果使能了省电模式且该触摸通道触摸需要唤醒MCU，则在通道序号基础上或上WAKEUP(|WAKEUP).
*/
/***********************************************************************************************/

/***********触摸按键消抖及按键输出时间定义************************************************************************************/	
#define FINGER_TOUCH_CONFIRM_TIME						3 				//触摸按键按下确认时间，单位为15.625ms
#define FINGER_TOUCH_RELEASE_CONFIRM_TIME		3 				//触摸按键释放确认时间，单位为15.625ms
#define FINGER_LONG_TOUCH_TIME_LIMIT				30    		//限制按键输出时间，单位为0.5s，若定义为0，则关闭此功能	
/***********************************************************************************************/	
#else
	#define KEY_CH_COUNT			0
#endif
/*********************************************************************************************************************/
#if SUPPORT_WHEEL_SLIDER
//-----------------------------------------------------------------------------
	#define PRINT_WHEEL_SLIDER_POSITION_EN			1		//打印触摸转盘或滑条位置开关

	#define PRINT_WHEEL_SLIDER_DATA_EN	0						//打印触摸转盘或滑条触摸数据开关
	#define PRINT_WHEEL_SLIDER_MAX_RATE_EN		0			//打印最大比率值
	#if (PRINT_WHEEL_SLIDER_DATA_EN || PRINT_WHEEL_SLIDER_MAX_RATE_EN)
		#define PRINT_WHEEL_SLIDER_NUM			0					//打印哪个转盘或滑条
	#endif
//-----------------------------------------------------------------------------
	#define WHEEL_SLIDER_COUNT					4			//转盘或滑条数量，必须正确填写
	
	#define WHEEL_SLIDER_TOUCH_CONFIRM_TIME						2 			//转盘或滑条按下确认时间，单位为15.625ms
	#define WHEEL_SLIDER_TOUCH_RELEASE_CONFIRM_TIME		2 			//转盘或滑条释放确认时间，单位为15.625ms	
	#define WHEEL_SLIDER_LONG_TOUCH_TIME_LIMIT				30    	//限制转盘和滑条触发时间，单位为0.5s，若定义为0，则关闭此功能			
//------------------------------------------------------------------------------------	
	#if (WHEEL_SLIDER_COUNT > 0)
		#define WHEEL_SLIDER0_CH_COUNT			4		//第一个转盘或滑条通道数目，必须正确填写
		
		#define WHEEL_SLIDER0_SEQ								\
		{TS_CH6|WAKEUP,0x30},		\
		{TS_CH7|WAKEUP,0x30},		\
		{TS_CH8|WAKEUP,0x28},		\		
		{TS_CH9|WAKEUP,0x30},		\		
/*		|								|
			|								|	
			|								------>对应触摸通道手指正常触摸时数据变化量，依次填写。注意：按键触发门限值、噪声门限值等都是在此数值基础上取一定比例
			-------------------->第一个转盘或滑条触摸通道序号，如果是转盘，按转盘顺时针方向依次填写，如果是滑条，按滑条从左到右或从下到上方向依次填写。 如果使能了省电模式且该触摸通道触摸需要唤醒MCU，则在通道序号基础上或上WAKEUP(|WAKEUP).
*/
		
		#define WHEEL_SLIDER0_CH_MAX_RATE		80,80,75,75		//手指触摸时当前通道数据变化量占变化量最大两通道变化量之和的最大比率，顺序与上述通道定义一一对应。
		#define WHEEL_SLIDER0_POSITION_CORRECTION_FACTOR	96	//转盘或滑条位置校正值
	#endif

		
//以下第二、第三、第四个转盘或滑条的说明参考第一个转盘或滑条的描述，不再赘述
		
//------------------------------------------------------------------------------------		
	#if (WHEEL_SLIDER_COUNT > 1)
		#define WHEEL_SLIDER1_CH_COUNT			3		//第二个转盘或滑条通道数目，必须正确填写
		
		#define WHEEL_SLIDER1_SEQ								\
		{TS_CH2|WAKEUP,0x48},		\
		{TS_CH1|WAKEUP,0x50},		\
		{TS_CH0|WAKEUP,0x20},		\		

		
		#define WHEEL_SLIDER1_CH_MAX_RATE		65,65,65
		#define WHEEL_SLIDER1_POSITION_CORRECTION_FACTOR	42
	#else
		#define WHEEL_SLIDER1_CH_COUNT			0
	#endif
//------------------------------------------------------------------------------------		
	#if (WHEEL_SLIDER_COUNT > 2)
		#define WHEEL_SLIDER2_CH_COUNT			3		//第三个转盘或滑条通道数目，必须正确填写
		
		#define WHEEL_SLIDER2_SEQ								\
		{TS_CH10|WAKEUP,0x50},		\
		{TS_CH11|WAKEUP,0x50},		\
		{TS_CH12|WAKEUP,0x30},		\		
	
		
		#define WHEEL_SLIDER2_CH_MAX_RATE		65,65,65
		#define WHEEL_SLIDER2_POSITION_CORRECTION_FACTOR	42
	#else
		#define WHEEL_SLIDER2_CH_COUNT			0
	#endif
//------------------------------------------------------------------------------------	
	#if (WHEEL_SLIDER_COUNT > 3)
		#define WHEEL_SLIDER3_CH_COUNT			3		//第四个转盘或滑条通道数目，必须正确填写
		
		#define WHEEL_SLIDER3_SEQ								\
		{TS_CH15|WAKEUP,0x50},		\
		{TS_CH14|WAKEUP,0x50},		\
		{TS_CH13|WAKEUP,0x30},		\		

		
		#define WHEEL_SLIDER3_CH_MAX_RATE		65,65,70
		#define WHEEL_SLIDER3_POSITION_CORRECTION_FACTOR	-42
	#else
		#define WHEEL_SLIDER3_CH_COUNT			0
	#endif
//------------------------------------------------------------------------------------
#else
		#define WHEEL_SLIDER0_CH_COUNT	0
		#define WHEEL_SLIDER1_CH_COUNT	0
		#define WHEEL_SLIDER2_CH_COUNT	0
		#define WHEEL_SLIDER3_CH_COUNT	0
#endif

#if SUPPORT_WATER_DETECT
/************************************************************************************************
要实现水位检测功能，需要选择一个触摸参考通道，在参考通道引脚靠近芯片端外接约10pF电容，精度为+-5%以内。
水位检测的原理是：当有水时，检测水位的触摸通道的容值会增加，对应的触摸数值会变小，检测水位的通道触摸数值
和参考通道触摸数值的比率会下降，当此比率超过某数值时，认为是有水。参考通道的作用是可以排除芯片一致性及环境
因素的影响。		
*************************************************************************************************/
		#define PRINT_WATER_DET_CAP_RATE_EN				1	//打印检水比例开关

		#define WATER_REF_CAP_CH		TS_CH0	//参考电容通道定义
		#define WATER_DET_CH				TS_CH1  //触摸检测水通道定义
		
//设置双门限值的目的是避免在临界值附近因数据抖动造成状态频繁变动
		#define WATER_RATE_THD				80			//从无水到有水的门限值定义
		#define NO_WATER_RATE_THD			85			//从有水到无水的门限值定义
		
		#define WATER_DET_FILTER_CNT	20			//滤波次数定义
		
		
//--------------------------------------------------------------------------------------		
		#define WATER_DETECT_CH_CNT		2
#else
		#define WATER_DETECT_CH_CNT		0
#endif
		
		
#define OPENED_TS_COUNT				(KEY_CH_COUNT+WHEEL_SLIDER0_CH_COUNT+WHEEL_SLIDER1_CH_COUNT+WHEEL_SLIDER2_CH_COUNT+WHEEL_SLIDER3_CH_COUNT+WATER_DETECT_CH_CNT)

//==============================================================================


#define FINGER_TOUCH_THRESHOLD_RATE  	70				//手指门限比例，当前设置为70%

#if (SUPPORT_STOP_MODE_AUTO_SCAN || SUPPORT_STOP_MODE_MANUAL_SCAN)
	#define WAKEUP_THRESHOLD_RATE  	10					//阈值门限比例,如定义为10，则为手指正常触摸数据变化量的10%
	#define ENTER_STOP_MODE_TIME		10						//进入触摸省电模式倒数时间，单位为0.5S
	#define SAVE_MODE_SCAN_INTERVAL_TIME	80		//省电模式扫描间隔时间，单位约为1ms  
#endif

#define NOISE_THRESHOLD_RATE  	15		//噪声门限值比率，当数据变化量在此范围内，视为噪声



#define AREA_PNOISE_PLUS_CONFIRM_TIME			90 		//单位为15.625ms
#define AREA_PNOISE_CONFIRM_TIME					80 		//单位为15.625ms
#define AREA_NNOISE_CONFIRM_TIME					50 		//单位为15.625ms
#define AREA_OVERLOW_CONFIRM_TIME					30 		//单位为15.625ms


#define AREA_PNOISE_PLUS_PERMITTED_DEVIATION_RATE			20 
#define AREA_PNOISE_PERMITTED_DEVIATION_RATE				15
#define AREA_NNOISE_PERMITTED_DEVIATION_RATE				15	
#define AREA_OVERLOW_PERMITTED_DEVIATION_RATE				15	


#define FILTER_COUNT								3		

//触摸寄存器参数配置
#define TKDIV_VAL			0			//时钟分频系数
#define TKTMS_VAL			8			//放电时间
#define VRS_VAL				7			//比较器阈值

//省电模式参数配置
#define STOP_TKDIV_VAL		0
#define STOP_TKTMS_VAL			5		//放电时间
#define STOP_VRS_VAL				7			//比较器阈值
//==============================================================================
/*******************************************************************************************************/
#endif



						 

