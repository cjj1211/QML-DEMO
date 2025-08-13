// ReSharper disable CppExpressionWithoutSideEffects
#include "CG3Controller.h"
#include <bitset>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#define SAMPLE_DATA_MS		1000
static std::string g_csvFilename;  // 全局唯一 CSV 文件名
static std::string g_csvSensorFilename;  // 全局唯一 CSV  文件名     
uint32_t sampleSeq = 0;
int missCount = 0;
int receiveSampleCount = 0;
int allReceiveSampleCount = 0;
int allMissCountReceiveSampleCount;
std::string SensorFilename() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm tm_time;
#ifdef _WIN32
	localtime_s(&tm_time, &in_time_t);
#else
	localtime_r(&in_time_t, &tm_time);
#endif

	std::ostringstream oss;
	oss << "sensor_" << std::put_time(&tm_time, "%Y-%m-%d_%H_%M_%S") << ".csv";
	return oss.str();
}
G3Controller::G3Controller() :is_connected(false)
{


	currentDev.charReadUUID = L"{6e400003-b5a3-f393-e0a9-e50e24dcca9e}";
	currentDev.charServiceUUID = L"{6e400001-b5a3-f393-e0a9-e50e24dcca9e}";
	currentDev.charWriteUUID = L"{6e400002-b5a3-f393-e0a9-e50e24dcca9e}";
	currentDev.mac_address = 0xd98d8b92e1a4;
	/*g_csvSensorFilename = SensorFilename();
	bool needHeader = !std::filesystem::exists(g_csvSensorFilename);
	std::ofstream csvFile(g_csvSensorFilename, std::ios::app);*/
	//if (!csvFile.is_open()) {
	//	std::cerr << "无法打开CSV文件: " << g_csvSensorFilename << std::endl;
	//	return;
	//}

	//// 写入表头
	//if (needHeader) {
	//	csvFile << "时间戳,温度/℃,湿度/%RH,电池电压/V,蓝牙信号强度/dBm,丢包率\n";
	//}
	
}

G3Controller::~G3Controller()
{
	//delete dev;
	//dev = nullptr;
}


std::string getFormattedTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;
	std::tm tm_time;
#ifdef _WIN32
	localtime_s(&tm_time, &in_time_t);
#else
	localtime_r(&in_time_t, &tm_time);
#endif

	std::ostringstream oss;
	oss << std::put_time(&tm_time, "%Y-%m-%d %H-%M-%S");
	oss << "." << std::setfill('0') << std::setw(3) << ms.count();  // 添加毫秒
	return oss.str();
}

// 生成用于文件名的时间戳（格式：frame_YYYY-MM-DD_HHMMSS.csv）
std::string generateFilename() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm tm_time;
#ifdef _WIN32
	localtime_s(&tm_time, &in_time_t);
#else
	localtime_r(&in_time_t, &tm_time);
#endif

	std::ostringstream oss;
	oss << "frame_" << std::put_time(&tm_time, "%Y-%m-%d_%H_%M_%S") << ".csv";
	return oss.str();
}

// 写入帧数据到 CSV 文件
void appendFrameDataToCSV(const uint8_t* data, size_t size, size_t channelsPerFrame, uint32_t frameIndex) {


	size_t bytesPerFrame = channelsPerFrame * 2;
	size_t frameCount = size / bytesPerFrame;

	if (size % bytesPerFrame != 0 || size < bytesPerFrame) {
		std::cerr << "数据大小不符合通道数量要求" << std::endl;
		return;
	}
	// 第一次调用时生成文件名
	if (g_csvFilename.empty()) {
		g_csvFilename = generateFilename();
	}

	bool needHeader = !std::filesystem::exists(g_csvFilename);
	std::ofstream csvFile(g_csvFilename, std::ios::app);
	if (!csvFile.is_open()) {
		std::cerr << "无法打开CSV文件: " << g_csvFilename << std::endl;
		return;
	}

	// 写入表头
	if (needHeader) {
		csvFile << "时间戳,帧序号";
		for (size_t i = 1; i <= channelsPerFrame; ++i) {
			csvFile << ",通道" << i;
		}
		csvFile << "\n";
	}
	for (size_t f = 0; f < frameCount; ++f) {

		std::string timestamp = getFormattedTimestamp();

		csvFile << timestamp << "," << frameIndex + f;  // 每帧都加 index

		// 获取当前帧的起始位置
		const uint8_t* framePtr = data + f * bytesPerFrame;

		// 对每帧中的每个通道数据进行写入
		for (size_t i = 0; i < channelsPerFrame; ++i) {
			size_t offset = i * 2;
			uint8_t lowByte = framePtr[offset + 1];     // 低字节
			uint8_t highByte = framePtr[offset];        // 高字节
			uint16_t value = (static_cast<uint16_t>(highByte) << 8) | lowByte;
			/*LOG_INFO(std::format(" timestamp:{},frameIndex:{},vaule :{}", timestamp, frameIndex,value).data());*/
			csvFile << "," << (value - 32768) * 0.195;  // 将每个通道数据写入当前行
		}
		csvFile << "\n";
		// 每处理完一帧数据，就换行
	}

	csvFile.close();
}
std::string G3Controller::getPID()
{
	return "G3";
}

std::string G3Controller::getDeviceId()
{
	return "G3";
}

std::string G3Controller::getVID()
{
	return "NeuroXess";
}

std::string G3Controller::getBoardId(int portIndex)
{
	return "G3_BoardId";
}

bool G3Controller::uploadFPGABitfile(const std::string& filename)
{
	return true;
}

void G3Controller::resetBoard()
{

	//TODO
}


void G3Controller::samplingControl(bool startSampling)
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_SAMPLING_CONTROL;
		frame->dataLen = 1;
		frame->data[0] = startSampling;
		sendFrame(frame);
	}
}

void G3Controller::run()
{

}

void G3Controller::setWireInValue()
{



}

void G3Controller::activeTriggerIn()
{

	/*dev->activeTriggerIn(TrigInSpiStart, 0);*/
}

