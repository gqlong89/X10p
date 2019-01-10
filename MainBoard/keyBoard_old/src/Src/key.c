#ifndef _KEY_C_
#define _KEY_C_

/*********************************************************************************************************************/
#include "includes.h"   
/*********************************************************************************************************************/
#if SUPPORT_KEY
/*************************************************************************************************
单键：
以K1为例，K1按键产生的流程如下所示：

						--> K1|KEY_BREAK(短按松开)
					-
K1(按下)--
					-
						--> K1|KEY_LONG_START(长按大约1秒)---> K1|KEY_LONG(一直长按，约每300ms产生长按键)---->K1|KEY_LONG_BREAK(长按松开)
																							 ^																							 |
																							 |																							 |
																							 |-----------------------------------------------|

复合键：
以K1(K1=0x0001),K2(K2=0x0002)为例， 当K1，K2同时按下时，产生按键为(K1<<5)|K2，即0x22, 产生按键的流程如下所示：


										--> ((K1<<5)|K2)|KEY_BREAK(短按松开)
									-
(K1<<5)|K2(按下)--
									-
										--> ((K1<<5)|K2)|KEY_LONG_START(长按大约1秒)---> ((K1<<5)|K2)|KEY_LONG(一直长按，约每300ms产生长按键)---->((K1<<5)|K2)|KEY_LONG_BREAK(长按松开)
																																 ^																							           |
																																 |																							           |
																																 |---------------------------------------------------------|
注意：由于复合键按下时，软件不一定同时检测到双键，而是先检测到单键按下，例如先检测到K1，此时会先产生K1键，等到检测到K2也按
下时，才会产生按键(K1<<5)|K2，检测到双键后，如果双键一直按下，会产生((K1<<5)|K2)|KEY_LONG_START和((K1<<5)|K2)|KEY_LONG，如果
有一个键先松手，会停止产生按键， 等到双键都松开时，才会产生((K1<<5)|K2)|KEY_BREAK(短按松开)或((K1<<5)|K2)|KEY_LONG_BREAK(长按松开)
*************************************************************************************************/

extern void key_onclick_callback(unsigned int values);




typedef struct 
{
	unsigned char 	TogKeyNum;
	unsigned char 	TogKeyList[2];
}xdata T_TogKeyInfo;


code unsigned char TS_KEY_TAB[]=
{
	K12,K11,K10,K9,K8,K7,K6,K5,K4,K3,K2,K1
};

void TSKey_init(void)
{
	TK_State = TK_STATE_RELEASE;
	TS_Key = 0;
}


void TS_GetTogKeyInfor(T_TogKeyInfo *pKeyInfo)
{
	unsigned char  i,index;
	pKeyInfo->TogKeyNum = 0;
	index = 0;
	for(i = 0; i < KEY_CH_COUNT; i++)
	{
		if(KeysFlagSN & MASK_TAB[i])
		{
			pKeyInfo->TogKeyNum++;
			
			if(pKeyInfo->TogKeyNum <= 2)
			{
				pKeyInfo->TogKeyList[index++] = TS_KEY_TAB[i];
			}
		}
	}
}

void TS_GetKey(void)
{
	static unsigned int xdata KeyBak;	
	static bit LongFlag;
	T_TogKeyInfo KeyInfo;
	TS_Key = 0;
	TS_GetTogKeyInfor(&KeyInfo);
		
	if(TK_State == TK_STATE_RELEASE)
	{
		if(KeyInfo.TogKeyNum != 0)
		{			
			if(KeyInfo.TogKeyNum == 1)
			{
				KeyBak = (unsigned int)KeyInfo.TogKeyList[0];
				TK_State = TK_STATE_SINGLE_KEY_PRESS;				
			}
			else if(KeyInfo.TogKeyNum == 2)
			{
				KeyBak = ((unsigned int)KeyInfo.TogKeyList[0]<<5)|(unsigned int)KeyInfo.TogKeyList[1];
				TK_State = TK_STATE_DOUBLE_KEY_PRESS;
			}
			
			TS_Key = KeyBak;
			LongFlag = 0;
			TS_LongKeyTimer = TS_LONG_START_TIME;
		}
	}
	else if(TK_State == TK_STATE_SINGLE_KEY_PRESS)
	{
		if(KeyInfo.TogKeyNum == 1)
		{
			if(KeyBak == (unsigned int)KeyInfo.TogKeyList[0])
			{
				if(!TS_LongKeyTimer)
				{
					if(!LongFlag)
					{
						LongFlag = 1;
						TS_Key = KeyBak | KEY_LONG_START;
					}
					else
					{
						TS_Key = KeyBak | KEY_LONG;
					}
					TS_LongKeyTimer = TS_LONG_TIME;
				}				
			}
			else 
			{
				if(!LongFlag)
				{
					TS_Key = KeyBak | KEY_BREAK;
				}
				else
				{
					TS_Key = KeyBak | KEY_LONG_BREAK;
				}
				TK_State = TK_STATE_RELEASE;	
			}
		}
		else if(KeyInfo.TogKeyNum == 2)
		{
			KeyBak = ((unsigned int)KeyInfo.TogKeyList[0]<<5) | (unsigned int)KeyInfo.TogKeyList[1];
			TK_State = TK_STATE_DOUBLE_KEY_PRESS;
			TS_Key = KeyBak;
			LongFlag = 0;
			TS_LongKeyTimer = TS_LONG_START_TIME;
		}
		else if(KeyInfo.TogKeyNum == 0)
		{
			if(!LongFlag)
			{
				TS_Key = KeyBak | KEY_BREAK;
			}
			else
			{
				TS_Key = KeyBak | KEY_LONG_BREAK;
			}
			TK_State = TK_STATE_RELEASE;				
		}
	}
	else if(TK_State == TK_STATE_DOUBLE_KEY_PRESS)
	{
		if(KeyInfo.TogKeyNum == 2)
		{
			if(KeyBak == ((unsigned int)KeyInfo.TogKeyList[0]<<5)|(unsigned int)KeyInfo.TogKeyList[1])
			{
				if(!TS_LongKeyTimer)
				{
					if(!LongFlag)
					{
						LongFlag = 1;
						TS_Key = KeyBak | KEY_LONG_START;
					}
					else
					{
						TS_Key = KeyBak | KEY_LONG;
					}
					TS_LongKeyTimer = TS_LONG_TIME;
				}					
			}
		}
		else if(KeyInfo.TogKeyNum == 0)
		{
			if(!LongFlag)
			{
				TS_Key = KeyBak | KEY_BREAK;
			}
			else
			{
				TS_Key = KeyBak | KEY_LONG_BREAK;
			}
			TK_State = TK_STATE_RELEASE;				
		}
	}
  
  //产生按键事件
	if(TS_Key != 0)
	{
		if((TS_Key & 0xFF00) == 0)
		{
			if(TS_Key > 0x1F)//double click
			{
	
			}
			else//click down
			{
                if(TS_Key == K11){
                    TS_Key = 0;
                }
                key_onclick_callback(TS_Key);
			}
      
		}
		else if(TS_Key & KEY_BREAK)//click up
		{
			
		}
		else if(TS_Key & KEY_LONG_START)//long click
		{

		}
		else if(TS_Key & KEY_LONG)
		{
	
		}
	}
}








#endif

/*********************************************************************************************************************/
#endif