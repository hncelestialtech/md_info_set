#pragma once
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <fstream>
#include <memory>
#include <array>
#include <iostream>
#include <iconv.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <locale> 
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <fcntl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <errno.h>

#define MAX_UINT32 4294967295

class CApiTool
{
public:
	template<class in_value>
	static void AppendValue(std::stringstream& sstream, const in_value& tValue)
	{
		if (std::is_floating_point<in_value>::value)
		{
			//由于double要定制化处理精度，因此必须处理成string搞进来
			std::cout << "AppendValue don't accep double" << std::endl;
			return;
		}
		sstream << tValue;
		sstream << ",";
	}
	template<class in_value>
	static void AppendValue(std::stringstream& sstream, const char* pFieldName, const in_value& tValue)
	{
		if (std::is_floating_point<in_value>::value)
		{
			//由于double要定制化处理精度，因此必须处理成string搞进来
			std::cout << "AppendValue don't accep double" << std::endl;
			return;
		}
		sstream << pFieldName;
		sstream << ":";
		sstream << tValue;
		sstream << ",";
	}
	static void AppendField(std::string& szDst, const char* pFieldName, const char* pFieldValue);

	static bool CreateDirectory_my(std::string szPath);
	static std::string& Replace_all(std::string& str, const std::string& old_value, const std::string& new_value);
	static bool IsDirectory(std::string path);

	static std::string GetTimeStamp(uint64_t iTimeStamp);
	static int CountDigit(uint64_t iSrc);
	static std::string DoubleToString(const double value, unsigned int precision);

	static std::string GetDate();
	static std::string GetSaveFilePath();
	static std::string GetChar(const char& cSrc);
	static std::string GetString(char* pBuf, int32_t iBufLen);

	static bool VerifySequence(const char* pMarketType, const uint32_t& sequence, uint32_t& iPreSequence,
		uint32_t& iLostSum);
};