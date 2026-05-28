#include "gfexRcv.h"

CGfexRcv::CGfexRcv(std::string szConfPath)
{
	m_szConfPath = szConfPath;
	m_pLogMarket = new CLogMarket();
}

CGfexRcv::~CGfexRcv()
{

}

bool CGfexRcv::Init()
{
	if (!InitConfig())
	{
		std::cout << "InitConfig fail" << std::endl;
		return false;
	}

	if (!InitWriteFile())
	{
		std::cout << "InitWriteFile fail" << std::endl;;
		return false;
	}

	if (!InitMulticastSocket())
	{
		std::cout << "InitMulticastSocket fail" << std::endl;
		return false;
	}
	return true;
}

bool CGfexRcv::InitConfig()
{
	std::string szDst = "";

	m_pIniFile = new xini_file_t(m_szConfPath);
	if (!m_pIniFile->is_open())
	{
		std::cout << "m_pIniFile open fail,m_szConfPath:" << m_szConfPath << std::endl;
		return false;
	}

	m_szMulticastIp = (*m_pIniFile)["gfex_conf"]["multicastIp"].m_xstr_value;
	szDst = (*m_pIniFile)["gfex_conf"]["multicastPort"].m_xstr_value;
	m_iMulticastPort = stoi(szDst);
	m_szMulticastInterface = (*m_pIniFile)["gfex_conf"]["multicast_interface"].m_xstr_value;

	szDst = (*m_pIniFile)["gfex_conf"]["lost_detect"].m_xstr_value;
	m_bLostDetect = stoi(szDst);

	szDst = (*m_pIniFile)["gfex_conf"]["logFile"].m_xstr_value;
	m_bLogFIle = stoi(szDst);

	szDst = (*m_pIniFile)["gfex_conf"]["SktRcvBuf"].m_xstr_value;
	m_iSktRcvBuf = stoi(szDst);

	m_pIniFile->close();
	return true;
}

bool CGfexRcv::InitWriteFile()
{
	if (!CApiTool::CreateDirectory_my(CApiTool::GetSaveFilePath()))
	{
		std::cout << "CreateDirectory_my fail," << CApiTool::GetSaveFilePath() << std::endl;
		return false;
	}
	InitFile(&m_pFile_BestDeep, "BestDeep.csv");
	InitFile(&m_pFile_ArbBestDeep, "ArbBestDeep.csv");
	InitFile(&m_pFile_TenEntrust, "TenEntrust.csv");
	InitFile(&m_pFile_OrderStatistic, "OrderStatistic.csv");
	InitFile(&m_pFile_MatchPriceQty, "MatchPriceQty.csv");

	InitFile(&m_pFile_Real, "Real.csv");

	return true;
}

void CGfexRcv::InitFile(FILE** pWriteFile, const std::string& szFileName)
{
	std::string szPath = CApiTool::GetSaveFilePath() + szFileName;
	*pWriteFile = fopen(szPath.c_str(), "w");
}

