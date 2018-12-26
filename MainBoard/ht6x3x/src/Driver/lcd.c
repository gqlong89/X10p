/*lcd.c
* 2017-10-17
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "lcd.h"
#include "ui.h"
#include "server.h"
#include "rtc.h"


#define    MaxSegNum                 25





//******************************************************************
//! \brief  	Lcd_All_SegCom_IO_Config
//! \param
//! \retval
//! \note
//******************************************************************
static void Lcd_All_SegCom_IO_Config(void)
{
	//COM0~COM3
	GPIO_Cfg(HT_GPIOD, GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11, GPIO_Mode_AF1, GPIO_Input_Up, GPIO_Output_PP);
	//SEG0~SEG15
	GPIO_Cfg(HT_GPIOB, GPIO_Pin_All, GPIO_Mode_AF1, GPIO_Input_Up,GPIO_Output_PP);
	//SEG16~SEG23
	GPIO_Cfg(HT_GPIOD, GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5
			|GPIO_Pin_6|GPIO_Pin_7,GPIO_Mode_AF1, GPIO_Input_Up, GPIO_Output_PP);
	//SEG24~SEG25
	GPIO_Cfg(HT_GPIOD, GPIO_Pin_12|GPIO_Pin_13, GPIO_Mode_AF2, GPIO_Input_Up, GPIO_Output_PP);

	GPIO_Cfg(HT_GPIOG, GPIO_Pin_7, GPIO_Mode_IOOUT, GPIO_Input_Up, GPIO_Output_PP);
}

//******************************************************************
//! \brief  	HT_LCD_Init
//! \param  	LCD_InitStruct
//!             1) LCD_Bias       : LCDƫѹ����
//!             2) LCD_Com        : LCDCom Number
//!             3) LCD_CLK        : LCDɨ��Ƶ��
//!             4) LCD_ChargeMode : LCD���ģʽ
//!             5) LCD_Vrsel[3..0]: LCD�Աȶȣ���ֵԽС���Աȶ�Խ��
//! \retval
//! \note  		�û�������LCD�Ĵ���ǰӦ��ʹ��LCDģ�飬����μ�HT_CMU_ClkCtrl0Config()����
//******************************************************************
void HT_LCD_Init(LCD_InitTypeDef* LCD_InitStruct)
{
    /*  assert_param  */
    uint32_t tempreg;

    tempreg  = LCD_InitStruct->LCD_Bias;                   /*!< LCDƫѹ����              */
    tempreg |= LCD_InitStruct->LCD_Com;                    /*!< LCDCom Number            */
    tempreg |= LCD_InitStruct->LCD_CLK;                    /*!< LCDɨ��Ƶ��              */

    HT_LCD->LCDCLK = tempreg;                              /*!< ����LCDCLK�Ĵ���         */

    tempreg  = (uint32_t)LCD_InitStruct->LCD_ChargeMode;   /*!< LCD���ģʽ����          */
    tempreg |= (LCD_InitStruct->LCD_Vrsel & LCD_LCDCON_VRSEL);
                                                           /*!< LCD�Աȶ�����            */
    HT_LCD->LCDCON = tempreg;                              /*!< ����LCDCON�Ĵ���         */
}

//******************************************************************
//! \brief  	LcdTurnOnLed
//! \param
//! \retval
//! \note
//******************************************************************
void LcdTurnOnLed(void)
{
    if (0 == gChgInfo.lcdPowerStatus) {
    	GPIO_SetBits(HT_GPIOG, GPIO_Pin_7);
        gChgInfo.lcdPowerStatus = 1;
        gChgInfo.turnOnLcdTime = GetRtcCount();
    }
}

//******************************************************************
//! \brief  	LcdTurnOffLed
//! \param
//! \retval
//! \note
//******************************************************************
void LcdTurnOffLed(void)
{
	GPIO_ResetBits(HT_GPIOG, GPIO_Pin_7);
    gChgInfo.lcdPowerStatus = 0;
    CL_LOG("turn off led.\n");
}

