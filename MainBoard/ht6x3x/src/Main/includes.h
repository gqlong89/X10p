
#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "systick.h"
#include "FreeRTOS.h"
#include "ht6xxx.h"
#include "CpuCfg.h"
#include "cmu.h"
#include "task.h"
#include "queue.h"
#include "gpio.h"
#include "semphr.h"
#include <time.h>
#include <math.h>

/*
002     FIFO_S_Get�����Ż� ������Ϊ60sһ�� ˢ����������Ż� showǹͷ״̬�����Ż� ��Զ�����������ӱ���� Э���޸�
003     ����ˢ����ͨ�ź��� ������������ ui����
004     2.4G·�������� ui�������� �����������Ժ�
005     �ֻ������Խӹ������ ������״̬�����ָܻ������޸� 485���ݲ������Э���·��Э�� ˢ��ui�Ż� һ�ſ����Կ�����ʶ��� �����ų����������ĳ����Ż� ���������Ż�
006     ��ԭ��¼���� ��������2.4G������������
007     �޸����ز���Ӱ��485ͨ�Ź������� ��綩�������12��Сʱ �豸��¼���������������ͺ�ģ�����͵� ��������У׼�Ż�
008     CkbTask��ʼ��������ģ��Ϊ���� 485/2.4G�������͵��� ��λ������׮��,ɾ��ʶ�����ǹͷ��Ϣ �����С��1Ԫ������ �Ӵ󰴼��߳�ջ
009     TestDataHandle�޸�׮�����ǹͷ���� ����ϵͳ/ע��/�����ɹ�������ʾ FtpGet���ƹ̼����������� ���Ӹ����¶���ʾ����
010     WholeModuleHandle�ж��ϱ������������Ƿ�Ϊ׮�ţ����������������������� ��������޸İ�����̼��汾�ϱ��������� ����������Ͼ͹رյ�Դ �Խ���ʽ��̨
012     ���2.4g����Э�����汨�ķ����ڴ��Ż� �����̼��汾�����޸�Ϊ20�ֽ� �ź�ǿ�ȷ���ǰ�� ��·�����������5A ������׮�����8700w ����7800w���ܿ����µĳ�� �Ż�UiStateLoop����
        BuleStatusSendProc����״̬����ϱ� CheckNetMacAddr����mac��ӷ������ж� CkbTask������������/����mac����ʱ���� ���߶������洦�� BlueSendHeartBeat����ռ��״̬�޸�
        WholeModuleHandle����������2.4G״̬ʶ���� OutNetConnetProc�ж���2.4g�ŷ��ͱ��� sim800_check_CSQ�ź�ֵΪ0��Ϊʧ��
013     ��ѯ�źŷ��ڲ忨���֮ǰ ShakeReqProc�������Ϣ�м�������ʱ BlueStartCharging����������/δ�ó��ģʽ/δ��¼��������� ��ȷ��ȡ������汾 ChargingProc�������̵߳���
014     BlueStartCharging�������������ý���ǹͷ���� ShakeReqProc׮ui�����������ӱ�ʶ SetRtcCount�޸�����ת���������� �����޸�Ϊ60��
014.2   ֧��1�Զ����485·������������ emuTaskȥ��ͳ�ƴ�ӡ ����ά��ָ�� RemoCtrlProc����Զ�̿���ָ�� ��λϵͳǰ�ȷ�ָ�λ������ ���ó�������ʱ�� GunChargingProc����ϱ���ǹ����
014.3   ���������޸�Ϊ30�룬�������Ӻ�����Ӧ�������������Ͽ����� 2.4g����δ���þͲ�����2.4g���ݽ��� ׮�ϵ����ж�485·�� ģ���ǹ�澯Ĭ�Ϸ�1��ǹͷ
014.5   ProcEventNoticeAck�ϱ�����״̬��ȷ��Ӧ����� 485Զ��������ʱ��Ϊ8���� �Ż���ǹ�жϻ��� flashд�幷�Ż� SysTask������ʱ�޸�Ϊ25ms 485���ĳ��Ȼ����޸�Ϊ1024
014.6   �Ż��ڲ����� BuleStatusSendProc/GunChargingProc������ǹ��Ϣ���ֻ��10�� �Ż���ǹ�ж�2�봦��һ�� ���󶩵��ط�����8 �̼�����������������
014.7   RecvAckProc�Ż�485���ؼ����һ֡���մ��� ���������485�����л�����
015     FlashWriteTradeRecord/FIFO_S_Put��дָ�������1���� StartProc�Ż� SetUpgradeInfo�̼�������ֹͣ������־ UpgradeTimeOutProc��Զ��·�Ͽ���ֹͣ���� OutNetConnetProc�����·60�볬ʱ�����ݽ��նϿ�
016     GunChargingProc32��ŷ���һ�ΰ�ǹ�澯/��ǹ��������Ϊ1.5w SysCfgInit��ǹʱ�����ӵ�4���� ������ˢ�����뵽ȫ�ֱ�����
        ����2.4g���Կ��غ� ��λ���޸�׮�������ԭ׮��һ�������ʶ���� �����������iccid�ϱ�/���Խ�������ϵͳ �ֻ������Խ�/�Ͽ��������� �̵������԰�14������ǹͷ�̵���8��
        WholeModuleHandle�޸������������� ��ʹ��ip�˿ڵ�ȫ�ֱ��� 485�����ȡ���ع������� KeyToUiProc�������ؿ��������������ά������ ����������Ϣ�ȴ�Ӧ�����˳�
017     CkbTask/ProcKbStatus��ⰴ����״̬��ʱ�·��������� SendHeartBeatFun�ϱ�������״̬ �Ż���ǹ�¼�32�뷢��һ�� CardModuleHandle���ܳ�д�� ��ʱ10���ӹر�lcd���� CkbTask����10�뿪���������Դ
017.1   GunChargingProc���ӱ���˿�ն��¼�֪ͨ sysTickRecover�ϵ绹ԭϵͳʱ��� gun_info�����Ż� NVIC_SystemReset�������жϲ��� ά��������ʾ485���� �̼�����ǰ���жϹ̼��Ƿ�һ�� ProcBtHeartBeatÿ15���Ӵ�һ������
018.1   �޸İ�ǹ����Ϊ8w����ʱΪ10�� UiKeyHandle_InputDeviceIdֱ�Ӱ�ȷ�ϲ������ʶ���� ����ϵͳ�ӹ���λ
019.0   StartProc���Ʊ������������ж� RecvServerData�޸ı��ĳ��ȴ����½���ʧ������ CkbTask�������ر���10�����һ�� StartProc�ж�ʱ�䲻�Ϸ��������趨ϵͳʱ��
        SendProtoPkt�Ա��ĳ���16�ֽڶ����޸ģ��޸����ܱ����ۼӺ������������� SycTimeCount��ʱ����ʱ������´��� �޸���Կ��������token ip_state���ƽ�������
        ���485����ʱ������id2���� 2�α��Ľ���ʧ�������ܳ׸��»��� ͳһ���ܴ���������ʾ ����ר������������������ ����˫���߹��� ����OTA���� MuxSempTake���ӵݹ����� GetGunStatus��ǰ�������ʼ������濪���Ĳ������ʵ�0.005 readAlldata�޸������Ϊ1100w
019.4   RecvBtData�޸ı��ĳ���Ϊ0����״̬����������� ���Ľ��ռ�״̬��ʱ��ԭ����
019.7   SendTradeRecordNotice�޸�δ��ȷ�ϴ�����ģʽ��������� MQTT_CMD_CARD_ID_REQ/RecvServerDataˢ��ɨ������������� GunChargingProc����й���ͻȻ�½����û��ȷ���Ʒѹ��ʣ�����ȷ���Ʒѹ��� �޸�׮�����������������
019.8   �޸İ�����û��Ӧ����
019.9   BlueSendStopChargingAck�����������ӿ�ʼ���ʱ��͵���
020     BlueCostTemplateReq�޸ı��ķ��ͳ��� LcdStatusCheck lcd�������ϲ��ر� OptFailNoticeȥ����������
020.1   BlueSendStopChargingAck�������ѽ�� BlueStopCharging��������ֹͣ���ԭ�� GunChargingProc����һֱ���ӳ���15����ǿ��ȷ����繦�ʣ�ȥ�����ƹ��ʵ�����Լ�� MainTask�ϵ�10���ſ�ʼ�����ƴ���
020.2   BlueSendStopChargingAck���ӳ�繦��
020.3   �Ʒ�ģ�����ݽṹ��ԭ��flashһ�� ������־��ӡ���ƹ��� �״��ϵ��ʼ����������ռ� BlueSendHeartBeat���Ƴ��״̬���� �������翪��/�رն˿ڹ��� ProcKBData�޸��쳣��ʱΪ2�� ChargingProc������ظ��򿪶˿� GunChargingProc���ʴ���1��������й��ʵ��� ���������������������� GunChargingProc����С��20w
020.4   GunChargingProc���ʴ���10w��������2���ӲŽ��е��� �޸�r8�������������ѶϿ����� ���������������� �̶��Ʒѹرճ�����ͣ��ʵʱ�ƷѲ��� ���ٷ��Ϳ�ʼ���֪ͨ
021.0   readAlldata����ͨѶʧ�����³�ʼ������ SendStartUpNotice������汾�Ÿĳ��Ӱ汾�ţ�������汾�ŷ���ͳ����Ϣ���� GunChargingProc����3�μ�⵽�����������л���繦�� SysCfgInit����˿�նϴ�����ʼ����ħ����
021.1   emuTask���Ӷ�ʱ��ѯ����оƬģ��У������쳣�ָ�����
021.2   readAlldata���ʴ���500w�����˴������ڲ���ʧ�ܽ����������� UpgradeTimeOutProc��Զ����15���ӳ�ʱ����������10���ӳ�ʱ 20���Ӽ�¼һ�γ������ 100/101���ظ�����
021.3   ����Ǯ����/�Ž���
021.4   PktRecvProc��Զ����������������ʾ SysTask��ʱ25ms WritePayCardHandle�������ǰ�������������
021.5   ������������ʱ���������𲽽�ģʽ
021.6   �Ʒ�ģ�����/��Ȩ��/Ǯ���������Ż� GetGunStatus����100�߰�1w���Ŵ��� �𲽽����Сbug���� SendHeartBeatFunһ�η���12��ǹͷ���� GunChargingProc�����繦��5����ƽ��С��3.5w������Ϊǹͷ�γ� �޸��¿����ܿ���������ͣ����
        ά�����水8���밴ȷ������ϵͳ Sc8042bSpeech����ģʽ���ر����� �������µ��ͬʱ���������崮�� Sc8042bSpeech�����������Ʋ�׼����
021.7   ProcKbStatus�԰������µ��ȰѴ���txrx�ܽ����� SwitchToUi_SocketOccupy��������ŵݹ�������������޸� ��UiStateLoop���ڰ����߳�,����̼���������ˢ�½��������� MakeTradeInfo/BlueStopCharging�޸����������ϱ��𲽽�ģʽ���ò�������
021.8   ResetKeyBoardϵͳ�ϵ����µ����������ϵ簴���� CardModuleHandleû�мƷ�ģ�治��ˢ����� Sc8042bSpeech�޸Ľ���ʱ�䲻��Ϊ0����
022.0   ���ʱ��ȥ������ʱ������ ChargingProc����ж�ͳһ�ý������ж�,�¿����ѽ������Ϊ99Ԫ,�������ֹͣ��� ��̨���������������������ӳ����ʣ����ʱ�� ά�����水7�����ӡ�������� UiDisplay_PosData���ˢ�����̫���������� ��system_info�д��ǹͷ��Ϣ��У��ֵ
022.1   ����������Ϣ����У�鼰�ָ����ƣ���������У�飬���ʧ�ܾͶ������� flash��eeprom��ȡ�޸�Ϊ����ֽڶ�ȡ ��������޸�Ϊ5.5A ÿ��1024��Լ���оƬǿ�����³�ʼ��У����� ProcFuseBreak����˿�նϼ���޸�Ϊ3��
022.2   ����x10c GetPktSum����У����޸� MainTask��ȡ����ɹ��ͷ��͵�¼��Ϣ FtpGet�����޸�ĳЩ��С����������ļ�������������������
022.3   MainTask�Ż���ȡ��id2�ͷ�������������Ϣ
022.4   ��������90�뷢��һ�� ǹͷ����д��flash��ϵͳ��Ϣ���������ǹͷ��У��ֵ�����¼������һ��
022.5   MainTask����ÿ64����������·�Ƿ�Ͽ� Ӧ���빦�ܺ���
022.6   Ӧ�������У�鹦�� ��������dfi���ó���ʱ��Ͱ�ǹֹͣ���ʱ�� emuTaskÿ64��ǿ��У��һ��
023.0   �̵������Կ������3���Ӻ��Զ�ֹͣ Ӧ���뿪���Ĳ��������ظ�ʹ��Ӧ���뿪����� ProcGunStatusӦ���뿪���Ĳ�����Ӧ��ʱ�䵽��������г��ڳ��Ҳ���ر� Ӧ���밴������������Ļ��ʾ����
023.1   TestCloseGunProcԶ��ȫ���رռ̵����������� SysCfgInit��������ʼ���Ż�
023.3   �޸İ�ǹֹͣ����ʱ����ֵΪ5���� ���Ƽ���оƬУ���� �𲽽��շ������޸� ��̨���Ӧ��ֹͣ������֡���¶���ָ���쳣�����޸� ��ǹֹͣ���ƽ�����ʵ���Ϊ2.5w
023.4   SendHistoryOrder�޸Ķ������ͺ�����ǹͷ��Ϣ��bug
023.5   GunChargingProc��⵽ǹͷ�Ͽ�����30��ִ������У��
023.6   StartCharging����������������緢�ʹ���
023.7   EepromWriteData����eepromд����������ʧ������
024.0   CheckBlueInfo��������ǰ���ж����������Ƿ�Ϸ� emuTaskоƬУ׼������������ǰ����ģ�������� RestoreGunStatus�����ϵ��޸�ǹͷ״̬Ϊδ��⵽���� SendStartUpNotice׮��ǿ������ǰ3�ֽ� GunChargingProc��⵽ǹͷ�Ͽ��ȹرռ̵��� CkbTask�̼߳Ӵ�ջ�ռ���˫�������������ϱ�����ϵͳ��λ����
024.4   readAlldata�����޸�Ϊ������ SendStartUpNotice�ϱ���������ʱ�� RecvServerData��������id2��Կԭ��
024.6   �̶��Ʒ�ȡ����ǹ��� ʵʱ�Ʒ�Ĭ�Ͽ���������ͣ GetGunStatus���Ͱ�ǹ��ֵ StartCharging�ֹ��ʼƷ�Ԥ���𲽽�ʱ��45���ӣ���ֹΪ0�������� �𲽽�ģʽ���𲽽�ʱ���ڲ��жϰ�ǹ����
024.7   �����޸�Ϊ20w
024.8   �̶��ƷѰ�ǹ�󲻿��ؼ̵��� ����5���ӿ��ؼ̵���4�θĳ�2��
024.9   ר��SendProtoPkt�������485�Ͳ��ü��ܱ�־
025.0   RestoreGunStatus�޸����ó���־�ټ���У��͵���У��ʹ��������
026.0   �̶��Ʒ�ģʽ���Ӱ�ǹ��⼰������ͣ���ؿ����� SendHeartBeatFun���ܰ汾���������ļ��ܴ��� ��׮��/ʶ�������Ҫ��Ϣ����һ��flash���������ݱ��洦����ֹeeprom��Ϣ�쳣��ʧ�����лָ�
026.4   UiKeyHandle_InputChargerSn�޸�׮�ű��ݴ����������
026.7   RemoteCtrlProc���������������ӷ���Ӧ�� SendStartUpNotice�ϱ���ǹ���ͳ�����ͣ��⿪�� RecvServerData��Կ����֪ͨ���ص�Ӧ�������к������󱣳�һ��
027.0   StartCharging���Ʊ������·�������ǹ״̬֮ǰ,�������������ͻȻֹͣ������
027.3   RecvServerDataע��ɹ��󱣴�ʶ���뵽����flash���򣬽������û�б���ʶ���������ע������������
027.6   SetChargingTime������ʱ������Ϊ��С45���� GunChargingProc�޸Ĺ̶��Ʒ��޷�����������ͣ������
028.0   StartChargingʵʱ���𲽽�ģʽ�¹��ʵ���45w��ʼ���� GunChargingProc�жϳ�����ͣʱ���ʱ仯����6w���ӳ�����ʱ�� �����¶ȴ���65��ֹͣ��繦�� ����Ĭ�ϵ�����ʱ��Ϊ45���� �����㷨�Ż�
029.0   SpeechCardBalance�Ż�ˢ����������������� DecryptData�����޸����ĳ��ȴ�u16תu8���־��ȶ�ʧ����,������϶�������������ֵ��쳣�������� �Ż����ܴ�������
029.3   ReadAuthCardIDHandleˢ����Ȩ�������ֻ��2�� StopCharging��������ǿ�����¼���һ�γ���������ֳ��ʱ��ͽ���һ�������� SendHeartBeatFun�ϱ�ÿ·��ѧϰ��־
029.6   BlueStopCharging�ж�����ǿ���ֱ�ӷ���ֹͣ�ɹ�������Ƕ����Ų�һ�·���ֹͣʧ�� ���ڿ��ܴ������⣬�رճ���¶ȹ���ֹͣ���ļ��
030.0   EmuMatrixInit�Լ�����������ȫ��У�鹦�� �����������ϱ�ʵ�ʳ��ʱ�� SendRegisterע����Ϣ����Ӳ��id�ֶ� �¿�����������������Ϊ20w,����ʱ���������Ϊ2Сʱ ���¼�֪ͨ���������ϱ���Կ���½�� SendRemoCtrlAckӦ��ṹ������
030.3   �ѽ������֪ͨ����ʵ�ʳ��ʱ���ֵ�ӷ����޸�Ϊ��
031.0   ��ǹʱ�����ӵ�10����
032.0   BlueTimingOpen����������ʱÿ��10���ӷ���һ�δ������㲥������ WriteCfgInfo�Ż����ò�����д���� SendBtRemoCtrlAck�����򿪲���Ӧ���Ľṹ������޸� BtSetPullGunTime�����������ð�ǹֹͣ���ʱ��
        MakeTradeInfo������֪ͨ�ϱ���ǹֹͣ��硢������ͣ���ء���ǹֹͣ���ʱ�����Ϣ GunChargingProc��翪ʼ����޵���������ʾ���� ��ʱ����Ʒ�ģ��ʱ���޸�Ϊ12Сʱ �����¿�����ʣ���������
        �̶��Ʒ�ģʽ�������˹رճ�����ͣ���¿���Ч ��¼�������ӹ̼�����ftp��ʽ �޸��ź��������ܻ������⣬���С���������쳣�������� �ѽ������֪ͨ����ʵ�ʳ��ʱ���ֵ�����޸�Ϊ���� ���Ӻ�̨�Ʒ�ģ��idΪ0��������ʾ
032.3   ȥ����ǹ���ؼ̵������� �޸�Զ���������ʱ������������ ���ߺ�����ָ����͵�¼����
033.0   �Ե����¿�����4Сʱ������ʱ�� �������书�ʿ����� �ж�����Ӧ������ļƷ�ģ��id�����׮�˲�һ�������̨����Ʒ�ģ�壬ȥ����ʱ����Ʒ�ģ����� �Ż��״��ϵ�С���ʵ�¼������
        ���ߺ������ָ�������ȴ�һ��ʱ��(2����֮��)�����·������ݵ���̨�������̨���ϻָ���׮��ͬʱ�ܼ��������ݵ���̨���Ի����̨��Ϣ����ѹ�� ��־��ӡ���ؿ���4����Զ��ر�
        ��ʱ���tcp��·״̬����·�쳣���Ͻ������������Զ�ʱ���ڻָ����� ���ӳ���24Сʱǿ�ƽ�����������
034.0   mcu��Ƶ����������쳣��λ����
*/

