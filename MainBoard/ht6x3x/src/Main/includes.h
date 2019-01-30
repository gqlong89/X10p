
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
002     FIFO_S_Get函数优化 心跳改为60s一次 刷卡充电语音优化 show枪头状态函数优化 拉远上网完善增加编译宏 协议修改
003     按键刷卡板通信合入 蓝牙组网合入 ui合入
004     2.4G路由器合入 ui功能完善 增加生产测试宏
005     手机蓝牙对接功能完成 按键板状态错误不能恢复问题修改 485兼容产测软件协议和路由协议 刷卡ui优化 一张卡可以开启多笔订单 订单号长度心跳报文长度优化 计量代码优化
006     还原记录代码 增加蓝牙2.4G组网上网功能
007     修复本地拨号影响485通信功能问题 充电订单最长限制12个小时 设备登录命令增加上网类型和模块类型等 计量参数校准优化
008     CkbTask初始化置蓝牙模块为故障 485/2.4G组网类型调整 上位机更改桩号,删除识别码和枪头信息 卡余额小于1元报余额不足 加大按键线程栈
009     TestDataHandle修改桩号清除枪头数据 重启系统/注册/升级成功语音提示 FtpGet完善固件升级防错功能 增加高温温度提示语音
010     WholeModuleHandle判断上报的蓝牙名称是否为桩号，不是则重新设置蓝牙名称 产测软件修改按键板固件版本上报错误问题 语音播放完毕就关闭电源 对接正式后台
012     结合2.4g外网协议整版报文发送内存优化 蓝牙固件版本长度修改为20字节 信号强度放在前面 单路电流限制最大5A 限制整桩最大功率8700w 大于7800w不能开启新的充电 优化UiStateLoop函数
        BuleStatusSendProc蓝牙状态检测上报 CheckNetMacAddr网关mac添加非数字判断 CkbTask调节蓝牙名称/网关mac配置时间间隔 离线订单缓存处理 BlueSendHeartBeat插座占用状态修改
        WholeModuleHandle增加蓝牙和2.4G状态识别处理 OutNetConnetProc判断有2.4g才发送报文 sim800_check_CSQ信号值为0视为失败
013     查询信号放在插卡检测之前 ShakeReqProc发多个消息中间增加延时 BlueStartCharging修正订单号/未置充电模式/未登录不开启充电 正确读取按键板版本 ChargingProc放在主线程调用
014     BlueStartCharging增加蓝牙开启置金额和枪头参数 ShakeReqProc桩ui增加蓝牙连接标识 SetRtcCount修改日期转换错误问题 心跳修改为60秒
014.2   支持1对多外接485路由器上网功能 emuTask去掉统计打印 蓝牙维测指令 RemoCtrlProc增加远程控制指令 复位系统前先发指令复位按键板 配置充满续冲时间 GunChargingProc检测上报拔枪动作
014.3   蓝牙心跳修改为30秒，蓝牙连接后心跳应答不正常接主动断开连接 2.4g网关未配置就不处理2.4g数据接收 桩上电先判断485路由 模拟拔枪告警默认发1号枪头
014.5   ProcEventNoticeAck上报蓝牙状态加确认应答机制 485远程升级超时改为8分钟 优化拔枪判断机制 flash写清狗优化 SysTask任务延时修改为25ms 485报文长度缓存修改为1024
014.6   优化内部变量 BuleStatusSendProc/GunChargingProc蓝牙拔枪消息最多只发10次 优化拔枪判断2秒处理一次 增大订单重发次数8 固件升级不发心跳报文
014.7   RecvAckProc优化485网关及最后一帧接收处理 本地网络和485网络切换处理
015     FlashWriteTradeRecord/FIFO_S_Put读写指针操作加1判满 StartProc优化 SetUpgradeInfo固件升级置停止发包标志 UpgradeTimeOutProc拉远链路断开就停止升级 OutNetConnetProc外出链路60秒超时无数据接收断开
016     GunChargingProc32秒才发送一次拔枪告警/拔枪功率设置为1.5w SysCfgInit拔枪时间增加到4分钟 不保存刷卡密码到全局变量中
        增加2.4g测试开关宏 上位机修改桩号如果和原桩号一样则不清除识别码 产测软件增加iccid上报/测试结束重启系统 手机蓝牙对接/断开功能完善 继电器测试按14轮流打开枪头继电器8秒
        WholeModuleHandle修改命令处理错误问题 不使用ip端口的全局变量 485拨码读取网关功能完善 KeyToUiProc待机加载可以输入密码进入维护界面 发送蓝牙消息等待应答再退出
