#pragma once

#include <fstream>
#include <time.h>
#include <sys/timeb.h>
#include <stdint.h>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <map>
#include <cstdlib>
#include <iostream>
#include "InnerData.h"
#include "DataWriter.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <HSNsqApi.h>

#define NUM_OF_ROUND 1

class CacheManage;

class CHSNsqSpiImpl : public CHSNsqSpi
{

public:
    CHSNsqSpiImpl(CHSNsqApi *lpHSNsqApi, OutPutData output_data);
    ~CHSNsqSpiImpl();

    /// Description: 当客户端与后台开始建立通信连接，连接成功后此方法被回调。
    virtual void OnFrontConnected();

    /// Description:当客户端与后台通信连接异常时，该方法被调用。
    /// Others     :通过GetApiErrorMsg(nResult)获取详细错误信息。
    virtual void OnFrontDisconnected(int nResult);

    /// Description:客户登录
    virtual void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ////以下是现货接口

    /// Description: 订阅-现货快照行情应答
    virtual void OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 订阅取消-现货快照行情应答
    virtual void OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-现货快照行情
    /// Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
    virtual void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    /// Description: 主推-现货盘后定价快照行情
    /// Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
    virtual void OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    /// Description: 订阅-现货逐笔行情应答
    virtual void OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 订阅取消-现货逐笔行情应答
    virtual void OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-现货逐笔成交行情
    virtual void OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData);

    /// Description: 主推-现货逐笔委托行情
    virtual void OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData);

    /// Description: 主推-现货盘后固定逐笔成交行情
    virtual void OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData);

    /// Description: 获取当前交易日现货合约应答
    virtual void OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期权订阅-行情应答
    virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期权订阅取消-行情应答
    virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-期权行情
    virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData);

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 获取合约的最新快照信息应答
    virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ////以下是现货逐笔重建接口

    /// Description: 重建应答-现货逐笔数据
    virtual void OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 重建应答超时-现货逐笔数据（本回调线程与其他回调线程不同）
    virtual void OnRspSecuTransactionDataTimeout(int nRequestID);

    /// Description: 主推-深证债券逐笔成交行情
    virtual void OnRtnBondTransactionTradeData(CHSNsqBondTransactionTradeDataField *pBondTransactionTradeData);

    /// Description: 主推-深证债券逐笔委托行情
    virtual void OnRtnBondTransactionEntrustData(CHSNsqBondTransactionEntrustDataField *pBondTransactionEntrustData);

    ////以下是港股通接口

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryHktInstruments(CHSNsqHktInstrumentStaticInfoField *pHktInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-港股通行情
    virtual void OnRtnHktDepthMarketData(CHSNsqHktDepthMarketDataField *pHktDepthMarketData);

    ////以下是现货快照Plus接口

    /// Description: 订阅-现货快照Plus行情应答
    virtual void OnRspSecuDepthMarketDataPlusSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 订阅取消-现货快照Plus行情应答
    virtual void OnRspSecuDepthMarketDataPlusCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-现货快照Plus行情
    virtual void OnRtnSecuDepthMarketDataPlus(CHSNsqSecuDepthMarketDataPlusField *pSecuDepthMarketDataPlus);

    /// Description: 主推-现货快照Plus行情停止通知
    /// Others     :当“快照Plus”服务发现交易所下发的某个通道的逐笔数据存在丢包时，“快照Plus”服务会自行发起对丢失逐笔数据的重建。
    ///           在重建完成之前，“快照Plus”服务会周期性推送具体通道的停止通知，从而触发本回调。
    ///           当重建完成后，具体通道的停止通知会停止推送，同时“快照Plus”服务会继续推送相应通道重建出来的快照
    virtual void OnRtnSecuDepthMarketDataPlusStopNotice(CHSNsqSecuDepthMarketDataPlusStopNoticeField *pSecuDepthMarketDataPlusStopNotice);

    /// Description: 期货订阅-行情应答
    virtual void OnRspFutuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期货订阅取消-行情应答
    virtual void OnRspFutuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-期货行情
    virtual void OnRtnFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData);

    /// Description: 主推-静态代码表数据初始化通知
    virtual void OnRtnInstrumentsDataChangeNotice(CHSNsqInstrumentsDataChangeNoticeField *pInstrumentsDataChangeNotice);

    ////以下是扩展订阅接口应答，目前仅用于对接 HDF2.0 接收网关场景

    /// Description: 订阅-行情请求应答
    virtual void OnRspProIDSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 订阅取消-行情请求应答
    virtual void OnRspProIDSubscribeCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 获取合TCP连接状态
    bool GetConnectStatus() { return m_isConnected; }

    /// Description: 获取登陆状态
    bool GetLoginStatus() { return m_isLogined; }

    /// Description: 获取静态代码表状态
    bool GetInstrumentsStatus() { return m_isAllInstrReady; }

    /// Description: 获取静态代码表数量
    int GetInstrumentsCount() { return m_iAllInstrCount; }

    ///  期货码表
    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryFutuInstruments(CHSNsqFutuInstrumentStaticInfoField *pFutuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 将行情数据写文件
    void WriteData();

    void writeInstrumentData(const char *key, const char *file_name, char *sz_buf, int buf_len, bool bIsLast);

private:
    CHSNsqApi *m_lpHSNsqApi;
    OutPutData m_output_data;

    bool m_isConnected{false};
    bool m_isLogined{false};
    bool m_isAllInstrReady{false};
    int m_iAllInstrCount{0};

    std::map<std::string, FILE *> m_static_file;
    std::unique_ptr<CacheManage> m_cache_manage;
};