#define SPECIAL_NET_CARD				0                   //0:������    1:ר����    2:X10c
#define ENVI_TYPE                       1                   //0:��ʽ���� 1:���Ի��� 2:Ԥ�������� 3:�������� 4:���Ի���2
#define FW_VERSION                      1
#define FW_VERSION_SUB                  (SPECIAL_NET_CARD)

#if ENVI_TYPE
	#define TEST_VERSION                    3		//
#endif

#if (1 == SPECIAL_NET_CARD) //====================== ר���� ========================
	#define X10C_TYPE                       0                   //0:����x10  1:x10c
	#define ID2                             1                   //0:������	 1:����
	#if (0 == ENVI_TYPE)
	#define NET_SERVER_IP                   "www.access.com"    //��ʽ����
	#define NET_SERVER_PORT                 10010
	#elif (1 == ENVI_TYPE)
	#define NET_SERVER_IP                   "test.access.com"
	#define NET_SERVER_PORT                 10010
	#elif (2 == ENVI_TYPE)
	#define NET_SERVER_IP                   "172.16.0.245"
	#define NET_SERVER_PORT                 10010
	#elif (3 == ENVI_TYPE)
	#define NET_SERVER_IP                   "172.16.0.228"
	#define NET_SERVER_PORT                 10010
	#elif (4 == ENVI_TYPE)
	#define NET_SERVER_IP                   "172.16.0.8"
	#define NET_SERVER_PORT                 10010
	#endif

