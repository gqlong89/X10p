/*
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
0.000002657
0.0000252

*/
#include "includes.h"
#include "emu.h"
#include "usart.h"
#include "simuart.h"
#include "outNetProto.h"
#include "rtc.h"
#include "server.h"
#include "flash.h"
#include "gun.h"
#include "relayCtrlTask.h"


EMU_CAL EmuCalation;
uint8_t EMUIFIFlag = 0;//�Ĵ����Ƿ��ʼ��


static int SaveEmuCalation(EMU_CAL *emu);

float invMatrixA[12*12];//�����

float sampleVol[12];	//��ѹ�ɼ�ԭʼֵ

float sampleCur[12];	//�����ɼ���ԭʼֵ
float checkCur[12];		//У׼��ĵ���ֵ


float samplePower[12];	//���ʲɼ�ԭʼֵ
float checkPower[12];	//�����Ĺ���ֵ

float powerF[12];		//��������

short direction = 1;//��������




EMU_CTRL_STR  gEmuCtrl[GUN_NUM_MAX+1];
GUN_STATUS_STR gun[13];//������һ���ռ� ����ȫ��1~12��Ч  0��Ч
uint8_t readerror[7];
EMU_STATIC_STR gEmuStatic[6];
uint8_t  gEmuResetFlag = 0;  //����1��ʼ��ʾǹͷ1�����Ϊ1����ʾ��Ҫ�������ò���
                                 //0, 1, 2, 3, 4, 5, 6, 7, 8, 9
const uint8_t gBuffToEmuIndex[] = {0, 0, 0, 1, 2, 3, 4, 6, 0, 5};
//                        3     4     5     6     9     7
const char EMUID[7] = {99,EMU_1,EMU_2,EMU_3,EMU_4,EMU_5,EMU_6};//��1��ʼ

