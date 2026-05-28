#ifndef H_MDSZL1_RECV_H
#define H_MDSZL1_RECV_H

#include <iostream>

typedef void (*MdszCallBack)(uint8_t *data, int len);

struct MdszL1Receiver;

#pragma pack(push, 1)
struct SzseMsgHead
{
    uint32_t MsgType;
    uint32_t MsgLen;
};

struct SzseSnapShot
{
    uint32_t MsgType;
    uint32_t MsgLen;
    uint64_t OrigTime;
    uint16_t ChannelNo;
    char MDStreamID[3];
    char SecurityID[8];
    char SecurityIDSource[4];
    char TradingPhaseCode[8];
    uint64_t PrevClosePx;
    uint64_t NumTrades;
    uint64_t TotalVolumeTrade;
    uint64_t TotalValueTrade;
    uint32_t NoMDEntries;
};

struct SzseOrderMd
{
    uint32_t MsgType;
    uint32_t MsgLen;
    uint16_t ChannelNo;
    uint64_t ApplSeqNum;
    char MDStreamID[3];
    char SecurityID[8];
    char SecurityIDSource[4];
    uint64_t Price;
    uint64_t OrderQty;
    char Side;
    uint64_t TransactTime;
    char OrdType;
};

struct SzseTradeMd
{
    uint32_t MsgType;
    uint32_t MsgLen;
    uint16_t ChannelNo;
    uint64_t ApplSeqNum;
    char MDStreamID[3];
    uint64_t BidApplSeqNum;
    uint64_t OfferApplSeqNum;
    char SecurityID[8];
    char SecurityIDSource[4];
    uint64_t TradePx;
    uint64_t TradeQty;
    char ExecType;
    uint64_t TransactTime;
};

struct SzseMDEntry_300111
{
    uint16_t  MdEntryType;
    int64_t MdEntryPx;
    uint64_t MdEntrySize;
    uint16_t MdPriceLevel;
    uint64_t NumberOfOrders;
    uint32_t NoOrders = {0};
};

struct SzseMDEntry_300211
{
    uint16_t  MdEntryType; 
    uint64_t MdEntryPx;
    uint64_t MdEntrySize;
    uint16_t MdPriceLevel;
    uint64_t NumberOfOrders;
    uint32_t NoOrders = {0};
};

struct SzseMDEntry_309011
{
    uint16_t  MdEntryType;
    uint64_t MdEntryPx;
};

struct PriceVolume
{
    uint64_t BidPrice;
    uint64_t BidVolume;
    uint64_t AskPrice;
    uint64_t AskVolume;
};

struct SampleIndexMd
{
    uint32_t MsgType;
    uint32_t MsgLen;
    uint64_t OrigTime;
    uint16_t ChannelNo;
    char MDStreamID[4] = {0};
    char SecurityID[9] = {0};
    char SecurityIDSource[5] = {0};
    char TradingPhaseCode[9] = {0};
    uint64_t PrevClosePx;
    uint64_t LastPx;
    uint64_t NumTrades;
    uint64_t TotalVolumeTraded;
    uint64_t TotalValueTraded;
};

struct SampleSnapShotMd
{
    uint32_t MsgType;
    uint32_t MsgLen;
    uint64_t OrigTime;
    uint16_t ChannelNo;
    char MDStreamID[4] = {0};
    char SecurityID[9] = {0};
    char SecurityIDSource[5] = {0};
    char TradingPhaseCode[9] = {0};
    uint64_t PrevClosePx;
    uint64_t LastPx;
    uint64_t NumTrades;
    uint64_t TotalVolumeTraded;
    uint64_t TotalValueTraded;
    uint64_t BidWeightPx;
    uint64_t AskWeightPx;
    uint64_t BidWeightSize;
    uint64_t AskWeightSize;
    PriceVolume BidAskInfo[5];
};

#pragma pack(pop)


auto MdszReceiveCreate(MdszL1Receiver **rev, const char *configFile) -> int;

auto MdszReceiveRun(MdszL1Receiver *rev, MdszCallBack) -> void;

auto MdszReceiverClose(MdszL1Receiver *rev) -> void;

auto MdszGetOutputPath(MdszL1Receiver *rev) -> const char *;

#endif