//	#define REGISTER_CODE					"928698494848"
	#define REGISTER_CODE					"259787389784"
#elif (0 == SPECIAL_NET_CARD) //====================== ������ ========================
	#define X10C_TYPE                       0                   //0:����x10  1:x10c
	#define ID2                             1                   //0:������	 1:����

	#if (0 == ENVI_TYPE)
	#define NET_SERVER_IP                   "access.sharecharger.com"     //��ʽ����
	#define NET_SERVER_PORT                 10010
	#elif (1 == ENVI_TYPE)
	#define NET_SERVER_IP                   "test.access.chargerlink.com"
	#define NET_SERVER_PORT                 10010
	#elif (2 == ENVI_TYPE)
	#define NET_SERVER_IP                   "47.97.236.187"
	#define NET_SERVER_PORT                 10010
	#elif (3 == ENVI_TYPE)
	#define NET_SERVER_IP                   "47.97.238.64"
	#define NET_SERVER_PORT                 10010
	#elif (4 == ENVI_TYPE)
	#define NET_SERVER_IP                   "47.97.237.248"
	#define NET_SERVER_PORT                 10010
	#endif
	
//	#define REGISTER_CODE					"928698494847"
	#define REGISTER_CODE					"259787389784"
#elif (2 == SPECIAL_NET_CARD) //====================== X10C ========================
	#define X10C_TYPE                       1                   //0:����x10  1:x10c
	#define NET_SERVER_IP                   "www.access.com"     //��ʽ����
	#define NET_SERVER_PORT                 10010
	#define ID2                             0                    //0:������	 1:����
//	#define REGISTER_CODE					"830484693193"
#endif

#define IS_CPU_DOU_FRE                  	1                   //�Ƿ�Ƶ  0����Ƶ��  1��Ƶ
#if (0 == IS_CPU_DOU_FRE)
	#define VOICE_DELAY_DIV               	2
#else
	#define VOICE_DELAY_DIV               	1
#endif

#define IS_THREE_DIAL                   0
#define PUT_OUT_GUN_POWER               15  //0.1w
#define PUT_OUT_GUN_TIME                300  //��

#define CHARGING_FULL_TIME              (45*60) //��
#define CLOSE_VIOCE_START_IIME          23
#define CLOSE_VIOCE_END_IIME            7

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;

#define MAGIC_NUM_BASE                  0x123456ab
#define STATION_MACHINE_TYPE            "DEVICE_CHARGER_CL_XA"
#define CHARGER_TYPE                    2  //1:x9��2:x10
#define CHARGER_NAME                    "X10P"

#define THIS_LINE                       __LINE__
#define CL_LINE                         THIS_LINE
#define CL_FUNC                         __func__
#define CL_OK                           0
#define CL_FAIL                         (-1)
#define CL_TRUE                         1
#define CL_FALSE                        0

#define LOG_OPEN                        1


#define UPGRADE_PACKAGE_SIZE    		64
#define UPGRADE_SUCCESS_FLAG    		0x5abe5b6c