const EMU_INFO EMUDATA[10] = {
//0.
{.setupconfig.EMUCFG_values =8,
},

//1.
{.setupconfig.EMUCFG_values =8,
},

//2.
{.setupconfig.EMUCFG_values =8,
},

//3.
{   .setupconfig.EMUCFG_values                  =8,
    .setupconfig.ADCCON_value                   =0x3C, //16  //����ͨ��1��ͨ��2����4������
    .setupconfig.ANAEN_value                    = 0x07, //2·����ͨ����1·��ѹͨ����
    .setupconfig.I2Gain_value                   = 0,
    .setupconfig.ModuleEn_value                 = 0x7E,
    .setupconfig.GP1_value                      =0x186,		//small
    .setupconfig.GQ1_value                      = 0x186,
    .setupconfig.GS1_value                      = 0x186,
    .setupconfig.GPhs1_value                    = 0xF6B8,
    .setupconfig.GP2_value                      = 0x2C2,
    .setupconfig.GQ2_value                      = 0x2C2,
    .setupconfig.GS2_value                      = 0x2C2,
    .setupconfig.GPhs2_value                    = 0xF6B8,
    .setupconfig.HFConst_value                  = 208,
    .setupconfig.I1RMSOFFSET                    = 0,
    .setupconfig.I2RMSOFFSET                    = 0,
    .setupconfig.P1OffsetL_vlue                 = 0,
    .setupconfig.P1OffsetH_value                = 0,
    .setupconfig.P2OffsetL_vlue                 = 0,
    .setupconfig.P2OffsetH_value                = 0,
    .setupconfig.PQStart_value                  = 0x40,
    .setupconfig.K_cur                          =  0.00000296288,
    .setupconfig.k_power                        = 0.002518601,
    .setupconfig.k_vol                          = 0.000103798,
},

//4.
{   .setupconfig.EMUCFG_values                  =8,        //��2·У׼
    .setupconfig.ADCCON_value                   = 0x3C,   //����ͨ��1��ͨ��2����4������
    .setupconfig.ANAEN_value                    = 0x07, //2·����ͨ����1·��ѹͨ����
    .setupconfig.ModuleEn_value                 = 0x7E,
    .setupconfig.I2Gain_value                   = 0,
    .setupconfig.GP1_value                      = 0x453,
    .setupconfig.GQ1_value                      = 0x453,
    .setupconfig.GS1_value                      = 0x453,
    .setupconfig.GPhs1_value                    = 0xF6C8,
    .setupconfig.GP2_value                      = 0xFF8A,
    .setupconfig.GQ2_value                      = 0xFF8A,
    .setupconfig.GS2_value                      = 0xFF8A,
    .setupconfig.GPhs2_value                    = 0xF6C8,
    .setupconfig.HFConst_value                  = 208,
    .setupconfig.I1RMSOFFSET                    = 0,
    .setupconfig.I2RMSOFFSET                    = 0,
    .setupconfig.P1OffsetL_vlue                 = 0,
    .setupconfig.P1OffsetH_value                = 0,
    .setupconfig.P2OffsetL_vlue                 = 0,
    .setupconfig.P2OffsetH_value                = 0,
    .setupconfig.PQStart_value                  = 0x40,
    .setupconfig.K_cur                          =  0.00000298661,
    .setupconfig.k_power                        = 0.002518601,
    .setupconfig.k_vol                          = 0.000104233,
},

//5.
{   .setupconfig.EMUCFG_values                  = 8,        //��2·У׼
    .setupconfig.ADCCON_value                   = 0x3C,   //����ͨ��1��ͨ��2����4������
    .setupconfig.ANAEN_value                    = 0x07, //2·����ͨ����1·��ѹͨ����
    .setupconfig.ModuleEn_value                 = 0x7E,
    .setupconfig.I2Gain_value                   = 0,
    .setupconfig.GP1_value                      = 0x29C,
    .setupconfig.GQ1_value                      = 0x29C,
    .setupconfig.GS1_value                      = 0x29C,
    .setupconfig.GPhs1_value                    = 0xF6C8,
    .setupconfig.GP2_value                      = 0xFE41,
    .setupconfig.GQ2_value                      = 0xFE41,
    .setupconfig.GS2_value                      = 0xFE41,
    .setupconfig.GPhs2_value                    = 0xF6C8,
    .setupconfig.HFConst_value                  = 208,
    .setupconfig.I1RMSOFFSET                    = 0,
    .setupconfig.I2RMSOFFSET                    = 0,
    .setupconfig.P1OffsetL_vlue                 = 0,
    .setupconfig.P1OffsetH_value                = 0,
    .setupconfig.P2OffsetL_vlue                 = 0,
    .setupconfig.P2OffsetH_value                = 0,
    .setupconfig.PQStart_value                  = 0x40,
    .setupconfig.K_cur                          =  0.00000294556,
    .setupconfig.k_power                        = 0.002518601,
    .setupconfig.k_vol                          = 0.00010431,
},

//6.
{   .setupconfig.EMUCFG_values                  = 8 ,        //��2·У׼
    .setupconfig.ADCCON_value                   = 0x3C,   //����ͨ��1��ͨ��2����4������
    .setupconfig.ANAEN_value                    = 0x07, //2·����ͨ����1·��ѹͨ����
    .setupconfig.ModuleEn_value                 = 0x7E,
    .setupconfig.I2Gain_value                   = 0,
    .setupconfig.GP1_value                      = 0x325,
    .setupconfig.GQ1_value                      = 0x325,
    .setupconfig.GS1_value                      = 0x325,
    .setupconfig.GPhs1_value                    = 0xF6C8,
    .setupconfig.GP2_value                      = 0xFB74,
    .setupconfig.GQ2_value                      = 0xFB74,
    .setupconfig.GS2_value                      = 0xFB74,
    .setupconfig.GPhs2_value                    = 0xF6C8,
    .setupconfig.HFConst_value                  = 208,
    .setupconfig.P1OffsetL_vlue                 = 0x1249,
    .setupconfig.P1OffsetH_value                = 0x1249,
    .setupconfig.P2OffsetL_vlue                 = 0x1249,
    .setupconfig.P2OffsetH_value                = 0,
    .setupconfig.PQStart_value                  = 0x40,
    .setupconfig.K_cur                          =  0.00000296854,
    .setupconfig.k_power                        = 0.002518601,
    .setupconfig.k_vol                          = 0.000103967,
},

//7.
{   .setupconfig.EMUCFG_values                  = 8,        //��2·У׼
    .setupconfig.ADCCON_value                   = 0x3C,   //����ͨ��1��ͨ��2����4������
    .setupconfig.ANAEN_value                    = 0x07, //2·����ͨ����1·��ѹͨ����
    .setupconfig.ModuleEn_value                 = 0x7E,
    .setupconfig.I2Gain_value                   = 0,
    .setupconfig.GP1_value                      = 0xFF5C,
    .setupconfig.GQ1_value                      = 0xFF5C,
    .setupconfig.GS1_value                      = 0xFF5C,
    .setupconfig.GPhs1_value                    = 0xF6C8,
    .setupconfig.GP2_value                      =0xff22,
    .setupconfig.GQ2_value                      = 0xff22,
    .setupconfig.GS2_value                      = 0xff22,
    .setupconfig.GPhs2_value                    = 0xF6C8,
    .setupconfig.HFConst_value                  = 208,
    .setupconfig.P1OffsetL_vlue                 = 0,
    .setupconfig.P1OffsetH_value                = 0,
    .setupconfig.P2OffsetL_vlue                 = 0,
    .setupconfig.P2OffsetH_value                = 0,
    .setupconfig.PQStart_value                  = 0x40,
    .setupconfig.K_cur                          =  0.00000287378,
    .setupconfig.k_power                        = 0.002518601,
    .setupconfig.k_vol                          = 0.000104263,
},

//8.
{.setupconfig.EMUCFG_values =8,
},

//9.
{   .setupconfig.EMUCFG_values                  = 8 ,        //��2·У׼
    .setupconfig.ADCCON_value                   = 0x3C,   //����ͨ��1��ͨ��2����4������
    .setupconfig.ANAEN_value                    = 0x07, //2·����ͨ����1·��ѹͨ����
    .setupconfig.ModuleEn_value                 = 0x7E,
    .setupconfig.I2Gain_value                   = 0,
    .setupconfig.GP1_value                      = 0x1F3,
    .setupconfig.GQ1_value                      = 0x1F3,
    .setupconfig.GS1_value                      = 0x1F3,
    .setupconfig.GPhs1_value                    = 0xF6C8,
    .setupconfig.GP2_value                      = 0xFCEF,
    .setupconfig.GQ2_value                      = 0xFCEF,
    .setupconfig.GS2_value                      = 0xFCEF,
    .setupconfig.GPhs2_value                    = 0xF6C8,
    .setupconfig.HFConst_value                  = 208,
    .setupconfig.P1OffsetL_vlue                 = 0,
    .setupconfig.P1OffsetH_value                = 0,
    .setupconfig.P2OffsetL_vlue                 = 0,
    .setupconfig.P2OffsetH_value                = 0,
    .setupconfig.PQStart_value                  = 0x40,
    .setupconfig.K_cur                          =  0.00000292901,
    .setupconfig.k_power                        = 0.002518601,
    .setupconfig.k_vol                          = 0.000104388,
},
};



