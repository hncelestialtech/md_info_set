#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include "inst_map.h"

#include "udp_receiver.h"


#include "macro.h"

#include "ringbuffer.h"

#include "exanic_receiver.h"


#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()


#define PRICE_NROMAL       0
#define PRICE_UP_LIMIT     1
#define PRICE_DOWN_LIMIT   2



#define  MDENTRYTYPE_0       0x2030
#define  MDENTRYTYPE_1       0x2031
#define  MDENTRYTYPE_2       0x2032 
#define  MDENTRYTYPE_4       0x2034 
#define  MDENTRYTYPE_7       0x2037 
#define  MDENTRYTYPE_8       0x2038 
#define  MDENTRYTYPE_xe      0x6578 
#define  MDENTRYTYPE_xf      0x6678 
#define  MDENTRYTYPE_xg      0x6778 
#define  MDENTRYTYPE_x1      0x3178 
#define  MDENTRYTYPE_x2      0x3278
#define  MDENTRYTYPE_xi      0x6978 

using fb_market_data_t = cffex::fb::api::market_data_entity;

#pragma pack(push, 1)

struct MarketDataField
{
    int packetLen;//报文长度
    unsigned char versionNo;//版本序号
    int       updateTime;//修改时间
    char   exchangeID[3];//交易所
    char   instrumentID[30];//合约代码
    bool   stopFlag;//停牌标识
    char   statusLatestPrice;//
    double latestPrice;//最新价
    char   statusMatchAmount;//
    int    matchAmount;//成交量
    char   statusPositionAmount;//
    int    positionAmount;//持仓量
    char   statusHighestPrice;//
    double highestPrice;//最高价    
    char   statusLowestPrice;//
    double lowestPrice;//最低价
    char   statusBuyPrice1;//
    double buyPrice1;//申买价1
    char   statusSellPrice1;//
    double sellPrice1;//申卖价1
    char   statusBuyAmount1;//
    int    buyAmount1;//申买量1
    char   statusSellAmount1;//
    int    sellAmount1;//申卖量1
    char   statusMatchMoney;//
    double macthMoney;//成交金额
    char   statusOpenPrice;//
    double openPrice;//开盘价
    char   statusAvgPrice;//
    double avgPrice;//当日均价
};










struct Lv2_OB_Quote
{
    int32_t PacketId;
    int32_t PacketLen;
    char    Date[9];
    char    Symbol[0x47];
    double  LastPrice;
    double  HighPrice;
    double  LowPrice;
    int32_t _unk1;
    int32_t Volume;
    double  Turnover;
    int32_t _lastOpenInterest;
    int32_t OpenInterest;
    int32_t _unk2;
    double  _clearPrice;
    int32_t  _unk3;
    int32_t  _unk4;
    int32_t  _unk5;
    int32_t  _unk6;
    double  PriceCeil;
    double  PriceFloor;
    double  _lastClear;
    double  _lastClose;

	// offset 0xc4 ?
    double  _bestBid;
    int32_t _bestBidQ;
    int32_t _bestBidImplQ;
    double  _bestAsk;
    int32_t _bestAskQ;
    int32_t _bestAskImplQ;
    double  _avgPrice;
    char    Time[16];
    double  _openPrice;
    double  _closePrice;
    int32_t _tableCount;

    struct Line
    {
        double  Price;
        int32_t Volume;
        int32_t _VolumeImpl;
        int8_t  _align[16];
    };

    Line Bid[5];
    Line Ask[5];

    uint8_t _align[84];



};

struct L2Quote_212
{
    char type;
    char Symbol[0x47];
    char res[12];
    double     bp1;
    uint32_t   bv1;
    uint32_t v2;
    uint32_t v3;
    uint64_t v4;
    uint64_t v5;
    char res2[12];
    double     ap1;
    uint32_t   av1;

};

struct L2Quote_128
{
    uint32_t type;
    char Symbol[0x47];




};



struct L2Quote_224
{
    uint32_t type;
    char     Symbol[0x47];
    char     res[13];
    double   price;


};

struct L2Quote_112
{
    uint32_t type;
    char Symbol[0x47];
    char res[13];
    double     price;
    int32_t Volume;



};



#pragma pack(pop)




