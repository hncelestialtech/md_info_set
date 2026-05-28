#include "logMarket.h"
#include "apiTool.h"

CLogMarket::CLogMarket()
{
}

CLogMarket::~CLogMarket()
{

}

bool CLogMarket::Init()
{
	return true;
}

std::string CLogMarket::DecodeBestDeep(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead)
{
	GFEX::Zx_BestDeep* pMsg = (GFEX::Zx_BestDeep*)arrBuf;

	std::stringstream sstream;
	CApiTool::AppendValue(sstream, "iZxSequence", pMsgHead->iSequence);
	CApiTool::AppendValue(sstream, "iMsgSize", pMsgHead->iMsgSize);
	CApiTool::AppendValue(sstream, "iMsgType", pMsgHead->iMsgType);

	CApiTool::AppendValue(sstream, "iSequence", pMsg->iSequence);
	CApiTool::AppendValue(sstream, "arrInstrument", CApiTool::GetString(pMsg->arrInstrument, sizeof(pMsg->arrInstrument)));
	CApiTool::AppendValue(sstream, "arrGenTime", CApiTool::GetString(pMsg->arrGenTime, sizeof(pMsg->arrGenTime)));

	CApiTool::AppendValue(sstream, "dLastPrice", DoubleToString(pMsg->dLastPrice,4));
	CApiTool::AppendValue(sstream, "dHighPrice", DoubleToString(pMsg->dHighPrice,4));
	CApiTool::AppendValue(sstream, "dLowPrice", DoubleToString(pMsg->dLowPrice,4));
	CApiTool::AppendValue(sstream, "iLastMatchQty", pMsg->iLastMatchQty);

	CApiTool::AppendValue(sstream, "iMatchTotQty", pMsg->iMatchTotQty);
	CApiTool::AppendValue(sstream, "dTurnover", DoubleToString(pMsg->dTurnover,4));
	CApiTool::AppendValue(sstream, "iLastOpenInterest", pMsg->iInitOpenInterest);
	CApiTool::AppendValue(sstream, "iOpenInterest", pMsg->iOpenInterest);
	CApiTool::AppendValue(sstream, "iInterestChg", pMsg->iInterestChg);

	CApiTool::AppendValue(sstream, "dClearPrice", DoubleToString(pMsg->dClearPrice, 4));
	CApiTool::AppendValue(sstream, "dLastClearPrice", DoubleToString(pMsg->dLastClearPrice, 4));
	CApiTool::AppendValue(sstream, "dClosePrice", DoubleToString(pMsg->dClosePrice, 4));
	CApiTool::AppendValue(sstream, "dLastClosePrice", DoubleToString(pMsg->dLastClosePrice, 4));
	CApiTool::AppendValue(sstream, "dLifeLow", DoubleToString(pMsg->dLifeLow, 4));

	CApiTool::AppendValue(sstream, "dLifeHigh", DoubleToString(pMsg->dLifeHigh, 4));
	CApiTool::AppendValue(sstream, "dRiseLimit", DoubleToString(pMsg->dRiseLimit, 4));
	CApiTool::AppendValue(sstream, "dFallLimit", DoubleToString(pMsg->dFallLimit, 4));
	CApiTool::AppendValue(sstream, "dAvePrice", DoubleToString(pMsg->dAvePrice, 4));
	CApiTool::AppendValue(sstream, "dOpenPrice", DoubleToString(pMsg->dOpenPrice, 4));

	CApiTool::AppendValue(sstream, "dBidPrice1", DoubleToString(pMsg->dBidPrice1, 4));
	CApiTool::AppendValue(sstream, "iBidVolume1", pMsg->iBidVolume1);
	CApiTool::AppendValue(sstream, "iBidImplyVolume1", pMsg->iBidImplyVolume1);
	CApiTool::AppendValue(sstream, "dBidPrice2", DoubleToString(pMsg->dBidPrice2, 4));
	CApiTool::AppendValue(sstream, "iBidVolume2", pMsg->iBidVolume2);
	CApiTool::AppendValue(sstream, "iBidImplyVolume2", pMsg->iBidImplyVolume2);

	CApiTool::AppendValue(sstream, "dBidPrice3", DoubleToString(pMsg->dBidPrice3, 4));
	CApiTool::AppendValue(sstream, "iBidVolume3", pMsg->iBidVolume3);
	CApiTool::AppendValue(sstream, "iBidImplyVolume3", pMsg->iBidImplyVolume3);
	CApiTool::AppendValue(sstream, "dBidPrice4", DoubleToString(pMsg->dBidPrice4, 4));
	CApiTool::AppendValue(sstream, "iBidVolume4", pMsg->iBidVolume4);
	CApiTool::AppendValue(sstream, "iBidImplyVolume4", pMsg->iBidImplyVolume4);

	CApiTool::AppendValue(sstream, "dBidPrice5", DoubleToString(pMsg->dBidPrice5, 4));
	CApiTool::AppendValue(sstream, "iBidVolume5", pMsg->iBidVolume5);
	CApiTool::AppendValue(sstream, "iBidImplyVolume5", pMsg->iBidImplyVolume5);

	CApiTool::AppendValue(sstream, "dAskPrice1", DoubleToString(pMsg->dAskPrice1, 4));
	CApiTool::AppendValue(sstream, "iAskVolume1", pMsg->iAskVolume1);
	CApiTool::AppendValue(sstream, "iAskImplyVolume1", pMsg->iAskImplyVolume1);
	CApiTool::AppendValue(sstream, "dAskPrice2", DoubleToString(pMsg->dAskPrice2, 4));
	CApiTool::AppendValue(sstream, "iAskVolume2", pMsg->iAskVolume2);
	CApiTool::AppendValue(sstream, "iAskImplyVolume2", pMsg->iAskImplyVolume2);

	CApiTool::AppendValue(sstream, "dAskPrice3", DoubleToString(pMsg->dAskPrice3, 4));
	CApiTool::AppendValue(sstream, "iAskVolume3", pMsg->iAskVolume3);
	CApiTool::AppendValue(sstream, "iAskImplyVolume3", pMsg->iAskImplyVolume3);
	CApiTool::AppendValue(sstream, "dAskPrice4", DoubleToString(pMsg->dAskPrice4, 4));
	CApiTool::AppendValue(sstream, "iAskVolume4", pMsg->iAskVolume4);
	CApiTool::AppendValue(sstream, "iAskImplyVolume4", pMsg->iAskImplyVolume4);

	CApiTool::AppendValue(sstream, "dAskPrice5", DoubleToString(pMsg->dAskPrice5, 4));
	CApiTool::AppendValue(sstream, "iAskVolume5", pMsg->iAskVolume5);
	CApiTool::AppendValue(sstream, "iAskImplyVolume5", pMsg->iAskImplyVolume5);

	return sstream.str();
}