//Ĭ�Ͼ���ϵ��
const float defaultMA[12*12]={
	1.000000,-0.006155,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,
	0.003230,1.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,
	-0.002858,-0.002858,1.000000,0.005002,0.002144,-0.002858,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,
	-0.003617,-0.003255,-0.001447,1.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,
	-0.002898,-0.002898,0.004346,0.003622,1.000000,0.006519,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,
	-0.002914,-0.002914,0.006193,0.003643,0.004736,1.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,
	-0.002838,-0.002838,0.003193,0.003193,-0.002483,0.003547,1.000000,0.006385,-0.001419,-0.003193,-0.001064,0.000000,
	-0.002886,-0.002886,0.003608,0.003247,-0.002886,0.003968,0.004329,1.000000,0.000000,-0.001443,0.000000,0.000000,
	-0.002522,-0.002882,0.003242,0.003242,-0.002522,0.003242,-0.003602,0.003963,1.000000,0.000000,0.000000,0.000000,
	-0.002562,-0.002928,0.003294,0.003294,-0.002562,0.003294,-0.006223,0.004392,-0.002928,1.000000,-0.001464,0.001830,
	-0.002580,-0.002949,0.003317,0.002949,-0.002212,0.003317,-0.002580,0.003686,0.003686,0.003317,1.000000,-0.002949,
	-0.002590,-0.002960,0.003330,0.003330,-0.002590,0.003330,-0.002960,0.003700,0.005179,0.003700,-0.003700,1.000000
};


//����ϵ���������ֵ
void matprod(const float AA[144], const float sample[12], float invout[12])
{
    int i0;
    int i1;

    for (i0 = 0; i0 < 12; i0++) 
	{
        invout[i0] = 0.0F;
        for (i1 = 0; i1 < 12; i1++) 
		{
            invout[i0] += AA[i0 + 12 * i1] * sample[i1];
        }
    }
}


//�������
void invNxN(const float x[144], float y[144])
{
    int i1;
    float b_x[144];
    int j;
    signed char ipiv[12];
    int c;
    int jBcol;
    int k;
    signed char p[12];
    int ix;
    float smax;
    float s;
    int kAcol;
    int i;

	for (i1 = 0; i1 < 144; i1++) 
	{
		y[i1] = 0.0F;
		b_x[i1] = x[i1];
	}

	for (i1 = 0; i1 < 12; i1++) 
	{
		ipiv[i1] = (signed char)(1 + i1);
	}

	for (j = 0; j < 11; j++) 
  	{
		c = j * 13;
		jBcol = 0;
		ix = c;
		smax = (float)fabs(b_x[c]);
		for (k = 2; k <= 12 - j; k++) {
      	ix++;
      	s = (float)fabs(b_x[ix]);
      	if (s > smax) 
		{
        	jBcol = k - 1;
        	smax = s;
      	}
	}

	if (b_x[c + jBcol] != 0.0F) 
	{
		if (jBcol != 0) 
		{
        	ipiv[j] = (signed char)((j + jBcol) + 1);
        	ix = j;
        	jBcol += j;
        	for (k = 0; k < 12; k++) 
			{
	          	smax = b_x[ix];
	         	b_x[ix] = b_x[jBcol];
	          	b_x[jBcol] = smax;
	          	ix += 12;
	          	jBcol += 12;
        	}
      	}

      	i1 = (c - j) + 12;
      	for (i = c + 1; i + 1 <= i1; i++) 
		{
        	b_x[i] /= b_x[c];
      	}
	}

    jBcol = c;
    kAcol = c + 12;
    for (i = 1; i <= 11 - j; i++) 
	{
		smax = b_x[kAcol];
		if (b_x[kAcol] != 0.0F) 
		{
        	ix = c + 1;
        	i1 = (jBcol - j) + 24;
        	for (k = 13 + jBcol; k + 1 <= i1; k++) 
			{
          		b_x[k] += b_x[ix] * -smax;
          		ix++;
        	}
      	}

      	kAcol += 12;
      	jBcol += 12;
    	}
	}

  for (i1 = 0; i1 < 12; i1++) {
    p[i1] = (signed char)(1 + i1);
  }

  for (k = 0; k < 11; k++) {
    if (ipiv[k] > 1 + k) {
      jBcol = p[ipiv[k] - 1];
      p[ipiv[k] - 1] = p[k];
      p[k] = (signed char)jBcol;
    }
  }

  for (k = 0; k < 12; k++) {
    c = p[k] - 1;
    y[k + 12 * (p[k] - 1)] = 1.0F;
    for (j = k; j + 1 < 13; j++) {
      if (y[j + 12 * c] != 0.0F) {
        for (i = j + 1; i + 1 < 13; i++) {
          y[i + 12 * c] -= y[j + 12 * c] * b_x[i + 12 * j];
        }
      }
    }
  }

  for (j = 0; j < 12; j++) {
    jBcol = 12 * j;
    for (k = 11; k >= 0; k += -1) {
      kAcol = 12 * k;
      if (y[k + jBcol] != 0.0F) {
        y[k + jBcol] /= b_x[k + kAcol];
        for (i = 0; i + 1 <= k; i++) {
          y[i + jBcol] -= y[k + jBcol] * b_x[i + kAcol];
        }
      }
    }
  }
}


//���¾���ϵ��
void updateMatrix(int id,float current,float sample[])
{
    int k = 0;
    float coeff[12];
    for(k = 0;k <= 11;k++){
        coeff[k] = sample[k]/current;
    }

    for(k = 0; k <= 11; k++)
    {
        EmuCalation.matrixA[12*id+k] = coeff[k];
    }
}


//����
void filterValue(float out[])
{
    for(int i = 0; i < 12 ; i++){
        if(fabs(out[i]) <= VPT_VALUE){
            out[i] = 0;
        }
    }
}


