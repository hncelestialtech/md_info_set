#include "DataWriter.h"
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <cinttypes>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <process.h>
#define PRId64 "lld"
#define localtime_r(a, b) localtime_s(b, a)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif // _WIN32

DataWriter::DataWriter()
{
    memset(m_fun_write, 0, sizeof(m_fun_write));

    m_fun_write[(int)DataType::DT_SNAPSHOT] = &DataWriter::WriteSnapshot;
    m_fun_write[(int)DataType::DT_STOCK_TRADE] = &DataWriter::WriteTrade;
    m_fun_write[(int)DataType::DT_STOCK_ORDER] = &DataWriter::WriteOrder;
    m_fun_write[(int)DataType::DT_OPT_SNAPSHOT] = &DataWriter::WriteOptSnapshot;
    m_fun_write[(int)DataType::DT_ATP_SNAPSHOT] = &DataWriter::WriteAtpSnapshot;
    m_fun_write[(int)DataType::DT_SNAPSHOT_PLUS] = &DataWriter::WriteSnapshotPlus;
    m_fun_write[(int)DataType::DT_HKT_SNAPSHOT] = &DataWriter::WriteHktSnapshot;
    m_fun_write[(int)DataType::DT_BOND_TRADE] = &DataWriter::WriteBondTrade;
    m_fun_write[(int)DataType::DT_BOND_ORDER] = &DataWriter::WriteBondOrder;
    m_fun_write[(int)DataType::DT_FUT_SNAPSHOT] = &DataWriter::WriteFutSnapshot;
    m_fun_write[(int)DataType::DT_FUT_OPT_SNAPSHOT] = &DataWriter::WriteFutOptSnapshot;

    memset(m_file_fp, 0, sizeof(m_file_fp));
    memset(m_tick_fp, 0, sizeof(m_tick_fp));

    struct tm time_tm;
    time_t now = time(NULL);
    (void)localtime_r(&now, &time_tm);
    sprintf(m_csv_name, "./%s_%02d%02d_%02d%02d%02d", "csv",
            time_tm.tm_mon + 1, time_tm.tm_mday, time_tm.tm_hour,
            time_tm.tm_min, time_tm.tm_sec);
}

DataWriter::~DataWriter()
{
    for (int i = 0; i < MAREKT_MAX_INDEX; ++i)
    {
        for (int j = 0; j < (int)DataType::DT_MAX; ++j)
        {
            if (NULL == m_file_fp[i][j])
                continue;
            fclose(m_file_fp[i][j]);
            m_file_fp[i][j] = NULL;
        }
        for (int j = 0; j < CHANNEL_NO_MAX_INDEX; ++j)
        {
            if (NULL == m_tick_fp[i][j])
                continue;
            fclose(m_tick_fp[i][j]);
            m_tick_fp[i][j] = NULL;
        }
    }
}

void DataWriter::WriteFile(DataType data_type, char *pData)
{
    if (DataType::DT_STOCK_TRADE == data_type)
    {
        /** 使用reinterpret_cast进行类型转换*/
        StockTick *data = reinterpret_cast<StockTick *>(pData);
        data_type = data->data_type;
    }
    if (DataType::DT_BOND_TRADE == data_type)
    {
        /** 使用reinterpret_cast进行类型转换*/
        BondTick *data = reinterpret_cast<BondTick *>(pData);
        data_type = data->data_type;
    }
    fn_write_file fn_write_file = m_fun_write[(int)data_type];
    (this->*fn_write_file)(pData);
}