std::string CLogMarket::DecodeArbBestDeep(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead)
{
	GFEX::Zx_ArbBestDeep* pMsg = (GFEX::Zx_ArbBestDeep*)arrBuf;

	std::stringstream sstream;
	CApiTool::AppendValue(sstream, "iZxSequence", pMsgHead->iSequence);
	CApiTool::AppendValue(sstream, "iMsgSize", pMsgHead->iMsgSize);
	CApiTool::AppendValue(sstream, "iMsgType", pMsgHead->iMsgType);

	CApiTool::AppendValue(sstream, "iSequence", pMsg->iSequence);
	CApiTool::AppendValue(sstream, "arrInstrument", CApiTool::GetString(pMsg->arrInstrument, sizeof(pMsg->arrInstrument)));
	CApiTool::AppendValue(sstream, "arrGenTime", CApiTool::GetString(pMsg->arrGenTime, sizeof(pMsg->arrGenTime)));
	CApiTool::AppendValue(sstream, "dLastPrice", DoubleToString(pMsg->dLastPrice, 4));
	CApiTool::AppendValue(sstream, "dLowPrice", DoubleToString(pMsg->dLowPrice, 4));

	CApiTool::AppendValue(sstream, "dHighPrice", DoubleToString(pMsg->dHighPrice, 4));
	CApiTool::AppendValue(sstream, "dLifeLow", DoubleToString(pMsg->dLifeLow, 4));
	CApiTool::AppendValue(sstream, "dLifeHigh", DoubleToString(pMsg->dLifeHigh, 4));
	CApiTool::AppendValue(sstream, "dRiseLimit", DoubleToString(pMsg->dRiseLimit, 4));
	CApiTool::AppendValue(sstream, "dFallLimit", DoubleToString(pMsg->dFallLimit, 4));

	CApiTool::AppendValue(sstream, "dBidPrice1", DoubleToString(pMsg->dBidPrice1, 4));
	CApiTool::AppendValue(sstream, "iBidVolume1", pMsg->iBidVolume1);
	CApiTool::AppendValue(sstream, "iBidImplyVolume1", pMsg->iBidImplyVolume1);
	CApiTool::AppendValue(sstream, "dBidPrice2", DoubleToString(pMsg->dBidPrice2, 4));
	CApiTool::AppendValue(sstream, "iBidVolume2", pMsg->iBidVolume2);
	CApiTool::AppendValue(sstream, "iBidImplyVolume2", pMsg->iBidImplyVolume2);

	CApiTool::AppendValue(sstream, "dBidPrice3", DoubleToString(pMsg->dBidPrice3, 4));
	CApiTool::AppendValue(sstream, "iBidVolume3", pMsg->iBidVolume3);
	CApiTool::AppendValue(sstream, "iBidImplyVolume3", pMsg->iBidImplyVolume3);
	CApiTool::AppendValue(sstream, "dBidPrice4", DoubleToString(pMsg->dBidPrice4, 4));
	CApiTool::AppendValue(sstream, "iBidVolume4", pMsg->iBidVolume4);
	CApiTool::AppendValue(sstream, "iBidImplyVolume4", pMsg->iBidImplyVolume4);

	CApiTool::AppendValue(sstream, "dBidPrice5", DoubleToString(pMsg->dBidPrice5, 4));
	CApiTool::AppendValue(sstream, "iBidVolume5", pMsg->iBidVolume5);
	CApiTool::AppendValue(sstream, "iBidImplyVolume5", pMsg->iBidImplyVolume5);

	CApiTool::AppendValue(sstream, "dAskPrice1", DoubleToString(pMsg->dAskPrice1, 4));
	CApiTool::AppendValue(sstream, "iAskVolume1", pMsg->iAskVolume1);
	CApiTool::AppendValue(sstream, "iAskImplyVolume1", pMsg->iAskImplyVolume1);
	CApiTool::AppendValue(sstream, "dAskPrice2", DoubleToString(pMsg->dAskPrice2, 4));
	CApiTool::AppendValue(sstream, "iAskVolume2", pMsg->iAskVolume2);
	CApiTool::AppendValue(sstream, "iAskImplyVolume2", pMsg->iAskImplyVolume2);

	CApiTool::AppendValue(sstream, "dAskPrice3", DoubleToString(pMsg->dAskPrice3, 4));
	CApiTool::AppendValue(sstream, "iAskVolume3", pMsg->iAskVolume3);
	CApiTool::AppendValue(sstream, "iAskImplyVolume3", pMsg->iAskImplyVolume3);
	CApiTool::AppendValue(sstream, "dAskPrice4", DoubleToString(pMsg->dAskPrice4, 4));
	CApiTool::AppendValue(sstream, "iAskVolume4", pMsg->iAskVolume4);
	CApiTool::AppendValue(sstream, "iAskImplyVolume4", pMsg->iAskImplyVolume4);

	CApiTool::AppendValue(sstream, "dAskPrice5", DoubleToString(pMsg->dAskPrice5, 4));
	CApiTool::AppendValue(sstream, "iAskVolume5", pMsg->iAskVolume5);
	CApiTool::AppendValue(sstream, "iAskImplyVolume5", pMsg->iAskImplyVolume5);

	return sstream.str();
}