017     CkbTask/ProcKbStatus检测按键板状态超时下发重启自愈 SendHeartBeatFun上报按键板状态 优化拔枪事件32秒发送一次 CardModuleHandle卡密匙写死 超时10分钟关闭lcd背光 CkbTask周期10秒开启按键板电源
017.1   GunChargingProc增加保险丝烧断事件通知 sysTickRecover上电还原系统时间戳 gun_info变量优化 NVIC_SystemReset增加锁中断操作 维护界面显示485拨码 固件升级前先判断固件是否一致 ProcBtHeartBeat每15分钟打开一次蓝牙
018.1   修改拔枪功率为8w，超时为10秒 UiKeyHandle_InputDeviceId直接按确认不能清掉识别码 重启系统加狗复位
019.0   StartProc完善本地网络类型判断 RecvServerData修改报文长度错误导致解密失败问题 CkbTask按键及关背光10秒操作一次 StartProc判断时间不合法才重新设定系统时间
        SendProtoPkt对报文长度16字节对齐修改，修复加密报文累加和域计算错误问题 SycTimeCount对时完善时间戳更新处理 修改密钥报文增加token ip_state完善建链步骤
        外接485网络时不开启id2功能 2次报文解密失败启动密匙更新机制 统一加密处理及语音提示 增加专网卡及域名解析功能 增加双离线功能 蓝牙OTA功能 MuxSempTake增加递归属性 GetGunStatus当前插座功率减掉后面开启的插座功率的0.005 readAlldata修改最大功率为1100w
019.4   RecvBtData修改报文长度为0导致状态机错误的问题 报文接收加状态超时还原机制
019.7   SendTradeRecordNotice修改未正确上传补贴模式与参数问题 MQTT_CMD_CARD_ID_REQ/RecvServerData刷卡扫码清除补贴参数 GunChargingProc充电中功率突然下降如果没有确定计费功率，就先确定计费功率 修改桩号清除订单缓存数据
019.8   修改按键板没反应问题
019.9   BlueSendStopChargingAck蓝牙结束增加开始充电时间和电量
020     BlueCostTemplateReq修改报文发送长度 LcdStatusCheck lcd背光晚上不关闭 OptFailNotice去掉错误两字
020.1   BlueSendStopChargingAck增加消费金额 BlueStopCharging完善蓝牙停止充电原因 GunChargingProc功率一直增加超过15分钟强制确定充电功率，去掉限制功率调整的约束 MainTask上电10秒后才开始充电控制处理
020.2   BlueSendStopChargingAck增加充电功率
020.3   计费模版数据结构还原与flash一致 增加日志打印控制功能 首次上电初始化订单缓存空间 BlueSendHeartBeat完善充电状态处理 增加网络开启/关闭端口功能 ProcKBData修改异常超时为2秒 ChargingProc充电中重复打开端口 GunChargingProc功率大于1分钟则进行功率调整 蓝牙升级不进行语音播报 GunChargingProc判满小于20w
020.4   GunChargingProc功率大于10w持续超过2分钟才进行调节 修复r8升级播报蓝牙已断开语音 合入消除语音扰民 固定计费关闭充满自停，实时计费不变 加速发送开始充电通知
021.0   readAlldata串口通讯失败重新初始化串口 SendStartUpNotice按键板版本号改成子版本号，按键板版本号放在统计信息里面 GunChargingProc连续3次检测到功率上升才切换充电功率 SysCfgInit保险丝烧断次数初始化加魔术字
021.1   emuTask增加定时查询计量芯片模块校表参数异常恢复机制
021.2   readAlldata功率大于500w做过滤处理，串口操作失败进行重启串口 UpgradeTimeOutProc拉远升级15分钟超时，本地升级10分钟超时 20分钟记录一次充电数据 100/101不重复播报
021.3   合入钱包卡/门禁卡
021.4   PktRecvProc拉远升级开启进度条显示 SysTask延时25ms WritePayCardHandle启动充电前先清掉补贴参数
021.5   增加语音禁播时间设置与起步金模式
021.6   计费模版接收/鉴权卡/钱包卡代码优化 GetGunStatus函数100瓦按1w干扰处理 起步金代码小bug处理 SendHeartBeatFun一次发送12个枪头数据 GunChargingProc如果充电功率5分钟平均小于3.5w，就认为枪头拔出 修改月卡不能开启充满自停问题
        维护界面按8进入按确认重启系统 Sc8042bSpeech测试模式不关闭语音 按键板下电的同时重启按键板串口 Sc8042bSpeech语音禁播控制不准问题