//******************************************************************
//! \brief  	HT_LCD_Write
//! \param  	SegNum 			0-41֮��
//! \param		SegValue		��Segmentд��ֵ
//! \retval 	none
//! \note
//******************************************************************
//static void HT_LCD_Write(uint8_t SegNum, uint8_t SegValue)
//{
//    /*  assert_param  */
//    if(SegNum>MaxSegNum)
//        return;
//    HT_LCD->LCDBUF[SegNum] = SegValue;
//}


//******************************************************************
//! \brief  	HT_LCD_Read
//! \param  	SegNum 			0-41֮��
//! \retval
//! \note
//******************************************************************
static uint8_t HT_LCD_Read(uint8_t SegNum)
{
    /*  assert_param  */
    return (uint8_t)HT_LCD->LCDBUF[SegNum];
}

void LcdAllOn(void)
{
	uint8_t i=0;

	for(i=0; i<=MaxSegNum; i++){
		HT_LCD->LCDBUF[i] = 0xFF;
	}
}

void LcdAllOff(void)
{
	uint8_t i=0;

	for(i=0; i<=MaxSegNum; i++){
		HT_LCD->LCDBUF[i] = 0x00;
	}
}

void LcdDisplay(uint8_t seg, uint8_t segVal, LCD_Display_TypeDef displayFlg)
{
	uint8_t lcdBuf = 0;
	lcdBuf = HT_LCD_Read(seg);
	if (LCD_DISPLAY == displayFlg) {
		lcdBuf |= segVal;
	} else {
		lcdBuf &= ~segVal;
	}
	HT_LCD->LCDBUF[seg] = lcdBuf;
}

//��ʾ���̨���ӱ�־
void LcdDisplayBackStageConnect(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(11, SEG11_BACKSTAGE_CONNECT, displayFlg);
}
//��ʾ�ź�
void LcdDisplaySingnal(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(11, SEG11_SIGNAL, displayFlg);
}
//��ʾ���ź�
void LcdDisplayNoSingnal(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(11, SEG11_NOSIGNAL, displayFlg);
}
//��ʾ����
void LcdDisplayBlutooth(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(22, SEG22_BLUTOOTH, displayFlg);
}
//��ˢ����ɨ��
void LcdDisplayChgMethod(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(11, SEG11_CHARGE_METHOD, displayFlg);
}
//��ʾ�ɹ�
void LcdDisplaySuccess(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(2, SEG2_SUCCESS, displayFlg);
}
//��ʾʧ��
void LcdDisplayFailure(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(0, SEG0_FAILURE, displayFlg);
}
//��ʾ������1
void LcdDisplayProgressBar1(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(1, SEG1_PROGRESS_BAR1, displayFlg);
}
//��ʾ������2
void LcdDisplayProgressBar2(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(1, SEG1_PROGRESS_BAR2, displayFlg);
}
//��ʾ������3
void LcdDisplayProgressBar3(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(1, SEG1_PROGRESS_BAR3, displayFlg);
}
//��ʾ������4
void LcdDisplayProgressBar4(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(1, SEG1_PROGRESS_BAR4, displayFlg);
}
//��ʾ�����
void LcdDisplayChargeing(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(2, SEG2_SOCKET_CHARGING, displayFlg);
}
//��ʾ����
void LcdDisplaySocketIdle(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(2, SEG2_SOCKET_IDLE, displayFlg);
}
//��ʾ��Ч
void LcdDisplayInvalid(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(0, SEG0_INVALID, displayFlg);
}
//��ʾռ��
void LcdDisplaySocketOccupied(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(2, SEG2_SOCKET_OCCUPIED, displayFlg);
}
//��ʾ����
void LcdDisplaySocketFault(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(0, SEG0_SOCKET_FAULT, displayFlg);
}
//��ʾ�����
void LcdDisplayCardBalance(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(25, SEG25_CARD_BALANCE, displayFlg);
}
//��ʾ�������
void LcdDisplaySocketNum(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(25, SEG25_SOCKET_NUM, displayFlg);
}
//��ʾ�����
void LcdDisplayChargeingMoney(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(25, SEG25_CHARING_MONEY, displayFlg);
}
//��ʾ���еı�Ե
void LcdDisplayAllEdge(void)
{
	LcdDisplayEdge(LCD_DISPLAY);
	LcdDisplayTab1Edge(LCD_DISPLAY);
	LcdDisplayTab2Edge(LCD_DISPLAY);
	LcdDisplayTab3Edge(LCD_DISPLAY);
}
//��ʾ��Ե
void LcdDisplayEdge(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(24, SEG24_EDGE, displayFlg);
}
//��ʾTab1��Ե
void LcdDisplayTab1Edge(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(24, SEG24_TAB1_EDGE, displayFlg);
}
//��ʾTab2��Ե
void LcdDisplayTab2Edge(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(24, SEG24_TAB2_EDGE, displayFlg);
}
//��ʾTab3��Ե
void LcdDisplayTab3Edge(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(24, SEG24_TAB3_EDGE, displayFlg);
}
//��ʾ��
void LcdDisplayNum(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(25, SEG25_NUM, displayFlg);
}
//��ʾԪ
void LcdDisplayYuan(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(3, SEG3_YUAN, displayFlg);
}
//��ʾʣ��ʱ��
void LcdDisplayRemainingTime(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(14, SEG14_REMAINING_TIME, displayFlg);
}
//��ʾʱ�ӱ���
void LcdDisplayClockDial(LCD_Display_TypeDef displayFlg)
{
	LcdDisplay(12, SEG12_CLOCK_DIAL, displayFlg);
}

