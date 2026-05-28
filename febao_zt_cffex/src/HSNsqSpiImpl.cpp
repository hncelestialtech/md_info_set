
#include "HSNsqSpiImpl.h"

#include <cinttypes>


#include "common.h"



CHSNsqSpiImpl::CHSNsqSpiImpl(CHSNsqApi *lpHSNsqApi)
    : m_lpHSNsqApi(lpHSNsqApi)
{

}
CHSNsqSpiImpl::~CHSNsqSpiImpl()
{

}

void CHSNsqSpiImpl::OnFrontConnected()
{
    printf("OnFrontConnected errinfo=%s\n", m_lpHSNsqApi->GetApiErrorMsg(0));

    static int _s_nLoginReqId = 10000;
    //// 连接成功或断开重连后，需要调用登录接口

    int iRet = m_lpHSNsqApi->ReqUserLogin(&m_user_info->reqLoginField, _s_nLoginReqId++);
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

void CHSNsqSpiImpl::OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData)
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


    printf("%s", sz_buf);



    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

/// Description: 获取合约的最新快照信息应答
void CHSNsqSpiImpl::OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

    printf("OnRspQryOptDepthMarketData: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]----------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);



}

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
    int64_t local_time_ns = get_nanoseconds();
    // CHSNsqFutuDepthMarketDataField *msgdata = pFutuDepthMarketData;
    // std::clog<<local_time_ns<<","<<msgdata->InstrumentID<<","<<msgdata->UpdateTime<<","<<msgdata->LastPrice<<","<<msgdata->TradeVolume<<","<<std::fixed<<msgdata->TradeBalance<<std::endl;

    m_future_quota_cb(m_ctx,pFutuDepthMarketData);
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






/// Description: 获取期货当前交易日合约应答
void CHSNsqSpiImpl::OnRspQryFutuInstruments(CHSNsqFutuInstrumentStaticInfoField *pFutuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

    printf("OnRspQryFutuInstruments: nRequestID[%d], ErrorID[%d], ErrorMsg[%s]----------------------\n", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);

    m_future_staticinfo_cb(m_ctx, pFutuInstrumentStaticInfo, bIsLast);

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}