FILE *DataWriter::GetMarketFP(DataType data_type, const char *ExchangeID, int channel_no)
{
    if (!m_is_create_csv)
    {
        CreateCsvFolder();
        m_is_create_csv = true;
    }
    FILE *fp = NULL;

    int index = ExchangeID[0] + ExchangeID[1];

    if (DataType::DT_BOND_TRADE == data_type || DataType::DT_STOCK_TRADE == data_type)
    {
        if (NULL == m_tick_fp[index][channel_no])
        {
            switch (data_type)
            {
            case DataType::DT_STOCK_TRADE:
                m_tick_fp[index][channel_no] = OpenTickFile(ExchangeID, channel_no);
                break;
            case DataType::DT_BOND_TRADE:
                m_tick_fp[index][channel_no] = OpenBondTickFile(ExchangeID, channel_no);
                break;
            default:
                break;
            }
        }
        fp = m_tick_fp[index][channel_no];
    }
    else
    {
        if (NULL == m_file_fp[index][(int)data_type])
        {
            switch (data_type)
            {
            case DataType::DT_SNAPSHOT:
                m_file_fp[index][(int)data_type] = OpenSnapFile(ExchangeID);
                break;
            case DataType::DT_OPT_SNAPSHOT:
                m_file_fp[index][(int)data_type] = OpenOptSnapFile(ExchangeID);
                break;
            case DataType::DT_ATP_SNAPSHOT:
                m_file_fp[index][(int)data_type] = OpenAtpSnapFile(ExchangeID);
                break;
            case DataType::DT_SNAPSHOT_PLUS:
                m_file_fp[index][(int)data_type] = OpenSnapPlusFile(ExchangeID);
                break;
            case DataType::DT_HKT_SNAPSHOT:
                m_file_fp[index][(int)data_type] = OpenHktSnapFile(ExchangeID);
                break;
            case DataType::DT_FUT_SNAPSHOT:
                m_file_fp[index][(int)data_type] = OpenFutSnapFile(ExchangeID, false);
                break;
            case DataType::DT_FUT_OPT_SNAPSHOT:
                m_file_fp[index][(int)data_type] = OpenFutSnapFile(ExchangeID, true);
                break;
            default:
                break;
            }
        }
        fp = m_file_fp[index][(int)data_type];
    }

    return fp;
}