//������Ӧλ������
void LcdClrData(DATA_POS_t pos)
{
	uint8_t segValue = 0;
	switch(pos){
		case FIRST:
			HT_LCD->LCDBUF[13] = 0;
			segValue = SEG12_1E|SEG12_1F|SEG12_1G;
			LcdDisplay(12, segValue, LCD_CLEAR);
			break;

		case SECOND:
			HT_LCD->LCDBUF[15] = 0;
			segValue = SEG14_2E|SEG14_2G|SEG14_2F;
			LcdDisplay(14, segValue, LCD_CLEAR);
			break;

		case THIRD:
			HT_LCD->LCDBUF[17] = 0;
			segValue = SEG16_3E|SEG16_3G|SEG16_3F;
			LcdDisplay(16, segValue, LCD_CLEAR);
			break;

		case FOURTH:
			HT_LCD->LCDBUF[19] = 0;
			segValue = SEG18_4E|SEG18_4G|SEG18_4F;
			LcdDisplay(18, segValue, LCD_CLEAR);
			break;

		case FIFTH:
			HT_LCD->LCDBUF[21] = 0;
			segValue = SEG20_5E|SEG20_5G|SEG20_5F;
			LcdDisplay(20, segValue, LCD_CLEAR);
			break;

		case SIXTH:
			HT_LCD->LCDBUF[23] = 0;
			segValue = SEG22_6E|SEG22_6G|SEG22_6F;
			LcdDisplay(22, segValue, LCD_CLEAR);
			break;

		case SEVENTH:
			segValue = HT_LCD_Read(9);
			segValue = SEG9_7C|SEG9_7G|SEG9_7B;
			LcdDisplay(9, segValue, LCD_CLEAR);
			segValue = 0;
			segValue = SEG10_7D|SEG10_7E|SEG10_7F|SEG10_7A;
			LcdDisplay(10, segValue, LCD_CLEAR);
			break;

		case EIGHTH:
			segValue = HT_LCD_Read(7);
			segValue = SEG7_8C|SEG7_8G|SEG7_8B;
			LcdDisplay(7, segValue, LCD_CLEAR);
			segValue = 0;
			segValue = SEG8_8D|SEG8_8E|SEG8_8F|SEG8_8A;
			LcdDisplay(8, segValue, LCD_CLEAR);
			break;

		case NINTH:
			segValue = HT_LCD_Read(5);
			segValue = SEG5_9C|SEG5_9G|SEG5_9B;
			LcdDisplay(5, segValue, LCD_CLEAR);
			segValue = 0;
			segValue = SEG6_9D|SEG6_9E|SEG6_9F|SEG6_9A;
			LcdDisplay(6, segValue, LCD_CLEAR);
			break;

		case TEN:
			segValue = HT_LCD_Read(3);
			segValue = SEG3_10C|SEG3_10G|SEG3_10B;
			LcdDisplay(3, segValue, LCD_CLEAR);
			segValue = 0;
			segValue = SEG4_10D|SEG4_10E|SEG4_10F|SEG4_10A;
			LcdDisplay(4, segValue, LCD_CLEAR);
			break;
		default:
			break;
	}
}