021.7   ProcKbStatus对按键板下电先把串口txrx管脚拉低 SwitchToUi_SocketOccupy输入插座号递归调用死机问题修改 把UiStateLoop放在按键线程,解决固件升级不能刷新进度条问题 MakeTradeInfo/BlueStopCharging修复蓝牙结束上报起步金模式费用不对问题
021.8   ResetKeyBoard系统上电先下单按键板再上电按键板 CardModuleHandle没有计费模版不能刷卡充电 Sc8042bSpeech修改禁音时间不能为0问题
022.0   充电时长去除掉电时长功能 ChargingProc充电判断统一用金额进行判断,月卡充电把金额设置为99元,避免金额不够停止充电 后台及蓝牙心跳报文里面增加充电中剩余充电时长 维护界面按7进入打印开关设置 UiDisplay_PosData解决刷卡金额太大死机问题 在system_info中存放枪头信息的校验值
022.1   订单缓存信息增加校验及恢复机制，订单增加校验，如果失败就丢弃报文 flash及eeprom读取修改为逐个字节读取 电流最大修改为5.5A 每隔1024秒对计量芯片强制重新初始化校表参数 ProcFuseBreak保险丝烧断检测修改为3次
022.2   整合x10c GetPktSum计算校验和修改 MainTask获取密码成功就发送登录消息 FtpGet函数修改某些大小特殊的升级文件不能正常升级的问题
022.3   MainTask优化获取到id2就发送密码请求消息
022.4   心跳报文90秒发送一次 枪头数据写入flash后，系统信息里面把所有枪头的校验值都重新计算更新一次
022.5   MainTask增加每64秒检查网络链路是否断开 应急码功能合入
022.6   应急码加入校验功能 增加蓝牙dfi设置充满时间和拔枪停止充电时间 emuTask每64秒强制校表一次
023.0   继电器测试开启充电3分钟后自动停止 应急码开启的插座不能重复使用应急码开启充电 ProcGunStatus应急码开启的插座，应急时间到了如果还有车在充电也不关闭 应急码按键输入增加屏幕显示功能
023.1   TestCloseGunProc远程全部关闭继电器处理完善 SysCfgInit充电参数初始化优化
023.3   修改拔枪停止订单时间阈值为5分钟 完善计量芯片校表功能 起步金收费问题修改 后台多次应答停止充电完成帧导致订单指针异常问题修改 拔枪停止充电平均功率调节为2.5w
023.4   SendHistoryOrder修改订单发送后误清枪头信息的bug
023.5   GunChargingProc检测到枪头断开超过30秒执行重新校表
023.6   StartCharging启动充电清除启动充电发送次数
023.7   EepromWriteData完善eeprom写操作，增加失败重试
024.0   CheckBlueInfo配置蓝牙前先判断蓝牙名称是否合法 emuTask芯片校准参数重新配置前增加模块软重启 RestoreGunStatus重新上电修改枪头状态为未检测到功率 SendStartUpNotice桩号强制清零前3字节 GunChargingProc检测到枪头断开先关闭继电器 CkbTask线程加大栈空间解决双离线蓝牙订单上报导致系统复位问题
024.4   readAlldata电流修改为浮点数 SendStartUpNotice上报禁音设置时间 RecvServerData完善请求id2密钥原因
024.6   固定计费取消拔枪检测 实时计费默认开启充满自停 GetGunStatus降低拔枪阈值 StartCharging分功率计费预置起步金时间45分钟，防止为0误结束充电 起步金模式在起步金时间内不判断拔枪结束
024.7   浮充修改为20w
024.8   固定计费拔枪后不开关继电器 并把5分钟开关继电器4次改成2次
024.9   专网SendProtoPkt如果是走485就不置加密标志
025.0   RestoreGunStatus修改先置充电标志再计算校验和导致校验和错误的问题
026.0   固定计费模式增加拔枪检测及充满自停开关可配置 SendHeartBeatFun加密版本对心跳报文加密处理 对桩号/识别码等重要信息增加一个flash扇区来备份保存处理，防止eeprom信息异常丢失及进行恢复
026.4   UiKeyHandle_InputChargerSn修改桩号备份处理错误问题
026.7   RemoteCtrlProc语音禁播设置增加返回应答 SendStartUpNotice上报拔枪检测和充满自停检测开关 RecvServerData密钥更新通知返回的应答报文序列号与请求保持一致
027.0   StartCharging控制变量更新放在设置枪状态之前,解决开启充电后又突然停止的问题
027.3   RecvServerData注册成功后保存识别码到备份flash区域，解决由于没有保存识别码而不断注册重启的问题
027.6   SetChargingTime对续冲时长设置为最小45分钟 GunChargingProc修改固定计费无法正常充满自停的问题
028.0   StartCharging实时和起步金模式下功率低于45w开始判满 GunChargingProc判断充满自停时功率变化大于6w才延长续充时间 增加温度大于65度停止充电功能 设置默认的续充时长为45分钟 计量算法优化
029.0   SpeechCardBalance优化刷卡余额语音播报函数 DecryptData函数修复报文长度从u16转u8出现精度丢失问题,解决线上多插座开启充电出现的异常离线问题 优化加密处理流程
029.3   ReadAuthCardIDHandle刷卡鉴权报文最多只发2次 StopCharging函数里面强制重新计算一次充电金额，避免出现充电时间和金额差一分钟问题 SendHeartBeatFun上报每路自学习标志
029.6   BlueStopCharging判断如果是空闲直接返回停止成功，如果是订单号不一致返回停止失败 由于可能存在误检测，关闭充电温度过高停止充电的检测
030.0   EmuMatrixInit对计量参数进行全局校验功能 充电结束订单上报实际充电时间 SendRegister注册消息增加硬件id字段 月卡充电的判满功率设置为20w,续冲时长最短设置为2小时 在事件通知里面增加上报密钥更新结果 SendRemoCtrlAck应答结构体修正
030.3   把结束充电通知里面实际充电时间的值从分钟修改为秒
031.0   拔枪时间增加到10分钟
032.0   BlueTimingOpen无蓝牙连接时每隔10分钟发送一次打开蓝牙广播的命令 WriteCfgInfo优化配置参数读写功能 SendBtRemoCtrlAck蓝牙打开插座应答报文结构体错误修改 BtSetPullGunTime开启蓝牙设置拔枪停止充电时间
        MakeTradeInfo充电结束通知上报拔枪停止充电、充满自停开关、拔枪停止充电时间等信息 GunChargingProc充电开始检测无电流播报提示语音 定时请求计费模板时间修改为12小时 增加月卡播放剩余次数语音
        固定计费模式下设置了关闭充满自停对月卡有效 登录命令增加固件下载ftp方式 修改信号量处理不能互斥问题，解决小概率网络异常重启现象 把结束充电通知里面实际充电时间的值从秒修改为分钟 增加后台计费模板id为0的语音提示