bool CGfexRcv::InitMulticastSocket()
{
	std::cout << "begin init multicast receive,m_szMulticastIp:" << m_szMulticastIp << " m_iMulticastPort:"
		<< m_iMulticastPort << " m_szMulticastInterface:" << m_szMulticastInterface << std::endl;

	//Init Multicast receive
	m_iSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_iSocket < 0)
	{
		std::cout << "socket error��" << m_iSocket << " error:" << strerror(m_iSocket) << std::endl;
		return false;
	}

	//���õ�ַ����
	int enable = 1;
	setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	//���÷�����ģʽ
	int flags = fcntl(m_iSocket, F_GETFL, 0);
	if (fcntl(m_iSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(m_iSocket);
		std::cout << "Set NONBLOCK failed" << std::endl;
		return false;
	}

	//Set Local address
	struct sockaddr_in local_addr;
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr(m_szMulticastIp.c_str()); ;
	local_addr.sin_port = htons(m_iMulticastPort);
	int ret = bind(m_iSocket, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if (ret < 0)
	{
		std::cout << "bind error,ret:" << ret << " error:" << strerror(ret) << std::endl;
		close(m_iSocket);
		return false;
	}

	struct ip_mreq mreq; // �ಥ��ַ�ṹ��
	mreq.imr_multiaddr.s_addr = inet_addr(m_szMulticastIp.c_str());
	mreq.imr_interface.s_addr = inet_addr(m_szMulticastInterface.c_str());

	ret = setsockopt(m_iSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
		sizeof(mreq));
	if (ret < 0)
	{
		std::cout <<"add membership error,ret:" << ret << " error:" << strerror(ret) << std::endl;
		close(m_iSocket);
		return false;
	}
	ret = setsockopt(m_iSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&m_iSktRcvBuf, sizeof(m_iSktRcvBuf));
	if (ret != 0)
	{
		std::cout << "setsockopt SO_RCVBUF,ret:" << ret << std::endl;
		close(m_iSocket);
		return false;
	}
	std::cout << "Init Multicast success" << std::endl;
	return true;
}

void CGfexRcv::Run()
{
	RecvMsg();
}

void CGfexRcv::RecvMsg()
{
	int iRcvLen = 0;
	struct sockaddr_in muticast_addr;
	socklen_t len = sizeof(sockaddr_in);
	while (true)
	{
		memset(m_arrBuf, 0, GFEX::g_iMaxFrameSize);
		iRcvLen = ::recvfrom(m_iSocket, m_arrBuf, GFEX::g_iMaxFrameSize, 0, (struct sockaddr*)&muticast_addr,(socklen_t*)&len);
		if (iRcvLen > 0)
		{
			DecodeMsg(m_arrBuf, iRcvLen);
		}
	}
}

void CGfexRcv::DecodeMsg(unsigned char* pBuf, int32_t iDataLen)
{
	int32_t iProcSize = 0;
	//���յ������ݰ��ṹ
	GFEX::Zx_PacketHead* pPacketHead = (GFEX::Zx_PacketHead*)pBuf;
	iProcSize += m_iLen_packetHead;
	assert(iDataLen == pPacketHead->iPktSize);

	while (true)
	{
		if (iDataLen - (iProcSize + m_iLen_msgHead) <= 0)
		{
			break;
		}
		GFEX::Zx_MsgHead* pMsgHead = (GFEX::Zx_MsgHead*)(pBuf + iProcSize);
		iProcSize += m_iLen_msgHead;

		if (m_bLostDetect)
		{
			CApiTool::VerifySequence("realtime ", pMsgHead->iSequence, m_iPreSequence, m_iLostSum);
		}
		std::string szDst = "";
		switch (pMsgHead->iMsgType)
		{
		case GFEX::MsgZx_BestDeep:
		{
			if (m_bLogFIle)
			{
				szDst = m_pLogMarket->DecodeBestDeep(pBuf + iProcSize, pMsgHead);
				szDst += "\n";
				fprintf(m_pFile_BestDeep, "%s", szDst.c_str());
			}
			break;
		}
		case GFEX::MsgZx_ArbBestDeep:
		{
			if (m_bLogFIle)
			{
				szDst = m_pLogMarket->DecodeArbBestDeep(pBuf + iProcSize, pMsgHead);
				szDst += "\n";
				fprintf(m_pFile_ArbBestDeep, "%s", szDst.c_str());
			}
			break;
		}
		case GFEX::MsgZx_TenEntrust:
		{
			if (m_bLogFIle)
			{
				szDst = m_pLogMarket->DecodeTenEntrust(pBuf + iProcSize, pMsgHead);
				szDst += "\n";
				fprintf(m_pFile_TenEntrust, "%s", szDst.c_str());
			}
			break;
		}
		case GFEX::MsgZx_Real:
		{
			if (m_bLogFIle)
			{
				szDst = m_pLogMarket->DecodeReal(pBuf + iProcSize, pMsgHead);
				szDst += "\n";
				fprintf(m_pFile_Real, "%s", szDst.c_str());
			}
			break;
		}
		case GFEX::MsgZx_OrderStatistic:
		{
			if (m_bLogFIle)
			{
				szDst = m_pLogMarket->DecodeOrderStatistic(pBuf + iProcSize, pMsgHead);
				szDst += "\n";
				fprintf(m_pFile_OrderStatistic, "%s", szDst.c_str());
			}
			break;
		}
		case GFEX::MsgZx_MatchPriceQty:
		{
			if (m_bLogFIle)
			{
				szDst = m_pLogMarket->DecodeMatchPriceQty(pBuf + iProcSize, pMsgHead);
				szDst += "\n";
				fprintf(m_pFile_MatchPriceQty, "%s", szDst.c_str());
			}
			break;
		}
		
		default:
			std::cout << "abnormal msgType:" << pMsgHead->iMsgType << std::endl;
			assert(0);
			break;
		}
		iProcSize += pMsgHead->iMsgSize;
	}
}