/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。                   
*  作者 : 崔俊杰 junjie.cui@neuroxess.com
*  描述 : G3Controller的具体实现
*  修改记录: 
*  
******************************************************************************/
#pragma once
#include "nx_g3_controller_global.h"
#include "HardwareDefine.h"
#include "G3_Protocol.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <queue>
#include <QObject>
typedef enum
{
	RANGE_MAX = 0,		// 最大量程±6mV[15:6]
	RANGE_200UV = 1,	// ±200μV[9:1]
	RANGE_400UV = 2,	// ±400μV[10:2]
	RANGE_3MV = 3,		// ±3mV[14:5]
}SampleRange;
class   G3Controller : public QObject
{
	Q_OBJECT
public:
	G3Controller();
    ~G3Controller() ;
	// 设置要传输通道数据  按序号排列 1-64  需要传输值为1.
	void  selectChannel(uint64_t data);
	//获取蓝牙温度
	double getBleTemperature();
	//获取蓝牙信号质量 
	int  getBleRssi();
	//获取蓝牙电量
	int getBlePower();
	//设置 蓝牙UUID
	void  setBleUUID(std::string uuid);
	//设置蓝牙名称
	void setBleName(std::string bleName);
	//设置蓝牙关机
	void setBlePowerOff();
	//设置蓝牙开机
	void setBlePowerOn();
	//获取蓝牙是否在充电状态
	bool  getBleIsCharging()const;
	// 获取Icu 状态
	bool getIcuStatus()const;
	//通过来判断是否开始采集  停止采集选择false
	void  samplingControl(bool startSampling) ;
	//开始采集
	void startRecord();
	//停止采集
	void stopRecord();;
	long readDataBlocksRaw(int numBlocks, uint8_t* buffer) ;
	//设置采样率  默认1KHZ
	//设置阻抗测试模式   
	void setImpedanceMode();
	//读取采集数据;
	void readRecordBlock(uint8_t* buffer);
	//获取FPGA版本;
	std::string getFPGAVersion();
	//获取icu版本;
	std::string getIcuVersion();
	//获取stm32版本;
	std::string getIcuMcuVersion();
	//获取充放电电流
	double getDischargeChargeCurrent();
	//获取主板温度
	double getBoardTemp();
	//获取探头电流
	double getProbeCurrent();
	  //获取PTX VDDC 电压
	double getPtxVddcVoltage();
	//获取ICU电池电压
	double getIcuBatteryVoltage();
	   //fpga 升级
	void updateFPGA(std::string fileName);
	//蓝牙mcu升级
	void updateBleMcu(std::string fileName);
	//ICU主控MCU升级
	void updateIcuMcu(const std::string& fileName);
	//获取升级进度
	int  getUpdateProcess();
	//获取设备信息
	std::string  getDeviceInfo();
	//写入设备信息
	void   setDeviceInfo(uint8_t* deviceInf);
	//蓝牙是否连接
	bool isConnect();
	//检查ICU 状态 是否在采集 0 为非采集状态
	int checkICUStatus();
	//判断	//蓝牙是否连接
	bool devIsConnect()const;
	//连接设备
	Q_INVOKABLE  bool connectDevice();
	//断开设备
	void disConnectDevice();
	//开启广播
	bool enableNotifications();
	//设置蓝牙MAC地址
	void setBleMacAdd(const uint64_t address);
	void setReadUUID(std::wstring charReadUUID);
	void setWriteUUID(std::wstring charWriteUUID);
	void setFrameCallback(std::function<void(uint8_t*, size_t)> cb);  // 注册回调
	void setTimeOut(int time);//0-60
	void setPowerOff();
	int  getPowerSOH();//0-100
	// MONO使用 屏蔽这部分代码
	void setIntanID();//设置探头ID TODO
	void getIntanID();//获取探头ID TODO
	void checkIcu(const char* password);
	void setCurrentTime();
	std::string formatTimestamp(uint64_t millis);
	void getIcuLog();
	void startImpedance();
	void setFPGAReg(uint8_t address,uint32_t val);
	uint32_t getFPGAReg(uint8_t address);
	void setFilterLowerCutoff(double lowerBandwidth);//设置低切参数
	void setDspCutoffFreq(double desiredCutoffFreq);  //设置截止频率	
	void getSensorData();
signals:
	void blePairedIdChanged(const QString& id);
private:
	winrt::Windows::Devices::Bluetooth::BluetoothLEDevice device{ nullptr };
	int isUpdateSuccess();
	std::string getPID();
	std::string getDeviceId();
	std::string getVID();
	std::string getBoardId(int portIndex);
	bool uploadFPGABitfile(const std::string& filename);
	void resetBoard();
	void enableDataStream(int stream, bool enabled);
	void enableDac(int dacChannel, bool enabled);
	unsigned int numWordsInFifo();
	void forceAllDataStreamsOff();
	void  setWireInValue();
	void  activeTriggerIn();
	void run();
	void setContinuousRunMode(bool continuousMode);
	void setMaxTimeStep(unsigned int maxTimeStep);
	void setDspSettle(bool enabled);
	int  getChannelNumber();
	uint8_t* hexStringToUint8Ptr(const std::wstring& hexStr, size_t& length);
	void transG3Frame2IntanFrame(unsigned char* sampleBuffer, unsigned  char* dataBlockBuffer) const;
	bool writeData(const uint8_t* data, int dataLen);;
	std::vector<uint8_t> readBleData();
	int getG3FrameSizeInBytes() const;
	void getOneFrame(uint8_t* data, Frame* sampleFrame);
	static  int getFrameLen(const Frame* frame) { return FRAME_HEADER_LEN + frame->dataLen + DATA_CRC_LEN; }
	bool sendFrame(Frame* frame);
	void getFrameData(uint8_t* data);
	void goOneFrame(uint8_t* data, uint32_t timestamp);
	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic currentReadCharacteristic{ nullptr };
	void Characteristic_ValueChanged(
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic sender,
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs args);
private:
	bool is_connected;
	std::vector<uint8_t> rspBuffer;
	BleDeviceInfo currentDev;
	uint32_t timestamp = 0;
	uint32_t FPGARegValue = 0;
	int process =0;
	int channelNumber = 64;
	uint64_t header = 0xd7a22aaa38132a53UL;
	uint64_t noFrameHeader = 0xd7a22aaa38132a57UL;
	uint16_t sendSeq = 0;
	int failCount = 5;
	winrt::event_token notificationToken;
	std::queue<std::vector<uint8_t>> sampleDataVector;
	std::function<void(uint8_t*, size_t)> frameCallback;  // 回调函数
	std::string deviceInfoString="";
	int IcuStatus;
	double IcuBatteryVoltageInt=0.0;
	double IcuBatteryVoltageIntSTM=0.0;
	double PtxVddcVoltageInt=0.0;
	double ProbeCurrent;
	double BoardTemp=0.0;
	double DischargeChargeCurrentInt=0.0;
	std::string IcuMcuVersion="";
	std::string IcuBleVersion="";
	std::string FPGAVersion="";
	int BleRssi=0;
	int BlePower=0;
	double bleTemperatureInt=0.0;
	int IcuBattery=100;
	uint64_t desiredChannelMask;
	uint64_t actualChannelMask;
	volatile bool inSampling{ false };
	SampleRange sampleRange{ RANGE_MAX };
	QString m_serialId = "ABC123456789";  // 模拟序列号
	
};