//��ʾ����0	\A\B\C\D\E\F
void LcdDisplayZero(DATA_POS_t pos)
{
	const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1E|SEG12_1F,SEG14_2E|SEG14_2F,SEG16_3E|SEG16_3F,SEG18_4E|SEG18_4F,SEG20_5E|SEG20_5F,SEG22_6E|SEG22_6F,SEG9_7B|SEG9_7C,SEG7_8B|SEG7_8C,SEG5_9B|SEG5_9C,SEG3_10B|SEG3_10C};
    const uint8_t para2[] = {SEG13_1A|SEG13_1B|SEG13_1C|SEG13_1D,SEG15_2A|SEG15_2B|SEG15_2C|SEG15_2D,SEG17_3A|SEG17_3B|SEG17_3C|SEG17_3D,SEG19_4A|SEG19_4B|SEG19_4C|SEG19_4D,SEG21_5A|SEG21_5B|SEG21_5C|SEG21_5D,
								SEG23_6A|SEG23_6B|SEG23_6C|SEG23_6D,SEG10_7A|SEG10_7D|SEG10_7E|SEG10_7F,SEG8_8A|SEG8_8D|SEG8_8E|SEG8_8F,SEG6_9A|SEG6_9D|SEG6_9E|SEG6_9F,SEG4_10A|SEG4_10D|SEG4_10E|SEG4_10F};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}
//��ʾ����1	\B\C
void LcdDisplayOne(DATA_POS_t pos)
{
	const uint8_t para0[] = {13,15,17,19,21,23,9,7,5,3};
    const uint8_t para1[] = {SEG13_1B|SEG13_1C,SEG15_2B|SEG15_2C,SEG17_3B|SEG17_3C,SEG19_4B|SEG19_4C,SEG21_5B|SEG21_5C,SEG23_6B|SEG23_6C,SEG9_7B|SEG9_7C,SEG7_8B|SEG7_8C,SEG5_9B|SEG5_9C,SEG3_10B|SEG3_10C};
    
    uint8_t  index = para0[pos-1];
    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
    }
	
}
//��ʾ����2	\A\B\D\E\G
void LcdDisplayTwo(DATA_POS_t pos)
{
	const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1E|SEG12_1G,SEG14_2E|SEG14_2G,SEG16_3E|SEG16_3G,SEG18_4E|SEG18_4G,SEG20_5E|SEG20_5G,SEG22_6E|SEG22_6G,SEG9_7B|SEG9_7G,SEG7_8B|SEG7_8G,SEG5_9B|SEG5_9G,SEG3_10B|SEG3_10G};
    const uint8_t para2[] = {SEG13_1A|SEG13_1B|SEG13_1D,SEG15_2A|SEG15_2B|SEG15_2D,SEG17_3A|SEG17_3B|SEG17_3D,SEG19_4A|SEG19_4B|SEG19_4D,SEG21_5A|SEG21_5B|SEG21_5D,
								SEG23_6A|SEG23_6B|SEG23_6D,SEG10_7A|SEG10_7D|SEG10_7E,SEG8_8A|SEG8_8D|SEG8_8E,SEG6_9A|SEG6_9D|SEG6_9E,SEG4_10A|SEG4_10D|SEG4_10E};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}