void G3Controller::startRecord()
{
	samplingControl(true);
	sampleSeq = 0;
	//g_csvFilename = generateFilename();
	IcuStatus = 1;

}

void G3Controller::stopRecord()
{
	samplingControl(false);
	IcuStatus = 0;
	sampleSeq = 0;
	
	double missPrecent = allMissCountReceiveSampleCount * 100.0 / (allMissCountReceiveSampleCount + allReceiveSampleCount);
	if (allMissCountReceiveSampleCount + allReceiveSampleCount == 0)
	{
		
	}
	else
	{
		
	}


}

long G3Controller::readDataBlocksRaw(int numBlocks, uint8_t* buffer)
{

	return numBlocks;
}

void G3Controller::setContinuousRunMode(bool continuousMode)
{
	return;


	/*if (continuousMode) {
		dev->setWireInValue(WireInResetRun, 0x02, 0x02);
	}
	else {
		dev->setWireInValue(WireInResetRun, 0x00, 0x02);
	}
	dev->updateWireInValue();*/
}

void G3Controller::setMaxTimeStep(unsigned int maxTimeStep)
{
	return;
	;


}


//bool G3Controller::setSampleRate(AmplifierSampleRate newSampleRate)
//{
//	
//	return true;
//}

void G3Controller::enableDataStream(int stream, bool enabled)
{
	return;
}

void G3Controller::enableDac(int dacChannel, bool enabled)
{
	return;
}

uint64_t extandChannelMask(uint64_t desiredChannelMask)
{
	uint64_t extandedMask = desiredChannelMask;
	int remainder = std::popcount(desiredChannelMask) % 4;

	if (remainder > 0)
	{
		int extandNum = 4 - remainder;

		int j = 0;
		for (int i = 0; i < 64 && (j < extandNum); i++)
		{
			if ((extandedMask & (1ULL << i)) == 0)
			{
				extandedMask |= (1ULL << i);
				j++;
			}
		}
		return extandedMask;
	}
	else
	{
		return desiredChannelMask;
	}

}
void G3Controller::selectChannel(uint64_t data)
{
	
	channelNumber = 0;
	desiredChannelMask = data;
	actualChannelMask = extandChannelMask(desiredChannelMask);  // TODO
	std::vector<int> channel;
	for (int i = 0; i < 64; i++)
	{
		if (actualChannelMask & (1ULL << i)) // 直接判断bit是否为1
		{
			channel.push_back(1);
			channelNumber += 2;  // 确保这个逻辑是正确的
		}
		else
		{
			channel.push_back(0);
		}
	}

	// 直接使用 data 进行 64-bit 拆分成 8 字节
	std::array<uint8_t, 8> result{};
	for (int i = 0; i < 8; ++i) {
		result[i] = (actualChannelMask >> (i * 8)) & 0xFF;
	}
	
	static EcuErrorCode errorCode;
	if (devIsConnect())
	{
		uint8_t data1[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data1);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_SELECT_CHANNEL;
		frame->dataLen = 8;
		uint8_t* content = static_cast<uint8_t*>(data1) + FRAME_HEADER_LEN;
		memcpy(content, &result, 8);
		sendFrame(frame);
	
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		auto readchannel = readBleData();
		if (readchannel.size() < FRAME_HEADER_LEN)
		{
		
		}
		else
		{
			uint8_t* ptr = readchannel.data();
			const auto frame1 = reinterpret_cast<Frame*>(ptr);
			
		}
	}
}

double G3Controller::getBleTemperature()
{

	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_TEMPERATURE;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return  bleTemperatureInt;

}
uint8_t* G3Controller::hexStringToUint8Ptr(const std::wstring& hexStr, size_t& length) {
	length = hexStr.length() / 2;
	uint8_t* buffer = static_cast<uint8_t*>(malloc(length));

	if (!buffer) {

		std::wcout << L"Memory allocation failed!" << std::endl;
		return nullptr;
	}

	for (size_t i = 0; i < length; i++) {
		std::wstring byteStr = hexStr.substr(i * 2, 2);
		buffer[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
	}

	return buffer;
}
int G3Controller::getBleRssi()
{

	if (devIsConnect())
	{
	
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = 0x17;
		frame->dataLen = 1;
		frame->data[0] = 0;
	
		sendFrame(frame);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	}
	return BleRssi;
}

int G3Controller::getBlePower()
{

	if (devIsConnect())
	{

		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = 0x18;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return BlePower;
}

void G3Controller::setBleUUID(std::string uuid)
{
}

void G3Controller::setBleName(std::string bleName)
{
}

void G3Controller::setBlePowerOff()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_OPENICU;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
	}
}
void G3Controller::setBlePowerOn()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_OPENICU;
		frame->dataLen = 1;
		frame->data[0] = 1;
		sendFrame(frame);
	}
}

bool G3Controller::getBleIsCharging()const
{
	return  false;
}

bool G3Controller::getIcuStatus() const
{
	return  true;

}

void G3Controller::setImpedanceMode()
{

}

void G3Controller::readRecordBlock(uint8_t* buffer)
{
	//getOneFrame(buffer);
}

std::string G3Controller::getFPGAVersion()
{
	if (devIsConnect())
	{

		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_VERSION;
		frame->dataLen = 1;
		frame->data[0] = 2;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		auto fpgaVersion = readBleData();

		uint8_t* ptr = fpgaVersion.data();
		const auto frame1 = reinterpret_cast<Frame*>(ptr);
		if (fpgaVersion.size() < FRAME_HEADER_LEN)
		{
		
			return  "";
		}
		else
		{
			if (frame1->dataLen < 4)
			{
				return  "";
			}
			FPGAVersion = std::to_string(static_cast<int>(frame1->data[1])) + "." +
				std::to_string(static_cast<int>(frame1->data[2])) + "." +
				std::to_string(static_cast<int>(frame1->data[3])) + "." +
				std::to_string(static_cast<int>(frame1->data[4]));
			if (FPGAVersion == "0.0.0.0" || FPGAVersion == "255.255.255.255")
			{
				FPGAVersion = "1.0.0";
			}
			FPGAVersion = "1.0.0";
		
			return  FPGAVersion;

		}
	}
	else
	{
		return  "1.0.0";
	}

}