int SendData_HT7017(int no, uint8_t *Data, uint16_t len)
{
    return UsartSend(no, Data, len);
}


//д�Ĵ���
int HT7017_Write(int no, uint8_t RegAdd, uint16_t Data)
{
    int i;
    uint8_t index = gBuffToEmuIndex[no];
	uint8_t checksum = 0;
	int num;
	uint8_t Buf[8] = {0};
	uint8_t rxbuff[4] = {0};
	int retry = 0;

	Buf[0] = FIXED_HEAD;
	Buf[1] = RegAdd | 0x80;
	Buf[2] = (uint8_t)((Data>>8)&0xff);//�ȷ����ֽ�
	Buf[3] = (uint8_t)(Data&0xff);     //�ٷ����ֽ�

	for (i=0; i<4; i++) 
    {
	    checksum += Buf[i];
	}
	Buf[4] = ~checksum;  //У���ȡ��

	while(++retry<RETRYTIME) 
    {
		FIFO_S_Flush(&(gUartPortAddr[no].rxBuffCtrl));
        gSimUartCtrl[0].recvStat = COM_STOP_BIT;
		gSimUartCtrl[2].recvStat = COM_STOP_BIT;
		SendData_HT7017(no, Buf, 5);
		vTaskDelay(15);//����д���յ�������
		num = FIFO_S_CountUsed(&(gUartPortAddr[no].rxBuffCtrl));
		if (num >= 1) 
        {
            for (i=0; i<num; i++) 
            {
    			UsartGetOneData(no,rxbuff);
    			if(*rxbuff == 0x54) 
                {
    				gEmuStatic[index-1].writeOk++;
					readerror[index]=0;
    				return 0;
    			}
                else
                {
    				//CL_LOG("send error!!!retry!!!!\n");
    				gEmuStatic[index-1].writeSend0Err++;
    			}
            }
		}
	}

	CL_LOG("tx err,no=%d.\n",no);
	if (readerror[index]<250) 
    {
        readerror[index]++;
	}
    gEmuStatic[index-1].writeSend1Err++;
	return CL_FAIL;
}


int  check7017_sum(unsigned char *head,unsigned char *rec)//���ֽڶ�ȡchecksum
{
	int correct;
	int correct2;

	rec[3] &= 0xFF;
	correct = head[0]+head[1]+rec[0]+rec[1]+rec[2];
	correct2 = ~correct;
	correct2 &= 0xFF;
	if(correct2 == rec[3])
		return 1;
	else
		return 0;
}


//���Ĵ���
int HT7017_Read(int no,uint8_t RegAdd,uint8_t *Data)
{
    uint8_t index = gBuffToEmuIndex[no];
	uint8_t retry = 0;
    uint8_t Buf[2];
    uint8_t rxbuff[8] = {0};
    int num;
	int ret;

    Buf[0] = FIXED_HEAD;
    Buf[1] = RegAdd & 0x7F;
    //��������---���жϽ��
	while(retry < RETRYTIME) 
	{
		retry++;
		FIFO_S_Flush(&(gUartPortAddr[no].rxBuffCtrl));//׼�����ܴ��ڶ���
		gSimUartCtrl[0].recvStat = COM_STOP_BIT;
		gSimUartCtrl[2].recvStat = COM_STOP_BIT;
		SendData_HT7017(no,Buf,2);//�����ֽڵ�ַ
 		vTaskDelay(15);
		num = FIFO_S_CountUsed(&(gUartPortAddr[no].rxBuffCtrl));
		if(num >= 4) 
		{
			for(int i = 0; i < 4; i++) 
			{
				UsartGetOneData(no,rxbuff + i);
			}

			ret = check7017_sum(Buf,rxbuff);
			if(ret) 
			{//У��ͨ�� ��������      ������������
				memcpy(Data,rxbuff,4);
                gEmuStatic[index-1].readOk++;
				readerror[index]=0;
				return CL_OK;
			}
			else 
			{ //У��ʧ�� �ظ�����
				//CL_LOG("rece error!!retry!!!\n");
				gEmuStatic[index-1].readCheckErr++;
			}
		}
	}
	
	//���ν���ʧ��  ������ݲ�����
	CL_LOG("rx fail, no = %d.\n", no);
    gEmuStatic[index-1].readRecv1Err++;
	if (readerror[index] < 250) 
	{
        readerror[index]++;
	}
	return CL_FAIL;
}


int readVol(int no, int *vol)//0.1v
{
	uint8_t ChipID[4];
	float f;
	uint8_t temp[3];

	if(HT7017_Read(no,EMU_RMSU,ChipID)==CL_FAIL)
		return CL_FAIL;

	*vol = 0;
	temp[0] = ChipID[2];
	temp[1] = ChipID[1];
	temp[2] = ChipID[0];

	memcpy((char *)vol,temp,3);
	f = EMUDATA[no].setupconfig.k_vol * (*vol)*10;
	*vol = (int)(f+0.5);
	return CL_OK;
}


int readCur(int no,int channel, int *cur )//1mA
{
	float f;
	uint8_t ChipID[4];
	uint8_t temp[3];

	if(channel==1)
	{
		if(HT7017_Read(no,EMU_RMSI1,ChipID)==CL_FAIL)
			return CL_FAIL;
	}
	else
	{
		if(HT7017_Read(no,EMU_RMSI2,ChipID)==CL_FAIL)
			return CL_FAIL;
	}

	*cur = 0;
	temp[0] = ChipID[2];
	temp[1] = ChipID[1];
	temp[2] = ChipID[0];
	memcpy((char *)cur,temp,3);
	f = EMUDATA[no].setupconfig.K_cur * (*cur)*1000; // ��λ1mA
	*cur = (int)(f+0.5);   //
	return CL_OK;
}


