#include <stdint.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <iostream>

using namespace std;

class CCfg
{
public:
	CCfg(const char* filename = NULL);
	~CCfg();

	int GetValue(const string& Section, const string& Key, string& Value, const string& Default = "");
	int GetValue(const string& Section, const string& Key, int32_t& Value, const int32_t& Default = 0);
	int GetValue(const string& Section, const string& Key, double& Value, const double& Default = 0);
	int GetValue(
		const string& Section, const string& Key, vector<string>& Value, const string& DeliStr, const string& Default = "");
	int Open(const char* filename);
	int Close();

private:
	int LocateSection(const string& Section);
	int IfInStr(const string& pBuffer, const string& pShortBuffer);
	int GetInnerValue(const string& Section, const string& Key, string& Value);
	string& LeftTrim(string& Str);
	string& Trim(string& Str);
	void DeliString(vector<string>& Value, const string& OrginStr, const string& DeliStr);

private:
	string m_File;
	ifstream m_Cfg;
	char m_ReadBuffer[4096];
};