std::string CLogMarket::DecodeTenEntrust(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead)
{
	GFEX::Zx_TenEntrust* pMsg = (GFEX::Zx_TenEntrust*)arrBuf;

	std::stringstream sstream;
	CApiTool::AppendValue(sstream, "iZxSequence", pMsgHead->iSequence);
	CApiTool::AppendValue(sstream, "iMsgSize", pMsgHead->iMsgSize);
	CApiTool::AppendValue(sstream, "iMsgType", pMsgHead->iMsgType);

	CApiTool::AppendValue(sstream, "iSequence", pMsg->iSequence);
	CApiTool::AppendValue(sstream, "arrInstrument", CApiTool::GetString(pMsg->arrInstrument, sizeof(pMsg->arrInstrument)));
	CApiTool::AppendValue(sstream, "arrGenTime", CApiTool::GetString(pMsg->arrGenTime, sizeof(pMsg->arrGenTime)));
	CApiTool::AppendValue(sstream, "dBestBuyOrderPrice", DoubleToString(pMsg->dBestBuyOrderPrice, 4));

	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyOne", pMsg->iBestBuyOrderQtyOne);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyTwo", pMsg->iBestBuyOrderQtyTwo);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyThree", pMsg->iBestBuyOrderQtyThree);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyFour", pMsg->iBestBuyOrderQtyFour);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyFive", pMsg->iBestBuyOrderQtyFive);


	CApiTool::AppendValue(sstream, "iBestBuyOrderQtySix", pMsg->iBestBuyOrderQtySix);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtySeven", pMsg->iBestBuyOrderQtySeven);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyEight", pMsg->iBestBuyOrderQtyEight);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyNine", pMsg->iBestBuyOrderQtyNine);
	CApiTool::AppendValue(sstream, "iBestBuyOrderQtyTen", pMsg->iBestBuyOrderQtyTen);

	CApiTool::AppendValue(sstream, "dBestSellOrderPrice", DoubleToString(pMsg->dBestSellOrderPrice, 4));

	CApiTool::AppendValue(sstream, "iBestSellOrderQtyOne", pMsg->iBestSellOrderQtyOne);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyTwo", pMsg->iBestSellOrderQtyTwo);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyThree", pMsg->iBestSellOrderQtyThree);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyFour", pMsg->iBestSellOrderQtyFour);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyFive", pMsg->iBestSellOrderQtyFive);

	CApiTool::AppendValue(sstream, "iBestSellOrderQtySix", pMsg->iBestSellOrderQtySix);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtySeven", pMsg->iBestSellOrderQtySeven);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyEight", pMsg->iBestSellOrderQtyEight);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyNine", pMsg->iBestSellOrderQtyNine);
	CApiTool::AppendValue(sstream, "iBestSellOrderQtyTen", pMsg->iBestSellOrderQtyTen);

	return sstream.str();
}