int readPow(int no,int channel,int *pow )//0.1w
{
	float f;
	uint8_t ChipID[4];
	uint8_t temp[3];

	if(channel==1)
	{
		if(HT7017_Read(no,EMU_PowerP1,ChipID)==CL_FAIL)
			return CL_FAIL;
	}
	else
	{
		if(HT7017_Read(no,EMU_PowerP2,ChipID)==CL_FAIL)
			return CL_FAIL;
	}

	*pow = 0;
	temp[0] = ChipID[2];
	temp[1] = ChipID[1];
	temp[2] = ChipID[0];

	memcpy((char *)pow,temp,3);

	if(*pow>0x800000)
	{
		*pow = 0x1000000- *pow;
		direction = -1;
	}else{
		direction = 1;
	}

    f =(*pow) *  EMUDATA[no].setupconfig.k_power*10;
	*pow = (int)(f+0.5);
	return CL_OK;
}


int writecalpara(int no)
{
    int ret;

//	CL_LOG("nowwwwwwwwwww=%d.\n", no);
    ret  = HT7017_Write(no,VAR_WPREG,0xBC);
    ret |= HT7017_Write(no,VAR_EMUCFG,EMUDATA[no].setupconfig.EMUCFG_values);
    ret |= HT7017_Write(no,VAR_ANAEN,EMUDATA[no].setupconfig.ANAEN_value);
    ret |= HT7017_Write(no,VAR_ModuleEn,EMUDATA[no].setupconfig.ModuleEn_value);
    //ʹ��д50H~7CHУ������Ĵ���
    ret |= HT7017_Write(no,VAR_WPREG,0xA6);
    ret |= HT7017_Write(no,VAR_I2Gain,EMUDATA[no].setupconfig.I2Gain_value);
    ret |= HT7017_Write(no,VAR_GP1,EMUDATA[no].setupconfig.GP1_value);
    ret |= HT7017_Write(no,VAR_GQ1,EMUDATA[no].setupconfig.GQ1_value);
    ret |= HT7017_Write(no,VAR_GS1,EMUDATA[no].setupconfig.GS1_value);
    ret |= HT7017_Write(no,VAR_GP2,EMUDATA[no].setupconfig.GP2_value);
    ret |= HT7017_Write(no,VAR_GQ2,EMUDATA[no].setupconfig.GQ2_value);
    ret |= HT7017_Write(no,VAR_GS2,EMUDATA[no].setupconfig.GS2_value);
    ret |= HT7017_Write(no,VAR_GPhs1,EMUDATA[no].setupconfig.GPhs1_value);
    ret |= HT7017_Write(no,VAR_GPhs2,EMUDATA[no].setupconfig.GPhs2_value);
    ret |= HT7017_Write(no,VAR_ADCCON,EMUDATA[no].setupconfig.ADCCON_value);
    ret |= HT7017_Write(no,VAR_PQStart,EMUDATA[no].setupconfig.PQStart_value);
    ret |= HT7017_Write(no,VAR_HFConst,EMUDATA[no].setupconfig.HFConst_value);
    ret |= HT7017_Write(no,VAR_I1RMSOFFSET,EMUDATA[no].setupconfig.I1RMSOFFSET);
    ret |= HT7017_Write(no,VAR_I2RMSOFFSET,EMUDATA[no].setupconfig.I2RMSOFFSET);
    ret |= HT7017_Write(no,VAR_P1OFFSETH,EMUDATA[no].setupconfig.P1OffsetH_value);
    ret |= HT7017_Write(no,VAR_P1OFFSETL,EMUDATA[no].setupconfig.P1OffsetL_vlue);
    ret |= HT7017_Write(no,VAR_P2OFFSETL,EMUDATA[no].setupconfig.P2OffsetL_vlue);
    ret |= HT7017_Write(no,VAR_P2OFFSETH,EMUDATA[no].setupconfig.P2OffsetH_value);

    return ret;
}


int ReadCheckSum(int no, uint32_t *pSum)
{
	uint8_t data[4];

	if (HT7017_Read(no, EMU_SUMChecksum, data) == CL_OK)
	{
		*pSum = (uint32_t)((data[0] << 16)|(data[1] << 8) | data[2]);
        return CL_OK;
	}
	return CL_FAIL;
}


int WritecalparaByGunId(uint8_t gunId)
{
    uint8_t  chipIndex = (gunId + 1) >> 1;

    gEmuResetFlag |= (1 << chipIndex);
	
    return CL_OK;
}


////У׼������֤����
//void checkCalreg(int no)
//{
//	uint8_t ChipID[16][4];

//	HT7017_Read(no, VAR_WPREG, ChipID[0]);
//	HT7017_Read(no, VAR_EMUCFG, ChipID[1]);
//	HT7017_Read(no, VAR_ANAEN,ChipID[2]);
//	HT7017_Read(no, VAR_ModuleEn, ChipID[3]);
//	HT7017_Read(no, VAR_GP1, ChipID[4]);
//	HT7017_Read(no, VAR_GQ1, ChipID[5]);
//	HT7017_Read(no, VAR_GS1, ChipID[6]);
//	HT7017_Read(no, VAR_GPhs1, ChipID[7]);
//	HT7017_Read(no, VAR_GQ2, ChipID[8]);
//	HT7017_Read(no, VAR_GS2, ChipID[9]);
//	HT7017_Read(no, VAR_GP2, ChipID[10]);
//	HT7017_Read(no, VAR_GPhs2, ChipID[11]);
//	HT7017_Read(no, VAR_I1RMSOFFSET, ChipID[12]);
//	HT7017_Read(no, VAR_I2RMSOFFSET, ChipID[13]);
//	HT7017_Read(no, VAR_I2Gain, ChipID[14]);
//	HT7017_Read(no, VAR_CHK, ChipID[15]);
//}


