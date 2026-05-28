#include <cstdio>
#include <errno.h>
#include <assert.h>
#include "InnerData.h"
#include "DataWriter.h"
#include <cstring>
#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
#if _MSC_VER
#define snprintf _snprintf
#endif
#define PRId64 "lld"
#endif

#include "HSNsqSpiImpl.h"

#include "Functions.h"


extern UserInfo g_user_info;

CHSNsqSpiImpl::CHSNsqSpiImpl(CHSNsqApi *lpHSNsqApi, OutPutData output_data)
    : m_lpHSNsqApi(lpHSNsqApi), m_output_data(output_data)
{
    if (OutPutData::SCREEN != output_data)
    {
        // 初始化缓存
        m_cache_manage.reset(new CacheManage());
        (void)m_cache_manage->Init(10000);
    }
}
CHSNsqSpiImpl::~CHSNsqSpiImpl()
{
    for (auto &fp : m_static_file)
    {
        if (fp.second)
        {
            fclose(fp.second);
            fp.second = NULL;
        }
    }
}

void CHSNsqSpiImpl::OnFrontConnected()
{
    printf("OnFrontConnected errinfo=%s\n", m_lpHSNsqApi->GetApiErrorMsg(0));

    static int _s_nLoginReqId = 10000;
    //// 连接成功或断开重连后，需要调用登录接口

    int iRet = m_lpHSNsqApi->ReqUserLogin(&g_user_info.reqLoginField, _s_nLoginReqId++);
    if (iRet != 0)
    {
        printf("ERROR: login failed: iRet %d, error: %s\n", iRet, m_lpHSNsqApi->GetApiErrorMsg(iRet));
    }

    m_isConnected = true;
}

void CHSNsqSpiImpl::OnFrontDisconnected(int nResult)
{
    printf("OnFrontDisconnected nResult %d errinfo=%s\n", nResult, m_lpHSNsqApi->GetApiErrorMsg(nResult));
    // 断线后，SDK自动重新连接
    // 重新连接成功后，OnFrontConnected中需要重新向服务器发起登陆请求
}

void CHSNsqSpiImpl::OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspUserLogin nRequestID[%d] errno=%d errinfo=%s bIsLast=%d\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg, bIsLast);
    if (0 == pRspInfo->ErrorID)
        m_isLogined = true;
}

void CHSNsqSpiImpl::OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspSecuDepthMarketDataSubscribe: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspSecuDepthMarketDataCancel: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count)
{
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        SockSnapshot snapshot;
        snapshot.max_ask1_count = MaxAsk1Count;
        snapshot.max_bid1_count = MaxBid1Count;
        snapshot.bid1_count = Bid1Count;
        snapshot.ask1_count = Ask1Count;
        memcpy(&snapshot.snapshot, pSecuDepthMarketData, sizeof(CHSNsqSecuDepthMarketDataField));
        memcpy(&snapshot.bid_volume, Bid1Volume, sizeof(int64_t) * (Bid1Count > 50 ? 50 : Bid1Count));
        memcpy(&snapshot.ask_volume, Ask1Volume, sizeof(int64_t) * (Ask1Count > 50 ? 50 : Ask1Count));
        m_cache_manage->PutCache(pSecuDepthMarketData->ExchangeID, (char *)&snapshot, DataType::DT_SNAPSHOT);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnSecuDepthMarketData]: ExchangeID %s, InstrumentID %s, TradeDate %d, UpdateTime %d, PreClosePrice %lf, "
               "UpperLimitPrice %lf, LowerLimitPrice %lf, LastPrice %lf, AveragePrice %lf, TradeVolume %" PRId64 ", ChannelNo %d, IOPV %lf, "
               "EtfBuycount %d, EtfSellCount %d, EtfBuyVolume %" PRId64 ", EtfBuyBalance %lf, EtfSellVolume %" PRId64 ", EtfSellBalance %lf, "
               "InstrumentTradeStatus %c,\n"
               "\tBid1Price %lf, Bid1Volume %" PRId64 ", Bid1Volume[0] %" PRId64 ", Bid1Count %d, MaxBid1Count %d\n"
               "\tAsk1Price %lf, Ask1Volume %" PRId64 ", Ask1Volume[0] %" PRId64 ", Ask1Count %d, MaxAsk1Count %d\n",
               pSecuDepthMarketData->ExchangeID,
               pSecuDepthMarketData->InstrumentID,
               pSecuDepthMarketData->TradeDate,
               pSecuDepthMarketData->UpdateTime,
               pSecuDepthMarketData->PreClosePrice,
               pSecuDepthMarketData->UpperLimitPrice,
               pSecuDepthMarketData->LowerLimitPrice,
               pSecuDepthMarketData->LastPrice,
               pSecuDepthMarketData->AveragePrice,
               pSecuDepthMarketData->TradeVolume,
               pSecuDepthMarketData->ChannelNo,
               pSecuDepthMarketData->IOPV,
               pSecuDepthMarketData->EtfBuycount,
               pSecuDepthMarketData->EtfSellCount,
               pSecuDepthMarketData->EtfBuyVolume,
               pSecuDepthMarketData->EtfBuyBalance,
               pSecuDepthMarketData->EtfSellVolume,
               pSecuDepthMarketData->EtfSellBalance,
               pSecuDepthMarketData->InstrumentTradeStatus,

               pSecuDepthMarketData->BidPrice[0], pSecuDepthMarketData->BidVolume[0], Bid1Count > 0 ? Bid1Volume[0] : 0, Bid1Count, MaxBid1Count,
               pSecuDepthMarketData->AskPrice[0], pSecuDepthMarketData->AskVolume[0], Ask1Count > 0 ? Ask1Volume[0] : 0, Ask1Count, MaxAsk1Count);
    }
}

