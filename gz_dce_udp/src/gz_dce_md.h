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

#include "udp_receiver.h"

#include "fb_md_plugin_api.h"

// #include "NanoDceMdApi.h"

#include "inst_map.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()


#define VALID_MAX_VOL    1e20
#define VALID_MAX_PRICE  1e10



#define IOPV_TAG_LV1     100000
#define IOPV_TAG_LV2     200000


#define OB_LEVEL_MASK_LV1     1
#define OB_LEVEL_MASK_LV2     5


#define KEY_LENGTH 64


#define DECRYPT_LOCAL_KEY     1
#define DECRYPT_REMOTE_KEY    2


struct UdpConfig{
    std::string interface_name;
    uint32_t    cpu_no;

    std::string group_ip;
    uint16_t    group_port;
    uint32_t    net_group_ip;
    uint16_t    net_group_port;
};





struct Config_t{
    UdpConfig        udpconfig;
    std::vector<int> cpu_id;
    std::string      filter_path;
    uint32_t         ob_level_mask;
    uint32_t         decrypt;  //1 用290msg生成,  2 用.a

    std::string      remote_key_path;
    std::string      remote_key_path_bak;
    uint32_t         channel;

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
    double   pre_settle;
    double   pre_close;

    uint32_t level;
    uint32_t av1;
    uint32_t bv1;
    uint32_t open_interest;
    uint32_t pre_open_interest;      // 初始持仓量
    
};




struct DceL2_366{
    int16_t    len;
    uint16_t   type;
    uint8_t    version;
    uint8_t    flag;
    uint8_t    mdgno;
    uint64_t   seqno;
    uint8_t    seqnum;
    uint64_t   sendtime;
    char       res_1[14];
    char       code[20];   //eb2605-C-12000
    char       res_2[109];
    char       product[5];
    char       product_class[5];
    uint32_t   date;
    char       extime[13];
    //
    char       res_3[12];

    struct DepthLevel {
        double   price;      // 价格 = 
        uint32_t qty;        // 数量
        uint32_t imp_qty;    // 推导量
    };

    DepthLevel bid[5];
    DepthLevel ask[5];
};



struct DceL2_370{
    int16_t    len;
    uint16_t   type;
    uint8_t    version;
    uint8_t    flag;
    uint8_t    mdgno;
    uint64_t   seqno;
    uint8_t    seqnum;
    uint64_t   sendtime;
    char       res_1[14];
    char       code[20];
    char       res_2[109];
    char       product[5];
    char       product_class[5];
    uint32_t   date;
    char       extime[13];
    char       res_3[12];
    double     last_price;
    double     high;
    double     low;
    uint32_t   last_match_vol;
    uint32_t   volume;
    double     turn_over;
    uint32_t   pre_open_interest;
    uint32_t   open_interest;
    int32_t    open_interest_chg;
    double     clear_price;   //ff ff ff ff ff ff ef 7f
    double     life_low;    //
    double     life_high;     //
    double     upper;
    double     lower;
    double     pre_settle;
    double     pre_close;

    double     bp1;
    uint32_t   bv1;
    uint32_t   bv1_imp;

    double     ap1;
    uint32_t   av1;
    uint32_t   av1_imp;

    double     avg_price;
    double     open;
    double     close;  //ff ff ff ff ff ff ef 7f
};





#pragma pack()



using fb_market_data_t = cffex::fb::api::market_data_entity;



class QxMdMgr : public cffex::fb::api::fb_md_plugin_api{
public:
    QxMdMgr();
    ~QxMdMgr();

private:
    inline void LoadJsonCfg();

    inline void decrypt(const char *input, int32_t msg_len, char *output);



    inline void GenKey_290(const char *msg, int16_t len);


    void DecodeMsg(const char* msgptr, size_t msg_len);

    static void UdpHandler(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port, int64_t t0=0);

    void RemoteKeyDecodeMsg(const char* data, size_t size);

    void Mds_Init();
    void Mds_Deinit();


    void mds_on_best(const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_best* best, void* dataAttachedInfo);
    void mds_on_deep(const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_deep* deep, void* dataAttachedInfo);


    static void on_best(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_best* best, void* dataAttachedInfo);
    static void on_arbi_best(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_arbi_best* arbiBest, void* dataAttachedInfo);
    static void on_deep(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_deep* deep, void* dataAttachedInfo);


public:            //override  cffex::fb::api::fb_i_md_spi
    int  init()    override;
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    
    Config_t                        m_config;

    std::shared_ptr<MulticastReceiver>          m_udp_receiver;

    std::set<std::string>        m_subscribe_insts;

    //febao
    std::atomic<bool>                                m_fb_initialized = false;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;


    InstMap<QuotaInfo>                                         m_quota_cache; 

    uint32_t                   m_ob_level_mask;




    uint64_t m_l1_time_err_sum = 0;
    uint64_t m_l1_vol_err_sum = 0;
    uint64_t m_l1_repeat = 0;

    uint64_t m_l2_time_err_sum = 0;
    uint64_t m_l2_vol_err_sum = 0;
    uint64_t m_l2_repeat = 0;



    unsigned char m_calc_key[KEY_LENGTH] = {0};
    std::atomic<bool> m_key_gen_finished{false};



    dcel2param_mds* m_mds_param{nullptr};

};