constexpr uint8_t decodeTbl[64] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    1, 2, 3, 0, 5, 6, 7, 4,
    3, 2, 1, 0, 7, 6, 5, 4,
    2, 3, 0, 1, 6, 7, 4, 5,
    2, 3, 0, 1, 4, 5, 6, 7,
    3, 2, 1, 0, 5, 6, 7, 4,
    1, 2, 3, 0, 7, 6, 5, 4,
    6, 7, 4, 5, 0, 1, 2, 3
};





class Decoder {
private:
    // 内部数据结构
    int8_t strategy_;      // 解密策略 (-1表示未初始化)
    uint8_t tried_mask_;   // 尝试过的策略掩码
    
    // 检查解密后的数据是否有效
    bool CheckDecrypt(const Lv2_OB_Quote* quote, uint8_t strategy) const {
        // 创建副本进行解密测试
        Lv2_OB_Quote test_quote = *quote;
        if (!FullDecrypt(&test_quote, strategy)) {
            return false;
        }
        
        // 检查数据有效性
        if (test_quote._tableCount <= 0) {
            return false;
        }
        
        // 检查Bid/Ask价格顺序
        if (test_quote._bestBid != 0x7FFFFFFFFFFFFFFFLL) {
            if (test_quote.LastPrice + 0.000001 < test_quote.HighPrice || 
                test_quote.LastPrice > test_quote.LowPrice + 0.000001) {
                return false;
            }
        }
        
        // 检查特定条件
        if (test_quote.Bid[0].Price == 49) { // '1'
            int valid_bids = 0;
            for (int i = 0; i < test_quote._tableCount; i++) {
                if (test_quote.Bid[i].Price == 49) {
                    valid_bids++;
                } else {
                    break;
                }
            }
            
            if (test_quote._bestBid == 0x7FFFFFFFFFFFFFFFLL) {
                if (test_quote._bestBid != test_quote._lastClear ||
                    test_quote._bestBidQ != test_quote._lastOpenInterest ||
                    test_quote._bestBidImplQ != test_quote.OpenInterest) {
                    return false;
                }
            }
            
            if (test_quote._tableCount != valid_bids) {
                return false;
            }
        } else {
            if (test_quote._bestBid == 0x7FFFFFFFFFFFFFFFLL) {
                // 检查特定字段匹配
                if (test_quote._bestBid != test_quote._lastClear ||
                    test_quote._bestBidQ != test_quote._lastOpenInterest ||
                    test_quote._bestBidImplQ != test_quote.OpenInterest) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // 完整的解密函数
    bool FullDecrypt(Lv2_OB_Quote* quote, uint8_t strategy) const {
        // 基于查找表计算解密类型
        uint32_t table_index = (quote->PacketId & 7) + 8 * strategy;
        uint8_t decrypt_type = decodeTbl[table_index];
        
        bool valid = true;
        
        // 根据解密类型进行不同的解密操作
        switch (decrypt_type) {
            case 0: {
                // 交换字段
                std::swap(quote->OpenInterest, quote->_lastOpenInterest);
                std::swap(quote->_bestBidQ, quote->_bestAskQ);
                std::swap(quote->_bestBidImplQ, quote->_bestAskImplQ);
                std::swap(quote->_bestBid, quote->_bestAsk);
                break;
            }
            case 1: {
                // 交换字段
                std::swap(quote->_bestBidQ, quote->_bestBidImplQ);
                std::swap(quote->_bestBid, quote->_bestAsk);
                break;
            }
            case 2: {
                // 条件解密
                if (quote->_bestBidQ > 0xE6) {
                    valid = quote->_bestBidImplQ > 0xB11;
                }
                quote->_bestBidImplQ -= 2834;
                quote->_bestBidQ -= 231;
                std::swap(quote->_bestBid, quote->_bestAsk);
                break;
            }
            case 3: {
                // 条件解密
                if (quote->_bestBidImplQ <= 0xC7) {
                    valid = false;
                }
                if (quote->_bestBidQ % 5 != 0) {
                    valid = false;
                }
                if (quote->_bestBidQ <= 0x63) {
                    valid = false;
                }
                quote->_bestBidQ = quote->_bestBidQ / 5 - 20;
                quote->_bestBidImplQ = (quote->_bestBidImplQ >> 1) - 100;
                std::swap(quote->_bestBid, quote->_avgPrice);
                break;
            }
            case 4: {
                // 交换字段和双精度值
                std::swap(quote->PacketLen, quote->_bestBidImplQ);
                std::swap(quote->_bestBidQ, quote->_bestBidImplQ);
                std::swap(quote->Volume, quote->_bestBidQ);
                // 交换双精度值
                double temp[2];
                memcpy(temp, &quote->_lastClear, sizeof(double) * 2);
                memcpy(&quote->_lastClear, &quote->_lastClose, sizeof(double) * 2);
                memcpy(&quote->_lastClose, temp, sizeof(double) * 2);
                break;
            }
            case 5: {
                // 交换字段
                std::swap(quote->_lastOpenInterest, quote->_bestBidImplQ);
                std::swap(quote->_bestBidQ, quote->_bestBidImplQ);
                std::swap(quote->PacketLen, quote->_bestBidQ);
                std::swap(quote->_bestBid, quote->_closePrice);
                break;
            }
            case 6: {
                // 条件解密
                if (quote->_bestBidQ > 0x140) {
                    valid = quote->_bestBidImplQ > 0x91D;
                }
                quote->_bestBidImplQ -= 2334;
                quote->_bestBidQ -= 321;
                std::swap(quote->_bestBid, quote->_bestAsk);
                std::swap(quote->_bestAsk, quote->_avgPrice);
                std::swap(quote->_avgPrice, quote->_closePrice);
                break;
            }
            case 7: {
                // 复杂条件解密
                if (quote->_bestBidImplQ > 0x11) {
                    valid = quote->_bestBidImplQ % 3 == 0;
                }
                if (quote->_bestBidQ & 3) {
                    valid = false;
                }
                if (quote->_bestBidQ <= 0x53) {
                    valid = false;
                }
                quote->_bestBidImplQ = quote->_bestBidImplQ / 3 - 6;
                quote->_bestBidQ = (quote->_bestBidQ >> 2) - 21;
                std::swap(quote->_bestBid, quote->_avgPrice);
                
                // 处理Bid/Ask数组
                if (quote->_tableCount > 0) {
                    for (int i = 0; i < quote->_tableCount; i++) {
                        switch (decrypt_type) {
                            case 0:
                            case 1:
                            case 4:
                            case 5:
                                std::swap(quote->Bid[i].Volume, quote->Bid[i]._VolumeImpl);
                                break;
                            case 2:
                                if (quote->Bid[i].Volume <= 0x178) valid = false;
                                quote->Bid[i].Volume -= 377;
                                break;
                            case 3:
                                if (quote->Bid[i].Volume % 3 != 0) valid = false;
                                quote->Bid[i].Volume /= 3;
                                break;
                            case 6:
                                if (quote->Bid[i].Volume <= 0xC4) valid = false;
                                quote->Bid[i].Volume -= 197;
                                break;
                            case 7:
                                if (quote->Bid[i].Volume & 1) valid = false;
                                quote->Bid[i].Volume >>= 1;
                                break;
                        }
                    }
                    
                    // 重新排列Bid数组
                    int valid_bids = 0;
                    if (quote->_tableCount > 9) {
                        return valid;
                    }
                    
                    char* bid_ptr = reinterpret_cast<char*>(quote->Bid);
                    if (quote->Bid[0].Price != 49) { // '1'
                        if (quote->_tableCount > 0) {
                            memmove(&quote->Bid[4], 
                                   &quote->Bid[quote->_tableCount - 4], 
                                   32 * (quote->_tableCount - 4));
                        }
                    } else {
                        // 计算有效Bid数量
                        int count = 0;
                        for (int i = 0; i < quote->_tableCount; i++) {
                            if (quote->Bid[i].Price == 49) {
                                count++;
                            } else {
                                break;
                            }
                        }
                        
                        if (count > 0) {
                            memmove(&quote->Bid[0], 
                                   &quote->Bid[count], 
                                   32 * (quote->_tableCount - count));
                        }
                    }
                }
                break;
            }
        }
        
        return valid;
    }

public:
    Decoder() : strategy_(-1), tried_mask_(0xFF) {}
    
    static Decoder* Create() {
        Decoder* decoder = new Decoder();
        return decoder;
    }
    
    static void Destroy(Decoder* decoder) {
        delete decoder;
    }
    
    enum InitResult { Ok, NeedMore, Error };

    InitResult Init(const Lv2_OB_Quote* ptr) {
        if (strategy_ != -1) {
            return Ok;
        }
        
        // 尝试不同的解密策略
        uint8_t mask = tried_mask_;
        
        // 检查8种可能的策略
        for (int i = 0; i < 8; i++) {
            if (mask & (1 << i)) {
                if (CheckDecrypt(ptr, i)) {
                    // 清除该策略位
                    mask &= ~(1 << i);
                    if (mask == 0) {
                        // 找到了唯一有效的策略
                        strategy_ = i;
                        
                        // 输出调试信息
                        std::cerr << "Decryption strategy set to " << (int)strategy_ << std::endl;
                        
                        if (strategy_ < 0) {
                            tried_mask_ = 0xFF;
                        }
                        return Ok;
                    }
                }
            }
        }
        
        // 没有找到有效策略
        std::cerr << "Failed to find decryption strategy" << std::endl;
        return Error;
    }

    bool Decode(Lv2_OB_Quote* ptr) const {
        if (strategy_ == -1) {
            return false;
        }
        
        return FullDecrypt(ptr, strategy_);
    }
};





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



struct IpPort
{
    uint32_t    ip;   
    uint16_t    port;
    IpPort(uint32_t a, uint16_t b):ip(a),port(b){}
};

struct MulticastConfig{
    std::string     ifname;

    std::string     group_ip;
    uint16_t        group_port;
    uint32_t        net_group_ip; 
    uint16_t        net_group_port; 
};

struct TCPConfig{
    std::string     ifname;

    std::string     ip;   
    uint16_t        port;
    uint32_t        net_ip;   
    uint16_t        net_port;
};


struct Config_t{
    MulticastConfig      multiconfig;
    TCPConfig            tcpconfig;
    std::vector<int>     cpu_id;
    std::string          filter_path;
    std::string          zx_ini;
};

//lc2601-p-234000



enum COM_RESULT {
    COM_RESULT_OK = 0,
    COM_RESULT_TIME_ERROR   = 1,     // 时间倒流
    COM_RESULT_VOL_ERROR    = 2,     // vol倒流
    COM_RESULT_REPEAT_L1    = 3,     // 无用的Lv1
    COM_RESULT_REPEAT_L2    = 4,     // 无用的Lv2
};

#define RING_BUFFER_SIZE 4096

#define IOPV_TAG_LV1     100000
#define IOPV_TAG_LV2     200000

struct QuotaInfo{
    uint64_t time;
    uint64_t volume;
    double  open;
    double  high;
    double  low;    
    double  upper;
    double  lower;
    int32_t last_openinterest;
    int32_t openinterest;    
    double  last_settle_price;
    double  settle_price;

    double   ap1;
    double   bp1;
    uint32_t av1;
    uint32_t bv1;

    uint32_t level;
};

class GFEXExanicQuota : public cffex::fb::api::fb_md_plugin_api{
public:
    GFEXExanicQuota();
    ~GFEXExanicQuota();

private:
    inline void LoadJsonCfg();

    inline bool level2_belongTo(uint32_t ip, uint16_t port);
    inline bool level1_belongTo(uint32_t ip, uint16_t port);

    void ProcessMsg(int32_t cpu_id);
    void DispatchMessage(const Slot& slot);

    void OnData_lv2(const char* data, size_t size);

    inline int32_t IsValidQuota(QuotaInfo *cache, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1);

    void OnData_lv1(const char* data, size_t size);

    static void UdpHandler(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port);
    static void ExanicHandler(void *ctx, const char* data, size_t size);


public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;

    std::shared_ptr<MulticastReceiver>          m_udp_receiver;

    std::shared_ptr<ExanicReceiver>             m_exanic_receiver;

    InstMap<QuotaInfo>                              m_quota_cache; 

    LockFreeRingBuffer        *m_ringbuffer;
    std::thread               *m_dispatcher;          


    std::set<std::string> m_inst_set;


    uint64_t m_l1_time_err_sum = 0;
    uint64_t m_l1_vol_err_sum = 0;
    uint64_t m_l1_useless_lv1_sum = 0;

    uint64_t m_l2_time_err_sum = 0;
    uint64_t m_l2_vol_err_sum = 0;


    uint64_t m_l2_apbp_err_sum = 0;

    //febao
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;




};