void CHSNsqSpiImpl::OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count)
{
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        AtpSockSnapshot atp_snapshot;
        atp_snapshot.max_ask1_count = MaxAsk1Count;
        atp_snapshot.max_bid1_count = MaxBid1Count;
        atp_snapshot.bid1_count = Bid1Count;
        atp_snapshot.ask1_count = Ask1Count;
        memcpy(&atp_snapshot.atp_snapshot, pSecuDepthMarketData, sizeof(CHSNsqSecuATPMarketDataField));
        memcpy(&atp_snapshot.bid_volume, Bid1Volume, sizeof(int64_t) * (Bid1Count > 50 ? 50 : Bid1Count));
        memcpy(&atp_snapshot.ask_volume, Ask1Volume, sizeof(int64_t) * (Ask1Count > 50 ? 50 : Ask1Count));
        m_cache_manage->PutCache(pSecuDepthMarketData->ExchangeID, (char *)&atp_snapshot, DataType::DT_ATP_SNAPSHOT);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("OnRtnSecuATPMarketData: ExchangeID %s, InstrumentID %s, TradeDate %d, UpdateTime %d, PreClosePrice %lf, "
               "ClosePrice %lf, InstrumentTradeStatus %c, TradeVolume  %" PRId64 ", ChannelNo %d,\n"
               "\tBidPrice1 %lf, BidVolume1 %" PRId64 ", Bid1Volume[0] %" PRId64 ", Bid1Count %d, MaxBid1Count %d\n"
               "\tAskPrice1 %lf, AskVolume1 %" PRId64 ", Ask1Volume[0] %" PRId64 ", Ask1Count %d, MaxAsk1Count %d\n",
               pSecuDepthMarketData->ExchangeID,
               pSecuDepthMarketData->InstrumentID,
               pSecuDepthMarketData->TradeDate,
               pSecuDepthMarketData->UpdateTime,
               pSecuDepthMarketData->PreClosePrice,
               pSecuDepthMarketData->ClosePrice,
               pSecuDepthMarketData->InstrumentTradeStatus,
               pSecuDepthMarketData->TradeVolume,
               pSecuDepthMarketData->ChannelNo,

               pSecuDepthMarketData->BidPrice1, pSecuDepthMarketData->BidVolume1, Bid1Count > 0 ? Bid1Volume[0] : 0, Bid1Count, MaxBid1Count,
               pSecuDepthMarketData->AskPrice1, pSecuDepthMarketData->AskVolume1, Ask1Count > 0 ? Ask1Volume[0] : 0, Ask1Count, MaxAsk1Count);
    }
}

void CHSNsqSpiImpl::OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspSecuTransactionSubscribe: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspSecuTransactionCancel: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData)
{
    assert(pSecuTransactionTradeData->ChannelNo > 0 && pSecuTransactionTradeData->ChannelNo < 5000);
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        StockTick tick;
        tick.data_type = DataType::DT_STOCK_TRADE;
        memcpy(&tick.trade, pSecuTransactionTradeData, sizeof(CHSNsqSecuTransactionTradeDataField));
        m_cache_manage->PutCache(pSecuTransactionTradeData->ExchangeID, (char *)&tick, DataType::DT_STOCK_TRADE);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnSecuTransactionTradeData]: ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
               " TradeDate %d, TransactTime %d, TrdPrice %lf, TrdVolume %" PRId64 ", TrdMoney %lf,"
               " TrdBuyNo %" PRId64 ", TrdSellNo %" PRId64 ", TrdBSFlag %c, BizIndex %" PRId64 "\n",
               pSecuTransactionTradeData->ExchangeID,
               pSecuTransactionTradeData->InstrumentID,
               pSecuTransactionTradeData->TransFlag,
               pSecuTransactionTradeData->SeqNo,
               pSecuTransactionTradeData->ChannelNo,
               pSecuTransactionTradeData->TradeDate,
               pSecuTransactionTradeData->TransactTime,
               pSecuTransactionTradeData->TrdPrice,
               pSecuTransactionTradeData->TrdVolume,
               pSecuTransactionTradeData->TrdMoney,
               pSecuTransactionTradeData->TrdBuyNo,
               pSecuTransactionTradeData->TrdSellNo,
               pSecuTransactionTradeData->TrdBSFlag,
               pSecuTransactionTradeData->BizIndex);
    }
}

void CHSNsqSpiImpl::OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData)
{
    assert(pSecuTransactionEntrustData->ChannelNo > 0 && pSecuTransactionEntrustData->ChannelNo < 5000);
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        StockTick tick;
        tick.data_type = DataType::DT_STOCK_ORDER;
        memcpy(&tick.order, pSecuTransactionEntrustData, sizeof(CHSNsqSecuTransactionEntrustDataField));
        m_cache_manage->PutCache(pSecuTransactionEntrustData->ExchangeID, (char *)&tick, DataType::DT_STOCK_TRADE);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnSecuTransactionEntrustData]: ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
               " TradeDate %d, TransactTime %d, OrdPrice %lf, OrdVolume %" PRId64 ", OrdSide %c, TickStatus %c,"
               " OrdType %c, OrdNo %" PRId64 ", BizIndex %" PRId64 ", TrdVolume %" PRId64 "\n",
               pSecuTransactionEntrustData->ExchangeID,
               pSecuTransactionEntrustData->InstrumentID,
               pSecuTransactionEntrustData->TransFlag,
               pSecuTransactionEntrustData->SeqNo,
               pSecuTransactionEntrustData->ChannelNo,
               pSecuTransactionEntrustData->TradeDate,
               pSecuTransactionEntrustData->TransactTime,
               pSecuTransactionEntrustData->OrdPrice,
               pSecuTransactionEntrustData->OrdVolume,
               pSecuTransactionEntrustData->OrdSide,
               pSecuTransactionEntrustData->TickStatus,
               pSecuTransactionEntrustData->OrdType,
               pSecuTransactionEntrustData->OrdNo,
               pSecuTransactionEntrustData->BizIndex,
               pSecuTransactionEntrustData->TrdVolume);
    }
}

void CHSNsqSpiImpl::OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData)
{
}