class CacheBase;
class DataWriter;

class CacheManage
{
public:
    CacheManage();
    ~CacheManage();
    int Init(int size);
    void PutCache(const char *ExchangeID, char *pData, DataType type);
    void WriteAllCacheData();
    void RealseAllCache();

private:
    CacheBase *m_all_cache[MAREKT_MAX_INDEX][(int)DataType::DT_MAX];
    DataWriter *m_data_writer{NULL};
    bool m_market[MAREKT_MAX_INDEX];
};

class CacheBase
{
public:
    virtual ~CacheBase() = default;
    virtual void push(char *pData) = 0;
    virtual bool WriteData() = 0;
    virtual size_t size() = 0;
};

template <typename T>
class Cache : public CacheBase
{
public:
    Cache(int size, DataType data_type, DataWriter *data_writer)
        : m_data_type(data_type), m_data_writer(data_writer)
    {
        m_customer.resize(size);
        m_producer.resize(size);
        m_customer.clear();
        m_producer.clear();
    }
    virtual ~Cache() = default;

    virtual void push(char *pData)
    {
        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            T *tData = reinterpret_cast<T *>(pData);

            m_producer.emplace_back(*tData);
            ++m_cur_size;
        }
        catch (const std::bad_alloc &e)
        {
            // 处理内存分配失败的情况
            std::cerr << "Cache::push(): memory allocation failed: " << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
        catch (...)
        {
            std::cerr << "Cache::push(): unknow error, exit!" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    virtual bool WriteData()
    {
        const auto cur_size = m_cur_size.load(std::memory_order_relaxed);
        if (0 == cur_size)
        {
            return false;
        }

        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_customer.swap(m_producer);
            m_cur_size = 0;
        }
        catch (const std::bad_alloc &e)
        {
            // 处理内存分配失败的情况
            std::cerr << "Cache::WriteData(): memory allocation failed: " << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
        catch (...)
        {
            std::cerr << "Cache::WriteData(): unknow error, exit!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        for (auto &val : m_customer)
        {
            m_data_writer->WriteFile(m_data_type, (char *)&val);
        }
        m_customer.clear();

        return true;
    }

    virtual size_t size()
    {
        return m_cur_size.load(std::memory_order_relaxed);
    }

private:
    std::vector<T> m_customer;
    std::vector<T> m_producer;
    std::atomic_size_t m_cur_size{0};
    std::mutex m_mutex;
    DataType m_data_type;
    DataWriter *m_data_writer;
};