032.3   去掉拔枪开关继电器功能 修复远程启动充电时长处理不对问题 离线后网络恢复发送登录请求
033.0   对电子月卡设置4小时的续充时长 充满续充功率可设置 判断心跳应答里面的计费模板id如果和桩端不一致则向后台请求计费模板，去掉定时请求计费模板机制 优化首次上电小概率登录慢问题
        离线后的网络恢复，随机等待一定时间(2分钟之内)再重新发送数据到后台，避免后台故障恢复后桩端同时密集发送数据到后台，以缓解后台消息处理压力 日志打印开关开启4天后自动关闭
        定时检查tcp链路状态，链路异常马上进行重连，尝试短时间内恢复网络 增加超过24小时强制结束订单机制
034.0   mcu降频，解决低温异常复位问题
*/

#define SPECIAL_NET_CARD				0                   //0:公网卡    1:专网卡    2:X10c
#define ENVI_TYPE                       1                   //0:正式环境 1:测试环境 2:预发布环境 3:开发环境 4:测试环境2
#define FW_VERSION                      1
#define FW_VERSION_SUB                  (SPECIAL_NET_CARD)

#if ENVI_TYPE
	#define TEST_VERSION                    3		//
#endif

#if (1 == SPECIAL_NET_CARD) //====================== 专网卡 ========================
	#define X10C_TYPE                       0                   //0:正常x10  1:x10c
	#define ID2                             1                   //0:不加密	 1:加密
	#if (0 == ENVI_TYPE)
	#define NET_SERVER_IP                   "www.access.com"    //正式环境
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
#elif (0 == SPECIAL_NET_CARD) //====================== 公网卡 ========================
	#define X10C_TYPE                       0                   //0:正常x10  1:x10c
	#define ID2                             1                   //0:不加密	 1:加密

	#if (0 == ENVI_TYPE)
	#define NET_SERVER_IP                   "access.sharecharger.com"     //正式环境
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
	#define X10C_TYPE                       1                   //0:正常x10  1:x10c
	#define NET_SERVER_IP                   "www.access.com"     //正式环境
	#define NET_SERVER_PORT                 10010
	#define ID2                             0                    //0:不加密	 1:加密
