
#include "includes.h"
#include "upgrade.h"
#include "flash.h"
#include "sc8042.h"
#include "server.h"



UPGRADE_STR upgradeInfo;

void Upgrade_Init(uint32_t package_num,uint8_t fw_version)
{
	memset(&upgradeInfo,0,sizeof(UPGRADE_STR));
	upgradeInfo.package_num = package_num;
	upgradeInfo.current_package = 1;
    upgradeInfo.fw_version  = fw_version;
    upgradeInfo.upgradeFlag = 1;

	FlashEraseAppBackup();
	Sc8042bSpeech(VOIC_START_UPGRADE);
}


uint8_t Upgrade_FlashCheck(void)
{
	uint16_t checksum = 0;
	uint8_t  rbyte = 0;

	for(uint32_t i = 0;i<upgradeInfo.size;i++){
		HT_Flash_ByteRead(&rbyte,AppUpBkpAddr+i,1);
		checksum += rbyte;
	}

	CL_LOG("checksum=%X  upgradeInfo.checkSum=%X.\n",checksum,upgradeInfo.checkSum);
	if(checksum != upgradeInfo.checkSum)  return 1;

	return 0;
}

uint8_t Upgrade_Write(uint8_t *data,uint16_t datalen)
{
	uint8_t index = data[0];
	uint8_t *pData = data+1;
	uint16_t len = datalen - 1;
    //CL_LOG("index=%d current_package=%d len=%d  \n",index,upgradeInfo.current_package,datalen);
	if(upgradeInfo.current_package == 1 && len > 32)
	{
		upgradeInfo.lastIndex = index;
		if(pData[0] == 0xAA && pData[1] == 0x55){
			FW_INFO_STR *pFwInfo = (void*)(pData+16);
			upgradeInfo.size = pFwInfo->size;
			upgradeInfo.checkSum = pFwInfo->checkSum;
			FlashWriteAppBackup(AppUpBkpAddr+upgradeInfo.write_size,pData+32,len-32);
			upgradeInfo.current_package++;
			upgradeInfo.write_size += len-32;
            CL_LOG("fw head info:size=%d  checkSum=%X \n",upgradeInfo.size,upgradeInfo.checkSum);
			return 1;

		}else{
			CL_LOG("fw head error. \n");
			return 0;
		}
	}

	if(upgradeInfo.lastIndex == index){
		CL_LOG("current pacakge is repeat transmate..\n");
		return 1;
	}
	upgradeInfo.lastIndex = index;
    FlashWriteAppBackup(AppUpBkpAddr+upgradeInfo.write_size,pData,len);
	upgradeInfo.write_size += len;
	if(upgradeInfo.current_package == upgradeInfo.package_num){

        upgradeInfo.upgradeFlag = 0;

		//数据包接受完成 校验数据和
		if(Upgrade_FlashCheck() != 0){
			CL_LOG("upgrade checksum error.\n");
			return 0;
		}
		CL_LOG("upgrade success. size=%d checksum=%X\n",upgradeInfo.size,upgradeInfo.checkSum);
		WriteUpdateInfo(upgradeInfo.size, upgradeInfo.checkSum);
		return 2;
	}else{
		upgradeInfo.current_package++;

	}
	return 1;
}

