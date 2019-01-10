#ifndef KEY_H
#define KEY_H
/**************************************************************************************************************/
#ifdef _KEY_C_
unsigned int  TS_Key;	
unsigned int  TS_LongKeyTimer;
unsigned char  TK_State;	
#else
extern unsigned int  TS_Key;	
extern unsigned int  TS_LongKeyTimer;
extern unsigned char  TK_State;	
#endif
#define 	KEY_BREAK					0x1000     	//短按抬起
#define 	KEY_LONG					0x2000     	//长按
#define 	KEY_LONG_BREAK		0x3000			//长按抬起
#define 	KEY_LONG_START		0x4000			//长按开始   



#define K1					0x0001
#define K2					0x0002
#define K3					0x0003
#define K4					0x0004
#define K5					0x0005
#define K6					0x0006
#define K7					0x0007
#define K8					0x0008
#define K9					0x0009
#define K10					0x000a
#define K11					0x000c
#define K12					0x000b


enum 
{
	TK_STATE_RELEASE,
	TK_STATE_SINGLE_KEY_PRESS,	
	TK_STATE_DOUBLE_KEY_PRESS,
};

#define TS_LONG_START_TIME	250
#define TS_LONG_TIME	40

void TS_GetKey(void);
void TSKey_init(void);
/*********************************************************************************************************************/
#endif										
