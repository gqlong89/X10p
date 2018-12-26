/*aes.h
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/
#ifndef __AES_H__
#define __AES_H__

#include "includes.h"



/*
* @brief  AES_key����ģʽ����
*/
typedef enum
{
    KEY_Mode_128bits = AES_AESCON_KEYMODE_AES128,       //AES-128
    KEY_Mode_192bits = AES_AESCON_KEYMODE_AES192,       //AES-192
    KEY_Mode_256bits = AES_AESCON_KEYMODE_AES256,       //AES-256
}AESKeyMode_TypeDef;                					//end of group AESKeyMode_TypeDef

/*
* @brief  AES/GHASH/RAND�ж�ʹ�ܶ���
*/
typedef enum
{
    AES_IE = GHASH_AESGHASHIE_AESIE,       				//AES�ж�ʹ��
    GHASH_IE = GHASH_AESGHASHIE_GHASHIE,       			//GHASH�ж�ʹ��
	RAND_IE = GHASH_AESGHASHIE_RANDIE,       			//RAND�ж�ʹ��
}AES_ITEnTypeDef;  										//end of group AES_ITEnTypeDef

/*
* @brief  AES/GHASH/RAND�жϱ�־����
*/
typedef enum
{
    AES_IF = GHASH_AESGHASHIF_AESIF,       				//AES�жϱ�־
    GHASH_IF = GHASH_AESGHASHIF_GHASHIF,       			//GHASH�жϱ�־
	RAND_IF = GHASH_AESGHASHIF_RANDIF,       			//RAND�жϱ�־
}AES_ITFlagTypeDef;

typedef union
{
	uint8_t u8[16];
	uint32_t u32[4];
	uint64_t u64[2];
} Parameter128bits_TypeDef;

typedef union
{
	uint8_t u8[16*2];
	uint32_t u32[4*2];
	uint64_t u64[2*2];
} Parameter256bits_TypeDef;

typedef union
{
	uint8_t u8[16*4];//64*8
	uint32_t u32[4*4];//16*4*8
	uint64_t u64[2*4];//8*2*4
} Parameter512bits_TypeDef;

typedef union
{
	uint8_t u8[16*8];
	uint32_t u32[4*8];
	uint64_t u64[2*8];
} Parameter1024bits_TypeDef;

/*
* @brief  ECB������������ṹ����
*/
typedef struct
{
	AESKeyMode_TypeDef AESKeyMode;			//���ܽ���ģʽѡ��
	uint32_t *pKeyAddr;						//��Կ��ַָ��
	uint8_t	*pPtextAddr;					//�������ݵ�ַָ��
	uint64_t LenPtext;						//�������ݳ��ȣ�byte��
}ECB_Encrypt_InputTypedef;					/*!< end of group ECB_Encrypt_InputTypedef*/

/*
* @brief  ECB������������ṹ����
*/
typedef struct
{
	uint8_t	*pCtextAddr;					//�Ѽ����������ĵ�ַָ��
	uint64_t LenCtext;						//�Ѽ����������ĳ��ȣ�byte��
}ECB_Encrypt_OutputTypedef;					/*!< end of group ECB_Encrypt_OutputTypedef*/

/*
* @brief  ECB������������ṹ����
*/
typedef struct
{
	AESKeyMode_TypeDef AESKeyMode;			//���ܽ���ģʽѡ��
	uint32_t *pKeyAddr;						//��Կ��ַָ��
	uint8_t	*pCtextAddr;					//�������ݵ�ַָ��
	uint64_t LenCtext;						//�������ݳ��ȣ�byte��
}ECB_Decrypt_InputTypedef;					/*!< end of group ECB_Decrypt_InputTypedef*/

/*
* @brief  ECB������������ṹ����
*/
typedef struct
{
	uint8_t	*pPtextAddr;					//�ѽ������ĵ�ַָ��
	uint64_t LenPtext;						//�ѽ������ĳ��ȣ�byte��
}ECB_Decrypt_OutputTypedef;					/*!< end of group ECB_Decrypt_OutputTypedef*/

/* This should be a hidden type, but EVP requires that the size be known */
struct aes_key_st {
    unsigned int rd_key[4 *(14 + 1)];
    int rounds;
};

typedef struct aes_key_st AES_KEY;



void HT_AES_Encrypt(AESKeyMode_TypeDef AESKeyMode, uint32_t *pKeyAddr, uint32_t *pIntDataAddr);
void HT_AES_Decrypt(AESKeyMode_TypeDef AESKeyMode, uint32_t *pKeyAddr, uint32_t *pIntDataAddr);
void HT_GHASH_Multiply(uint32_t *pIntDataAddr1, uint32_t *pIntDataAddr2);
void HT_AES_Read(uint32_t *pOutDataAddr);
void HT_GHASH_Multiply_Read(uint32_t *pOutDataAddr);
void HT_AES_GHASH_RAND_ITConfig(AES_ITEnTypeDef ITEn, FunctionalState NewState);
ITStatus HT_AES_GHASH_RAND_ITFlagStatusGet(AES_ITFlagTypeDef ITFlag);
void HT_AES_GHASH_RAND_ClearITPendingBit(AES_ITFlagTypeDef ITFlag);
ITStatus HT_AES_Busy_StatusGet(void);
ITStatus HT_GHASH_Busy_StatusGet(void);
void HT_AES_Xor128bits(uint32_t *pIntDataAddr1, uint32_t *pIntDataAddr2,uint32_t *pOutDataAddr);
int EncrypData(uint8_t *in, uint8_t *out, uint16_t len, AES_KEY *pAesKey);
int DecryptData(uint8_t *in, uint8_t *out, uint16_t len, AES_KEY *pAesKey);



void AES_decrypt(unsigned char* in, unsigned char* out, AES_KEY* aes);
void AES_encrypt(unsigned char* in, unsigned char* out, AES_KEY* aes);
void AES_encrypt_hw(unsigned char* in, unsigned char* out, uint32_t* aes);
void AES_decrypt_hw(unsigned char* in, unsigned char* out, uint32_t* aes);

int AES_set_encrypt_key(unsigned char *userKey, int bits, AES_KEY *key);
int AES_set_decrypt_key(unsigned char *userKey, int bits, AES_KEY *key);

#endif //__AES_H__