//gunId 1~12
int GetGunStatus(int gunId, GUN_STATUS_STR *pGunStatus)
{
    #if 0
    uint8_t  other = (gunId & 1) ? (gunId + 1) : (gunId - 1);
    uint8_t  startGun = (gunId & 1) ? (gunId + 2) : (gunId + 1);
    uint16_t val;

    val = gun[other].power / 150;
    for (; startGun<=GUN_NUM_MAX; startGun++) {
        val += gun[startGun].power / 200;
    }
	memcpy(pGunStatus, &gun[gunId], sizeof(GUN_STATUS_STR));
    if (val <= pGunStatus->power) {
        pGunStatus->power -= val;
    }
    #else
    memcpy(pGunStatus, &gun[gunId], sizeof(GUN_STATUS_STR));
    #endif
    return CL_OK;
}


int readAlldata(void)
{
	int data;
	float fs;
	float Voltage;
	int i = 0;
    EMU_CTRL_STR *pEmu = NULL;

	for (int j = 1; j < 7; j++) 
	{
		//�ȶ��ڶ�·���ٶȵ�һ·
		if(j == 1)
		{
			i = 2;
		}
		else if( j == 2)
		{
			i = 1;
		}
		else
		{
			i = j;
		}

        if (CL_OK == readVol(EMUID[i], &data)) 
		{  //. 1 2  2 3 4   3 5 6   4 7 8   5 9 10   6 11 12
			sampleVol[(i<<1)-1] = data;
			sampleVol[(i<<1)-2] = data;
			if(i == 2)
			{
				Voltage = data;
			}
        }

		//ż��·
		if (CL_OK == readPow(EMUID[i],1, &data)) 
		{
		    samplePower[(i << 1) - 1] = data;
        }

        if (CL_OK == readCur(EMUID[i],1, &data)) 
		{
			sampleCur[(i<<1)-1] = (float)data*direction;
        }

		//��������
		fs = (sampleVol[(i<<1)-1] * sampleCur[(i<<1)-1]);
		if(fs != 0)
		{
			powerF[(i<<1)-1] = samplePower[(i<<1)-1]/ fs;
			if(powerF[(i<<1)-1] > 1)
			{
				powerF[(i<<1)-1] = 1;
			}
		}
		else
		{
			powerF[(i<<1)-1] = 0;
		}

		//����·
		if (CL_OK == readPow(EMUID[i],2, &data)) 
		{
		    samplePower[(i<<1)-2] = data;
        }

        if (CL_OK == readCur(EMUID[i],2, &data)) 
		{
			sampleCur[(i<<1)-2] = (float)data*direction*(-1);
        }

		//��������
		fs = (sampleVol[(i<<1)-2] * sampleCur[(i<<1)-2]);
		if(fs != 0)
		{
			powerF[(i<<1)-2] = samplePower[(i<<1)-2]/ fs;
			if(powerF[(i<<1)-2] > 1)
			{
				powerF[(i<<1)-2] = 1;
			}
		}
		else
		{
			powerF[(i<<1)-2] = 0;
			CL_LOG("��������Ϊ0.\n");
		}

		data = readerror[i];
        if ((5<data) && (data<10)) 
		{
            CL_LOG("reinit uart,i=%d.\n",i);
            gUartInitFun[EMUID[i]](); //���³�ʼ������
        }
		else if (data>10) 
		{
			setbit(gun[2*i-1].status,0);
			setbit(gun[2*i].status,0);
		}
		else
		{
			clrbit(gun[2*i].status,0);
			clrbit(gun[2*i-1].status,0);
		}
	}

	//���Ź���
	matprod(invMatrixA,sampleCur,checkCur);

	//��ֵ
	filterValue(checkCur);

	//���ʼ���  �������͹��ʸ�ֵ��GUN_STATUS_STR�ṹ����
	for(i = 0; i<12; i++) 
	{
		checkPower[i] = checkCur[i] * Voltage * powerF[i];
        gun[i+1].power = checkPower[i];
        gun[i+1].voltage = Voltage;
        gun[i+1].current = checkCur[i];
	}

	for (i=1; i<13; i++) 
	{//��ѹ �����ж�  ��ѹ���� ����
	    pEmu = &gEmuCtrl[i];
		if (gun[i].voltage > 2600) 
		{
            CL_LOG("v=%d,err.\n",gun[i].voltage);
			if (4 <= ++pEmu->overvolcount) 
			{
                setbit(gun[i].status,2);
                CL_LOG("v over,err.\n");
            }
		}
		else
		{
			pEmu->overvolcount = 0;
            clrbit(gun[i].status, 2);
		}

		if (gun[i].current > 5000) 
		{
            CL_LOG("c=%d,err.\n",gun[i].current);
            if (1 <= ++pEmu->overcurcount) 
			{
                CL_LOG("c over,err.\n");
			    setbit(gun[i].status,1);
            }
		}
		else
		{
			pEmu->overcurcount = 0;
            clrbit(gun[i].status,1);
		}
	}

	return 0;
}