void DataWriter::WriteSnapshot(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    SockSnapshot *snapshot = reinterpret_cast<SockSnapshot *>(pData);
    CHSNsqSecuDepthMarketDataField *p = &snapshot->snapshot;

    HSIntVolume *bid1_qty = &snapshot->bid_volume[0];
    HSIntVolume *ask1_qty = &snapshot->ask_volume[0];
    HSNum max_bid1_count = snapshot->max_bid1_count;
    HSNum max_ask1_count = snapshot->max_ask1_count;
    HSNum bid1_count = snapshot->bid1_count;
    HSNum ask1_count = snapshot->ask1_count;

    FILE *fp = GetMarketFP(DataType::DT_SNAPSHOT, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,", p->InstrumentID);
    fprintf(fp, "%s,", p->ExchangeID);
    fprintf(fp, "%d,", p->ChannelNo);

    fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%" PRId64 ","
                "%.2lf,%lf,%" PRId64 ",%c,%" PRId64 ",%" PRId64 ",%lf,%lf,%lf,%lf,"
                "%lf,%lf,%d,%d,%" PRId64 ",%lf,%" PRId64 ",%lf,%" PRId64 ",%lf,"
                "%lf,%d,%d,%" PRId64 ",%" PRId64 ",%lf,%lf,%d,%d,%d,"
                "%d,%d,%d,%lf,%lf,%" PRId64 ",%lf,%d",
            p->LastPrice, p->PreClosePrice, p->OpenPrice, p->HighPrice, p->LowPrice, p->ClosePrice, p->UpperLimitPrice, p->LowerLimitPrice, p->TradeDate, p->UpdateTime, p->TradeVolume,
            p->TradeBalance, p->AveragePrice, p->TradesNum, p->InstrumentTradeStatus == '\0' ? ' ' : p->InstrumentTradeStatus, p->TotalBidVolume, p->TotalAskVolume, p->MaBidPrice, p->MaAskPrice, p->MaBondBidPrice, p->MaBondAskPrice,
            p->YieldToMaturity, p->IOPV, p->EtfBuycount, p->EtfSellCount, p->EtfBuyVolume, p->EtfBuyBalance, p->EtfSellVolume, p->EtfSellBalance, p->TotalWarrantExecVolume, p->WarrantLowerPrice,
            p->WarrantUpperPrice, p->CancelBuyNum, p->CancelSellNum, p->CancelBuyVolume, p->CancelSellVolume, p->CancelBuyValue, p->CancelSellValue, p->TotalBuyNum, p->TotalSellNum, p->DurationAfterBuy,
            p->DurationAfterSell, p->BidOrdersNum, p->AskOrdersNum, p->PreIOPV,
            p->BondLastAuctionPrice, p->BondAuctionVolume, p->BondAuctionBalance, (int)(p->BondLastTradeType));

    // 新债新增
    fprintf(fp, ",");
    for (int i = 0; i < (int)sizeof(p->BondTradeStatus); i++)
    {
        fprintf(fp, "%c", p->BondTradeStatus[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 ",%d,%lf,%" PRId64 ",%d", p->BidPrice[i], p->BidVolume[i], p->BidNumOrders[i], p->AskPrice[i], p->AskVolume[i], p->AskNumOrders[i]);
    }

    fprintf(fp, ",%d,%d,%d,%d", bid1_count, max_bid1_count, ask1_count, max_ask1_count);

    for (int i = 0; i < 50; i++)
    {
        fprintf(fp, ", %" PRId64 ",%" PRId64 "", bid1_qty[i], ask1_qty[i]);
    }

    fprintf(fp, "\n");
}

void DataWriter::WriteTrade(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    StockTick *data = reinterpret_cast<StockTick *>(pData);
    CHSNsqSecuTransactionTradeDataField *trade = &data->trade;
    FILE *fp = GetMarketFP(DataType::DT_STOCK_TRADE, trade->ExchangeID, trade->ChannelNo);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,", trade->ExchangeID);
    fprintf(fp, "%d,%" PRId64 ",%s,%d,%d,%d,%lf,%" PRId64 ",%lf"
                ",%c,%c"
                ",%c,%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%d,%d,%c\n",
            trade->ChannelNo, trade->SeqNo, trade->InstrumentID, 2, trade->TradeDate, trade->TransactTime, trade->TrdPrice, trade->TrdVolume, trade->TrdMoney,
            '-', '-',
            trade->TrdBSFlag, trade->TrdBuyNo, trade->TrdSellNo, (int64_t)0, trade->BizIndex, trade->TransFlag, 0, '-');
}

void DataWriter::WriteOrder(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    StockTick *data = reinterpret_cast<StockTick *>(pData);

    CHSNsqSecuTransactionEntrustDataField *tbt_data = &data->order;
    // 逐笔落到一个文件中
    FILE *fp = GetMarketFP(DataType::DT_STOCK_TRADE, tbt_data->ExchangeID, tbt_data->ChannelNo);

    if (NULL == fp)
    {
        return;
    }
    fprintf(fp, "%s,", tbt_data->ExchangeID);
    fprintf(fp, "%d,%" PRId64 ",%s,%d,%d,%d,%lf,%" PRId64 ",%lf"
                ",%c,%c"
                ",%c,%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%d,%" PRId64 ",%c\n",
            tbt_data->ChannelNo, tbt_data->SeqNo, tbt_data->InstrumentID, 1, tbt_data->TradeDate, tbt_data->TransactTime, tbt_data->OrdPrice, tbt_data->OrdVolume, 0.0, //(int64_t)tbt_data->TradeDate*1000000000+tbt_data->TransactTime
            tbt_data->OrdSide == '\0' ? ' ' : tbt_data->OrdSide, tbt_data->OrdType,
            '-', (int64_t)0, (int64_t)0, tbt_data->OrdNo, tbt_data->BizIndex, tbt_data->TransFlag, tbt_data->TrdVolume, tbt_data->TickStatus == '\0' ? ' ' : tbt_data->TickStatus);
}

void DataWriter::WriteOptSnapshot(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    CHSNsqOptDepthMarketDataField *p = reinterpret_cast<CHSNsqOptDepthMarketDataField *>(pData);
    FILE *fp = GetMarketFP(DataType::DT_OPT_SNAPSHOT, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,%s,%d,%lf,%lf,%lf,%lf,%lf,%lf,%" PRId64 ",%" PRId64 ",%lf,"
                "%lf,%lf,%lf,%lf,%lf,%d,%d,%" PRId64 ",%.2lf,"
                "%lf,%" PRId64 ",%c,%c,%lf,%" PRId64 ",%d,%" PRId64 "",
            p->InstrumentID, p->ExchangeID, p->ChannelNo, p->LastPrice, p->PreClosePrice, p->OpenPrice, p->HighPrice, p->LowPrice, p->ClosePrice, p->PreOpenInterest, p->OpenInterest, p->PreSettlementPrice,
            p->SettlementPrice, p->UpperLimitPrice, p->LowerLimitPrice, p->PreDelta, p->CurDelta, p->TradeDate, p->UpdateTime, p->TradeVolume, p->TradeBalance,
            p->AveragePrice, p->TradesNum, p->InstrumentTradeStatus, p->OpenRestriction[0], p->AuctionPrice, p->AuctionVolume, p->LastEnquiryTime, p->LeaveQty);
    int i;
    for (i = 0; i < 10; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 "", p->BidPrice[i], p->BidVolume[i]);
    }
    for (i = 0; i < 10; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 "", p->AskPrice[i], p->AskVolume[i]);
    }
    fprintf(fp, "\n");
}