#define	NOP()				            __NOP()
#define OS_DELAY_MS                     vTaskDelay
#define GUN_NUM_MAX				        12
#define ORDER_SECTION_LEN               10
#define CHARGER_SN_LEN			        8   //bcd�ĳ���
#define EVEN_DISCRI_LEN                 32
#define OUT_NET_PKT_LEN                 350
#define NET_485_PKT_LEN                 (1024+32)
#define BLUE_VERSION_LEN                20
#define BLUE_NAME_LEN                   10
#define BLUE_GWADDR_LEN                 5
#define START_POWER_LIMIT               7800  //���������׮��������� w
#define CHARGING_POWER_LIMIT            8700  //��׮��������� w


#define setbit(x,y) 					x |= (1<<y)           //��X�ĵ�Yλ��1
#define clrbit(x,y) 					x &= ~(1<<y)          //��X�ĵ�Yλ��0
#define revbit(x,y) 					x ^= (1<<y)           //��X�ĵ�Yλ��ת


enum{
	FW_X10P = 0,  		//X10P
	FW_X10_KEY_BOARD,  	//ˢ����
	DW_NUM,     		//�̼�����
};

typedef struct {
    uint8_t  status;                            //�ź���״̬ 0:���У���0:ռ��
    uint8_t  semTakeCnt;
    TaskHandle_t pTaskHandle;                   //ռ�õ�������
}MUX_SEM_STR;

#pragma pack(1)
typedef struct{
    uint8_t  aa;
    uint8_t  five;
    uint8_t  fwCnt;
    uint8_t  fwVer1;
}FW_HEAD_STR;


typedef struct{
    uint32_t size;
    uint16_t checkSum;
    uint8_t  name[10];
}FW_INFO_STR;
#pragma pack()


#endif //__INCLUDE_H__