//	#define REGISTER_CODE					"830484693193"
#endif

#define IS_CPU_DOU_FRE                  	1                   //是否倍频  0不倍频；  1倍频
#if (0 == IS_CPU_DOU_FRE)
	#define VOICE_DELAY_DIV               	2
#else
	#define VOICE_DELAY_DIV               	1
#endif

#define IS_THREE_DIAL                   0
#define PUT_OUT_GUN_POWER               15  //0.1w
#define PUT_OUT_GUN_TIME                300  //秒

#define CHARGING_FULL_TIME              (45*60) //秒
#define CLOSE_VIOCE_START_IIME          23
#define CLOSE_VIOCE_END_IIME            7

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;

#define MAGIC_NUM_BASE                  0x123456ab
#define STATION_MACHINE_TYPE            "DEVICE_CHARGER_CL_XA"
#define CHARGER_TYPE                    2  //1:x9；2:x10
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
#define CHARGER_SN_LEN			        8   //bcd的长度
#define EVEN_DISCRI_LEN                 32
#define OUT_NET_PKT_LEN                 350
#define NET_485_PKT_LEN                 (1024+32)
#define BLUE_VERSION_LEN                20
#define BLUE_NAME_LEN                   10
#define BLUE_GWADDR_LEN                 5
#define START_POWER_LIMIT               7800  //开启充电整桩最大功率限制 w
#define CHARGING_POWER_LIMIT            8700  //整桩最大功率限制 w


#define setbit(x,y) 					x |= (1<<y)           //将X的第Y位置1
#define clrbit(x,y) 					x &= ~(1<<y)          //将X的第Y位清0
#define revbit(x,y) 					x ^= (1<<y)           //将X的第Y位反转


enum{
	FW_X10P = 0,  		//X10P
	FW_X10_KEY_BOARD,  	//刷卡版
	DW_NUM,     		//固件个数
};

typedef struct {
    uint8_t  status;                            //信号量状态 0:空闲；非0:占用
    uint8_t  semTakeCnt;
    TaskHandle_t pTaskHandle;                   //占用的任务句柄
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