void DataWriter::WriteAtpSnapshot(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    AtpSockSnapshot *atp_snapshot = reinterpret_cast<AtpSockSnapshot *>(pData);
    CHSNsqSecuATPMarketDataField *p = &atp_snapshot->atp_snapshot;
    FILE *fp = GetMarketFP(DataType::DT_ATP_SNAPSHOT, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }
    fprintf(fp, "%s,%s,%d,%lf,%lf,%d,%d,%c,",
            p->InstrumentID, p->ExchangeID, p->ChannelNo, p->PreClosePrice, p->ClosePrice, p->TradeDate, p->UpdateTime, p->InstrumentTradeStatus);

    fprintf(fp, "%" PRId64 ",%.2lf,%" PRId64 ",%" PRId64 ",%" PRId64 ",",
            p->TradeVolume, p->TradeBalance, p->TradesNum, p->TotalBidVolume, p->TotalAskVolume);

    fprintf(fp, "%d,%d,%" PRId64 ",%" PRId64 ",",
            p->CancelBuyNum, p->CancelSellNum, p->CancelBuyVolume, p->CancelSellVolume);

    fprintf(fp, "%lf,%lf,%" PRId64 ",%" PRId64 ",",
            p->BidPrice1, p->AskPrice1, p->BidVolume1, p->AskVolume1);
    fprintf(fp, "%d,%d,%d,%d",
            atp_snapshot->bid1_count, atp_snapshot->ask1_count, atp_snapshot->max_bid1_count, atp_snapshot->max_ask1_count);

    for (int i = 0; i < 50; i++)
    {
        fprintf(fp, ",%" PRId64 ",%" PRId64 "",
                atp_snapshot->bid_volume[i], atp_snapshot->ask_volume[i]);
    }
    fprintf(fp, "\n");
}

void DataWriter::WriteSnapshotPlus(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    CHSNsqSecuDepthMarketDataPlusField *p = reinterpret_cast<CHSNsqSecuDepthMarketDataPlusField *>(pData);
    FILE *fp = GetMarketFP(DataType::DT_SNAPSHOT_PLUS, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }
    fprintf(fp, "%s,%s,%d,", p->InstrumentID, p->ExchangeID, p->ChannelNo);
    fprintf(fp, "%lf,", p->LastPrice);
    fprintf(fp, "%lf,", p->OpenPrice);
    fprintf(fp, "%lf,", p->HighPrice);
    fprintf(fp, "%lf,", p->LowPrice);
    fprintf(fp, "%" PRId64 ",", p->TradeVolume);
    fprintf(fp, "%lf,", p->TradeBalance);
    fprintf(fp, "%" PRId64 ",", p->TradesNum);
    fprintf(fp, "%d,", p->TradeDate);
    fprintf(fp, "%d,", p->UpdateTime);
    fprintf(fp, "%" PRId64 ",", p->TotalBidVolume);
    fprintf(fp, "%" PRId64 "", p->TotalAskVolume);

    for (int i = 0; i < 10; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 "", p->AskPrice[i], p->AskVolume[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 "", p->BidPrice[i], p->BidVolume[i]);
    }
    fprintf(fp, ",%c", p->TickType);
    fprintf(fp, ",%" PRId64 "", p->SeqNo);
    fprintf(fp, ",%lf", p->TickPrice);
    fprintf(fp, ",%" PRId64 "", p->TickVolume);

    fprintf(fp, "\n");
}

void DataWriter::WriteHktSnapshot(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    CHSNsqHktDepthMarketDataField *p = reinterpret_cast<CHSNsqHktDepthMarketDataField *>(pData);
    FILE *fp = GetMarketFP(DataType::DT_HKT_SNAPSHOT, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,%s,%d,%lf,%lf,%lf,%lf,%d,%d,%" PRId64 ","
                "%lf,%c",
            p->InstrumentID, p->ExchangeID, p->ChannelNo, p->LastPrice, p->PreClosePrice, p->HighPrice, p->LowPrice, /*p->NomianlPrice,*/
            p->TradeDate, p->UpdateTime, p->TradeVolume,
            p->TradeBalance, p->InstrumentTradeStatus);

    int i;
    for (i = 0; i < 1; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 "", p->BidPrice[i], p->BidVolume[i]);
    }

    for (i = 0; i < 1; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 "", p->AskPrice[i], p->AskVolume[i]);
    }

    fprintf(fp, ",%c,%c,%c,%c,%lf", p->BoardLotOrderBidLimit, p->BoardLotOrderAskLimit, p->OddLotOrderBidLimit, p->OddLotOrderAskLimit, p->NomianlPrice);

    fprintf(fp, "\n");
}