std::string G3Controller::getIcuVersion()
{
	
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_VERSION;
		frame->dataLen = 1;
		frame->data[0] = 1;
	
		sendFrame(frame);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		auto fpgaVersion = readBleData();
		uint8_t* ptr = fpgaVersion.data();
		const auto frame1 = reinterpret_cast<Frame*>(ptr);
		if (frame1->dataLen < 4)
		{
			return "";
		}

		IcuBleVersion = std::to_string(static_cast<int>(frame1->data[1])) + "." +
			std::to_string(static_cast<int>(frame1->data[2])) + "." +
			std::to_string(static_cast<int>(frame1->data[3])) + "." +
			std::to_string(static_cast<int>(frame1->data[4]));
	
		if (IcuBleVersion == "0.0.0.0" || IcuBleVersion == "255.255.255.255")
		{
			IcuBleVersion = "1.0.0.0";
		}
		IcuBleVersion = "1.0.0";
		return  IcuBleVersion;
	}
	else
	{
		return  "1.0.0";
	}
}

std::string G3Controller::getIcuMcuVersion()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_VERSION;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		auto fpgaVersion = readBleData();
		uint8_t* ptr = fpgaVersion.data();
		const auto frame1 = reinterpret_cast<Frame*>(ptr);
		if (frame1->dataLen < 4)
		{
			return "";
		}

		IcuMcuVersion = std::to_string(static_cast<int>(frame1->data[1])) + "." +
			std::to_string(static_cast<int>(frame1->data[2])) + "." +
			std::to_string(static_cast<int>(frame1->data[3])) + "." +
			std::to_string(static_cast<int>(frame1->data[4]));
		
		if (IcuMcuVersion == "0.0.0.0" || IcuMcuVersion == "255.255.255.255")
		{
			IcuMcuVersion = "1.0.0";
		}
		IcuMcuVersion = "1.0.0";
		return  IcuMcuVersion;
	}
	else
	{
		return  "1.0.0";
	}
}

double G3Controller::getDischargeChargeCurrent()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_BATTERY_CURRENT;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return  DischargeChargeCurrentInt;

}

double G3Controller::getBoardTemp()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_TITANIUM_TEMP;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return  BoardTemp;
}

double G3Controller::getProbeCurrent()
{
	if (devIsConnect())
	{

		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_INTAN_ELECTRUC;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	}
	return ProbeCurrent;
}

double G3Controller::getPtxVddcVoltage()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_PTX_VDDC_VOLTAGE;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return  PtxVddcVoltageInt;
}

double G3Controller::getIcuBatteryVoltage()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_ICU_BATTERY_VOLTAGE;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return  IcuBatteryVoltageInt;
}

void G3Controller::updateFPGA(std::string fileName)
{
	return;
}

void G3Controller::updateBleMcu(std::string fileName)
{

	return;
}

void G3Controller::updateIcuMcu(const std::string& fileName)
{

	return;
}

int G3Controller::getUpdateProcess()
{

	return  1;
}

bool G3Controller::isConnect()
{
	return devIsConnect();
}

int G3Controller::checkICUStatus()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_ICU_STATUS;
		frame->dataLen = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return  IcuStatus;
}

bool G3Controller::devIsConnect() const
{
	return  is_connected;
}

#include <future>
auto getGattServicesSync = [](winrt::Windows::Devices::Bluetooth::BluetoothLEDevice device) {
	return std::async(std::launch::async, [device] {
		return device.GetGattServicesAsync(winrt::Windows::Devices::Bluetooth::BluetoothCacheMode::Uncached).get();
	}).get();
};
auto getGattCharacteristicsSync = [](winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService service) {
	return std::async(std::launch::async, [service] {
		return service.GetCharacteristicsAsync(winrt::Windows::Devices::Bluetooth::BluetoothCacheMode::Uncached).get();
	}).get();
};
auto writeGattDescriptorSync = [](winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic, winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue config) {
	return std::async(std::launch::async, [characteristic, config] {
		return characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(config).get();
	}).get();
};
auto writeGattSync = [](auto characteristic, auto buffer) {
	return std::async(std::launch::async, [characteristic, buffer] {
		return characteristic.WriteValueAsync(buffer, winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption::WriteWithoutResponse).get();
	}).get();
};

auto readGattCharacteristicSync = [](auto characteristic) {
	return std::async(std::launch::async, [characteristic] {
		// Perform the asynchronous read and block until it completes
		return characteristic.ReadValueAsync(winrt::Windows::Devices::Bluetooth::BluetoothCacheMode::Cached).get();
	}).get();
};
bool G3Controller::connectDevice()
{
	emit blePairedIdChanged(m_serialId);
	return true;

}
void G3Controller::disConnectDevice()
{
	if (!is_connected) return;
	device = nullptr;
	is_connected = false;
}

// 在 DirectBLEDevice 类中添加一个辅助函数来将 UUID 转换为字符串
std::wstring GuidToString(const winrt::guid& guid) {
	wchar_t buffer[39];
	swprintf_s(buffer, 39, L"{%08x-%04x-%04x-%04x-%012llx}",
		guid.Data1, guid.Data2, guid.Data3,
		(guid.Data4[0] << 8) | guid.Data4[1],
		(static_cast<uint64_t>(guid.Data4[2]) << 40) |
		(static_cast<uint64_t>(guid.Data4[3]) << 32) |
		(static_cast<uint64_t>(guid.Data4[4]) << 24) |
		(static_cast<uint64_t>(guid.Data4[5]) << 16) |
		(static_cast<uint64_t>(guid.Data4[6]) << 8) |
		static_cast<uint64_t>(guid.Data4[7]));
	return std::wstring(buffer);
}