void CHSNsqSpiImpl::OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        printf("OnRspQrySecuInstruments: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        return;
    }
    char sz_buf[1024];
    int buf_len;
    if (0 == m_iAllInstrCount)
    {
        printf("RspQrySecuInstruments Successed !\n");
    }
    buf_len = snprintf(sz_buf, 1024, "[OnRspQrySecuInstruments]: ExchangeID %s, InstrumentID %s, InstrumentName %s, "
                                     "SecurityType %c, PreClosePrice %lf, UpperLimitPrice %lf, LowerLimitPrice %lf, PriceTick %lf, BuyVolumeUnit %d, "
                                     "SellVolumeUnit %d, TradeDate %d, SubSecurityType %s, ListDate %d, "
                                     "SpecialMarker %#llx, OutstandingShare %" PRId64 ", PublicFloatShareQuantity %" PRId64 ", nRequestID %d, bIsLast %d\n",
                       pSecuInstrumentStaticInfo->ExchangeID,
                       pSecuInstrumentStaticInfo->InstrumentID,
                       pSecuInstrumentStaticInfo->InstrumentName,
                       pSecuInstrumentStaticInfo->SecurityType,
                       pSecuInstrumentStaticInfo->PreClosePrice,
                       pSecuInstrumentStaticInfo->UpperLimitPrice,
                       pSecuInstrumentStaticInfo->LowerLimitPrice,
                       pSecuInstrumentStaticInfo->PriceTick,
                       pSecuInstrumentStaticInfo->BuyVolumeUnit,
                       pSecuInstrumentStaticInfo->SellVolumeUnit,
                       pSecuInstrumentStaticInfo->TradeDate,
                       pSecuInstrumentStaticInfo->SubSecurityType,
                       pSecuInstrumentStaticInfo->ListDate,
                       pSecuInstrumentStaticInfo->SpecialMarker,
                       pSecuInstrumentStaticInfo->OutstandingShare,
                       pSecuInstrumentStaticInfo->PublicFloatShareQuantity,
                       nRequestID,
                       bIsLast);

#ifdef ENABLE_IPC_TIMESTAMP_DEBUG
    printf("%s", sz_buf);
#endif

    if (strcmp(pSecuInstrumentStaticInfo->ExchangeID, HS_EI_SZSE) == 0)
    {
        writeInstrumentData(pSecuInstrumentStaticInfo->ExchangeID, "sz_stock_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pSecuInstrumentStaticInfo->ExchangeID, HS_EI_SSE) == 0)
    {
        writeInstrumentData(pSecuInstrumentStaticInfo->ExchangeID, "sh_stock_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pSecuInstrumentStaticInfo->ExchangeID, HS_EI_TZASE) == 0)
    {
        writeInstrumentData(pSecuInstrumentStaticInfo->ExchangeID, "neeq_stock_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pSecuInstrumentStaticInfo->ExchangeID, HS_EI_SWI) == 0)
    {
        writeInstrumentData(pSecuInstrumentStaticInfo->ExchangeID, "swi_stock_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pSecuInstrumentStaticInfo->ExchangeID, HS_EI_BJSE) == 0)
    {
        writeInstrumentData(pSecuInstrumentStaticInfo->ExchangeID, "bj_code.txt", sz_buf, buf_len, bIsLast);
    }

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

/// Description: 期权订阅-行情应答
void CHSNsqSpiImpl::OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspOptDepthMarketDataSubscribe: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

/// Description: 期权订阅取消-行情应答
void CHSNsqSpiImpl::OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspOptDepthMarketDataCancel: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}
extern FILE *all_opt;
/// Description: 主推-期权行情
void CHSNsqSpiImpl::OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData)
{
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        m_cache_manage->PutCache(pOptDepthMarketData->ExchangeID, (char *)pOptDepthMarketData, DataType::DT_OPT_SNAPSHOT);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        CHSNsqOptDepthMarketDataField *p = pOptDepthMarketData;
        printf("[OnRtnOptDepthMarketData] ExchangeID %s, InstrumentID %s, TradeDate %d, UpdateTime %d, LastPrice %lf, PreClosePrice %lf, OpenPrice %lf, HighPrice %lf, LowPrice %lf, ClosePrice %lf, "
               "PreOpenInterest %" PRId64 ", OpenInterest %" PRId64 ", PreSettlementPrice %lf, SettlementPrice %lf, UpperLimitPrice %lf, LowerLimitPrice %lf, PreDelta %lf, CurDelta %lf, "
               "TradeVolume %" PRId64 ", TradeBalance %lf, AveragePrice %lf, TradesNum %" PRId64 ", InstrumentTradeStatus %c, OpenRestriction %c, AuctionPrice %lf, AuctionVolume %" PRId64 ", "
               "LastEnquiryTime %d, LeaveQty %" PRId64 "\n",
               p->ExchangeID, p->InstrumentID, p->TradeDate, p->UpdateTime, p->LastPrice, p->PreClosePrice, p->OpenPrice, p->HighPrice, p->LowPrice, p->ClosePrice,
               p->PreOpenInterest, p->OpenInterest, p->PreSettlementPrice, p->SettlementPrice, p->UpperLimitPrice, p->LowerLimitPrice, p->PreDelta, p->CurDelta,
               p->TradeVolume, p->TradeBalance, p->AveragePrice, p->TradesNum, p->InstrumentTradeStatus, p->OpenRestriction[0], p->AuctionPrice, p->AuctionVolume,
               p->LastEnquiryTime, p->LeaveQty);
        int i;
        for (i = 0; i < 5; i++)
        {
            printf("bid[%d]:Price:%lf,Volume:%" PRId64 "\n", i, p->BidPrice[i], p->BidVolume[i]);
        }
        for (i = 0; i < 5; i++)
        {
            printf("ask[%d]:Price:%lf,Volume:%" PRId64 "\n", i, p->AskPrice[i], p->AskVolume[i]);
        }
    }
}

/// Description: 获取当前交易日合约应答
void CHSNsqSpiImpl::OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        printf("OnRspQryOptInstruments: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        return;
    }
    char sz_buf[1024];
    int buf_len;
    CHSNsqOptInstrumentStaticInfoField *p = pOptInstrumentStaticInfo;
    if (0 == m_iAllInstrCount)
    {
        printf("RspQryOptInstruments Successed !\n");
    }

    if (strcmp(p->ExchangeID, HS_EI_SZSE) == 0 || strcmp(p->ExchangeID, HS_EI_SSE) == 0)
    {
        buf_len = snprintf(sz_buf, 1024, "ExchangeID[%s], InstrumentID[%s], InstrumentTradeID[%s], InstrumentName[%s], ", p->ExchangeID, p->InstrumentID, p->InstrumentTradeID, p->InstrumentName);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "SecurityType[%c], UnderlyingInstrID[%s], OptionsType[%c], ExerciseStyle[%c], ", p->SecurityType, p->UnderlyingInstrID, p->OptionsType, p->ExerciseStyle);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "ContractMultiplierUnit[%" PRId64 "], ExercisePrice[%lf], StartDate[%d], EndDate[%d], ", p->ContractMultiplierUnit, p->ExercisePrice, p->StartDate, p->EndDate);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "ExerciseDate[%d], DeliveryDate[%d], ExpireDate[%d], TotalLongPosition[%" PRId64 "], ", p->ExerciseDate, p->DeliveryDate, p->ExpireDate, p->TotalLongPosition);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "PreClosePrice[%lf], PreSettlPrice[%lf], UnderlyingClosePrice[%lf], UpperLimitPrice[%lf], LowerLimitPrice[%lf], ", p->PreClosePrice, p->PreSettlPrice, p->UnderlyingClosePrice, p->UpperLimitPrice, p->LowerLimitPrice);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "MarginUnit[%lf], MarginRatioParam1[%lf], MarginRatioParam2[%lf], VolumeMultiple[%" PRId64 "], ", p->MarginUnit, p->MarginRatioParam1, p->MarginRatioParam2, p->VolumeMultiple);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "MinLimitOrderVolume[%" PRId64 "], MaxLimitOrderVolume[%" PRId64 "], MinMarketOrderVolume[%" PRId64 "], ", p->MinLimitOrderVolume, p->MaxLimitOrderVolume, p->MinMarketOrderVolume);
        buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len, "MaxMarketOrderVolume[%" PRId64 "], PriceTick[%lf], TradeDate[%d], nRequestID[%d], bIsLast [%d]\n", p->MaxMarketOrderVolume, p->PriceTick, p->TradeDate, nRequestID, bIsLast);
    }
    else
    {
        buf_len = snprintf(sz_buf, 1024, "[OnRspQryOptInstruments]: ExchangeID %s, InstrumentID %s, InstrumentName %s, SecurityType %c, PreClosePrice %lf, UpperLimitPrice %lf, LowerLimitPrice %lf, PriceTick %lf, BuyVolumeUnit %d, SellVolumeUnit %d, TradeDate %d, bIsLast %d\n",
                           p->ExchangeID,
                           p->InstrumentID,
                           p->InstrumentName,
                           p->SecurityType,
                           p->PreClosePrice,
                           p->UpperLimitPrice,
                           p->LowerLimitPrice,
                           p->PriceTick,
                           (int)p->ContractMultiplierUnit,
                           (int)p->ContractMultiplierUnit,
                           p->TradeDate,
                           bIsLast);
    }

