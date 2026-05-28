#pragma once
#include "apiTool.h"
#include "define_zx.h"
#include <sstream>


class CLogMarket
{
public:	
	CLogMarket();
	~CLogMarket();
	bool Init();

	std::string DecodeBestDeep(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead);
	std::string DecodeArbBestDeep(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead);
	std::string DecodeTenEntrust(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead);
	std::string DecodeReal(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead);
	std::string DecodeOrderStatistic(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead);

	std::string DecodeMatchPriceQty(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead);
private:
	//为true时，最大double值置0
	std::string DoubleToString(double iValue, unsigned int precision, bool bFlag = true);

};