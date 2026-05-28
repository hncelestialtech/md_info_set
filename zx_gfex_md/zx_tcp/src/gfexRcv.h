 #pragma once
#include "define_zx.h"
#include "apiTool.h"
#include "logMarket.h"
#include "ini_parser.h"


class CGfexRcv
{
public:
	CGfexRcv(std::string szConfPath);
	~CGfexRcv();
	bool Init();
	void Run();

private:
	bool InitConfig();
	bool InitWriteFile();
	void InitFile(FILE** pWriteFile, const std::string& szFileName);
	bool InitMulticastSocket();

	void RecvMsg();
	void DecodeMsg(unsigned char* pBuf, int32_t iDataLen);
private:
	CLogMarket* m_pLogMarket;

	xini_file_t* m_pIniFile;
	std::string m_szConfPath = "";
	std::string m_szMulticastIp = "";//行情源组播ip
	int m_iMulticastPort = 0;//行情源组播端口
	std::string m_szMulticastInterface = "";//本地接收组播ip

	int m_iSocket = 0;
	int32_t m_iSktRcvBuf = 0;//socket的接收缓存区
	bool m_bLogFIle = false;
	bool m_bLostDetect = false;
	uint32_t m_iPreSequence = 0;
	uint32_t m_iLostSum = 0;

	unsigned char m_arrBuf[GFEX::g_iMaxFrameSize] = { 0 };
	int m_iLen_packetHead = sizeof(GFEX::Zx_PacketHead);
	int m_iLen_msgHead = sizeof(GFEX::Zx_MsgHead);

	FILE* m_pFile_BestDeep;
	FILE* m_pFile_ArbBestDeep;
	FILE* m_pFile_TenEntrust;
	FILE* m_pFile_OrderStatistic;
	FILE* m_pFile_MatchPriceQty;

	FILE* m_pFile_Real;
public:
};
