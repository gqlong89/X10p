/*lcd.h
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __LCD_H__
#define __LCD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"

typedef enum{
	DATA1 = 0,
	DATA2,
	DATA3,
	DATA4,
	DATA5,
	DATA6,
	DATA7,
	DATA8,
	DATA9,
	DATA10,
	DATA11
}DATA_List_t;

/*
* @brief  LCDƫѹ����ѡ��λ����
*/
typedef enum
{
    LCDBias_OneThird = 0x80,               /*!< 1/3 bias ƫѹ����              */
    LCDBias_OneFourth = 0x00,              /*!< 1/4 bias ƫѹ����              */
}LCDBias_TypeDef;                          /*!< end of group LCDBias_TypeDef   */

/*
* @brief  LCDCom����
*/
typedef enum
{
    LCD_4COM = 0x00,                       /*!< 4Com                           */
    LCD_6COM = 0x08,                       /*!< 6Com                           */
    LCD_8COM = 0x10,                       /*!< 8Com                           */
}LCDCom_TypeDef;                           /*!< end of group LCDCom_TypeDef    */

/*
* @brief  LCDɨ��Ƶ��ѡ��λ����
*/
typedef enum
{
    LCDCLK_Div1 = 0x04,                    /*!< LCDɨ��Ƶ�����                */
    LCDCLK_Div2 = 0x00,                    /*!< LCDɨ��Ƶ��/2                  */
    LCDCLK_Div4 = 0x01,                    /*!< LCDɨ��Ƶ��/4                  */
    LCDCLK_Div8 = 0x02,                    /*!< LCDɨ��Ƶ��/8                  */
    LCDCLK_Div16 = 0x03,                   /*!< LCDɨ��Ƶ��/16                 */
}LCDCLK_TypeDef;                           /*!< end of group LCDCLK_TypeDef    */

/*
* @brief  LCD���ģʽ����
*/
typedef enum
{
    LCDCharge_HighCurrent = 0x00,          /*!< LCD��������ģʽ                   */
    LCDCharge_LittleCurrent = 0x10,        /*!< LCDС�������ģʽ                   */
    LCDChargeQuick_PeriodDiv8 = 0x30,      /*!< ���ٳ��ģʽ�����ʱ��1/8��Flcd���� */
    LCDChargeQuick_PeriodDiv16 = 0x70,     /*!< ���ٳ��ģʽ�����ʱ��1/16��Flcd����*/
    LCDChargeQuick_PeriodDiv32 = 0xb0,     /*!< ���ٳ��ģʽ�����ʱ��1/32��Flcd����*/
    LCDChargeQuick_PeriodDiv64 = 0xf0,     /*!< ���ٳ��ģʽ�����ʱ��1/64��Flcd����*/
}LCDChargeMode_TypeDef;                    /*!< end of group LCDChargeMode_TypeDef  */

/*
* @brief  LCDģ���ʼ���ṹ����
*/
typedef struct
{
    LCDBias_TypeDef LCD_Bias;              /*!< LCDƫѹ��������                     */
    LCDCom_TypeDef  LCD_Com;               /*!< LCDCom Number����                   */
    LCDCLK_TypeDef  LCD_CLK;               /*!< LCDɨ��Ƶ�ʶ���                     */
    LCDChargeMode_TypeDef LCD_ChargeMode;  /*!< LCD���ģʽ����                     */
    uint16_t LCD_Vrsel;                    /*!< LCD_Vrsel[3..0]: �Աȶ�����         */
}LCD_InitTypeDef;                          /*!< end of group LCD_InitTypeDef        */


//1/4 bias �Աȶ�
typedef enum
{
	VRsel_OneFourth_P99= 0x0000,     /*!< 99.1%  */
	VRsel_OneFourth_P93= 0x0001,     /*!< 94.0%  */
	VRsel_OneFourth_P88= 0x0010,     /*!< 88.8%  */
	VRsel_OneFourth_P83= 0x0011,     /*!< 84.2%  */
	VRsel_OneFourth_P78= 0x0100,     /*!< 80.0%  */
	VRsel_OneFourth_P74= 0x0101,     /*!< 76.2%  */
	VRsel_OneFourth_P71= 0x0110,     /*!< 72.8%  */
	VRsel_OneFourth_P68= 0x0111,     /*!< 69.6%  */
	VRsel_OneFourth_P65= 0x1000,     /*!< 66.6%  */
	VRsel_OneFourth_P64= 0x1001,     /*!< 64.0%  */
	VRsel_OneFourth_P60= 0x1010,     /*!< 60.0%  */
	VRsel_OneFourth_P59= 0x1011,     /*!< 59.2%  */
	VRsel_OneFourth_P57= 0x1100,     /*!< 57.2%  */
	VRsel_OneFourth_P55= 0x1101,     /*!< 55.2%  */
	VRsel_OneFourth_P53= 0x1110,     /*!< 53.4%  */
	VRsel_OneFourth_P51= 0x1111,     /*!< 51.6%  */

}LCD_VRsel_OneFourth_TypeDef;