bool G3Controller::writeData(const uint8_t* data, int dataLen)
{
	//LOG_INFO("write data -----------");
	if (!is_connected)
	{
		
		return false;
	}
	try {
		auto servicesResult = getGattServicesSync(device);
		if (servicesResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {

		
			return false;
		}

		// 声明变量前获取集合，然后使用索引遍历
		auto services = servicesResult.Services();
		//	LOG_INFO(" get services -----------");
		for (uint32_t i = 0; i < services.Size(); i++) {
			// 完全初始化每个变量后再使用
			auto service = services.GetAt(i);

			auto charsResult = getGattCharacteristicsSync(service);
			if (charsResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
				continue;
			}

			auto characteristics = charsResult.Characteristics();
			for (uint32_t j = 0; j < characteristics.Size(); j++) {
				auto characteristic = characteristics.GetAt(j);
				// 将UUID转换为字符串比较
				std::wstring uuidString = GuidToString(characteristic.Uuid());
				std::string charWriteUUID = winrt::to_string(uuidString);
				std::string charcurrentWriteUUID = winrt::to_string(currentDev.charWriteUUID);
				if (uuidString == currentDev.charWriteUUID) {
	
					// 使用Windows::Storage::Streams命名空间
					winrt::Windows::Storage::Streams::DataWriter writer;
					
					// 依次初始化写入缓冲区
					for (size_t i = 0; i < dataLen; ++i) {

						writer.WriteByte(data[i]);
					}
					// 获取缓冲区并完成写操作
					auto buffer = writer.DetachBuffer();
					// 等待操作完成并获取结果
					auto writeResult = writeGattSync(characteristic, buffer);

					if (writeResult == winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
						return true;
					}
					else {
					
						return false;
					}
				}
			}
		}

		return false;
	}
	catch (winrt::hresult_error const& ex) {
		std::wcout << ("Write error");
		return false;
	}

}

std::vector<uint8_t> G3Controller::readBleData()
{
	if (!is_connected)
	{
	
		return {};
	}
	auto servicesResult = getGattServicesSync(device);
	if (servicesResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
	{
	
	}

	auto readResult = readGattCharacteristicSync(currentReadCharacteristic);// characteristic.ReadValueAsync(BluetoothCacheMode::Cached).get();
	if (readResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
	{
	
		return {};
	}

	// 解析数据
	auto buffer = readResult.Value();
	auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);
	std::vector<uint8_t> data;
	while (reader.UnconsumedBufferLength() > 0) {
		data.push_back(reader.ReadByte());
	}
	return data;
	auto services = servicesResult.Services();
	for (uint32_t i = 0; i < services.Size(); i++) {
		auto service = services.GetAt(i);
		auto charsResult = getGattCharacteristicsSync(service);

		if (charsResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
			continue;
		}

		auto characteristics = charsResult.Characteristics();
		for (uint32_t j = 0; j < characteristics.Size(); j++) {
			auto characteristic = characteristics.GetAt(j);
			std::wstring uuidString = GuidToString(characteristic.Uuid());

			if (uuidString == currentDev.charReadUUID)
			{

				if (!characteristic) {

	
					return {};
				}

				// 读取特征值数据

			}
		}
	}
}

bool G3Controller::enableNotifications()
{
	if (!is_connected) {

		
		return false;
	}
	try {
		auto servicesResult = getGattServicesSync(device);
		if (servicesResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {

		
			return false;
		}

		auto services = servicesResult.Services();

		for (uint32_t i = 0; i < services.Size(); i++) {
			auto service = services.GetAt(i);
			std::wstring serviceUUID = GuidToString(service.Uuid());
			std::string currentserviceUUID = winrt::to_string(serviceUUID);
			// Only process the service with the desired UUID
			if (serviceUUID == currentDev.charServiceUUID) {
				// Fetch all characteristics of this service
				auto charsResult = getGattCharacteristicsSync(service);
				if (charsResult.Status() != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
				
					continue;
				}

				auto characteristics = charsResult.Characteristics();
				for (uint32_t j = 0; j < characteristics.Size(); j++) {
					auto characteristic = characteristics.GetAt(j);
					std::wstring uuidString = GuidToString(characteristic.Uuid());
					std::string characteristicUUID = winrt::to_string(uuidString);
					
					if (uuidString == currentDev.charReadUUID)
					{
						std::string charReadUUID = winrt::to_string(uuidString);
						
						currentReadCharacteristic = characteristic;
					}
					if (uuidString == currentDev.charWriteUUID)
					{
						std::string charWriteUUID = winrt::to_string(currentDev.charWriteUUID);
			
					};
					if (static_cast<int>(characteristic.CharacteristicProperties()) & static_cast<int>(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties::Notify)) {
						std::wstring charUUID = GuidToString(characteristic.Uuid());
						// Enable notifications
						auto config = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue::Notify;
						auto writeResult = writeGattDescriptorSync(characteristic, config);
						if (writeResult != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success) {
						
							return false;
						}
						else {

							this->notificationToken = characteristic.ValueChanged(
								winrt::Windows::Foundation::TypedEventHandler<
								winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic, winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs>(this, &G3Controller::Characteristic_ValueChanged)
							);

				
							//return  true;
						}
					}
				}
				return  true;
			}
		}
	}
	catch (winrt::hresult_error const& ex) {
		std::wcout << L"Error fetching UUIDs: " << ex.message().c_str() << std::endl;

	}
	return  false;
}

void G3Controller::setBleMacAdd(const uint64_t address)
{
	currentDev.mac_address = address;

}

void G3Controller::setReadUUID(std::wstring charReadUUID)
{
	currentDev.charReadUUID = charReadUUID;
}

void G3Controller::setWriteUUID(std::wstring charWriteUUID)
{
	currentDev.charWriteUUID = charWriteUUID;
}

void G3Controller::setFrameCallback(std::function<void(uint8_t*, size_t)> cb)
{
	frameCallback = std::move(cb);
}

void G3Controller::setTimeOut(int time)
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_SET_TIMEOUT;
		frame->dataLen = 2;
		frame->data[0] = 0x01;
		frame->data[1] = time;
		sendFrame(frame);
	}
}

void G3Controller::setPowerOff()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_OPENICU;
		frame->dataLen = 1;
		frame->data[0] = 0x00;
		sendFrame(frame);
	}
}

