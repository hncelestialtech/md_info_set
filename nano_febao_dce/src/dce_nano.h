#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

#include "ringbuffer.h"


#include "fb_md_plugin_api.h"

#include "NanoDceMdApi.h"

#include "inst_map.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()


#define VALID_MAX_VOL    1e20
#define VALID_MAX_PRICE  1e10



#define IOPV_TAG_LV1     100000
#define IOPV_TAG_LV2     200000


#define OB_LEVEL_MASK_LV1     1
#define OB_LEVEL_MASK_LV2     5


struct Config_t{
    std::string config_path;
    std::vector<int> cpu_id;
    std::string filter_path;
    uint32_t     ob_level_mask;
};


struct QuotaMerge_t{
    uint64_t time;
    uint64_t volume;
    uint32_t level;

    double   ap1;
    double   bp1;
    uint32_t av1;
    uint32_t bv1;

    QuotaMerge_t(uint64_t a, uint64_t b, uint32_t c, double ap1, double bp1, uint32_t av1, uint32_t bv1):time(a),volume(b),level(c),ap1(ap1),bp1(bp1),av1(av1),bv1(bv1){};
};

enum COM_RESULT {
    COM_RESULT_OK = 0,
    COM_RESULT_TIME_ERROR   = 1,     // 时间倒流
    COM_RESULT_VOL_ERROR    = 2,     // vol倒流
    COM_RESULT_REPEAT_L1    = 3,     // 无用的Lv1
    COM_RESULT_REPEAT_L2    = 4,     // 无用的Lv2
};


#pragma pack(1)
struct QuotaInfo{
    uint64_t time;
    uint64_t volume;
    double   ap1;
    double   bp1;
	double	 turn_over;               // 成交金额
    double   open;
    double   high;
    double   low;
    double   last_price;
    double   upper;            
    double   lower;
    double   last_settlement_price;
    double   last_closing_price;

    uint32_t level;
    uint32_t av1;
    uint32_t bv1;
    uint32_t open_interest;
    uint32_t init_open_interest;      // 初始持仓量
    
};
#pragma pack()



// struct InstKey {
//     char data[20];

//     InstKey() {
//         memset(data, 0, 20);
//     }

//     InstKey(const char* str) {
//         strncpy(data, str, 19);
//         data[19] = '\0';
//     }

//     InstKey(const InstKey& other) {
//         memcpy(data, other.data, 20);
//     }

//     bool operator==(const InstKey& other) const {
//         return memcmp(data, other.data, 20) == 0;
//     }
// };



// struct InstKeyHash {
//     uint64_t operator()(const InstKey& key) const {
//         // 将20个字节视为两个uint64_t和一个uint32_t
//         const uint64_t* p1 = reinterpret_cast<const uint64_t*>(key.data);
//         const uint64_t* p2 = reinterpret_cast<const uint64_t*>(key.data + 8);
//         const uint32_t* p3 = reinterpret_cast<const uint32_t*>(key.data + 16);
//         uint64_t h1 = *p1;
//         uint64_t h2 = *p2;
//         uint64_t h3 = *p3;
//         return h1 ^ (h2 << 1) ^ (h3 << 2);
//     }
// };




















#define RING_BUFFER_SIZE 4096

using fb_market_data_t = cffex::fb::api::market_data_entity;

class CNanoMdReceiver : public CNanoDceMdSpi
{
public:
    CNanoMdReceiver(int32_t instanceIndex);
    ~CNanoMdReceiver();

    void RegistSender(cffex::fb::api::fb_i_md_spi *fb_spi, fb_market_data_t *fb_md);

public:
    //一档行情回调接口
    virtual void OnNanoDceL1Md(const NanoDceL1MdType& refNanoDceL1Md);

    //五档行情回调接口
    virtual void OnNanoDceL2ContractBestPriceMd(const NanoDceL2ContractBestPriceMdType& refNanoDceL2ContractBestPriceMd);
    virtual void OnNanoDceL2ArbBestPriceMd(const NanoDceL2ArbBestPriceMdType& refNanoDceL2ArbBestPriceMd);
    virtual void OnNanoDceL2SegQuotaMd(const NanoDceL2SegQuotaMdType& refNanoDceL2SegQuotaMd);
    virtual void OnNanoDceL2OrderStatisticsMd(const NanoDceL2OrderStatisticsMdType& refNanoDceL2OrderStatisticsMd);
    virtual void OnNanoDceL2DeepOrderVolumeMd(const NanoDceL2DeepOrderVolumeMdType& refDeepOrderVolumeMd);
    virtual void OnNanoDceL2DeepQuoteMd(const NanoDceL2DeepQuoteMdType& refNanoDceL2DeepMd);

    virtual void OnEvent(const NanoEventType& refEventType);

public:

    inline void SaveInstStaticInfo(CNanoDceMdApi& refNanoDceMdApi);

    inline int32_t IsValidQuota(QuotaInfo *cache, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1);

    inline void SetSubscribeInst(std::set<std::string> &vec);

    void SetCpu( int cpu);
    void SetOBMask( uint32_t mask);

    void ProcessMsg();

    void DispatchMessage(const Slot& slot);

private:

    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;

    InstMap<QuotaInfo>                                         m_quota_cache; 

    std::set<std::string>     m_subscribe_insts;

    int32_t                                                  m_index = 0;  //对应通道

    LockFreeRingBuffer        *m_ringbuffer_lv1;
    LockFreeRingBuffer        *m_ringbuffer_lv2;


    std::thread               *m_dispatcher;          
    int                        m_dispatcher_cpu;

    uint32_t                   m_ob_level_mask;

    uint64_t m_l1_time_err_sum = 0;
    uint64_t m_l1_vol_err_sum = 0;
    uint64_t m_l1_repeat = 0;

    uint64_t m_l2_time_err_sum = 0;
    uint64_t m_l2_vol_err_sum = 0;
    uint64_t m_l2_repeat = 0;

};

using fb_market_data_t = cffex::fb::api::market_data_entity;

class QxNanoMdMgr : public cffex::fb::api::fb_md_plugin_api{
public:
    QxNanoMdMgr();
    ~QxNanoMdMgr();

private:
    inline void LoadJsonCfg();
    void Routine(int index);

public:            //override  cffex::fb::api::fb_i_md_spi
    int  init()    override;
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    
    Config_t                        m_config;
    std::vector<std::thread*>       m_worker;

    std::set<std::string>        m_subscribe_insts;

    //febao
    std::atomic<bool>                                m_fb_initialized = false;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;





};