//1/3 bias �Աȶ�
typedef enum
{
	VRsel_OneThree_P97= 0x0000,     /*!< 97.4%  */
	VRsel_OneThree_P92= 0x0001,     /*!< 92.4%  */
	VRsel_OneThree_P85= 0x0010,     /*!< 85.8%  */
	VRsel_OneThree_P80= 0x0011,     /*!< 80.0%  */
	VRsel_OneThree_P75= 0x0100,     /*!< 75.0%  */
	VRsel_OneThree_P70= 0x0101,     /*!< 70.6%  */
	VRsel_OneThree_P66= 0x0110,     /*!< 66.6%  */
	VRsel_OneThree_P63= 0x0111,     /*!< 63.2%  */
	VRsel_OneThree_P60= 0x1000,     /*!< 60.0%  */
	VRsel_OneThree_P57= 0x1001,     /*!< 57.2%  */
	VRsel_OneThree_P54= 0x1010,     /*!< 54.6%  */
	VRsel_OneThree_P52= 0x1011,     /*!< 52.2%  */
	VRsel_OneThree_P50= 0x1100,     /*!< 50.0%  */
	VRsel_OneThree_P48= 0x1101,     /*!< 48.0%  */
	VRsel_OneThree_P46= 0x1110,     /*!< 46.2%  */
	VRsel_OneThree_P44= 0x1111,     /*!< 44.4%  */

}LCD_VRsel_OneThree_TypeDef;


typedef enum{
	LCD_CLEAR = 0,			//���
	LCD_DISPLAY = 1,		//��ʾ
}LCD_Display_TypeDef;


typedef enum{
	SEG0_SOCKET_FAULT 		= 0X02,//����
	SEG0_INVALID 			= 0X04,//��Ч
	SEG0_FAILURE			= 0x08,//ʧ��
}SEG0_DISPLAY_t;

typedef enum{
	SEG1_PROGRESS_BAR4 		= 0x01,	//������T4
	SEG1_PROGRESS_BAR3   	= 0x02,	//������T3
	SEG1_PROGRESS_BAR2 		= 0x04,	//������T2
	SEG1_PROGRESS_BAR1		= 0x08,	//������T1
}SEG1_DISPLAY_t;

typedef enum{
	SEG2_SOCKET_CHARGING	= 0x01,	//�����
	SEG2_SOCKET_IDLE		= 0x02,	//����
	SEG2_SOCKET_OCCUPIED 	= 0x04,	//ռ��
	SEG2_SUCCESS	  		= 0x08, //�ɹ�
}SEG2_DISPLAY_t;

typedef enum{
	SEG3_YUAN				= 0x01,//Ԫ
	SEG3_10C   				= 0x02,//10C
	SEG3_10G 				= 0x04,//10G
	SEG3_10B  				= 0x08,//10B
}SEG3_DISPLAY_t;

typedef enum{
	SEG4_10D				= 0x01,//10D
	SEG4_10E   				= 0x02,//10E
	SEG4_10F				= 0x04,//10F
	SEG4_10A				= 0x08,//10A
}SEG4_DISPLAY_t;

typedef enum{
	SEG5_DP3 				= 0x01,//С����DP3
	SEG5_9C   				= 0x02,//9C
	SEG5_9G 				= 0x04,//9G
	SEG5_9B					= 0x08,//9B
}SEG5_DISPLAY_t;

typedef enum{
	SEG6_9D 				= 0x01,//9D
	SEG6_9E   				= 0x02,//9E
	SEG6_9F 				= 0x04,//9F
	SEG6_9A					= 0x08,//9A
}SEG6_DISPLAY_t;

typedef enum{
	SEG7_DP2 				= 0x01,//С����DP2
	SEG7_8C   				= 0x02,//8C
	SEG7_8G 				= 0x04,//8G
	SEG7_8B					= 0x08,//8B
}SEG7_DISPLAY_t;

