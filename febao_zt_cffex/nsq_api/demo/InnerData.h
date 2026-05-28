#ifndef __LANDFILE_INNER_DATA_H_
#define __LANDFILE_INNER_DATA_H_

#include "HSNsqStruct.h"
#include "HSDataType.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_set>
#include <unordered_map>

// 采用HSExchangeID：交易所代码 两个字母的ASC值做索引，所以索引最大值为 128 + 128
constexpr int MAREKT_MAX_INDEX = 128 + 128;

constexpr int CHANNEL_NO_MAX_INDEX = 10000;

// 数据类型
enum class DataType: uint8_t
{
    DT_SNAPSHOT,
    DT_STOCK_TRADE,
    DT_STOCK_ORDER = DT_STOCK_TRADE + 1,
    DT_OPT_SNAPSHOT,
    DT_ATP_SNAPSHOT,
    DT_SNAPSHOT_PLUS,
    DT_HKT_SNAPSHOT,
    DT_BOND_TRADE,
    DT_BOND_ORDER = DT_BOND_TRADE + 1,
    DT_FUT_SNAPSHOT,
    DT_FUT_OPT_SNAPSHOT,
    DT_MAX
};


struct SockSnapshot
{
	CHSNsqSecuDepthMarketDataField snapshot;
	int bid1_count;
	int ask1_count;
	int max_bid1_count;
	int max_ask1_count;
	HSIntVolume bid_volume[50];
	HSIntVolume ask_volume[50];
    SockSnapshot()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct AtpSockSnapshot
{
	CHSNsqSecuATPMarketDataField atp_snapshot;
	int bid1_count;
	int ask1_count;
	int max_bid1_count;
	int max_ask1_count;
	HSIntVolume bid_volume[50];
	HSIntVolume ask_volume[50];
    AtpSockSnapshot()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct StockTick
{
    DataType data_type;
    union
    {
        CHSNsqSecuTransactionTradeDataField     trade;
        CHSNsqSecuTransactionEntrustDataField   order;
    }; 
    StockTick()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct BondTick
{
    DataType data_type;
    union
    {
        CHSNsqBondTransactionTradeDataField     trade;
        CHSNsqBondTransactionEntrustDataField   order;
    }; 
    BondTick()
    {
        memset(this, 0, sizeof(*this));
    }
};


// 行情数据输出
enum class OutPutData
{
    SCREEN = 0,             // 输出到屏幕
    CSV,                    // 输出到csv文件
    ALL                     // 输出到屏幕和CSV文件
};
// 按市场保存vector
using MarketCodeCache = std::unordered_map < std::string, std::vector<std::string>>;

struct UserInfo
{
    // 保存登录、重建信息
    CHSNsqReqUserLoginField reqLoginField;
    CHSNsqReqSecuTransactionRebuildField reqTransRebuild;
    // 行情数据输出位置
    OutPutData output_data{OutPutData::SCREEN};
    // user.properties指定代码保存到相应的vector中
    MarketCodeCache market_codes;
    // 保存sdk_config.ini中配置的市场
    std::unordered_set<std::string> sdk_config_market;
    UserInfo()
    {
        memset(&reqLoginField, 0, sizeof(reqLoginField));
        memset(&reqTransRebuild, 0, sizeof(reqTransRebuild));
    }
};

const std::unordered_map<std::string, std::string> MARET_TYPE_CONVERT =
    {
        {"sh", HS_EI_SSE},
        {"sz", HS_EI_SZSE},
        {"swi", HS_EI_SWI},
        {"neeq", HS_EI_TZASE},
        {"bj", HS_EI_BJSE},
        {"shfe", HS_EI_SHFE},
        {"dce", HS_EI_DCE},
        {"czce", HS_EI_CZCE},
        {"cffex", HS_EI_CFFEX},
        {"ine", HS_EI_INE},
    };

#endif