//void ShowEmuStatic(EMU_STATIC_STR *pEmuStatic)
//{
//    CL_LOG("writeSend0Err =%d.\n",pEmuStatic->writeSend0Err);
//    CL_LOG("writeSend1Err =%d.\n",pEmuStatic->writeSend1Err);
//    CL_LOG("writeOk       =%d.\n",pEmuStatic->writeOk);
//    CL_LOG("readCheckErr  =%d.\n",pEmuStatic->readCheckErr);
//    CL_LOG("readRecv1Err  =%d.\n",pEmuStatic->readRecv1Err);
//    CL_LOG("readOk        =%d.\n",pEmuStatic->readOk);
//    printf("\n");
//}

//void ShowEmuAllStatic(void)
//{
//    int i;

//    for (i=0; i<6; i++) {
//        CL_LOG("emu %d static.\n",i+1);
//        ShowEmuStatic(&gEmuStatic[i]);
//    }
//}


//////////////////�������
int timer2_init(void) //tx
{
	EnWr_WPREG();
	setbit(HT_CMU->CLKCTRL1 ,2);//�򿪶�ʱ��0
					  //���ϼ���   //MODE���ڼ���
	HT_TMR2->TMRCON = (0x3<<1) | (0<<0);
	HT_TMR2->TMRDIV = 4400 - 1;  //0.1ms
	HT_TMR2->TMRPRD = 10000;		//1s
	HT_TMR2->TMRIE = 1;
	NVIC_EnableIRQ(TIMER_2_IRQn);
    DisWr_WPREG();
	return 0;
}


void TIMER_2_IRQHandler(void)
{
	int i;
    EMU_CTRL_STR *pEmu = NULL;

    if (HT_TMR2->TMRIF == 1) 
	{
        HT_TMR2->TMRIF = 0;
        for (i=1; i<13; i++) 
		{
            pEmu = &gEmuCtrl[i];
			pEmu->elec += (float)(gun[i].power); //1s����
        }
    }
}


int ElecHandle(void)
{
	EMU_CTRL_STR *pEmu = NULL;

	for (int i=1; i<13; i++) 
	{
        pEmu = &gEmuCtrl[i];
		gun[i].elec = (uint32_t)(pEmu->elec/360000); //36000000 / 100
	}
	return 0;
}


//�ж��Ƿ�ֻ��1·��磬�����ص���
int IsOneGunCharging(uint16_t *sample)
{
	if(GetChargingGunCnt() != 1)
	{
		return CL_FAIL;
	}

	for (int i=0; i < GUN_NUM_MAX; i++) 
	{
		if (gun_info[i].is_load_on && sampleCur[i] >= 300) 
		{//300mA(66W)���ϲ�����ѧϰ
			*sample = sampleCur[i];
			return i+1;		//����ǹͷid
		}
	}
	
	return CL_FAIL;
}


//�Զ�ѧϰ
void AutoLearnMatrix(void)
{
	int gun_id ;
	uint16_t current=0;

	//CL_LOG("autoLearnFlag=%X chargerCnt=%d\n",EmuCalation.autoLearnFlag,GetChargingGunCnt());
	//�ж��Ƿ�ֻ��1·���
	if (0xFFF != EmuCalation.autoLearnFlag && CL_FAIL != (gun_id = IsOneGunCharging(&current))) {
		//CL_LOG("gun_id = %d IsOneGunCharging curr = %d\n",gun_id,current);
		//�жϸ�·�Ƿ�ѧϰ��
		if (((EmuCalation.autoLearnFlag>>(gun_id-1)) & 0x01) == 0) {
			//������ѧϰ����
			if(gun_id == 1 || gun_id == 2){
				for(int i = 2;i<12;i++){
					sampleCur[i] = 0;
				}
			}else if(gun_id == 3 || gun_id == 5 || gun_id == 6){
				for(int i = 6;i<12;i++){
					sampleCur[i] = 0;
				}
			}else if(gun_id == 4){
				for(int i = 4;i<12;i++){
					sampleCur[i] = 0;
				}
			}else if(gun_id == 7){
				sampleCur[11] = 0;
			}else if(gun_id == 8){
				sampleCur[8] = sampleCur[10] = sampleCur[11] = 0;
			}

			//���¾���ϵ��
			updateMatrix(gun_id-1,(float)current,sampleCur);

			//�޸ı�־
			EmuCalation.autoLearnFlag |= (1<<(gun_id-1));

			//�������
			if (SaveEmuCalation(&EmuCalation) == CL_FAIL){
				CL_LOG("err.\n");
			}
			//�������
			invNxN(EmuCalation.matrixA,invMatrixA);
			CL_LOG("gun %d learn=%X.\n",gun_id,EmuCalation.autoLearnFlag);
		}else{
			//CL_LOG("gun %d has autolearn.\n",gun_id);
		}
	}
}


int SaveEmuCalation(EMU_CAL *emu)
{
	emu->checkSum = GetPktSum((uint8_t*)emu,EMU_CAL_SIZE-2);

	return FlashWritMatrix((void*)emu,EMU_CAL_SIZE);
}

void EmuMatrixInit(void)
{
	FlashReadMatrix((void*)&EmuCalation,sizeof(EMU_CAL));

	uint16_t checkSum = GetPktSum((uint8_t*)&EmuCalation,EMU_CAL_SIZE-2);

	if (!(EmuCalation.magic_number == 0x55AA && checkSum == EmuCalation.checkSum)) {
		CL_LOG("first init.\n");
		EmuCalation.magic_number = 0x55AA;
		EmuCalation.autoLearnFlag = 0;
		memcpy((void*)EmuCalation.matrixA,(void*)defaultMA,sizeof(defaultMA));
		SaveEmuCalation(&EmuCalation);
	}else{
		CL_LOG("second init.\n");
	}
	CL_LOG("f=%X.\n",EmuCalation.autoLearnFlag);
}

