#ifndef _KEY_C_
#define _KEY_C_

/*********************************************************************************************************************/
#include "includes.h"   
/*********************************************************************************************************************/
#if SUPPORT_KEY
/*************************************************************************************************
������
��K1Ϊ����K1��������������������ʾ��

						--> K1|KEY_BREAK(�̰��ɿ�)
					-
K1(����)--
					-
						--> K1|KEY_LONG_START(������Լ1��)---> K1|KEY_LONG(һֱ������Լÿ300ms����������)---->K1|KEY_LONG_BREAK(�����ɿ�)
																							 ^																							 |
																							 |																							 |
																							 |-----------------------------------------------|

���ϼ���
��K1(K1=0x0001),K2(K2=0x0002)Ϊ���� ��K1��K2ͬʱ����ʱ����������Ϊ(K1<<5)|K2����0x22, ��������������������ʾ��


										--> ((K1<<5)|K2)|KEY_BREAK(�̰��ɿ�)
									-
(K1<<5)|K2(����)--
									-
										--> ((K1<<5)|K2)|KEY_LONG_START(������Լ1��)---> ((K1<<5)|K2)|KEY_LONG(һֱ������Լÿ300ms����������)---->((K1<<5)|K2)|KEY_LONG_BREAK(�����ɿ�)
																																 ^																							           |
																																 |																							           |
																																 |---------------------------------------------------------|
ע�⣺���ڸ��ϼ�����ʱ�������һ��ͬʱ��⵽˫���������ȼ�⵽�������£������ȼ�⵽K1����ʱ���Ȳ���K1�����ȵ���⵽K2Ҳ��
��ʱ���Ż��������(K1<<5)|K2����⵽˫�������˫��һֱ���£������((K1<<5)|K2)|KEY_LONG_START��((K1<<5)|K2)|KEY_LONG�����
��һ���������֣���ֹͣ���������� �ȵ�˫�����ɿ�ʱ���Ż����((K1<<5)|K2)|KEY_BREAK(�̰��ɿ�)��((K1<<5)|K2)|KEY_LONG_BREAK(�����ɿ�)
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
  
  //���������¼�
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