std::string CLogMarket::DecodeReal(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead)
{
	GFEX::Zx_Real* pMsg = (GFEX::Zx_Real*)arrBuf;

	std::stringstream sstream;
	CApiTool::AppendValue(sstream, "iZxSequence", pMsgHead->iSequence);
	CApiTool::AppendValue(sstream, "iMsgSize", pMsgHead->iMsgSize);
	CApiTool::AppendValue(sstream, "iMsgType", pMsgHead->iMsgType);

	CApiTool::AppendValue(sstream, "iSequence", pMsg->iSequence);
	CApiTool::AppendValue(sstream, "arrInstrument", CApiTool::GetString(pMsg->arrInstrument, sizeof(pMsg->arrInstrument)));
	CApiTool::AppendValue(sstream, "dRealTimePrice", DoubleToString(pMsg->dRealTimePrice, 4));

	return sstream.str();
}

std::string CLogMarket::DecodeOrderStatistic(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead)
{
	GFEX::Zx_OrderStatistic* pMsg = (GFEX::Zx_OrderStatistic*)arrBuf;

	std::stringstream sstream;
	CApiTool::AppendValue(sstream, "iZxSequence", pMsgHead->iSequence);
	CApiTool::AppendValue(sstream, "iMsgSize", pMsgHead->iMsgSize);
	CApiTool::AppendValue(sstream, "iMsgType", pMsgHead->iMsgType);

	CApiTool::AppendValue(sstream, "iSequence", pMsg->iSequence);
	CApiTool::AppendValue(sstream, "arrInstrument", CApiTool::GetString(pMsg->arrInstrument, sizeof(pMsg->arrInstrument)));

	CApiTool::AppendValue(sstream, "iTotalBuyOrderNum", pMsg->iTotalBuyOrderNum);
	CApiTool::AppendValue(sstream, "iTotalSellOrderNum", pMsg->iTotalSellOrderNum);
	CApiTool::AppendValue(sstream, "dWeightedAverageBuyOrderPrice", DoubleToString(pMsg->dWeightedAverageBuyOrderPrice, 4));
	CApiTool::AppendValue(sstream, "dWeightedAverageSellOrderPrice", DoubleToString(pMsg->dWeightedAverageSellOrderPrice, 4));

	return sstream.str();
}

