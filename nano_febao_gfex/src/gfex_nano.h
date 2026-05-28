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

#include "NanoGfexMdApi.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()


#define VALID_MAX_VOL    1e20
#define VALID_MAX_PRICE  1e10



#define IOPV_TAG_LV1     100000
#define IOPV_TAG_LV2     200000


struct Config_t{
    std::string config_path;
    std::vector<int> cpu_id;
    std::string filter_path;
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
    COM_RESULT_USELESS_LV1 = 3,     // 无用的Lv1
};

#define RING_BUFFER_SIZE 4096



// union Slot {
//     NanoGfexL1MdType l1_data;
//     NanoGfexL2MdType l2_data;
// };

// struct alignas(64) RingBuffer {
//     std::array<Slot, RING_BUFFER_SIZE> slots;
//     std::atomic<uint64_t> write_idx{0};
//     std::atomic<uint64_t> read_idx{0};
// };


using fb_market_data_t = cffex::fb::api::market_data_entity;

class CNanoMdReceiver : public CNanoGfexMdSpi
{
public:
    CNanoMdReceiver(int32_t instanceIndex);
    ~CNanoMdReceiver();

    void RegistSender(cffex::fb::api::fb_i_md_spi *fb_spi, fb_market_data_t *fb_md);

public:
    //一档行情回调接口
    virtual void OnNanoGfexL1Md(const NanoGfexL1MdType& refNanoGfexL1Md);

    //五档行情回调接口
    virtual void OnNanoGfexL2Md(const NanoGfexL2MdType& refNanoGfexL2Md);

public:

    inline void SaveInstStaticInfo(CNanoGfexMdApi& refNanoGfexMdApi);

    inline int32_t IsValidQuota(const std::string &inst, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1);

    inline void SetSubscribeInst(std::vector<std::string> &vec);

    void SetFilter( OptionInfoFilter * filter);
    void SetCpu( int cpu);

    void ProcessMsg();

    void DispatchMessage(const Slot& slot);

private:

    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;

    struct fb_md_interval_t{
        double open;
        double high;
        double low;
        void clear(){
            open = FLOAT64_NAN;
            high = FLOAT64_NAN;
            low = FLOAT64_NAN;
        };

        fb_md_interval_t(double a, double b, double c):open(a),high(b),low(c){};
    };

    std::unordered_map<std::string, QuotaMerge_t>   m_quota_merge;

    std::vector<std::string>   m_subscribe_insts;

    std::unordered_map<std::string, fb_md_interval_t>        m_fb_md_interval;

    std::unordered_map<std::string, NanoGfexInstStaticInfo>  m_static_info;

    int32_t                                                  m_index = 0;  //对应通道

    OptionInfoFilter          *m_option_info;

    LockFreeRingBuffer        *m_ringbuffer;
    std::thread               *m_dispatcher;          
    int                        m_dispatcher_cpu;


    uint64_t m_l1_time_err_sum = 0;
    uint64_t m_l1_vol_err_sum = 0;
    uint64_t m_l1_useless_lv1_sum = 0;

    uint64_t m_l2_time_err_sum = 0;
    uint64_t m_l2_vol_err_sum = 0;


};

using fb_market_data_t = cffex::fb::api::market_data_entity;

class QxNanoMdMgr : public cffex::fb::api::fb_md_plugin_api{
public:
    QxNanoMdMgr();
    ~QxNanoMdMgr();

private:
    inline void LoadJsonCfg();
    void Routine(int index);


    inline bool belongTo(uint32_t ip, uint16_t port);

    inline void MsgRoutine(CNanoGfexMdApi& refNanoGfexMdApi);

    void inst_stats(std::string inst);


public:            //override  cffex::fb::api::fb_i_md_spi
    int  init()    override;
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    
    Config_t                        m_config;
    std::vector<std::thread*>       m_worker;

    //filter
    OptionInfoFilter               *m_option_info;


    std::vector<std::string>   m_subscribe_insts;

    //febao
    std::atomic<bool>                                m_fb_initialized = false;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;





};