#ifdef ENABLE_IPC_TIMESTAMP_DEBUG
    printf("%s", sz_buf);
#endif
    std::string key = std::string(p->ExchangeID) + "_opt";
    if (strcmp(p->ExchangeID, HS_EI_SZSE) == 0)
    {
        writeInstrumentData(key.c_str(), "sz_opt_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(p->ExchangeID, HS_EI_SSE) == 0)
    {
        writeInstrumentData(key.c_str(), "sh_opt_code.txt", sz_buf, buf_len, bIsLast);
    }

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

/// Description: 获取合约的最新快照信息应答
void CHSNsqSpiImpl::OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

void CHSNsqSpiImpl::OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("[OnRspSecuTransactionData]: CHSNsqRspInfoField ErrorID %d ErrorMsg %s bIsLast %d\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg, bIsLast);
    // SF_REBU_OK:0, SF_REBU_PART_DISCARD:1
    if (pRspInfo->ErrorID == 0 || pRspInfo->ErrorID == 1)
    {
        CHSNsqSecuTransactionDataField *p = pSecuTransactionData;
        if (p->TransType == HS_RTT_Trade)
        {
            printf("[OnRspSecuTransactionData]: ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
                   " TradeDate %d, TransactTime %d, TrdPrice %lf, TrdVolume %" PRId64 ", TrdMoney %lf,"
                   " TrdBuyNo %" PRId64 ", TrdSellNo %" PRId64 ", TrdBSFlag %c, BizIndex %" PRId64 "\n",
                   p->TradeData.ExchangeID,
                   p->TradeData.InstrumentID,
                   p->TradeData.TransFlag,
                   p->TradeData.SeqNo,
                   p->TradeData.ChannelNo,
                   p->TradeData.TradeDate,
                   p->TradeData.TransactTime,
                   p->TradeData.TrdPrice,
                   p->TradeData.TrdVolume,
                   p->TradeData.TrdMoney,
                   p->TradeData.TrdBuyNo,
                   p->TradeData.TrdSellNo,
                   p->TradeData.TrdBSFlag,
                   p->TradeData.BizIndex);
        }
        else if (p->TransType == HS_RTT_Entrust)
        {
            printf("[OnRspSecuTransactionData]: ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
                   " TradeDate %d, TransactTime %d, OrdPrice %lf, OrdVolume %" PRId64 ", OrdSide %c,"
                   " OrdType %c, OrdNo %" PRId64 ", BizIndex %" PRId64 ", TrdVolume %" PRId64 "\n",
                   p->EntrustData.ExchangeID,
                   p->EntrustData.InstrumentID,
                   p->EntrustData.TransFlag,
                   p->EntrustData.SeqNo,
                   p->EntrustData.ChannelNo,
                   p->EntrustData.TradeDate,
                   p->EntrustData.TransactTime,
                   p->EntrustData.OrdPrice,
                   p->EntrustData.OrdVolume,
                   p->EntrustData.OrdSide,
                   p->EntrustData.OrdType,
                   p->EntrustData.OrdNo,
                   p->EntrustData.BizIndex,
                   p->EntrustData.TrdVolume);
        }
        else if (p->TransType == HS_RTT_BOND_Trade)
        {

            CHSNsqBondTransactionTradeDataField *pTransactionTradeData = &(p->BondTradeData);
            printf("ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
                   " TradeDate %d, TransactTime %d, TrdPrice %lf, TrdVolume %" PRId64 ","
                   " TrdBuyNo %" PRId64 ", TrdSellNo %" PRId64 ", TrdBSFlag %c, TradeType %d, SettlPeriod %d, SettlType %d\n",
                   pTransactionTradeData->ExchangeID,
                   pTransactionTradeData->InstrumentID,
                   pTransactionTradeData->TransFlag,
                   pTransactionTradeData->SeqNo,
                   pTransactionTradeData->ChannelNo,
                   pTransactionTradeData->TradeDate,
                   pTransactionTradeData->TransactTime,
                   pTransactionTradeData->TrdPrice,
                   pTransactionTradeData->TrdVolume,
                   pTransactionTradeData->TrdBuyNo,
                   pTransactionTradeData->TrdSellNo,
                   pTransactionTradeData->TrdBSFlag,
                   pTransactionTradeData->TradeType,
                   pTransactionTradeData->SettlPeriod,
                   pTransactionTradeData->SettlType);
        }
        else if (p->TransType == HS_RTT_BOND_Entrust)
        {
            CHSNsqBondTransactionEntrustDataField *pTransactionEntrustData = &(p->BondEntrustData);
            printf("ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
                   " TradeDate %d, TransactTime %d, OrdPrice %lf, OrdVolume %" PRId64 ", OrdSide %c,"
                   " OrdType %c, TradeType %d, SettlPeriod %d, SettlType %d, QuoteID %s, MemberID %s, InvestorType %s, InvestorID %s, TraderCode %s\n",
                   pTransactionEntrustData->ExchangeID,
                   pTransactionEntrustData->InstrumentID,
                   pTransactionEntrustData->TransFlag,
                   pTransactionEntrustData->SeqNo,
                   pTransactionEntrustData->ChannelNo,
                   pTransactionEntrustData->TradeDate,
                   pTransactionEntrustData->TransactTime,
                   pTransactionEntrustData->OrdPrice,
                   pTransactionEntrustData->OrdVolume,
                   pTransactionEntrustData->OrdSide,
                   pTransactionEntrustData->OrdType,
                   pTransactionEntrustData->TradeType,
                   pTransactionEntrustData->SettlPeriod,
                   pTransactionEntrustData->SettlType,
                   pTransactionEntrustData->QuoteID,
                   pTransactionEntrustData->MemberID,
                   pTransactionEntrustData->InvestorType,
                   pTransactionEntrustData->InvestorID,
                   pTransactionEntrustData->TraderCode);
        }
        else
        {
            printf("error TransType:%c\n", p->TransType);
        }
    }
}

void CHSNsqSpiImpl::OnRspSecuTransactionDataTimeout(int nRequestID)
{
    printf("OnRspSecuTransactionDataTimeout nRequestID=%d\n", nRequestID);
}

/// Description: 获取港股通当前交易日合约应答
void CHSNsqSpiImpl::OnRspQryHktInstruments(CHSNsqHktInstrumentStaticInfoField *pHktInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        printf("OnRspQryHktInstruments: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        return;
    }
    char sz_buf[1024];
    int buf_len;

    if (0 == m_iAllInstrCount)
    {
        printf("RspQryHktInstruments Successed !\n");
    }

    buf_len = snprintf(sz_buf, 1024, "[OnRspQryHktInstruments]: ExchangeID %s, InstrumentID %s, InstrumentName %s, SecurityType %c, PreClosePrice %lf, BuyVolumeUnit %d, SellVolumeUnit %d, TradeDate %d, nRequestID %d, bIsLast %d\n",
                       pHktInstrumentStaticInfo->ExchangeID,
                       pHktInstrumentStaticInfo->InstrumentID,
                       pHktInstrumentStaticInfo->InstrumentName,
                       pHktInstrumentStaticInfo->SecurityType,
                       pHktInstrumentStaticInfo->PreClosePrice,
                       pHktInstrumentStaticInfo->BuyVolumeUnit,
                       pHktInstrumentStaticInfo->SellVolumeUnit,
                       pHktInstrumentStaticInfo->TradeDate,
                       nRequestID,
                       bIsLast);

#ifdef ENABLE_IPC_TIMESTAMP_DEBUG
    printf("%s", sz_buf);
#endif

    if (strcmp(pHktInstrumentStaticInfo->ExchangeID, HS_EI_SZSEHK) == 0)
    {
        writeInstrumentData(pHktInstrumentStaticInfo->ExchangeID, "sz_hkt_code.txt", sz_buf, buf_len, bIsLast);
    }
    else
    {
        writeInstrumentData(pHktInstrumentStaticInfo->ExchangeID, "sh_hkt_code.txt", sz_buf, buf_len, bIsLast);
    }

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

// Description: 主推-港股通行情
void CHSNsqSpiImpl::OnRtnHktDepthMarketData(CHSNsqHktDepthMarketDataField *pHktDepthMarketData)
{
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        const char *ExchangeID = pHktDepthMarketData->ExchangeID[0] == HS_EI_SSEHK[0] ? HS_EI_SSE : HS_EI_SZSE;
        m_cache_manage->PutCache(ExchangeID, (char *)pHktDepthMarketData, DataType::DT_HKT_SNAPSHOT);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnHktDepthMarketData]: ExchangeID %s, InstrumentID %s, TradeDate %d, UpdateTime %d, PreClosePrice %lf, "
               "UpperPrice %lf, LowPrice %lf, LastPrice %lf, NomianlPrice %lf, TradeVolume %" PRId64 ", ChannelNo %d,\n"
               "\tBid1Price %lf, Bid1Volume %" PRId64 "\n"
               "\tAsk1Price %lf, Ask1Volume %" PRId64 "\n",
               pHktDepthMarketData->ExchangeID,
               pHktDepthMarketData->InstrumentID,
               pHktDepthMarketData->TradeDate,
               pHktDepthMarketData->UpdateTime,
               pHktDepthMarketData->PreClosePrice,
               pHktDepthMarketData->HighPrice,
               pHktDepthMarketData->LowPrice,
               pHktDepthMarketData->LastPrice,
               pHktDepthMarketData->NomianlPrice,
               pHktDepthMarketData->TradeVolume,
               pHktDepthMarketData->ChannelNo,

               pHktDepthMarketData->BidPrice[0], pHktDepthMarketData->BidVolume[0],
               pHktDepthMarketData->AskPrice[0], pHktDepthMarketData->AskVolume[0]);
    }
}