std::string CLogMarket::DecodeMatchPriceQty(unsigned char* arrBuf, GFEX::Zx_MsgHead* pMsgHead)
{
	GFEX::Zx_MatchPriceQty* pMsg = (GFEX::Zx_MatchPriceQty*)arrBuf;

	std::stringstream sstream;
	CApiTool::AppendValue(sstream, "iZxSequence", pMsgHead->iSequence);
	CApiTool::AppendValue(sstream, "iMsgSize", pMsgHead->iMsgSize);
	CApiTool::AppendValue(sstream, "iMsgType", pMsgHead->iMsgType);

	CApiTool::AppendValue(sstream, "iSequence", pMsg->iSequence);
	CApiTool::AppendValue(sstream, "arrInstrument", CApiTool::GetString(pMsg->arrInstrument, sizeof(pMsg->arrInstrument)));

	CApiTool::AppendValue(sstream, "dPriceOne", DoubleToString(pMsg->dPriceOne, 4));
	CApiTool::AppendValue(sstream, "iPriceOneBOQty", pMsg->iPriceOneBOQty);
	CApiTool::AppendValue(sstream, "iPriceOneBEQty", pMsg->iPriceOneBEQty);
	CApiTool::AppendValue(sstream, "iPriceOneSOQty", pMsg->iPriceOneSOQty);
	CApiTool::AppendValue(sstream, "iPriceOneSEQty", pMsg->iPriceOneSEQty);

	CApiTool::AppendValue(sstream, "dPriceTwo", DoubleToString(pMsg->dPriceTwo, 4));
	CApiTool::AppendValue(sstream, "iPriceTwoBOQty", pMsg->iPriceTwoBOQty);
	CApiTool::AppendValue(sstream, "iPriceTwoBEQty", pMsg->iPriceTwoBEQty);
	CApiTool::AppendValue(sstream, "iPriceTwoSOQty", pMsg->iPriceTwoSOQty);
	CApiTool::AppendValue(sstream, "iPriceTwoSEQty", pMsg->iPriceTwoSEQty);

	CApiTool::AppendValue(sstream, "dPriceThree", DoubleToString(pMsg->dPriceThree, 4));
	CApiTool::AppendValue(sstream, "iPriceThreeBOQty", pMsg->iPriceThreeBOQty);
	CApiTool::AppendValue(sstream, "iPriceThreeBEQty", pMsg->iPriceThreeBEQty);
	CApiTool::AppendValue(sstream, "iPriceThreeSOQty", pMsg->iPriceThreeSOQty);
	CApiTool::AppendValue(sstream, "iPriceThreeSEQty", pMsg->iPriceThreeSEQty);

	CApiTool::AppendValue(sstream, "dPriceFour", DoubleToString(pMsg->dPriceFour, 4));
	CApiTool::AppendValue(sstream, "iPriceFourBOQty", pMsg->iPriceFourBOQty);
	CApiTool::AppendValue(sstream, "iPriceFourBEQty", pMsg->iPriceFourBEQty);
	CApiTool::AppendValue(sstream, "iPriceFourSOQty", pMsg->iPriceFourSOQty);
	CApiTool::AppendValue(sstream, "iPriceFourSEQty", pMsg->iPriceFourSEQty);

	CApiTool::AppendValue(sstream, "dPriceFive", DoubleToString(pMsg->dPriceFive, 4));
	CApiTool::AppendValue(sstream, "iPriceFiveBOQty", pMsg->iPriceFiveBOQty);
	CApiTool::AppendValue(sstream, "iPriceFiveBEQty", pMsg->iPriceFiveBEQty);
	CApiTool::AppendValue(sstream, "iPriceFiveSOQty", pMsg->iPriceFiveSOQty);
	CApiTool::AppendValue(sstream, "iPriceFiveSEQty", pMsg->iPriceFiveSEQty);

	return sstream.str();
}

std::string CLogMarket::DoubleToString(double iValue, unsigned int precision, bool bFlag)
{
	if (bFlag)
	{
		if (iValue == std::numeric_limits<double>::max())
		{
			iValue = 0;
		}
	}
	return CApiTool::DoubleToString(iValue, 4);
}
