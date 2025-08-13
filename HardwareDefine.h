/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
*  作者 : 崔俊杰 junjie.cui@neuroxess.com
*  修改记录:
*
******************************************************************************/

#ifndef HARDWARE_DEFINE_H
#define HARDWARE_DEFINE_H


#include <string>

typedef signed char         INT8;
typedef signed short        INT16;
typedef signed int          INT32;
typedef signed __int64      INT64;
typedef unsigned char		UINT8;
typedef unsigned short      UINT16;
typedef unsigned int		UINT32;
typedef unsigned __int64    UINT64;

enum EcuErrorCode
{
	ECU_NO_ERROR = 0x00,
	ECU_INVALID_FRAME = 0x01,
	ECU_INVALID_HEADER = 0x02,
	ECU_INVALID_CRC = 0x03,
	ECU_INVALID_SEQ = 0x04,
	ECU_INVALID_TYPE = 0x05,
	ECU_INVALID_DATA_SEG_FLAG = 0x06,
	ECU_INVALID_DATA_LEN = 0x07,
	ECU_INVALID_EP_ADDR = 0x08,
	ECU_INVALID_PARAM = 0x09,
	ECU_SEND_FAILED = 0x0A,
	ECU_NO_DEVICE_FOUND = 0x0B,
	ECU_OPEN_DEVICE_FAILED = 0x0C,
	ECU_DEVICE_NOT_OPEN = 0x0D,
	ECU_TIMEOUT = 0x0E,
	ECU_DATA_NOT_AVAILABLE = 0x0F,
	ECU_UNKNOWN_ERROR = 0xFF,
};

enum NotchFilterType {
	NF_NONE = 0,
	NF_50Hz,
	NF_60Hz
};

enum  IcuStatus {
	ICU_SAMPLING = 0x00,  //采集
	ICU_NOT_SAMPLING = 0x01, //停止采集
};

enum DeviceCtrl {
	RESET_DEVICE = 0x00,
	POWER_OFF = 0x01,
};

enum DeviceType {
	DEVICE_ICU = 0x00,
	DEVICE_ECU = 0x01,
};

struct DeviceModel
{
	std::string PID;
	std::string SN;
	std::string VID;
};

enum SpeedMode
{
	LOW_SPEED = 0x00,
	HIGH_SPEED = 0x01,
	SUPER_SPEED = 0x02
};

struct DeviceInfo
{
	std::string  deviceName = "OP";
	std::string   deviceType = "D";
	std::string   year = "D";
	std::string   month = "C";
	std::string   deviceID = "0003";
};

struct BleDeviceInfo
{
	uint64_t mac_address;
	std::wstring charServiceUUID;
	std::wstring charReadUUID;
	std::wstring charWriteUUID;
};

enum FpgaAddr
{
	FPGA_SAMPLE_RANGE = 0x40,
	FPGA_DSP_CUTOFF = 0x84,
};

#endif // HARDWARE_DEFINE_H