int G3Controller::getPowerSOH()
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_ICU_SOH;
		frame->dataLen = 1;
		frame->data[0] = 0x00;
		sendFrame(frame);
	}
	return IcuBattery;
}


void G3Controller::setIntanID()
{

}

void G3Controller::getIntanID()
{

}

void G3Controller::checkIcu(const char* password)
{
	const char* payload = "U2FsdGVkX19i9yJ/ff7H4HODByI94iF9Qw";
	uint8_t data1[WIRELESS_BUF_SIZE] = { 0x00 };
	const auto frame = reinterpret_cast<Frame*>(data1);
	frame->header = HEADER;
	frame->seq = 3;  // 可以根据需要设置序列号
	frame->type = FRAME_SEND_PASSWORD;
	frame->dataLen = static_cast<uint8_t>(strlen(payload));  // 注意长度不能超过255
	uint8_t* content = static_cast<uint8_t*>(data1) + FRAME_HEADER_LEN;
	memcpy(content, payload, frame->dataLen);
	sendFrame(frame);

}

void G3Controller::setCurrentTime()
{
	uint8_t data1[WIRELESS_BUF_SIZE] = { 0x00 };
	const auto frame = reinterpret_cast<Frame*>(data1);
	frame->header = HEADER;
	frame->seq = 1;  // 序列号自行设定
	frame->type = FRAME_SET_CURRENTTIME;  // 设定一个 type 表示“设置当前时间”
	frame->dataLen = sizeof(uint64_t);
	auto now = std::chrono::system_clock::now();
	auto epoch = now.time_since_epoch();
	uint64_t currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
	memcpy(data1 + FRAME_HEADER_LEN, &currentTimeMs, sizeof(currentTimeMs));
	sendFrame(frame);

}

std::string G3Controller::formatTimestamp(uint64_t millis)
{
	std::time_t seconds = static_cast<std::time_t>(millis / 1000);
	uint64_t milliseconds = millis % 1000;
	std::tm localTime;
#if defined(_WIN32) || defined(_WIN64)
	localtime_s(&localTime, &seconds);
#else
	localtime_r(&seconds, &localTime);
#endif
	std::ostringstream oss;
	oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
	oss << "." << std::setfill('0') << std::setw(3) << milliseconds;
	return oss.str();
}

void G3Controller::getIcuLog()
{
	uint8_t data1[WIRELESS_BUF_SIZE] = { 0x00 };
	const auto frame = reinterpret_cast<Frame*>(data1);
	frame->header = HEADER;
	frame->seq = 1;  
	frame->type = FRAME_GET_LOG;  
	frame->dataLen = 1;
	sendFrame(frame);

}
void recordDataToCSV(std::vector<std::vector<double>>& data) {

	// 第一次调用时生成文件名
	g_csvFilename = generateFilename();

	std::ofstream csvFile(g_csvFilename, std::ios::app);
	if (!csvFile.is_open()) {
		std::cerr << "无法打开CSV文件: " << g_csvFilename << std::endl;
		return;
	}

	// 写入表头
	for (size_t i = 0; i < 3; ++i) {
		csvFile << "CAP" << i;
		if (i < 2)
		{
			csvFile << ",";
		}
	}
	csvFile << "\n";

	for (int i = 0; i < data[0].size(); i++)
	{
		for (int j = 0; j < data.size(); j++)
		{
			csvFile << data[j][i] << ",";
		}
		csvFile << "\n";
	}

	csvFile.close();
}
void G3Controller::startImpedance()
{
	return;
	
}

void G3Controller::setFPGAReg(uint8_t address, uint32_t val)
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_FPGA_REG;
		frame->dataLen = 6;
		uint8_t* content = static_cast<uint8_t*>(data) + FRAME_HEADER_LEN;
		content[0] = 1;
		memcpy(content+1, &address, 1);
		memcpy(content+2, &val, 4);
		sendFrame(frame);
	}
}

uint32_t G3Controller::getFPGAReg(uint8_t address)
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_FPGA_REG;
		frame->dataLen = 2;
		frame->data[0] = 0;
		frame->data[1] = address;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		return FPGARegValue;
	}
}
int G3Controller::isUpdateSuccess()
{
	return  1;
}
std::string   G3Controller::getDeviceInfo()
{

	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_DEVICE_INFO;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	
	return  deviceInfoString;
}

void G3Controller::setDeviceInfo(uint8_t* deviceInf)
{
	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_DEVICE_INFO;
		frame->dataLen = 9;   // 记录长度
		memcpy(frame->data, deviceInf, 9);
		sendFrame(frame);
	}
}

void G3Controller::setDspSettle(bool enabled)
{
	return;
}

int G3Controller::getChannelNumber()
{

	return 1;//dev->getChannelNumber();
}


unsigned int G3Controller::numWordsInFifo()
{

	return 1;
}

void G3Controller::forceAllDataStreamsOff()
{
	return;
}

void G3Controller::transG3Frame2IntanFrame(unsigned char* sampleBuffer, unsigned char* dataBlockBuffer) const
{

}