//��ʾ����3	\A\B\C\D\G
void LcdDisplayThree(DATA_POS_t pos)
{
	const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1G,SEG14_2G,SEG16_3G,SEG18_4G,SEG20_5G,SEG22_6G,SEG9_7B|SEG9_7C|SEG9_7G,SEG7_8B|SEG7_8C|SEG7_8G,SEG5_9B|SEG5_9C|SEG5_9G,SEG3_10B|SEG3_10C|SEG3_10G};
    const uint8_t para2[] = {SEG13_1A|SEG13_1B|SEG13_1C|SEG13_1D,SEG15_2A|SEG15_2B|SEG15_2C|SEG15_2D,SEG17_3A|SEG17_3B|SEG17_3C|SEG17_3D,SEG19_4A|SEG19_4B|SEG19_4C|SEG19_4D,SEG21_5A|SEG21_5B|SEG21_5C|SEG21_5D,
								SEG23_6A|SEG23_6B|SEG23_6C|SEG23_6D,SEG10_7A|SEG10_7D,SEG8_8A|SEG8_8D,SEG6_9A|SEG6_9D,SEG4_10A|SEG4_10D};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}
//��ʾ����4	\B\C\F\G
void LcdDisplayFour(DATA_POS_t pos)
{
	LcdClrData(pos);
	switch(pos){
		case FIRST:
			HT_LCD->LCDBUF[12] = SEG12_1G|SEG12_1F|HT_LCD_Read(12);
			HT_LCD->LCDBUF[13] = SEG13_1B|SEG13_1C;;
			break;

		case SECOND:
			HT_LCD->LCDBUF[14] = SEG14_2G|SEG14_2F|HT_LCD_Read(14);
			HT_LCD->LCDBUF[15] = SEG15_2B|SEG15_2C;;
			break;

		case THIRD:
			HT_LCD->LCDBUF[16] = SEG16_3G|SEG16_3F|HT_LCD_Read(16);
			HT_LCD->LCDBUF[17] = SEG17_3B|SEG17_3C;;
			break;

		case FOURTH:
			HT_LCD->LCDBUF[18] = SEG18_4G|SEG18_4F|HT_LCD_Read(18);
			HT_LCD->LCDBUF[19] = SEG19_4B|SEG19_4C;
			break;

		case FIFTH:
			HT_LCD->LCDBUF[20] = SEG20_5G|SEG20_5F|HT_LCD_Read(20);
			HT_LCD->LCDBUF[21] = SEG21_5B|SEG21_5C;;
			break;

		case SIXTH:
			HT_LCD->LCDBUF[22] = SEG22_6G|SEG22_6F|HT_LCD_Read(22);
			HT_LCD->LCDBUF[23] = SEG23_6B|SEG23_6C;
			break;

		case SEVENTH:
			HT_LCD->LCDBUF[9] = SEG9_7B|SEG9_7C|SEG9_7G|HT_LCD_Read(9);
			HT_LCD->LCDBUF[10] = SEG10_7F;
			break;

		case EIGHTH:
			HT_LCD->LCDBUF[7] = SEG7_8B|SEG7_8C|SEG7_8G|HT_LCD_Read(7);
			HT_LCD->LCDBUF[8] = SEG8_8F;
			break;

		case NINTH:
			HT_LCD->LCDBUF[5] = SEG5_9B|SEG5_9C|SEG5_9G|HT_LCD_Read(5);
			HT_LCD->LCDBUF[6] = SEG6_9F;
			break;

		case TEN:
			HT_LCD->LCDBUF[3] = SEG3_10B|SEG3_10C|SEG3_10G|HT_LCD_Read(3);
			HT_LCD->LCDBUF[4] = SEG4_10F;
			break;
	}
}
//��ʾ����5	\A\C\D\F\G
void LcdDisplayFive(DATA_POS_t pos)
{
	const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1G|SEG12_1F,SEG14_2G|SEG14_2F,SEG16_3G|SEG16_3F,SEG18_4G|SEG18_4F,SEG20_5G|SEG20_5F,
								SEG22_6G|SEG22_6F,SEG9_7C|SEG9_7G,SEG7_8C|SEG7_8G,SEG5_9C|SEG5_9G,SEG3_10C|SEG3_10G};
    const uint8_t para2[] = {SEG13_1A|SEG13_1C|SEG13_1D,SEG15_2A|SEG15_2C|SEG15_2D,SEG17_3A|SEG17_3C|SEG17_3D,SEG19_4A|SEG19_4C|SEG19_4D,SEG21_5A|SEG21_5C|SEG21_5D,
								SEG23_6A|SEG23_6C|SEG23_6D,SEG10_7A|SEG10_7D|SEG10_7F,SEG8_8A|SEG8_8D|SEG8_8F,SEG6_9A|SEG6_9D|SEG6_9F,SEG4_10A|SEG4_10D|SEG4_10F};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}