typedef enum{
	SEG8_8D 				= 0x01,//8D
	SEG8_8E   				= 0x02,//8E
	SEG8_8F 				= 0x04,//8F
	SEG8_8A					= 0x08,//8A
}SEG8_DISPLAY_t;

typedef enum{
	SEG9_DP1 				= 0x01,//С����DP1
	SEG9_7C   				= 0x02,//7C
	SEG9_7G 				= 0x04,//7G
	SEG9_7B					= 0x08,//7B
}SEG9_DISPLAY_t;

typedef enum{
	SEG10_7D 				= 0x01,//7D
	SEG10_7E   				= 0x02,//7E
	SEG10_7F 				= 0x04,//7F
	SEG10_7A				= 0x08,//7A
}SEG10_DISPLAY_t;

typedef enum{
	SEG11_NOSIGNAL 			= 0x01,//���ź�
	SEG11_SIGNAL   			= 0x02,//���ź�
	SEG11_BACKSTAGE_CONNECT = 0x04,//��̨���ӷ���
	SEG11_CHARGE_METHOD		= 0x08,//��ˢ����ɨ�뿪�����
}SEG11_DISPLAY_t;

typedef enum{
	SEG12_CLOCK_DIAL		= 0x01,//ʱ�ӱ���
	SEG12_1E   				= 0x02,//1E
	SEG12_1G 				= 0x04,//1G
	SEG12_1F				= 0x08,//1F
}SEG12_DISPLAY_t;

typedef enum{
	SEG13_1D				= 0x01,//1D
	SEG13_1C   				= 0x02,//1C
	SEG13_1B 				= 0x04,//1B
	SEG13_1A				= 0x08,//1A
}SEG13_DISPLAY_t;

typedef enum{
	SEG14_REMAINING_TIME	= 0x01,//ʣ��ʱ��
	SEG14_2E   				= 0x02,//2E
	SEG14_2G 				= 0x04,//2G
	SEG14_2F				= 0x08,//2F
}SEG14_DISPLAY_t;

typedef enum{
	SEG15_2D				= 0x01,//2D
	SEG15_2C   				= 0x02,//2C
	SEG15_2B 				= 0x04,//2B
	SEG15_2A				= 0x08,//2A
}SEG15_DISPLAY_t;

typedef enum{
	SEG16_COL1				= 0x01,//ð��1
	SEG16_3E   				= 0x02,//3E
	SEG16_3G 				= 0x04,//3G
	SEG16_3F				= 0x08,//3F
}SEG16_DISPLAY_t;

typedef enum{
	SEG17_3D				= 0x01,//3D
	SEG17_3C   				= 0x02,//3C
	SEG17_3B 				= 0x04,//3B
	SEG17_3A				= 0x08,//3A
}SEG17_DISPLAY_t;

typedef enum{
	SEG18_4E   				= 0x02,//4E
	SEG18_4G 				= 0x04,//4G
	SEG18_4F				= 0x08,//4F
}SEG18_DISPLAY_t;

typedef enum{
	SEG19_4D				= 0x01,//4D
	SEG19_4C   				= 0x02,//4C
	SEG19_4B 				= 0x04,//4B
	SEG19_4A				= 0x08,//4A
}SEG19_DISPLAY_t;

typedef enum{
	SEG20_COL2				= 0x01,//ð��2
	SEG20_5E   				= 0x02,//5E
	SEG20_5G 				= 0x04,//5G
	SEG20_5F				= 0x08,//5F
}SEG20_DISPLAY_t;

typedef enum{
	SEG21_5D				= 0x01,//5D
	SEG21_5C   				= 0x02,//5C
	SEG21_5B 				= 0x04,//5B
	SEG21_5A				= 0x08,//5A
}SEG21_DISPLAY_t;

typedef enum{
	SEG22_BLUTOOTH			= 0x01,//��������
	SEG22_6E   				= 0x02,//6E
	SEG22_6G 				= 0x04,//6G
	SEG22_6F				= 0x08,//6F
}SEG22_DISPLAY_t;

typedef enum{
	SEG23_6D				= 0x01,//6D
	SEG23_6C   				= 0x02,//6C
	SEG23_6B 				= 0x04,//6B
	SEG23_6A				= 0x08,//6A
}SEG23_DISPLAY_t;

typedef enum{
	SEG24_EDGE				= 0x01,//��Ե
	SEG24_TAB1_EDGE   		= 0x02,//TAB1��Ե
	SEG24_TAB2_EDGE			= 0x04,//TAB2��Ե
	SEG24_TAB3_EDGE			= 0x08,//TAB3��Ե
}SEG24_DISPLAY_t;