/// Description: 主推-深证债券逐笔成交行情
void CHSNsqSpiImpl::OnRtnBondTransactionTradeData(CHSNsqBondTransactionTradeDataField *pTransactionTradeData)
{
    assert(pTransactionTradeData->ChannelNo > 0 && pTransactionTradeData->ChannelNo < 5000);
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        BondTick tick;
        tick.data_type = DataType::DT_BOND_TRADE;
        memcpy(&tick.trade, pTransactionTradeData, sizeof(tick.trade));
        m_cache_manage->PutCache(pTransactionTradeData->ExchangeID, (char *)&tick, DataType::DT_BOND_TRADE);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnBondTransactionTradeData]: ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
               " TradeDate %d, TransactTime %d, TrdPrice %lf, TrdVolume %" PRId64 ","
               " TrdBuyNo %" PRId64 ", TrdSellNo %" PRId64 ", TrdBSFlag %c, TradeType %d, SettlPeriod %d, SettlType %d\n",
               pTransactionTradeData->ExchangeID,
               pTransactionTradeData->InstrumentID,
               pTransactionTradeData->TransFlag,
               pTransactionTradeData->SeqNo,
               pTransactionTradeData->ChannelNo,
               pTransactionTradeData->TradeDate,
               pTransactionTradeData->TransactTime,
               pTransactionTradeData->TrdPrice,
               pTransactionTradeData->TrdVolume,
               pTransactionTradeData->TrdBuyNo,
               pTransactionTradeData->TrdSellNo,
               pTransactionTradeData->TrdBSFlag,
               pTransactionTradeData->TradeType,
               pTransactionTradeData->SettlPeriod,
               pTransactionTradeData->SettlType);
    }
}

