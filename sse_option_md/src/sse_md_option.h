#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "fb_md_plugin_api.h"
#include "common.h"



#include "inst_map.h"

#include "config_json.h"


#include "macro.h"

#include "udp_receiver.h"



#pragma pack(push, 1)

#define SNAPSHOT_LEVEL                         10


struct priceQty {
    uint64_t           price;                            //实际值除以100000
    uint64_t           qty;
};

//基金,IOPV
struct FundSseL1 {
    uint8_t            messageType;                      //消息类型，基金为0x4
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;
    uint64_t           tradeNum;
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              
    uint64_t           lastPrice;                        //实际值除以100000
    uint64_t           openPrice;                        //实际值除以100000
    uint64_t           closePrice;                       //实际值除以100000
    uint64_t           settlePrice;                      //实际值除以100000
    uint64_t           highPrice;                        //实际值除以100000
    uint64_t           lowPrice;                         //实际值除以100000
    uint64_t           preSettlePrice;                   //实际值除以100000
    uint64_t           IOPV;                             //实际值除以100000
    uint64_t           preCloseIOPV;                     //实际值除以100000
    struct priceQty    bidPriceQty[5];
    struct priceQty    askPriceQty[5];
};


//上交静态信息结构体
typedef struct StaticInfo
{
    uint8_t            messageType;                      //消息类型，静态消息类型为0x0F
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所：1，深交所：2
    char               securityID[9];                    //证券代码
    /*
    涨跌停限制类型
    ‘N’表示交易规则（2013修订版）3.4.13规定的有涨跌幅限制类型或者权证管理办法第22条规定
    ‘R’表示交易规则（2013修订版）3.4.15和3.4.16规定的无涨跌幅限制类型
    ‘S’表示回购涨跌幅控制类型
    ‘F’表示基于参考价格的涨跌幅控制
    ‘P’表示IPO上市首日的涨跌幅控制类型
    ‘U’表示无任何价格涨跌幅控制类型
    */
    char               priceLimitType;
    double             upperLimitPrice;                  //涨停价
    double             lowerLimitPrice;                  //跌停价
    uint64_t           buyUnit;                          //买数量单位
    uint64_t           sellUnit;                         //卖数量单位
    uint64_t           upperQuantityLimitPriceDeclare;   //限价申报数量上限
    uint64_t           lowerQuantityLimitPriceDeclare;   //限价申报数量下限
    double             priceGear;                        //价格档位,申报价格的最小变动单位
    uint64_t           upperQuantityMarketPriceDeclare;  //市价申报数量上限
    uint64_t           lowerQuantityMarketPriceDeclare;  //市价申报数量下限
    /** 
    * 证券类别
    * ‘ES’表示股票；‘EU’表示基金；‘D’表示债券； ‘RWS’表示权证；‘FF’表示期货；
    * 'CB'表示公募REITs。（参考ISO10962），集合资产管理计划、债券预发行、定向可转债取‘D’
    */
    char               securityType[7];
    char               securityName[9];                  //证券名称                      
    char               fileDate[9];                      //文件日期(YYYYMMDD)
    char               resv[3];                          //保留字段                   
    char               securitySubType[4];               //详细证券类别,参考《上海证券市场竞价撮合平台市场参与者接口规格说明书》
    char               financeFlag;                      //融资标的标志,‘T’表示是融资标的证券,‘F’表示不是融资标的证券
    char               shortSaleFlag;                    //融券标的标志,‘T’表示是融券标的证券,‘F’表示不是融券标的证券
    char               productStatus[21];                //产品状态,参考《上海证券市场竞价撮合平台市场参与者接口规格说明书》
    char               listDate[9];                      //上市日期(YYYYMMDD)
    
} StaticInfoSse;





//期权
struct OptionSseL1 {
    uint8_t            messageType;                      //消息类型，期权为0x5
    uint32_t           sequence;                         //udp输出包序号，从1开始
    uint8_t            exchangeID;                       //交易所id，上交所L2：1，深交所：2，上交L1：3
    char               securityID[9];                    //证券代码
    uint16_t           flag;
    char               recv[3];
    uint8_t            securityType;
    uint8_t            tradSesMode;                      //交易盘交易模式：1 = 系统测试，2 = 模拟交易，3 = 产品（正常交易）
    uint64_t           sendingTime;                      //发送时间，格式： YYYYMMDDHHmmSSsss	
    uint32_t           tradeDate;                        //交易日期 YYYYMMDD 
    uint32_t           lastUpdateTime;                   //最新更新时间 HHMMSSsss 
    uint64_t           preClosePrice;                    //实际值除以100000
    uint64_t           totalVolumeTraded;                
    uint64_t           tradeNum;                         
    uint64_t           totalValueTraded;                 //实际值除以100
    char               tradingPhaseCode[8];              //实际值除以100000
    uint64_t           lastPrice;                        //实际值除以100000
    uint64_t           openPrice;                        //实际值除以100000  
    uint64_t           closePrice;                       //实际值除以100000
    uint64_t           settlePrice;                      //实际值除以100000
    uint64_t           highPrice;                        //实际值除以100000
    uint64_t           lowPrice;                         //实际值除以100000
    uint64_t           preSettlePrice;                   //实际值除以100000
    uint64_t           auctionPrice;                     //实际值除以100000
    uint64_t           auctionQty;                       //实际值除以100000
    uint64_t           totalLongPosition;
    struct priceQty    bidPriceQty[5];
    struct priceQty    askPriceQty[5];
};
   
#pragma pack(pop)


using fb_market_data_t = cffex::fb::api::market_data_entity;

#define IOPV_TAG_LV1     100000
#define IOPV_TAG_LV2     200000

inline uint64_t time_str_to_utc_s(const char *time_str){
    // 直接访问字符数组
    const char* h  = time_str;
    const char* m  = time_str + 3;
    const char* s  = time_str + 6;
    const char* ms = time_str + 9;

    int hours        = (h[0] - '0') * 10 + (h[1] - '0');
    int minutes      = (m[0] - '0') * 10 + (m[1] - '0');
    int seconds      = (s[0] - '0') * 10 + (s[1] - '0');
    int milliseconds = (ms[0] - '0') * 100 +(ms[1] - '0') * 10 +(ms[2] - '0');

    const int64_t HOURS_TO_MS = 3600000LL;
    const int64_t MIN_TO_MS   = 60000LL;
    const int64_t SEC_TO_MS   = 1000LL;
    return (hours * HOURS_TO_MS) +(minutes * MIN_TO_MS) + (seconds * SEC_TO_MS) +milliseconds;
}

struct QuotaInfo{
    double   upper;
    double   lower;
};

class SSEQuota : public cffex::fb::api::fb_md_plugin_api{
public:
    SSEQuota();
    ~SSEQuota();

private:


    void onStaticInfoSse(StaticInfoSse* p, int64_t t0);

    void onOptionSseL1(OptionSseL1* msgdata, int64_t t0);

    void onFundSseL1(FundSseL1* msgdata, int64_t t0);

    static void UdpHandler(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port, int64_t t0);

public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    ConfigManager             m_config_mgr;

    InstMap<QuotaInfo>                          m_quota_cache; 


    //febao
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;

    std::shared_ptr<MulticastReceiver>          m_udp_receiver;



};