//��ʾ����6	\A\C\D\E\F\G
void LcdDisplaySix(DATA_POS_t pos)
{
    const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1E|SEG12_1F|SEG12_1G,SEG14_2E|SEG14_2F|SEG14_2G,SEG16_3E|SEG16_3F|SEG16_3G,SEG18_4E|SEG18_4F|SEG18_4G,SEG20_5E|SEG20_5F|SEG20_5G,SEG22_6E|SEG22_6F|SEG22_6G,SEG9_7C|SEG9_7G,SEG7_8C|SEG7_8G,SEG5_9C|SEG5_9G,SEG3_10C|SEG3_10G};
    const uint8_t para2[] = {SEG13_1A|SEG13_1C|SEG13_1D,SEG15_2A|SEG15_2C|SEG15_2D,SEG17_3A|SEG17_3C|SEG17_3D,SEG19_4A|SEG19_4C|SEG19_4D,SEG21_5A|SEG21_5C|SEG21_5D,SEG23_6A|SEG23_6C|SEG23_6D,SEG10_7A|SEG10_7D|SEG10_7E|SEG10_7F,SEG8_8A|SEG8_8D|SEG8_8E|SEG8_8F,SEG6_9A|SEG6_9D|SEG6_9E|SEG6_9F,SEG4_10A|SEG4_10D|SEG4_10E|SEG4_10F};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}

//��ʾ����7	\A\B\C
void LcdDisplaySeven(DATA_POS_t pos)
{
    const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {0,0,0,0,0,0,SEG9_7B|SEG9_7C,SEG7_8B|SEG7_8C,SEG5_9B|SEG5_9C,SEG3_10B|SEG3_10C};
    const uint8_t para2[] = {SEG13_1A|SEG13_1B|SEG13_1C,SEG15_2A|SEG15_2B|SEG15_2C,SEG17_3A|SEG17_3B|SEG17_3C,SEG19_4A|SEG19_4B|SEG19_4C,SEG21_5A|SEG21_5B|SEG21_5C,
								SEG23_6A|SEG23_6B|SEG23_6C,SEG10_7A,SEG8_8A,SEG6_9A,SEG4_10A};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}