/// Description: 主推-深证债券逐笔委托行情
void CHSNsqSpiImpl::OnRtnBondTransactionEntrustData(CHSNsqBondTransactionEntrustDataField *pTransactionEntrustData)
{
    assert(pTransactionEntrustData->ChannelNo > 0 && pTransactionEntrustData->ChannelNo < 5000);
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        BondTick tick;
        tick.data_type = DataType::DT_BOND_ORDER;
        memcpy(&tick.order, pTransactionEntrustData, sizeof(CHSNsqBondTransactionEntrustDataField));
        m_cache_manage->PutCache(pTransactionEntrustData->ExchangeID, (char *)&tick, DataType::DT_BOND_TRADE);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnBondTransactionEntrustData]: ExchangeID %s, InstrumentID %s, TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
               " TradeDate %d, TransactTime %d, OrdPrice %lf, OrdVolume %" PRId64 ", OrdSide %c,"
               " OrdType %c, TradeType %d, SettlPeriod %d, SettlType %d, QuoteID %s, MemberID %s, InvestorType %s, InvestorID %s, TraderCode %s\n",
               pTransactionEntrustData->ExchangeID,
               pTransactionEntrustData->InstrumentID,
               pTransactionEntrustData->TransFlag,
               pTransactionEntrustData->SeqNo,
               pTransactionEntrustData->ChannelNo,
               pTransactionEntrustData->TradeDate,
               pTransactionEntrustData->TransactTime,
               pTransactionEntrustData->OrdPrice,
               pTransactionEntrustData->OrdVolume,
               pTransactionEntrustData->OrdSide,
               pTransactionEntrustData->OrdType == 0 ? '-' : pTransactionEntrustData->OrdType,
               pTransactionEntrustData->TradeType,
               pTransactionEntrustData->SettlPeriod,
               pTransactionEntrustData->SettlType,
               pTransactionEntrustData->QuoteID,
               pTransactionEntrustData->MemberID,
               pTransactionEntrustData->InvestorType,
               pTransactionEntrustData->InvestorID,
               pTransactionEntrustData->TraderCode);
    }
}

/// Description: 订阅-现货快照Plus行情应答
void CHSNsqSpiImpl::OnRspSecuDepthMarketDataPlusSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspSecuDepthMarketDataPlusSubscribe: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

/// Description: 订阅取消-现货快照Plus行情应答
void CHSNsqSpiImpl::OnRspSecuDepthMarketDataPlusCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspSecuDepthMarketDataPlusCancel: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

/// Description: 主推-现货快照Plus行情
void CHSNsqSpiImpl::OnRtnSecuDepthMarketDataPlus(CHSNsqSecuDepthMarketDataPlusField *pSecuDepthMarketDataPlus)
{
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        m_cache_manage->PutCache(pSecuDepthMarketDataPlus->ExchangeID, (char *)pSecuDepthMarketDataPlus, DataType::DT_SNAPSHOT_PLUS);
    }

    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnSecuDepthMarketDataPlus]: ExchangeID %s, InstrumentID %s, ChannelNo %d, TradeDate %d, UpdateTime %d, LastPrice %lf, "
               "OpenPrice %lf, HighPrice %lf, LowPrice %lf, TradeVolume %" PRId64 ", TradeBalance %lf, TotalBidVolume %" PRId64 ", TotalAskVolume %" PRId64 ", "
               "TradesNum %" PRId64 ", TickType %c, SeqNo %" PRId64 ",\n"
               "\tBid1Price %lf, Bid1Volume %" PRId64 "\n"
               "\tAsk1Price %lf, Ask1Volume %" PRId64 "\n",
               pSecuDepthMarketDataPlus->ExchangeID,
               pSecuDepthMarketDataPlus->InstrumentID,
               pSecuDepthMarketDataPlus->ChannelNo,
               pSecuDepthMarketDataPlus->TradeDate,
               pSecuDepthMarketDataPlus->UpdateTime,
               pSecuDepthMarketDataPlus->LastPrice,
               pSecuDepthMarketDataPlus->OpenPrice,
               pSecuDepthMarketDataPlus->HighPrice,
               pSecuDepthMarketDataPlus->LowPrice,
               pSecuDepthMarketDataPlus->TradeVolume,
               pSecuDepthMarketDataPlus->TradeBalance,
               pSecuDepthMarketDataPlus->TotalBidVolume,
               pSecuDepthMarketDataPlus->TotalAskVolume,
               pSecuDepthMarketDataPlus->TradesNum,
               pSecuDepthMarketDataPlus->TickType,
               pSecuDepthMarketDataPlus->SeqNo,

               pSecuDepthMarketDataPlus->BidPrice[0], pSecuDepthMarketDataPlus->BidVolume[0],
               pSecuDepthMarketDataPlus->AskPrice[0], pSecuDepthMarketDataPlus->AskVolume[0]);
    }
}

/// Description: 主推-现货快照Plus行情停止通知
void CHSNsqSpiImpl::OnRtnSecuDepthMarketDataPlusStopNotice(CHSNsqSecuDepthMarketDataPlusStopNoticeField *pSecuDepthMarketDataPlusStopNotice)
{
    printf("OnRtnSecuDepthMarketDataPlusStopNotice: ChannelNo[%d]\n", pSecuDepthMarketDataPlusStopNotice->ChannelNo);
}