void DataWriter::WriteBondTrade(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    BondTick *data = reinterpret_cast<BondTick *>(pData);

    CHSNsqBondTransactionTradeDataField *trade = &data->trade;
    FILE *fp = GetMarketFP(DataType::DT_BOND_TRADE, trade->ExchangeID, trade->ChannelNo);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,", trade->ExchangeID);
    fprintf(fp, "%d,%" PRId64 ",%s,%d,%d,%d,%lf,%" PRId64 ",%lf,"
                "%c,%c,%c,%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%d,%d,%d,"
                "%d,%c,%c,%c,%c,%c",
            trade->ChannelNo, trade->SeqNo, trade->InstrumentID, 2, trade->TradeDate, trade->TransactTime, trade->TrdPrice, trade->TrdVolume, 0.0,
            '-', '-', trade->TrdBSFlag, trade->TrdBuyNo, trade->TrdSellNo, (int64_t)0, (int64_t)0, trade->TransFlag, trade->TradeType, trade->SettlPeriod,
            trade->SettlType, '-', '-', '-', '-', '-');

    fprintf(fp, ",%d,%s,%lf,-,-,-,-,-,-", trade->BidExecInstType, trade->SecondaryOrderID, trade->MarginPrice);

    fprintf(fp, "\n");
}

void DataWriter::WriteBondOrder(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    BondTick *data = reinterpret_cast<BondTick *>(pData);

    CHSNsqBondTransactionEntrustDataField *tbt_data = &data->order;
    // 逐笔落到一个文件中
    FILE *fp = GetMarketFP(DataType::DT_BOND_TRADE, tbt_data->ExchangeID, tbt_data->ChannelNo);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,", tbt_data->ExchangeID);
    fprintf(fp, "%d,%" PRId64 ",%s,%d,%d,%d,%lf,%" PRId64 ",%lf,"
                "%c,%c,%c,0,0,0,0,%d,%d,%d,"
                "%d,%s,%s,%s,%s,%s",
            tbt_data->ChannelNo, tbt_data->SeqNo, tbt_data->InstrumentID, 1, tbt_data->TradeDate, tbt_data->TransactTime, tbt_data->OrdPrice, tbt_data->OrdVolume, 0.0,
            tbt_data->OrdSide, tbt_data->OrdType == 0 ? '-' : tbt_data->OrdType, '-', tbt_data->TransFlag, tbt_data->TradeType, tbt_data->SettlPeriod,
            tbt_data->SettlType,
            tbt_data->QuoteID[0] == ' ' ? " " : tbt_data->QuoteID,
            tbt_data->MemberID[0] == ' ' ? " " : tbt_data->MemberID,
            tbt_data->InvestorType[0] == ' ' ? " " : tbt_data->InvestorType,
            tbt_data->InvestorID[0] == ' ' ? " " : tbt_data->InvestorID,
            tbt_data->TraderCode[0] == ' ' ? " " : tbt_data->TraderCode);

    fprintf(fp, ",%d,%s,-,%d,%s,%lf,%lf,%" PRId64 ",%d", tbt_data->BidExecInstType, tbt_data->SecondaryOrderID, tbt_data->BidTransType,
            tbt_data->Memo[0] == ' ' ? " " : tbt_data->Memo, tbt_data->HighLimitPrice, tbt_data->LowLimitPrice, tbt_data->MinQty, tbt_data->BidTradeDate);

    fprintf(fp, "\n");
}