//��ʾ����8	\A\B\C\D\E\F\G
void LcdDisplayEight(DATA_POS_t pos)
{
    const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1E|SEG12_1F|SEG12_1G,SEG14_2E|SEG14_2F|SEG14_2G,SEG16_3E|SEG16_3F|SEG16_3G,SEG18_4E|SEG18_4F|SEG18_4G,SEG20_5E|SEG20_5F|SEG20_5G,
								SEG22_6E|SEG22_6F|SEG22_6G,SEG9_7B|SEG9_7C|SEG9_7G,SEG7_8B|SEG7_8C|SEG7_8G,SEG5_9B|SEG5_9C|SEG5_9G,SEG3_10B|SEG3_10C|SEG3_10G};
    const uint8_t para2[] = {SEG13_1A|SEG13_1B|SEG13_1C|SEG13_1D,SEG15_2A|SEG15_2B|SEG15_2C|SEG15_2D,SEG17_3A|SEG17_3B|SEG17_3C|SEG17_3D,SEG19_4A|SEG19_4B|SEG19_4C|SEG19_4D,SEG21_5A|SEG21_5B|SEG21_5C|SEG21_5D,
								SEG23_6A|SEG23_6B|SEG23_6C|SEG23_6D,SEG10_7A|SEG10_7D|SEG10_7E|SEG10_7F,SEG8_8A|SEG8_8D|SEG8_8E|SEG8_8F,SEG6_9A|SEG6_9D|SEG6_9E|SEG6_9F,SEG4_10A|SEG4_10D|SEG4_10E|SEG4_10F};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}
//��ʾ����9 \A\B\C\D\F\G
void LcdDisplayNine(DATA_POS_t pos)
{
    const uint8_t para0[] = {12,14,16,18,20,22,9,7,5,3};
    const uint8_t para1[] = {SEG12_1F|SEG12_1G,SEG14_2F|SEG14_2G,SEG16_3G|SEG16_3F,SEG18_4G|SEG18_4F,SEG20_5G|SEG20_5F,SEG22_6G|SEG22_6F,
								SEG9_7B|SEG9_7C|SEG9_7G,SEG9_7B|SEG9_7C|SEG9_7G,SEG7_8B|SEG7_8C|SEG7_8G,SEG5_9B|SEG5_9C|SEG5_9G,SEG3_10B|SEG3_10C|SEG3_10G};
    const uint8_t para2[] = {SEG13_1A|SEG13_1B|SEG13_1C|SEG13_1D,SEG15_2A|SEG15_2B|SEG15_2C|SEG15_2D,SEG17_3A|SEG17_3B|SEG17_3C|SEG17_3D,SEG19_4A|SEG19_4B|SEG19_4C|SEG19_4D,SEG21_5A|SEG21_5B|SEG21_5C|SEG21_5D,
								SEG23_6A|SEG23_6B|SEG23_6C|SEG23_6D,SEG10_7A|SEG10_7D|SEG10_7F,SEG8_8A|SEG8_8D|SEG8_8F,SEG6_9A|SEG6_9D|SEG6_9F,SEG4_10A|SEG4_10D|SEG4_10F};
    uint8_t  index = para0[pos-1];

    LcdClrData(pos);
    if ((FIRST <= pos) && (pos <= TEN)) {
        HT_LCD->LCDBUF[index] = HT_LCD_Read(index) | para1[pos-1];
        HT_LCD->LCDBUF[index+1] = para2[pos-1];
    }
}

////��ʾ\�������С����.
void LcdDisplayPoint( DATA_POS_t pos, LCD_Display_TypeDef displayFlg)
{
	switch(pos){
		case FIRST:
			LcdDisplay(9, SEG9_DP1, displayFlg);
			break;

		case SECOND:
			LcdDisplay(7, SEG7_DP2, displayFlg);
			break;

		case THIRD:
			LcdDisplay(5, SEG5_DP3, displayFlg);
			break;
		default:
			break;
	}
}

//��ʾ\�������ð��:
void LcdDisplayCol(DATA_POS_t pos, LCD_Display_TypeDef displayFlg)
{
	switch(pos){
		case 1:
			LcdDisplay(16, SEG16_COL1, displayFlg);
			break;
		case 2:
			LcdDisplay(20, SEG20_COL2, displayFlg);
			break;
		default:
			break;
	}
}

void LcdEnterInitStu(void)
{
	LcdTurnOnLed();
	SwitchToUi_PowerOn();
}