/// Description: 主推-静态代码表数据初始化成功通知
void CHSNsqSpiImpl::OnRtnInstrumentsDataChangeNotice(CHSNsqInstrumentsDataChangeNoticeField *pInstrumentsDataChangeNotice)
{
    printf("OnRtnInstrumentsDataChangeNotice: ExchangeID[%s], MarketBizType[%c], TradeDate[%d]\n", pInstrumentsDataChangeNotice->ExchangeID,
           pInstrumentsDataChangeNotice->MarketBizType, pInstrumentsDataChangeNotice->TradeDate);

    // 静态代码表有变化时候，该接口会调用，可在此处重新调用ReqQryXXXInstruments接口获取更新的码表
    // NOTE1: ExchangeID 可能会超过配置的市场范围，请过滤想要的市场类型进行处理
    // NOTE2: 日期非当日、类型不存在或市场不存在等异常情况请自行处理
}

/// Description: 期货订阅-行情应答
void CHSNsqSpiImpl::OnRspFutuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspFutuDepthMarketDataSubscribe: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]----------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

/// Description: 期货订阅取消-行情应答
void CHSNsqSpiImpl::OnRspFutuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspFutuDepthMarketDataCancel: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]----------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

/// Description: 主推-期货行情
void CHSNsqSpiImpl::OnRtnFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData)
{
    if (OutPutData::ALL == m_output_data || OutPutData::CSV == m_output_data)
    {
        if (strlen(pFutuDepthMarketData->InstrumentID) < 7)
            m_cache_manage->PutCache(pFutuDepthMarketData->ExchangeID, (char *)pFutuDepthMarketData, DataType::DT_FUT_SNAPSHOT);
        else
            m_cache_manage->PutCache(pFutuDepthMarketData->ExchangeID, (char *)pFutuDepthMarketData, DataType::DT_FUT_OPT_SNAPSHOT);
    }
    if (OutPutData::ALL == m_output_data || OutPutData::SCREEN == m_output_data)
    {
        printf("[OnRtnFutuDepthMarketData]: ExchangeID %s, InstrumentID %s, TradeDate %d, UpdateTime %d, LastPrice %lf, "
               "OpenPrice %lf, HighPrice %lf, LowPrice %lf, TradeVolume %" PRId64 ", TradeBalance %lf, PreSettlementPrice %lf,"
               "PreClosePrice %lf, OpenInterest %" PRId64 ", ClosePrice %lf, SettlementPrice %lf, UpLimitPx %lf, DownLimitPx %lf, "
               "AveragePrice %lf, PreOpenInterest %" PRId64 ", \n "
               "\tBid1Price %lf, Bid1Volume %" PRId64 "\n"
               "\tAsk1Price %lf, Ask1Volume %" PRId64 "\n",
               pFutuDepthMarketData->ExchangeID,
               pFutuDepthMarketData->InstrumentID,
               pFutuDepthMarketData->TradingDay,
               pFutuDepthMarketData->UpdateTime,
               pFutuDepthMarketData->LastPrice,
               pFutuDepthMarketData->OpenPrice,
               pFutuDepthMarketData->HighestPrice,
               pFutuDepthMarketData->LowestPrice,
               pFutuDepthMarketData->TradeVolume,
               pFutuDepthMarketData->TradeBalance,
               pFutuDepthMarketData->PreSettlementPrice,
               pFutuDepthMarketData->PreClosePrice,
               pFutuDepthMarketData->OpenInterest,
               pFutuDepthMarketData->ClosePrice,
               pFutuDepthMarketData->SettlementPrice,
               pFutuDepthMarketData->UpperLimitPrice,
               pFutuDepthMarketData->LowerLimitPrice,
               pFutuDepthMarketData->AveragePrice,
               pFutuDepthMarketData->PreOpenInterest,
               pFutuDepthMarketData->BidPrice[0], pFutuDepthMarketData->BidVolume[0],
               pFutuDepthMarketData->AskPrice[0], pFutuDepthMarketData->AskVolume[0]);
    }
}

void CHSNsqSpiImpl::writeInstrumentData(const char *key, const char *file_name, char *sz_buf, int buf_len, bool bIsLast)
{
    FILE *fp = NULL;
    if (!m_static_file.count(key))
    {
        m_static_file[key] = fopen(file_name, "w+");
        if (NULL == m_static_file[key])
        {
            printf("fopen %s fail: %s\n", file_name, strerror(errno));
        }
    }
    fp = m_static_file[key];
    if (sz_buf && fp)
        (void)fwrite(sz_buf, buf_len, 1, fp);

    if (bIsLast)
    {
        (void)fflush(fp);
    }
}