void DataWriter::WriteFutSnapshot(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    CHSNsqFutuDepthMarketDataField *p = reinterpret_cast<CHSNsqFutuDepthMarketDataField *>(pData);

    FILE *fp = GetMarketFP(DataType::DT_FUT_SNAPSHOT, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,%s,", p->InstrumentID, p->ExchangeID);
    fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,", p->LastPrice, p->PreSettlementPrice, p->PreClosePrice, p->OpenPrice, p->HighestPrice, p->LowestPrice);
    fprintf(fp, "%" PRId64 ",%lf,%" PRId64 ",%lf,%lf,%lf,", p->TradeVolume, p->TradeBalance, p->OpenInterest, p->ClosePrice, p->SettlementPrice, p->UpperLimitPrice);
    fprintf(fp, "%lf,%d,%d,%d", p->LowerLimitPrice, p->TradingDay, p->ActionDay, p->UpdateTime);

    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 ",%d", p->BidPrice[i], p->BidVolume[i], p->BidNumOrders[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 ",%d", p->AskPrice[i], p->AskVolume[i], p->AskNumOrders[i]);
    }
    fprintf(fp, ",%lf,%" PRId64 ",%lf,%lf", p->AveragePrice, p->PreOpenInterest, p->PreDelta, p->CurDelta);
    fprintf(fp, ",%lf,%lf,%lf,%lf,%d,%d", p->MaBidPrice, p->MaAskPrice, p->BidBalance, p->AskBalance, p->TotalBidVolume, p->TotalAskVolume);
    fprintf(fp, "\n");
}

void DataWriter::WriteFutOptSnapshot(char *pData)
{
    /** 使用reinterpret_cast进行类型转换*/
    CHSNsqFutuDepthMarketDataField *p = reinterpret_cast<CHSNsqFutuDepthMarketDataField *>(pData);

    FILE *fp = GetMarketFP(DataType::DT_FUT_OPT_SNAPSHOT, p->ExchangeID);

    if (NULL == fp)
    {
        return;
    }

    fprintf(fp, "%s,%s,", p->InstrumentID, p->ExchangeID);
    fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,", p->LastPrice, p->PreSettlementPrice, p->PreClosePrice, p->OpenPrice, p->HighestPrice, p->LowestPrice);
    fprintf(fp, "%" PRId64 ",%lf,%" PRId64 ",%lf,%lf,%lf,", p->TradeVolume, p->TradeBalance, p->OpenInterest, p->ClosePrice, p->SettlementPrice, p->UpperLimitPrice);
    fprintf(fp, "%lf,%d,%d,%d", p->LowerLimitPrice, p->TradingDay, p->ActionDay, p->UpdateTime);

    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 ",%d", p->BidPrice[i], p->BidVolume[i], p->BidNumOrders[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, ",%lf,%" PRId64 ",%d", p->AskPrice[i], p->AskVolume[i], p->AskNumOrders[i]);
    }
    fprintf(fp, ",%lf,%" PRId64 ",%lf,%lf", p->AveragePrice, p->PreOpenInterest, p->PreDelta, p->CurDelta);
    fprintf(fp, ",%lf,%lf,%lf,%lf,%d,%d", p->MaBidPrice, p->MaAskPrice, p->BidBalance, p->AskBalance, p->TotalBidVolume, p->TotalAskVolume);
    fprintf(fp, "\n");
}

inline void FPErrorProcess(const FILE *fp)
{
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
}

FILE *DataWriter::OpenOptSnapFile(const char *ExchangeID)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_opt_snapshot.csv", m_csv_name, ExchangeID);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
    // 期权快照
    fprintf(fp, "InstrumentID,ExchangeID,ChannelNo,");
    fprintf(fp, "LastPrice,PreClosePrice,OpenPrice,HighPrice,LowPrice,ClosePrice,PreOpenInterest,OpenInterest,PreSettlementPrice,SettlementPrice,");
    fprintf(fp, "UpperLimitPrice,LowerLimitPrice,PreDelta,CurDelta,TradeDate,UpdateTime,TradeVolume,TradeBalance,AveragePrice,TradesNum,InstrumentTradeStatus,");
    fprintf(fp, "OpenRestriction,AuctionPrice,AuctionVolume,LastEnquiryTime,LeaveQty");

    for (int i = 0; i < 10; i++)
    {
        fprintf(fp, ",BidPrice%d,BidVolume%d", i, i);
    }

    for (int i = 0; i < 10; i++)
    {
        fprintf(fp, ",AskPrice%d,AskVolume%d", i, i);
    }

    fprintf(fp, "\n");
    return fp;
}

