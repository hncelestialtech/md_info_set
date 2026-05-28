#include "apiTool.h"

void CApiTool::AppendField(std::string& szDst, const char* pFieldName, const char* pFieldValue)
{
	szDst.append(pFieldName);
	szDst.append(":");
	szDst.append(pFieldValue);
	szDst.append(",");
}

bool CApiTool::CreateDirectory_my(std::string szPath)
{
	Replace_all(szPath, "\\", "/");
	std::string::size_type pos = szPath.find('/');
	if (pos == std::string::npos)
	{
		//没有'/',则直接代表不需要创建目录
		return true;
	}

	std::string path_directory;
	char key = '/';
	int pos_begin = szPath.find_last_of(key);
	path_directory = szPath.substr(0, pos_begin + 1);

	if (IsDirectory(path_directory))
	{
		//原有目录存在
		return true;
	}
#ifdef WIN32
	string commond = "md " + path_directory;
	Replace_all(commond, "/", "\\");
	system(commond.c_str());
	return IsDirectory(path_directory);
#else
	std::string commond = "mkdir -p " + path_directory;
	system(commond.c_str());
	return IsDirectory(path_directory);
#endif
	return true;
}

std::string& CApiTool::Replace_all(std::string& str, const std::string& old_value, const std::string& new_value)
{
	while (true)
	{
		std::string::size_type   pos(0);
		if ((pos = str.find(old_value)) != std::string::npos)
			str.replace(pos, old_value.length(), new_value);
		else break;
	}
	return str;
}

bool CApiTool::IsDirectory(std::string path)
{
#ifdef WIN32
	return PathIsDirectoryA(path.c_str()) ? true : false;
#else
	DIR* pdir = opendir(path.c_str());
	if (pdir == NULL)
	{
		return false;
	}
	else
	{
		closedir(pdir);
		pdir = NULL;
		return true;
	}
#endif
}

std::string CApiTool::GetTimeStamp(uint64_t iTimeStamp)
{
	/*
	* 1705448554==2024-01-17 07:42:34
	* 17 0544 8554 794000000 10位精确到s，13位ms、16位us、19为ns
	*/
	int64_t iThousand = 1000;
	int32_t iMs = 0;
	int32_t iUs = 0;
	int32_t iNs = 0;

	int iDigitCount = CountDigit(iTimeStamp);

	char cBuf[64] = { 0 };
	time_t currentTime;

	switch (iDigitCount)
	{
	case 10:
	{
		currentTime = iTimeStamp;
		struct tm pt;
		localtime_r(&currentTime, &pt);
		sprintf(cBuf, "%02d%02d%02d", pt.tm_hour, pt.tm_min, pt.tm_sec);
		break;
	}
	case 13:
	{
		iMs = iTimeStamp % iThousand;
		iTimeStamp = iTimeStamp / iThousand;

		currentTime = iTimeStamp;
		struct tm pt;
		localtime_r(&currentTime, &pt);
		sprintf(cBuf, "%02d%02d%02d:%03d", pt.tm_hour, pt.tm_min, pt.tm_sec, iMs);
		break;
	}
	case 16:
	{
		iUs = iTimeStamp % iThousand;
		iTimeStamp = iTimeStamp / iThousand;
		iMs = iTimeStamp % iThousand;
		iTimeStamp = iTimeStamp / iThousand;

		currentTime = iTimeStamp;
		struct tm pt;
		localtime_r(&currentTime, &pt);
		sprintf(cBuf, "%02d%02d%02d:%03d%03d", pt.tm_hour, pt.tm_min, pt.tm_sec, iMs, iUs);
		break;
	}
	case 19:
	{
		iNs = iTimeStamp % iThousand;
		iTimeStamp = iTimeStamp / iThousand;
		iUs = iTimeStamp % iThousand;
		iTimeStamp = iTimeStamp / iThousand;
		iMs = iTimeStamp % iThousand;
		iTimeStamp = iTimeStamp / iThousand;

		currentTime = iTimeStamp;
		struct tm pt;
		localtime_r(&currentTime, &pt);
		sprintf(cBuf, "%02d%02d%02d:%03d%03d%03d", pt.tm_hour, pt.tm_min, pt.tm_sec, iMs, iUs, iNs);
		break;
	}
	default:
		return std::to_string(iTimeStamp);
		break;
	}
	return std::string(cBuf);
}

int CApiTool::CountDigit(uint64_t iSrc)
{
	int iCount = 0;
	while (iSrc != 0)
	{
		iSrc = iSrc / 10;
		iCount++;
	}
	return iCount;
}

std::string CApiTool::DoubleToString(const double value, unsigned int precision)
{
	std::stringstream sstream;
	if (precision > 0)
	{
		sstream.precision(precision);
		sstream << std::fixed;//小数点后的数字
	}
	sstream << value;
	return sstream.str();
}

//YYYYMMDD
std::string CApiTool::GetDate()
{
	char cBuf[32] = { 0 };
	time_t currentTime = time(NULL);
	tm* pt = localtime(&currentTime);
	sprintf(cBuf, "%04d%02d%02d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday);
	return std::string(cBuf);
}

std::string CApiTool::GetSaveFilePath()
{
	std::string szDst = "./data/sgxd_";
	szDst += GetDate();
	szDst += "/";
	return szDst;
}

std::string CApiTool::GetChar(const char& cSrc)
{
	if (0 == cSrc)
	{
		return "";
	}
	else
	{
		return std::string(1, cSrc);
	}
}

std::string CApiTool::GetString(char* pBuf, int32_t iBufLen)
{
	std::string szDst = "";
	for (int i = 0; i < iBufLen; i++)
	{
		if (0 == pBuf[i])
		{
			break;
		}
		else
		{
			szDst.push_back(pBuf[i]);
		}
	}
	return szDst;
}


bool CApiTool::VerifySequence(const char* pMarketType, const uint32_t& sequence, uint32_t& iPreSequence,
	uint32_t& iLostSum)
{
	/*
	* 发包的消息序号 从 1 开始，确认会发生回环监控首先判断是否为0，为0则异常，
	* 其次判断是否递增，如果没有则检查丢包数量，并落日志
	*/
	if (0 == sequence)
	{
		std::cout << pMarketType << " sequence is abnormal," << sequence << std::endl;
		return false;
	}
	if (0 == iPreSequence)
	{
		//程序第一次运行或者发生回环时，不统计丢包情况
		iPreSequence = sequence;
		return true;
	}
	if (MAX_UINT32 == sequence)
	{
		//最大值会回环,暂定不考虑最大值会丢包的可能性
		iPreSequence = 0;
		std::cout << pMarketType << " sequence is looped" << std::endl;
		return true;
	}
	else
	{
		if (++iPreSequence != sequence)
		{
			--iPreSequence;
			int iLostSumTmp = sequence - iPreSequence;
			iLostSum += iLostSumTmp;
			std::cout << pMarketType << " lost " << iLostSumTmp << " message,preSequence:" << iPreSequence << " sequence:"
				<< sequence << " totalLost:" << iLostSum << std::endl;
			iPreSequence = sequence;
			return false;
		}
	}
	return true;
}
