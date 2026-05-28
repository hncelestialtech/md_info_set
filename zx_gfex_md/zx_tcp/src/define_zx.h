#pragma once
#include <stdint.h>
#include <string.h>
#include <unordered_map>
#include <iostream>
#include <memory>

namespace GFEX
{
	static const int g_iMaxFrameSize = 2048;

	//自定义消息
	const int32_t MsgZx_BestDeep = 1;
	const int32_t MsgZx_ArbBestDeep = 2;
	const int32_t MsgZx_TenEntrust = 3;
	const int32_t MsgZx_Real = 4;
	const int32_t MsgZx_OrderStatistic = 5;

	const int32_t MsgZx_MatchPriceQty = 6;


#pragma pack(push, 1)
	struct Zx_PacketHead
	{
		uint16_t iPktSize;				//包括Zx_Packet头的payload长度
		uint32_t iReserved;
	};

	struct Zx_MsgHead
	{
		uint32_t iSequence;				//自研转发系统sequence，按组播通道从1连续递增，用于判定是否丢包
		uint16_t iMsgSize;
		uint16_t iMsgType;
	};

	//深度行情_单腿0x31
	struct Zx_BestDeep
	{
		uint32_t iSequence;	//交易所原始sequence
		char arrInstrument[23];
		char arrGenTime[12];

		double dLastPrice;
		double dHighPrice;
		double dLowPrice;
		int32_t iLastMatchQty;

		int32_t iMatchTotQty;
		double dTurnover;
		int32_t iInitOpenInterest;
		int32_t iOpenInterest;
		int32_t iInterestChg;

		double dClearPrice;
		double dLastClearPrice;
		double dClosePrice;
		double dLastClosePrice;
		double dOpenPrice;

		double dLifeLow;
		double dLifeHigh;
		double dRiseLimit;
		double dFallLimit;
		double dAvePrice;


		double dBidPrice1;
		int32_t iBidVolume1;
		int32_t iBidImplyVolume1;
		double dBidPrice2;
		int32_t iBidVolume2;
		int32_t iBidImplyVolume2;

		double dBidPrice3;
		int32_t iBidVolume3;
		int32_t iBidImplyVolume3;
		double dBidPrice4;
		int32_t iBidVolume4;
		int32_t iBidImplyVolume4;

		double dBidPrice5;
		int32_t iBidVolume5;
		int32_t iBidImplyVolume5;

		double dAskPrice1;
		int32_t iAskVolume1;
		int32_t iAskImplyVolume1;
		double dAskPrice2;
		int32_t iAskVolume2;
		int32_t iAskImplyVolume2;

		double dAskPrice3;
		int32_t iAskVolume3;
		int32_t iAskImplyVolume3;
		double dAskPrice4;
		int32_t iAskVolume4;
		int32_t iAskImplyVolume4;

		double dAskPrice5;
		int32_t iAskVolume5;
		int32_t iAskImplyVolume5;
	};

	//深度行情_组合0x32
	struct Zx_ArbBestDeep
	{
		uint32_t iSequence;	//交易所原始sequence
		char arrInstrument[23];
		char arrGenTime[12];
		double dLastPrice;
		double dLowPrice;

		double dHighPrice;
		double dLifeLow;
		double dLifeHigh;
		double dRiseLimit;
		double dFallLimit;

		double dBidPrice1;
		int32_t iBidVolume1;
		int32_t iBidImplyVolume1;
		double dBidPrice2;
		int32_t iBidVolume2;
		int32_t iBidImplyVolume2;

		double dBidPrice3;
		int32_t iBidVolume3;
		int32_t iBidImplyVolume3;
		double dBidPrice4;
		int32_t iBidVolume4;
		int32_t iBidImplyVolume4;

		double dBidPrice5;
		int32_t iBidVolume5;
		int32_t iBidImplyVolume5;

		double dAskPrice1;
		int32_t iAskVolume1;
		int32_t iAskImplyVolume1;
		double dAskPrice2;
		int32_t iAskVolume2;
		int32_t iAskImplyVolume2;

		double dAskPrice3;
		int32_t iAskVolume3;
		int32_t iAskImplyVolume3;
		double dAskPrice4;
		int32_t iAskVolume4;
		int32_t iAskImplyVolume4;

		double dAskPrice5;
		int32_t iAskVolume5;
		int32_t iAskImplyVolume5;
	};

	struct Zx_TenEntrust//0x34
	{
		uint32_t iSequence;	//交易所原始sequence
		char arrInstrument[23];
		char arrGenTime[12];
		double dBestBuyOrderPrice;

		int32_t iBestBuyOrderQtyOne;
		int32_t iBestBuyOrderQtyTwo;
		int32_t iBestBuyOrderQtyThree;
		int32_t iBestBuyOrderQtyFour;
		int32_t iBestBuyOrderQtyFive;

		int32_t iBestBuyOrderQtySix;
		int32_t iBestBuyOrderQtySeven;
		int32_t iBestBuyOrderQtyEight;
		int32_t iBestBuyOrderQtyNine;
		int32_t iBestBuyOrderQtyTen;

		double dBestSellOrderPrice;

		int32_t iBestSellOrderQtyOne;
		int32_t iBestSellOrderQtyTwo;
		int32_t iBestSellOrderQtyThree;
		int32_t iBestSellOrderQtyFour;
		int32_t iBestSellOrderQtyFive;

		int32_t iBestSellOrderQtySix;
		int32_t iBestSellOrderQtySeven;
		int32_t iBestSellOrderQtyEight;
		int32_t iBestSellOrderQtyNine;
		int32_t iBestSellOrderQtyTen;
	};

	struct Zx_OrderStatistic//0x36
	{
		uint32_t iSequence;	//交易所原始sequence
		char arrInstrument[23];

		int32_t iTotalBuyOrderNum;
		int32_t iTotalSellOrderNum;
		double dWeightedAverageBuyOrderPrice;
		double dWeightedAverageSellOrderPrice;
	};

	struct Zx_Real//0x35
	{
		uint32_t iSequence;	//交易所原始sequence
		char arrInstrument[23];
		double dRealTimePrice;
	};

	struct Zx_MatchPriceQty//0x37
	{
		uint32_t iSequence;	//交易所原始sequence
		char arrInstrument[23];

		double dPriceOne;
		int32_t iPriceOneBOQty;
		int32_t iPriceOneBEQty;
		int32_t iPriceOneSOQty;
		int32_t iPriceOneSEQty;

		double dPriceTwo;
		int32_t iPriceTwoBOQty;
		int32_t iPriceTwoBEQty;
		int32_t iPriceTwoSOQty;
		int32_t iPriceTwoSEQty;

		double dPriceThree;
		int32_t iPriceThreeBOQty;
		int32_t iPriceThreeBEQty;
		int32_t iPriceThreeSOQty;
		int32_t iPriceThreeSEQty;

		double dPriceFour;
		int32_t iPriceFourBOQty;
		int32_t iPriceFourBEQty;
		int32_t iPriceFourSOQty;
		int32_t iPriceFourSEQty;

		double dPriceFive;
		int32_t iPriceFiveBOQty;
		int32_t iPriceFiveBEQty;
		int32_t iPriceFiveSOQty;
		int32_t iPriceFiveSEQty;
	};

#pragma pack(pop)
}