FILE *DataWriter::OpenSnapFile(const char *ExchangeID)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_snapshot.csv", m_csv_name, ExchangeID);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }

    fprintf(fp, "InstrumentID,ExchangeID,ChannelNo");
    fprintf(fp, ",LastPrice,PreClosePrice,OpenPrice,HighPrice,LowPrice,ClosePrice,UpperLimitPrice,LowerLimitPrice,TradeDate,UpdateTime,TradeVolume,"
                "TradeBalance,AveragePrice,TradesNum,InstrumentTradeStatus,TotalBidVolume,TotalAskVolume,MaBidPrice,MaAskPrice,MaBondBidPrice,MaBondAskPrice,"
                "YieldToMaturity,IOPV,EtfBuycount,EtfSellCount,EtfBuyVolume,EtfBuyBalance,EtfSellVolume,EtfSellBalance,TotalWarrantExecVolume,WarrantLowerPrice,"
                "WarrantUpperPrice,CancelBuyNum,CancelSellNum,CancelBuyVolume,CancelSellVolume,CancelBuyValue,CancelSellValue,TotalBuyNum,TotalSellNum,DurationAfterBuy,"
                "DurationAfterSell,BidOrdersNum,AskOrdersNum,PreIOPV,BondLastAuctionPrice,BondAuctionVolume,BondAuctionBalance,BondLastTradeType,BondTradeStatus");

    for (int i = 0; i < 10; i++)
    {
        fprintf(fp, ",BidPrice[%d],BidVolume[%d],BidNumOrders[%d],AskPrice[%d],AskVolume[%d],AskNumOrders[%d]", i, i, i, i, i, i);
    }

    fprintf(fp, ",Bid1Count,MaxBid1Count,Ask1Count,MaxAsk1Count");

    for (int i = 0; i < 50; i++)
    {
        fprintf(fp, ",Bid1Volume[%d],Ask1Volume[%d]", i, i);
    }

    fprintf(fp, "\n");
    return fp;
}

FILE *DataWriter::OpenAtpSnapFile(const char *ExchangeID)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_atp_snapshot.csv", m_csv_name, ExchangeID);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
    fprintf(fp, "InstrumentID,ExchangeID,ChannelNo,PreClosePrice,ClosePrice,TradeDate,UpdateTime,InstrumentTradeStatus,TradeVolume");
    fprintf(fp, ",TradeBalance,TradesNum,TotalBidVolume,TotalAskVolume,CancelBuyNum,CancelSellNum,CancelBuyVolume,CancelSellVolume");
    fprintf(fp, ",BidPrice1,AskPrice1,BidVolume1,AskVolume1");
    fprintf(fp, ",Bid1Count,MaxBid1Count,Ask1Count,MaxAsk1Count");
    for (int i = 0; i < 50; i++)
    {
        fprintf(fp, ",Bid1Volume[%d],Ask1Volume[%d]", i, i);
    }
    fprintf(fp, "\n");
    return fp;
}

FILE *DataWriter::OpenTickFile(const char *ExchangeID, int channel_no)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_channel_%d.csv", m_csv_name, ExchangeID, channel_no);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
    fprintf(fp, "ExchangeID,ChannelNo,SeqNo,InstrumentID,Trade2_Order1,TradeDate,TransactTime,Price,Volume,TrdMoney,"
                "OrdSide,OrdType,TrdBSFlag,TrdBuyNo,TrdSellNo,OrdNo,BizIndex,TransFlag,OrderTrdVolume,TickStatus\n");
    return fp;
}

FILE *DataWriter::OpenSnapPlusFile(const char *ExchangeID)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_snapshot_plus.csv", m_csv_name, ExchangeID);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
    fprintf(fp, "InstrumentID,ExchangeID,ChannelNo,LastPrice,OpenPrice,HighPrice,LowPrice,TradeVolume,TradeBalance,TradesNum,TradeDate,UpdateTime,TotalBidVolume,TotalAskVolume");

    for (int i = 1; i <= 10; i++)
    {
        fprintf(fp, ",AskPrice%d,AskVolume%d", i, i);
    }

    for (int i = 1; i <= 10; i++)
    {
        fprintf(fp, ",BidPrice%d,BidVolume%d", i, i);
    }
    fprintf(fp, ",TickType,SeqNo,TickPrice,TickVolume");

    fprintf(fp, "\n");

    return fp;
}