int G3Controller::getG3FrameSizeInBytes() const
{
	return  1;
	//	return (4 + 2 + numDataStreams * RHXDataBlock::channelsPerStream(ControllerRecordUSB3) + 3 * numDataStreams / 2 + 2) * BytesPerWord;
}
void G3Controller::getFrameData(uint8_t* data) {
	// 分配扩展后的数据（每个通道扩展为 2 字节）
	uint16_t expandedData[64 * 2];

	// 处理每个通道的数据
	for (int i = 0; i < channelNumber; i++) {
		int byteOffset = (i / 4) * 5;  // 计算数据块起始索引
		int32_t channelValue = 0;
		// 根据不同的通道拼接规则
		switch (i % 4) {
		case 0:
			// 通道 1: B 低 2 位 + A
			channelValue = ((data[byteOffset + 1] & 0x03) << 8) | data[byteOffset];
			break;
		case 1:
			// 通道 2: C 低 4 位 + B 高 6 位
			channelValue = ((data[byteOffset + 2] & 0x0F) << 6) | ((data[byteOffset + 1] >> 2) & 0x3F);
			break;
		case 2:
			// 通道 3: D 低 6 位 + C 高 4 位
			channelValue = ((data[byteOffset + 3] & 0x3F) << 4) | ((data[byteOffset + 2] >> 4) & 0x0F);
			break;
		case 3:
			// 通道 4: E + D 高 2 位
			channelValue = (data[byteOffset + 4] << 2) | ((data[byteOffset + 3] >> 6) & 0x03);
			break;
		}

		// 扩展为16位（低6位补0）
		int32_t expandedValue;

		switch (sampleRange)
		{
		case RANGE_MAX:
			expandedValue = channelValue << 6;
			break;
		case RANGE_200UV:
			expandedValue = 0x8000 + ((channelValue - 512) << 1);
			break;
		case RANGE_400UV:
			expandedValue = 0x8000 + ((channelValue - 512) << 2);
			break;
		case RANGE_3MV:
			expandedValue = 0x8000 + ((channelValue - 512) << 5);
			break;
		default:
			expandedValue = channelValue << 6;
			break;
		}

		// 存储扩展后的 16 位数据
		expandedData[i] = expandedValue;
	}

	// 拷贝扩展后的数据回原数据数组
	memcpy(data, expandedData, channelNumber * 2);
	// 释放动态内存

}
void G3Controller::getOneFrame(uint8_t* data, Frame* sampleFrame)
{

	auto frameHeader = noFrameHeader;
	uint16_t* pWrite = (uint16_t*)data;
	if (devIsConnect())
	{
		const auto reader = reinterpret_cast<char*>(sampleFrame->data);
		const byte segmentType = reader[0];
		static  int seq = 0;
		static  int oldseq = 0;
		seq = sampleFrame->seq;
		receiveSampleCount+=2;
		allReceiveSampleCount += 2;

		if (seq - oldseq > 2)
		{
			//LOG_INFO(std::format("seq not continue, current seq {}, old seq{}", seq, oldseq).data());
			missCount += 2;
			allMissCountReceiveSampleCount += 2;
		}
		oldseq = seq;
		channelNumber = (sampleFrame->dataLen - 1) * 8 / 10;
		if (receiveSampleCount >= SAMPLE_DATA_MS * 60)
		{
			double missPrecent = missCount * 1.0 * 100 / (SAMPLE_DATA_MS * 60 * 1.0);
			auto now = std::chrono::system_clock::now();
			auto in_time_t = std::chrono::system_clock::to_time_t(now);

			std::tm tm_time;
#ifdef _WIN32
			localtime_s(&tm_time, &in_time_t);
#else
			localtime_r(&in_time_t, &tm_time);
#endif



			std::stringstream ss;
			ss << std::put_time(&tm_time, "%Y-%m-%d_%H_%M_%S");
			std::string currentTim = ss.str();
			receiveSampleCount = 0;
			missCount = 0;
		}
		
		// 16bit 数据 表示单通道 所以要2倍,
		uint8_t* predata = new uint8_t[channelNumber * 2];
		// predata[channelNumber * 2];
		//将原始数据 拷贝到 最后要处理的数据数组中
		memcpy(predata, sampleFrame->data + 1, sampleFrame->dataLen - 1);
		//数据还原  
		getFrameData(predata);
		static int frameCnt = 0;
		const int g3BlockSizeInBytes = (channelNumber * 2) + 4 + 2;
		if (sampleFrame->type == FRAME_READ_BLOCK_DATA)
		{
			frameHeader = header;
			pWrite[0] = (uint16_t)((frameHeader & 0x000000000000ffffUL) >> 0);
			pWrite[1] = (uint16_t)((frameHeader & 0x00000000ffff0000UL) >> 16);
			pWrite[2] = (uint16_t)((frameHeader & 0x0000ffff00000000UL) >> 32);
			pWrite[3] = (uint16_t)((frameHeader & 0xffff000000000000UL) >> 48);

			if (segmentType == 0x03 || segmentType == 0x00)
			{
				pWrite[4] = static_cast<uint16_t>((sampleSeq & 0x0000ffffU) >> 0);
				pWrite[5] = static_cast<uint16_t>((sampleSeq & 0xffff0000U) >> 16);
				pWrite[6] = static_cast<uint16_t>(channelNumber / 2);
				memcpy(&pWrite[7], predata, channelNumber * 2);;
				sampleSeq += 2;
			}
		}
		else
		{
			pWrite[0] = (uint16_t)((frameHeader & 0x000000000000ffffUL) >> 0);
			pWrite[1] = (uint16_t)((frameHeader & 0x00000000ffff0000UL) >> 16);
			pWrite[2] = (uint16_t)((frameHeader & 0x0000ffff00000000UL) >> 32);
			pWrite[3] = (uint16_t)((frameHeader & 0xffff000000000000UL) >> 48);
			pWrite += 4;
		}
		delete[] predata;
	}
	else
	{
		pWrite[0] = (uint16_t)((frameHeader & 0x000000000000ffffUL) >> 0);
		pWrite[1] = (uint16_t)((frameHeader & 0x00000000ffff0000UL) >> 16);
		pWrite[2] = (uint16_t)((frameHeader & 0x0000ffff00000000UL) >> 32);
		pWrite[3] = (uint16_t)((frameHeader & 0xffff000000000000UL) >> 48);
		pWrite += 4;
		return;
	}

}
void G3Controller::goOneFrame(uint8_t* data, uint32_t timestamp)
{
	constexpr uint64_t header = 0xd7a22aaa38132a53UL;
	uint16_t* pWrite = reinterpret_cast<uint16_t*>(data);
	// Write timestamp.
	pWrite[0] = static_cast<uint16_t>((timestamp & 0x0000ffffU) >> 0);
	pWrite[1] = static_cast<uint16_t>((timestamp & 0xffff0000U) >> 16);
	pWrite += 2;
	pWrite[0] = static_cast<uint16_t>(channelNumber / 2);
}