void LcdDisplayAll(void)
{
	LcdTurnOnLed();
	LcdAllOff();
	//vTaskDelay(2000);
	LcdDisplayBackStageConnect(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplaySingnal(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayNoSingnal(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayBlutooth(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayChgMethod(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplaySuccess(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayFailure(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayProgressBar1(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayProgressBar2(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayProgressBar3(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayProgressBar4(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayChargeing(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplaySocketIdle(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayInvalid(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplaySocketOccupied(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplaySocketFault(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayCardBalance(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplaySocketNum(LCD_DISPLAY );
	//vTaskDelay(200);
	LcdDisplayChargeingMoney(LCD_DISPLAY );
	//vTaskDelay(200);
	LcdDisplayEdge(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayTab1Edge(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayTab2Edge(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayTab3Edge(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayNum( LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayYuan(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayRemainingTime(LCD_DISPLAY);
	//vTaskDelay(200);
	LcdDisplayClockDial(LCD_DISPLAY);
	//vTaskDelay(200);
    #if 0
    for(int i=1; i<11; i++){
      LcdDisplayZero((DATA_POS_t)i);
      vTaskDelay(200);
    }
    for(int i=1; i<11; i++){
      LcdDisplayOne((DATA_POS_t)i);
      vTaskDelay(200);
    }
    for(int i=1; i<11; i++){
      LcdDisplayTwo((DATA_POS_t)i);
      vTaskDelay(200);
    }
    for(int i=1; i<11; i++){
      LcdDisplayThree((DATA_POS_t)i);
      vTaskDelay(200);
    }

    for(int i=1; i<11; i++){
      LcdDisplayFour((DATA_POS_t)i);
      vTaskDelay(200);
    }
    for(int i=1; i<11; i++){
      LcdDisplayFive((DATA_POS_t)i);
      vTaskDelay(200);
    }
    for(int i=1; i<11; i++){
      LcdDisplaySix((DATA_POS_t)i);
      vTaskDelay(200);
    }

    for(int i=1; i<11; i++){
      LcdDisplaySeven((DATA_POS_t)i);
      vTaskDelay(200);
    }
	for(int i=1; i<11; i++){
      LcdDisplayNine((DATA_POS_t)i);
      vTaskDelay(200);
    }
    #endif
    for(int i=1; i<11; i++){
      LcdDisplayEight((DATA_POS_t)i);
      //vTaskDelay(200);
    }
    for(int i=1; i<4; i++){
      LcdDisplayPoint((DATA_POS_t)i, LCD_DISPLAY);
      //vTaskDelay(200);
    }
    for(int i=1; i<3; i++){
      LcdDisplayCol((DATA_POS_t)i, LCD_DISPLAY);
      //vTaskDelay(200);
    }
}


//******************************************************************
//! \brief  	Lcd_Init
//! \param  	none
//! \retval
//! \note
//******************************************************************
void Lcd_Init(void)
{
	LCD_InitTypeDef LCD_Init;

	//IO��ʼ��
	Lcd_All_SegCom_IO_Config();
	//ʱ��ʹ��
	HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_LCDEN, ENABLE);

	// 1/4bias, 4COM, Ffrm=64Hz
	// big current charge mode, 99.1%VLCD
//	LCD_Init.LCD_Com=LCD_4COM;
//	LCD_Init.LCD_Bias=LCDBias_OneThird;
//	LCD_Init.LCD_ChargeMode=LCDCharge_HighCurrent;
//	LCD_Init.LCD_CLK=LCDCLK_Div2;
//	LCD_Init.LCD_Vrsel=VRsel_OneThree_P44;
	LCD_Init.LCD_Com=LCD_4COM;
	LCD_Init.LCD_Bias=LCDBias_OneFourth;
	LCD_Init.LCD_ChargeMode=LCDCharge_HighCurrent;
	LCD_Init.LCD_CLK=LCDCLK_Div2;
	LCD_Init.LCD_Vrsel=VRsel_OneFourth_P99;

	HT_LCD_Init(&LCD_Init);

	LcdAllOn();

    CL_LOG("init ok.\n");
}