//��ѹ������ж�ʹ��
int EnableZXIE(int no)
{
	uint8_t data[4];

	HT7017_Write(no,VAR_WPREG,0xA6);
	HT7017_Write(no,VAR_MODECFG,0x20);
	HT7017_Read(no,VAR_MODECFG,data);
		
	if(HT7017_Read(no, VAR_EMUIE, data) != CL_FAIL)
	{
		uint32_t reg = (uint32_t)((data[0] << 16)|(data[1] << 8) | data[2]);
		CL_LOG("EnableZXIE reg=%X\r\n",0x02|reg);
		return HT7017_Write(no,VAR_EMUIE,0x02|reg);
	}
	return CL_FAIL;
}

//emu 3 5  usart 5 1
void emuTask(void)
{
	uint8_t openFlag = 0;
    uint32_t old;
    uint32_t second = 0;
    int i;
    int j;
    uint32_t sum;
    uint32_t checkSum[8];
    uint8_t  failFlag = 0x7e;
	uint32_t qqqqqz = GetRtcCount();
	
	CL_LOG("emu task start!!!!\n");
    memset(gEmuStatic, 0, sizeof(gEmuStatic));
    memset(gEmuCtrl, 0, sizeof(gEmuCtrl));

	EmuMatrixInit();

    timer2_init();
    setbit(HT_TMR2->TMRCON,0);

	invNxN(EmuCalation.matrixA,invMatrixA);//�������

	for (i = 1; i < 7; i++) 
    { //1~6  д���������
	    j = 0;
	    while (++j < 4) 
        {
    		if (CL_OK == writecalpara(EMUID[i])) 
            {
                if (CL_OK == ReadCheckSum(EMUID[i], &sum)) 
                {
                    checkSum[i] = sum;
                    CL_LOG("emu check ok, i = %d.\n", i);
                    failFlag = failFlag & ~(1<<i);
                    break;
                }
                else
                {
                    CL_LOG("fail, i = %d.\n", i);
                    vTaskDelay(1000);
                }
            }
            else
            {
                CL_LOG("fail,i=%d.\n",i);
                vTaskDelay(1000);
            }
	    }
        vTaskDelay(200);
	}
    if (failFlag) 
    {
        SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_EMU, 100, EVENT_OCCUR, NULL);
    }

	while(1) 
	{
        vTaskDelay(200);
        if (old != GetRtcCount()) 
		{
            old = GetRtcCount();
            second++;
            if (0 == (second % 60)) 
			{
				ElecHandle();//�������
            }
            if (0 == (second & 0x3ff)) 
			{
                CL_LOG("running,second=%d.\n",second);
            }

            readAlldata();

			if (0 == (second % 10)) 
			{
				AutoLearnMatrix();
				//for(int k = 0;k < 12 ;k++){
				//	printf("gun_id=%d vol=%d current=%d checkCur=%d | power=%d checkPower=%d\n",k+1,(int)sampleVol[k],(int)sampleCur[k],(int)checkCur[k],(int)samplePower[k],(int)checkPower[k]);
				//}
				//printf("...................\n");
            }
			#if 0
			if (((qqqqqz + 30) <= GetRtcCount())) 
			{
				uint8_t  PayCardOrder[ORDER_SECTION_LEN] = {0,};
				qqqqqz = GetRtcCount();
				
				#if 1
//				AutoLearnMatrix();
				if(openFlag == 0)
				{
					openFlag = 1;
					#if 0
					TurnOnAllGun();
					#elif 0
					for(uint32_t i = 1; i <= 1; i++)
					{
						printf("��ǹͷ��[%d]\n", i);
						RelayCtrl(i,1);
					//	vTaskDelay(2000);
					}
					#else
					StartChargingqqqqqq((ORDER_AUTH_CARD == gChgInfo.cardType) ? START_TYPE_AUTH_CARD : START_TYPE_MONTH_CARD, 2*100, gChgInfo.current_usr_card_id, PayCardOrder, ORDER_SOURCE_AUTH_CARD);
					#endif
				//	CL_LOG("TurnOnAllGun wwwwwwwwwwwwwwwwwr\n");
					//gEmuResetFlag = 2;
				}
				else
				{
					openFlag = 0;
					#if 0
					TurnOffAllGun();
					#elif 0
					for(uint32_t i = 1; i <= 1; i++)
					{
						printf("�ر�ǹͷ��[%d]\n", i);
						RelayCtrl(i,0);
					//	vTaskDelay(2000);
					}
					#else
					StopCharging(2);
					#endif
				//	CL_LOG("TurnOffAllGun wwwwwwwwwwwwwwwwwr\n");
				}
				#endif
            }
			#endif
			
            if (gEmuResetFlag) 
			{
                for (i = 1; i < 7; i++) 
				{
                    if (gEmuResetFlag & (1 << i)) 
					{
                        HT7017_Write(EMUID[i], VAR_SRSTREG, 0x55);
                        vTaskDelay(1000);
                        for (j=0; j<3; j++) 
						{
                            if (CL_OK == writecalpara(EMUID[i])) 
							{
                                //CL_LOG("call writecalpara ok.\n");
                                break;
                            }
                        }
                        gEmuResetFlag = gEmuResetFlag & (~(1 << i));
                    }
                }
            }

			if (0 == (second & 0x7f)) 
			{
                for (i = 1; i < 7; i++) 
				{
    				if (CL_OK == ReadCheckSum(EMUID[i], &sum)) 
					{
                        if (checkSum[i] != sum) 
						{
                            gChgInfo.statusErr |= (1<<i);
        					writecalpara(EMUID[i]);
                            CL_LOG("check emu data fail,i=%d.\n",i);
                        }
    				}
                }
			}
        }
	}
}