void G3Controller::Characteristic_ValueChanged(
	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic sender,
	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs args)
{
	// 读取数据
	auto buffer = args.CharacteristicValue();
	auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);
	std::vector<uint8_t> data;
	while (reader.UnconsumedBufferLength() > 0) {
		data.push_back(reader.ReadByte());
	}

	uint8_t* ptr = data.data();
	const auto frame = reinterpret_cast<Frame*>(ptr);
	if (frame->type == FRAME_GET_IMPEDANCE_VALUE)
	{
		rspBuffer = data;
	}
	if (frame->type == FRAME_FPGA_REG)
	{
		auto data = frame->data;
		uint32_t value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
		FPGARegValue = static_cast<int>(value);

	}
	if (frame->type == FRAME_READ_BLOCK_DATA)
	{
		//sampleDataVector.push(data);
		// 分配合适大小的缓冲区用于存储数据
		std::vector<uint8_t> frameBuffer(512);  // 假设 1024 字节足够
		uint8_t* framedata1 = frameBuffer.data();

		// 获取一帧数据
		getOneFrame(framedata1, frame);

		std::vector<uint16_t> finalFrameBuffer;
		uint16_t* wordPtr = (uint16_t*)framedata1;
		uint16_t* dataPtr = (uint16_t*)&framedata1[14];
		for (int i = 0; i < 7; i++)
		{
			finalFrameBuffer.push_back(wordPtr[i]);
		}
		int index = 0;
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < 64; i++)
			{
				if (actualChannelMask & (1ULL << i))
				{
					if ((desiredChannelMask & (1ULL << i)))
					{
						finalFrameBuffer.push_back(dataPtr[index]);
					}
					index++;
				}
			}
		}

		finalFrameBuffer[6] = channelNumber / 2 - std::popcount(desiredChannelMask ^ actualChannelMask);
		if (frameCallback && inSampling) {
			frameCallback((uint8_t*)finalFrameBuffer.data(), finalFrameBuffer.size() * 2);
		}

	}
	if (frame->type == FRAME_DEVICE_INFO)
	{
		DeviceInfo currentDeviceInfo;
		deviceInfoString = reinterpret_cast<char*>(frame->data);	

	}
	if (frame->type == FRAME_ICU_STATUS)
	{
		const auto power = frame->data[0];
	}
	if (frame->type == FRAME_GET_ICU_BATTERY_VOLTAGE)
	{

		IcuBatteryVoltageIntSTM = (frame->data[2] + frame->data[3] * 0.1);//STM32读取电压
		IcuBatteryVoltageInt= (frame->data[0] + frame->data[1] * 0.1); //电量计读取电压
	}
	if (frame->type == FRAME_GET_PTX_VDDC_VOLTAGE)
	{
		PtxVddcVoltageInt = (frame->data[0] + frame->data[1] * 0.1);
	}
	if (frame->type == FRAME_GET_INTAN_ELECTRUC)
	{
		ProbeCurrent = (frame->data[0]);
	}
	if (frame->type == FRAME_GET_TITANIUM_TEMP)
	{
		BoardTemp = (frame->data[0] + frame->data[1] * 0.1);
	}
	if (frame->type == FRAME_GET_BATTERY_CURRENT)
	{

		/*int16_t combinedValue = (static_cast<int16_t>(frame->data[1]) << 8) | frame->data[0];
		DischargeChargeCurrentInt = static_cast<double>(combinedValue);*/

		int16_t combinedValue = (static_cast<int16_t>(frame->data[1]) << 8) | frame->data[0];
		DischargeChargeCurrentInt = combinedValue;
	

	}
	if (frame->type == 0x17)
	{
		BleRssi = static_cast<int8_t>(frame->data[0]);
	}
	if (frame->type == 0x18)
	{
		BlePower = static_cast<int8_t>(frame->data[0]);;
	}
	if (frame->type == FRAME_GET_TEMPERATURE)
	{
		bleTemperatureInt = static_cast<double>(frame->data[0]) + static_cast<double>(frame->data[1]) * 0.1;
	}
	if (frame->type == FRAME_GET_HUMIDITY)
	{
		const auto reader = reinterpret_cast<char*>(frame->data);
		auto index = 0;
		double temp = static_cast<double>(frame->data[0]) + static_cast<double>(frame->data[1]) * 0.01;
		double humidity = static_cast<double>(frame->data[2]) + static_cast<double>(frame->data[3]) * 0.01;
		double powerBattery = static_cast<double>(frame->data[4]) + static_cast<double>(frame->data[5]) * 0.01;
		int8_t bleRssi = static_cast<int8_t>(frame->data[6]);
		double ntcBattery = static_cast<double>(frame->data[7]) + static_cast<double>(frame->data[8]) * 0.01;
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm tm_time;
#ifdef _WIN32
		localtime_s(&tm_time, &in_time_t);
#else
		localtime_r(&in_time_t, &tm_time);
#endif

		std::stringstream ss;
		ss << std::put_time(&tm_time, "%Y-%m-%d_%H_%M_%S");
		std::string currentTim = ss.str();
	
	}
}


bool G3Controller::sendFrame(Frame* frame)
{
	if (frame == nullptr) {
		return false;
	}
	if (!devIsConnect()) {
		return false;
	}
	// ① 分配足够的内存
	uint8_t* data = new uint8_t[FRAME_HEADER_LEN + frame->dataLen + 2];
	// ② 复制 frame 数据
	memcpy(data, frame, FRAME_HEADER_LEN + frame->dataLen);
	// ③ 追加 0x00 和 0x0A
	data[FRAME_HEADER_LEN + frame->dataLen] = 0x00;
	data[FRAME_HEADER_LEN + frame->dataLen + 1] = 0x0A;
	const auto errCode = writeData(data, FRAME_HEADER_LEN + frame->dataLen + 2);
	delete[] data;
	return errCode;

}

