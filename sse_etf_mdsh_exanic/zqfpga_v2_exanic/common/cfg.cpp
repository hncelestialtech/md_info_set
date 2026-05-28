#include <stdlib.h>
#include "cfg.h"

CCfg::CCfg(const char* filename)
{
	if (filename)
		Open(filename);
}

CCfg::~CCfg() {}

int CCfg::GetInnerValue(const string& Section, const string& Key, string& Value)
{
	if (LocateSection(Section))
		return -1;
	int32_t pos = 0;
	while (!m_Cfg.eof())
	{
		memset(m_ReadBuffer,0,sizeof(m_ReadBuffer));
		m_Cfg.getline(m_ReadBuffer, 2046);
		string&& readstr = string(m_ReadBuffer);
		if (IfInStr(readstr, string("[")) != -1)
			return GetInnerValue(Section, Key, Value);
		if ((pos = IfInStr(readstr, Key)) != -1)
		{
			Value = m_ReadBuffer + (readstr.find("=", pos)) + 1;
			Value = Trim(Value);
			return 0;
		}
	}
	cout << "Can not Find Section:" << Section.c_str() << " Key:" << Key.c_str() << endl;
	return -1;
}

int CCfg::Close()
{
	if (!m_Cfg.is_open())
		return 0;
	m_Cfg.close();
	if (m_Cfg.is_open())
		return -1;
	return 0;
}

int CCfg::LocateSection(const string& Section)
{
	while (!m_Cfg.eof())
	{
		m_Cfg.getline(m_ReadBuffer, 2549);
		string&& readstr = string(m_ReadBuffer);
		if (-1 != IfInStr(readstr, Section))
		{
			return 0;
		}
	}
	return -1;
}

int CCfg::IfInStr(const string& pBuffer, const string& pShortBuffer)
{
	int32_t pos = 0;
	if ((pos = pBuffer.find(pShortBuffer)) != string::npos)
		return pos;
	else
		return -1;
}

int CCfg::Open(const char* filename)
{
	if (m_Cfg.is_open())
	{
		printf("上次打开的配置文件没有关闭!");
		return -1;
	}
	m_Cfg.open(filename);
	if (!m_Cfg.is_open())
	{
		printf("loadcf:不能打开配置文件[%s]\n", filename);
		return false;
	}
	m_File = filename;
	return 0;
}

string& CCfg::LeftTrim(string& Str)
{
	int32_t i = 0;
	char c = 0;
	while ((c = Str.at(i)) != string::npos)
	{
		if (c != ' ')
			break;
		i++;
	}
	Str.erase(0, i);
	return Str;
}

string& CCfg::Trim(string& Str)
{
	int32_t i = 0;
	char c = 0;
	while ((c = Str.at(i)) != string::npos)
	{
		if (c != ' ')
			break;
		i++;
	}
	Str.erase(0, i);
	i = Str.length();
	while (i > 0)
	{
		if (Str.at(i-1) != ' '&&Str.at(i-1) != '\r')
			break;
		i--;
	}
	Str.erase(i);
	return Str;
}

int CCfg::GetValue(const string& Section, const string& Key, string& Value, const string& Default)
{
	char SectionName[200] = { 0 };
	snprintf(SectionName, Section.length() + 3, "[%s]", Section.c_str());
	m_Cfg.seekg(0, std::ios::beg);
	Value = Default;
	return GetInnerValue(SectionName, Key, Value);
}

int CCfg::GetValue(const string& Section, const string& Key, int32_t& Value, const int32_t& Default)
{
	char SectionName[200];
	snprintf(SectionName, Section.length() + 3, "[%s]", Section.c_str());
	m_Cfg.seekg(0, std::ios::beg);
	string strValue;
	int8_t&& ret = GetInnerValue(SectionName, Key, strValue);
	if (0 == ret)
	{
		Value = atoi(strValue.c_str());
	}
	else
		Value = Default;
	return ret;
}

int CCfg::GetValue(const string& Section, const string& Key, double& Value, const double& Default)
{
	char SectionName[200];
	snprintf(SectionName, Section.length() + 3, "[%s]", Section.c_str());
	m_Cfg.seekg(0, std::ios::beg);
	string strValue;
	int8_t&& ret = GetInnerValue(SectionName, Key, strValue);
	if (0 == ret)
	{
		Value = atof(strValue.c_str());
	}
	else
		Value = Default;
	return ret;
}

int CCfg::GetValue(
	const string& Section, const string& Key, vector<string>& Value, const string& DeliStr, const string& Default)
{
	char SectionName[200];
	snprintf(SectionName, Section.length() + 3, "[%s]", Section.c_str());
	m_Cfg.seekg(0, std::ios::beg);
	string strValue;
	int8_t&& ret = GetInnerValue(SectionName, Key, strValue);
	if (0 == ret)
	{
		DeliString(Value, strValue, DeliStr);
	}
	return ret;
}

void CCfg::DeliString(vector<string>& Value, const string& OrginStr, const string& DeliStr)
{
	if (0 == OrginStr.length())
		return;
	int32_t startpos = 0;
	int32_t endpos = 0;
	while (endpos != string::npos)
	{
		endpos = OrginStr.find(DeliStr, startpos);
		if (string::npos == endpos)
			Value.push_back(OrginStr.substr(startpos));
		else
			Value.push_back(OrginStr.substr(startpos,endpos-startpos));
		startpos = endpos == string::npos ? startpos : endpos + 1;
	}
	return;
}