FILE *DataWriter::OpenBondTickFile(const char *ExchangeID, int channel_no)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_channel_%d.csv", m_csv_name, ExchangeID, channel_no);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
    fprintf(fp, "ExchangeID,ChannelNo,SeqNo,InstrumentID,Trade2_Order1,TradeDate,TransactTime,Price,Volume,TrdMoney,"
                "OrdSide,OrdType,TrdBSFlag,TrdBuyNo,TrdSellNo,OrdNo,BizIndex,TransFlag,TradeType,SettlPeriod,"
                "SettlType,QuoteID,MemberID,InvestorType,InvestorID,TraderCode,BidExecInstType,SecondaryOrderID,MarginPrice,BidTransType,"
                "Memo,HighLimitPrice,LowLimitPrice,MinQty,BidTradeDate\n");

    return fp;
}

FILE *DataWriter::OpenHktSnapFile(const char *ExchangeID)
{
    char file_name[128];
    snprintf(file_name, sizeof(file_name), "%s/%s_hkt_snapshot.csv", m_csv_name, ExchangeID);
    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }
    fprintf(fp, "InstrumentID,ExchangeID,ChannelNo");
    fprintf(fp, ",LastPrice,PreClosePrice,HighPrice,LowPrice,TradeDate,UpdateTime,TradeVolume,"
                "TradeBalance,InstrumentTradeStatus");

    for (int i = 0; i < 1; i++)
    {
        fprintf(fp, ",BidPrice[%d],BidVolume[%d],AskPrice[%d],AskVolume[%d]", i, i, i, i);
    }

    fprintf(fp, ",BoardLotOrderBidLimit,BoardLotOrderAskLimit,OddLotOrderBidLimit,OddLotOrderAskLimit,NomianlPrice");
    fprintf(fp, "\n");
    return fp;
}

FILE *DataWriter::OpenFutSnapFile(const char *ExchangeID, bool is_opt)
{
    char file_name[128];
    if (is_opt)
        snprintf(file_name, sizeof(file_name), "%s/%s_opt_snapshot.csv", m_csv_name, ExchangeID);
    else
        snprintf(file_name, sizeof(file_name), "%s/%s_fut_snapshot.csv", m_csv_name, ExchangeID);

    FILE *fp = fopen(file_name, "w+");
    // FPErrorProcess(fp);
    if (NULL == fp)
    {
        perror("[Error] file open failed");
        exit(-1);
    }

    fprintf(fp, "InstrumentID,ExchangeID,");
    fprintf(fp, "LastPrice,PreSettlementPrice,PreClosePrice,OpenPrice,HighestPrice,LowestPrice,");
    fprintf(fp, "TradeVolume,TradeBalance,OpenInterest,ClosePrice,SettlementPrice,UpperLimitPrice,");
    fprintf(fp, "LowerLimitPrice,TradingDay,ActionDay,UpdateTime");

    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, ",BidPrice%d,BidVolume%d,BidNumOrders%d", i, i, i);
    }

    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, ",AskPrice%d,AskVolume%d,AskNumOrders%d", i, i, i);
    }
    fprintf(fp, ",AveragePrice,PreOpenInterest,PreDelta,CurDelta");
    fprintf(fp, ",MaBidPrice,MaAskPrice,BidBalance,AskBalance,TotalBidVolume,TotalAskVolume");

    fprintf(fp, "\n");
    return fp;
}

/**
 * @brief 创建CSV文件夹
 *
 * 在指定路径下创建名为m_csv_name的文件夹，用于保存CSV文件。
 *
 * 在Windows环境下，使用CreateDirectory函数创建文件夹。如果创建成功，则打印创建路径；如果创建失败，则打印错误信息并退出程序。
 * 在非Windows环境下，使用mkdir函数创建文件夹，并设置文件夹权限为所有用户可读可写可执行。如果创建成功，则打印创建路径；如果创建失败，则打印错误信息并退出程序。
 */
void DataWriter::CreateCsvFolder()
{
#ifdef _WIN32
    bool isCreate = CreateDirectory(m_csv_name, NULL);
    if (isCreate)
    {
        printf("create path: %s\n", m_csv_name);
    }
    else
    {
        printf("[Error] create path  %s failed: %s\n", m_csv_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
#else
    // 创建所有用户可读可写可执行的文件夹，保存csv文件
    int isCreate = mkdir(m_csv_name, S_IRWXU | S_IRWXG | S_IRWXO);
    if (!isCreate)
    {
        printf("create path: %s\n", m_csv_name);
    }
    else
    {
        printf("[Error] create path  %s failed: %s\n", m_csv_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
}