void onTimeout()
{


}
/**************************** 滤波参数计算 ***********************************/

double rLFromLowerBandwidth(double lowerBandwidth)
{
	double log10f = log10(lowerBandwidth);

	if (lowerBandwidth < 4.0) {
		return 1.0061 * pow(10.0, (4.9391 - 1.2088 * log10f + 0.5698 * log10f * log10f +
			0.1442 * log10f * log10f * log10f));
	}
	else {
		return 1.0061 * pow(10.0, (4.7351 - 0.5916 * log10f + 0.08482 * log10f * log10f));
	}
}

double lowerBandwidthFromRL(double rL)
{
	double a, b, c;

	// Quadratic fit below is invalid for values of RL less than 5.1 kOhm
	if (rL < 5100.0) {
		rL = 5100.0;
	}

	if (rL < 30000.0) {
		a = 0.08482;
		b = -0.5916;
		c = 4.7351 - log10(rL / 1.0061);
	}
	else {
		a = 0.3303;
		b = -1.2100;
		c = 4.9873 - log10(rL / 1.0061);
	}

	return pow(10.0, ((-b - sqrt(b * b - 4 * a * c)) / (2 * a)));
}

double setLowerBandwidth(double lowerBandwidth, int& rLDac1, int& rLDac2, int& rLDac3)
{
	const double RLBase = 3500.0;
	const double RLDac1Unit = 175.0;
	const double RLDac2Unit = 12700.0;
	const double RLDac3Unit = 3000000.0;
	const int RLDac1Steps = 127;
	const int RLDac2Steps = 63;

	double actualLowerBandwidth;
	double rLTarget;
	double rLActual;
	int i;

	// Lower bandwidths higher than 1.5 kHz don't work well with the RHD2000 amplifiers
	if (lowerBandwidth > 1500.0) {
		lowerBandwidth = 1500.0;
	}

	rLTarget = rLFromLowerBandwidth(lowerBandwidth);

	rLDac1 = 0;
	rLDac2 = 0;
	rLDac3 = 0;
	rLActual = RLBase;

	if (lowerBandwidth < 0.15) {
		rLActual += RLDac3Unit;
		++rLDac3;
	}

	for (i = 0; i < RLDac2Steps; ++i) {
		if (rLActual < rLTarget - (RLDac2Unit - RLDac1Unit / 2)) {
			rLActual += RLDac2Unit;
			++rLDac2;
		}
	}

	for (i = 0; i < RLDac1Steps; ++i) {
		if (rLActual < rLTarget - (RLDac1Unit / 2)) {
			rLActual += RLDac1Unit;
			++rLDac1;
		}
	}

	actualLowerBandwidth = lowerBandwidthFromRL(rLActual);

	return actualLowerBandwidth;
}
void G3Controller::setFilterLowerCutoff(double lowerBandwidth)
{
	uint8_t reg12, reg13;
	int rLDac1, rLDac2, rLDac3;
	double actualLowerBandwidth;

	actualLowerBandwidth = setLowerBandwidth(lowerBandwidth, rLDac1, rLDac2, rLDac3);
	
	reg12 = rLDac1;
	reg13 = (1 << 7) + (rLDac3 << 6) + rLDac2;
}

double calcDspCutoffFreq(double newDspCutoffFreq, int& dspCutoffFreq)
{
	int n;
	double x, fCutoff[16], logNewDspCutoffFreq, logFCutoff[16], minLogDiff;
	const double Pi = 2 * acos(0.0);
	double sampleRate = 1000.0;

	fCutoff[0] = 0.0;   // We will not be using fCutoff[0], but we initialize it to be safe

	logNewDspCutoffFreq = log10(newDspCutoffFreq);

	// Generate table of all possible DSP cutoff frequencies
	for (n = 1; n < 16; ++n) {
		x = pow(2.0, (double)n);
		fCutoff[n] = sampleRate * log(x / (x - 1.0)) / (2 * Pi);
		logFCutoff[n] = log10(fCutoff[n]);
		// cout << "  fCutoff[" << n << "] = " << fCutoff[n] << " Hz" << endl;
	}

	// Now find the closest value to the requested cutoff frequency (on a logarithmic scale)
	if (newDspCutoffFreq > fCutoff[1]) {
		dspCutoffFreq = 1;
	}
	else if (newDspCutoffFreq < fCutoff[15]) {
		dspCutoffFreq = 15;
	}
	else {
		minLogDiff = 10000000.0;
		for (n = 1; n < 16; ++n) {
			if (fabs(logNewDspCutoffFreq - logFCutoff[n]) < minLogDiff) {
				minLogDiff = fabs(logNewDspCutoffFreq - logFCutoff[n]);
				dspCutoffFreq = n;
			}
		}
	}

	return fCutoff[dspCutoffFreq];
}

void G3Controller::setDspCutoffFreq(double desiredCutoffFreq)
{
	int dspCutoffVal;
	double actualFreq;
	uint8_t reg4;
	actualFreq = calcDspCutoffFreq(desiredCutoffFreq, dspCutoffVal);
	
	reg4 = 0x90 | dspCutoffVal;
	setFPGAReg(FPGA_DSP_CUTOFF, (uint32_t)reg4);
}

void G3Controller::getSensorData()
{

	if (devIsConnect())
	{
		uint8_t data[WIRELESS_BUF_SIZE] = { 0x00 };
		const auto frame = reinterpret_cast<Frame*>(data);
		frame->header = HEADER;
		frame->seq = 1;
		frame->type = FRAME_GET_HUMIDITY;
		frame->dataLen = 1;
		frame->data[0] = 0;
		sendFrame(frame);
	}
}