/// Description: 获取期货当前交易日合约应答
void CHSNsqSpiImpl::OnRspQryFutuInstruments(CHSNsqFutuInstrumentStaticInfoField *pFutuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        printf("OnRspQryFutuInstruments: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]---------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        return;
    }
    char sz_buf[1024];
    int buf_len;

    if (0 == m_iAllInstrCount)
    {
        printf("OnRspQryFutuInstruments Successed !\n");
    }

    buf_len = snprintf(sz_buf, 1024, "[OnRspQryFutuInstruments]: ExchangeID %s, InstrumentID %s, InstrumentName %s, SecurityType %c, PreClosePrice %lf, UpperLimitPrice %lf, LowerLimitPrice %lf, PriceTick %lf, BuyVolumeUnit %d, SellVolumeUnit %d, TradeDate %d, nRequestID %d, bIsLast %d\n",
                       pFutuInstrumentStaticInfo->ExchangeID,
                       pFutuInstrumentStaticInfo->InstrumentID,
                       pFutuInstrumentStaticInfo->InstrumentName,
                       pFutuInstrumentStaticInfo->SecurityType,
                       pFutuInstrumentStaticInfo->PreClosePrice,
                       pFutuInstrumentStaticInfo->UpperLimitPrice,
                       pFutuInstrumentStaticInfo->LowerLimitPrice,
                       pFutuInstrumentStaticInfo->PriceTick,
                       pFutuInstrumentStaticInfo->BuyVolumeUnit,
                       pFutuInstrumentStaticInfo->SellVolumeUnit,
                       pFutuInstrumentStaticInfo->TradeDate,
                       nRequestID,
                       bIsLast);

#ifdef ENABLE_IPC_TIMESTAMP_DEBUG
    printf("%s", sz_buf);
#endif

    if (strcmp(pFutuInstrumentStaticInfo->ExchangeID, HS_EI_CZCE) == 0)
    {
        writeInstrumentData(pFutuInstrumentStaticInfo->ExchangeID, "czce_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pFutuInstrumentStaticInfo->ExchangeID, HS_EI_DCE) == 0)
    {
        writeInstrumentData(pFutuInstrumentStaticInfo->ExchangeID, "dce_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pFutuInstrumentStaticInfo->ExchangeID, HS_EI_SHFE) == 0)
    {
        writeInstrumentData(pFutuInstrumentStaticInfo->ExchangeID, "shfe_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pFutuInstrumentStaticInfo->ExchangeID, HS_EI_CFFEX) == 0)
    {
        writeInstrumentData(pFutuInstrumentStaticInfo->ExchangeID, "cffex_code.txt", sz_buf, buf_len, bIsLast);
    }
    else if (strcmp(pFutuInstrumentStaticInfo->ExchangeID, HS_EI_INE) == 0)
    {
        writeInstrumentData(pFutuInstrumentStaticInfo->ExchangeID, "ine_code.txt", sz_buf, buf_len, bIsLast);
    }

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

/// Description: 订阅-行情请求应答
void CHSNsqSpiImpl::OnRspProIDSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspProIDSubscribe: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]----------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

/// Description: 订阅取消-行情请求应答
void CHSNsqSpiImpl::OnRspProIDSubscribeCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspProIDSubscribeCancel: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]----------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::WriteData()
{
    m_cache_manage->WriteAllCacheData();
}

CacheManage::CacheManage()
{
    memset(m_all_cache, 0, sizeof(m_all_cache));
    memset(m_market, 0, sizeof(m_market));
}

CacheManage::~CacheManage()
{
    RealseAllCache();
    delete m_data_writer;
}

int CacheManage::Init(int size)
{
    if (0 == size)
        size = 10000;

    m_data_writer = new DataWriter();

    for (int i = 0; i < MAREKT_MAX_INDEX; ++i)
    {
        switch (i)
        {
        case HS_EI_SSE[0] + '\0':
        case HS_EI_SZSE[0] + '\0':
            m_all_cache[i][(int)DataType::DT_SNAPSHOT] = new Cache<SockSnapshot>(size, DataType::DT_SNAPSHOT, m_data_writer);
            m_all_cache[i][(int)DataType::DT_STOCK_TRADE] = new Cache<StockTick>(size * 10, DataType::DT_STOCK_TRADE, m_data_writer);
            m_all_cache[i][(int)DataType::DT_OPT_SNAPSHOT] = new Cache<CHSNsqOptDepthMarketDataField>(size, DataType::DT_OPT_SNAPSHOT, m_data_writer);
            m_all_cache[i][(int)DataType::DT_ATP_SNAPSHOT] = new Cache<AtpSockSnapshot>(size, DataType::DT_ATP_SNAPSHOT, m_data_writer);
            m_all_cache[i][(int)DataType::DT_SNAPSHOT_PLUS] = new Cache<CHSNsqSecuDepthMarketDataPlusField>(size, DataType::DT_SNAPSHOT_PLUS, m_data_writer);
            m_all_cache[i][(int)DataType::DT_HKT_SNAPSHOT] = new Cache<CHSNsqHktDepthMarketDataField>(size, DataType::DT_HKT_SNAPSHOT, m_data_writer);
            m_all_cache[i][(int)DataType::DT_BOND_TRADE] = new Cache<BondTick>(size * 5, DataType::DT_BOND_TRADE, m_data_writer);
            m_market[i] = true;
            break;
        case HS_EI_SWI[0] + '\0':
        case HS_EI_TZASE[0] + '\0':
        case HS_EI_BJSE[0] + '\0':
            m_all_cache[i][(int)DataType::DT_SNAPSHOT] = new Cache<SockSnapshot>(size, DataType::DT_SNAPSHOT, m_data_writer);
            m_market[i] = true;
            break;
        case HS_EI_CZCE[0] + HS_EI_CZCE[1]:
        case HS_EI_DCE[0] + HS_EI_DCE[1]:
        case HS_EI_SHFE[0] + HS_EI_SHFE[1]:
        case HS_EI_CFFEX[0] + HS_EI_CFFEX[1]:
        case HS_EI_INE[0] + HS_EI_INE[1]:
            m_all_cache[i][(int)DataType::DT_FUT_SNAPSHOT] = new Cache<CHSNsqFutuDepthMarketDataField>(size, DataType::DT_FUT_SNAPSHOT, m_data_writer);
            m_all_cache[i][(int)DataType::DT_FUT_OPT_SNAPSHOT] = new Cache<CHSNsqFutuDepthMarketDataField>(size, DataType::DT_FUT_OPT_SNAPSHOT, m_data_writer);
            m_market[i] = true;
            break;
        default:
            break;
        }
    }
    return 0;
}

void CacheManage::PutCache(const char *ExchangeID, char *pData, DataType type)
{
    if (m_all_cache[ExchangeID[0] + ExchangeID[1]][(int)type] != NULL)
        m_all_cache[ExchangeID[0] + ExchangeID[1]][(int)type]->push(pData);
}

void CacheManage::WriteAllCacheData()
{
    for (int i = 0; i < MAREKT_MAX_INDEX; ++i)
    {
        if (!(m_market[i]))
            continue;
        for (int j = 0; j < (int)DataType::DT_MAX; ++j)
        {
            if (NULL == m_all_cache[i][j])
                continue;
            m_all_cache[i][j]->WriteData();
        }
    }
}

void CacheManage::RealseAllCache()
{
    WriteAllCacheData();
    for (int i = 0; i < MAREKT_MAX_INDEX; ++i)
    {
        if (!(m_market[i]))
            continue;
        for (int j = 0; j < (int)DataType::DT_MAX; ++j)
        {
            if (NULL == m_all_cache[i][j])
                continue;
            delete m_all_cache[i][j];
            m_all_cache[i][j] = NULL;
            // if(j == (int)DataType::DT_STOCK_TRADE || j == (int)DataType::DT_BOND_TRADE)
            //     m_all_cache[i][j + 1] = NULL;
        }
    }
}