typedef enum{
	SEG25_NUM				= 0x01,//��
	SEG25_CARD_BALANCE   	= 0x02,//��Ƭ���
	SEG25_SOCKET_NUM 		= 0x04,//�������
	SEG25_CHARING_MONEY		= 0x08,//�����
}SEG25_DISPLAY_t;


typedef enum{
	FIRST = 1,
	SECOND = 2,
	THIRD = 3,
	FOURTH = 4,
	FIFTH = 5,
	SIXTH = 6,
	SEVENTH = 7,
	EIGHTH = 8,
	NINTH = 9,
	TEN = 10,
	ELEVENTH = 11,
}DATA_POS_t;



static void HT_LCD_Init(LCD_InitTypeDef* LCD_InitStruct);
static void Lcd_All_SegCom_IO_Config(void);
static void HT_LCD_Write(uint8_t SegNum, uint8_t SegValue);
static uint8_t HT_LCD_Read(uint8_t SegNum);


void Lcd_Init(void);
void LcdTurnOnLed(void);
void LcdTurnOffLed(void);
void LcdAllOn(void);
void LcdAllOff(void);

void LcdDisplayBackStageConnect(LCD_Display_TypeDef displayFlg);
void LcdDisplaySingnal(LCD_Display_TypeDef displayFlg);
void LcdDisplayNoSingnal(LCD_Display_TypeDef displayFlg);
void LcdDisplayBlutooth(LCD_Display_TypeDef displayFlg);
void LcdDisplayChgMethod(LCD_Display_TypeDef displayFlg);
void LcdDisplaySuccess(LCD_Display_TypeDef displayFlg);
void LcdDisplayFailure(LCD_Display_TypeDef displayFlg);
void LcdDisplayProgressBar1(LCD_Display_TypeDef displayFlg);
void LcdDisplayProgressBar2(LCD_Display_TypeDef displayFlg);
void LcdDisplayProgressBar3(LCD_Display_TypeDef displayFlg);
void LcdDisplayProgressBar4(LCD_Display_TypeDef displayFlg);
void LcdDisplayChargeing(LCD_Display_TypeDef displayFlg);
void LcdDisplaySocketIdle(LCD_Display_TypeDef displayFlg);
void LcdDisplayInvalid(LCD_Display_TypeDef displayFlg);
void LcdDisplaySocketOccupied(LCD_Display_TypeDef displayFlg);
void LcdDisplaySocketFault(LCD_Display_TypeDef displayFlg);
void LcdDisplayCardBalance(LCD_Display_TypeDef displayFlg);
void LcdDisplaySocketNum(LCD_Display_TypeDef displayFlg);
void LcdDisplayChargeingMoney(LCD_Display_TypeDef displayFlg);
void LcdDisplayAllEdge(void);
void LcdDisplayEdge(LCD_Display_TypeDef displayFlg);
void LcdDisplayTab1Edge(LCD_Display_TypeDef displayFlg);
void LcdDisplayTab2Edge(LCD_Display_TypeDef displayFlg);
void LcdDisplayTab3Edge(LCD_Display_TypeDef displayFlg);
void LcdDisplayNum(LCD_Display_TypeDef displayFlg);
void LcdDisplayYuan(LCD_Display_TypeDef displayFlg);
void LcdDisplayRemainingTime(LCD_Display_TypeDef displayFlg);
void LcdDisplayClockDial(LCD_Display_TypeDef displayFlg);

void LcdClrData(DATA_POS_t pos);
void LcdDisplayZero(DATA_POS_t pos);
void LcdDisplayOne(DATA_POS_t pos);
void LcdDisplayTwo(DATA_POS_t pos);
void LcdDisplayThree(DATA_POS_t pos);
void LcdDisplayFour(DATA_POS_t pos);
void LcdDisplayFive(DATA_POS_t pos);
void LcdDisplaySix(DATA_POS_t pos);
void LcdDisplaySeven(DATA_POS_t pos);
void LcdDisplayEight(DATA_POS_t pos);
void LcdDisplayNine(DATA_POS_t pos);

void LcdDisplayPoint( DATA_POS_t pos, LCD_Display_TypeDef displayFlg);
void LcdDisplayCol(DATA_POS_t pos, LCD_Display_TypeDef displayFlg);

void lcdTestTask(void);
void LcdEnterInitStu(void);
void LcdDisplayAll(void);

#endif //__LCD_H__



