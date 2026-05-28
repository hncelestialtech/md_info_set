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


#include "macro.h"

// #include "tcp_ringbuffer.h"

#include "raw_tcp_receiver.h"

// #include "udp_receiver.h"


#include "ringbuffer.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()


#define PRICE_NROMAL       0
#define PRICE_UP_LIMIT     1
#define PRICE_DOWN_LIMIT   2


constexpr size_t LV2_OB_QUOTE_LEN = 676;

using fb_market_data_t = cffex::fb::api::market_data_entity;

#pragma pack(push, 1)








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



struct QuotaInfo{
    uint64_t time;
    uint64_t volume;
    double   ap1;
    double   bp1;
    uint32_t av1;
    uint32_t bv1;
    uint32_t level;
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
    char     Symbol[0x47];
    char     res[13];
    double   price;
    int32_t  Volume;
};

struct static_info_mark{  
	uint16_t res;
	uint32_t mark;
};


struct common_head{
    uint16_t  mark1;     //0100
    uint8_t   msg_type;  //  00   quota   01 time   0f  index
    uint8_t   len;      
    uint32_t  mark2;     // 00000000
    uint16_t  mark3;     // 0100
    uint16_t  mark4;     // time 0002   // 0008
    uint16_t  mark5;     // 0353  1343  3353
    uint16_t  mark6;     // 0004
    uint8_t   mark7;     // 00
    uint8_t   mark8;     // 00
    uint16_t  seq; 
    uint8_t   mark9;     //00
    uint8_t   sub_type;
    uint8_t   res1;
    uint8_t   q_len; 
};


// 0100
// 0f
// de
// 00000000
// 0100
// 0008
// 1346
// 0004
// 00
// 00
// 0000
// 00
// 2b
// 0f
// ca

// 0100
// 05
// 38
// 00000000
// 0100
// 0008
// 134c
// 0004
// 00
// 00
// 0077
// 00
// 0e
// 05
// 24




struct ip_info{
    uint32_t  res1;
    uint32_t  res2;
	uint16_t port;
	char ip[15];
};

struct server_t{
    std::string ip;
    uint16_t    port;
    uint32_t    net_ip;
    uint16_t    net_port;
    server_t(std::string a, uint16_t b, uint32_t c, uint16_t d):ip(a),port(b),net_ip(c),net_port(d){};
};



struct code_mapping_info{  
	uint16_t res;
	uint32_t mark;
    char  code[20]; 
    char    res2[62]; 
    uint32_t   idx; 
    char       res3; 
    char       type; 
}; 

struct daily_static_info{  
	uint16_t res;
	uint32_t mark;
    char     date[8]; 
    char     code[24]; 
    char     code2[40]; 
    double   res1[3];
    uint32_t res2[4];
    uint32_t pre_openinterest;
    uint32_t pre_openinterest2;
    uint32_t uk1;
    double   res3;
    double   uk2[2];
    double   upper;
    double   lower;
    double   pre_settle;
    double   pre_close;
    double   res4[5];
    char     time[12];
    double   res5[2];
}; 


// 0000
// 000700f8
// 3230323630343137
// 6c63323630382d502d363830303000000000000000000000
// 6c63323630382d502d36383030300000000000000000000000000000000000000000000000000000
// 7fefffffffffffff7fefffffffffffff7fefffffffffffff
// 00000000
// 00000000
// 00000000
// 00000000
// 00000003
// 00000003
// 00000000
// 7fefffffffffffff
// 4071800000000000
// 40b3240000000000
// 40d5ec8000000000
// 4024000000000000
// 4024000000000000
// 4024000000000000
// 7fefffffffffffff00000000000000007fefffffffffffff00000000000000007fefffffffffffff
// 30373a30343a333700000000
// 7fefffffffffffff
// 7fefffffffffffff





struct QuotaBase{
    uint64_t mark;
    uint16_t idx;
    char     extime[12];
};

struct quota_head{
    uint32_t type;
    uint16_t  d_type;
};






struct StaticInfo{
    char     code[20];
    uint32_t idx;    
    uint32_t pre_openinterest;
    double   upper;
    double   lower;
    double   pre_settle;
    double   pre_close;

    // StaticInfo(std::string inst, uint32_t idx, uint32_t oi, double up, double lo,double ps,double pc)
    //     :code{},idx(idx),pre_openinterest(oi),upper(up),lower(lo),pre_settle(ps),pre_close(pc)
    // {
    //     size_t len = std::min((size_t)20, inst.size());
    //     memcpy(code, inst.c_str(), len);
    //     code[len] = '\0';
    // }
}; 







#pragma pack(pop)


#define   MARK_INFO   0x58007900   //94  = 0x


struct MulticastConfig{
    std::string     ifname;
    std::string     group_ip;
    uint16_t        group_port;
    uint32_t        net_group_ip; 
    uint16_t        net_group_port; 
};

struct IpPort
{
    uint32_t    ip;   
    uint16_t    port;
    IpPort(uint32_t a, uint16_t b):ip(a),port(b){}
};


struct TCPFilter{
    IpPort src;
    IpPort dst;
};




struct Config_t{
    TcpConfig            lv2config;
    TcpConfig            lv1config;

    std::vector<int> cpu_id;
    std::string      filter_path;

    std::string       mapping_local;
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



#define TYPE_INDEX   1
#define TYPE_TIME    2
#define TYPE_QUOTA   3
#define TYPE_ARBITRAGE    4



inline double reverseDouble(double value) {
    uint64_t bits = std::bit_cast<uint64_t>(value);
    bits = __builtin_bswap64(bits);  // GCC/Clang
    // 或 bits = _byteswap_uint64(bits);  // MSVC
    return std::bit_cast<double>(bits);
}

inline uint32_t reverseU32(uint32_t value) {
    return __builtin_bswap32(value);
}

inline int32_t reverseInt32(uint32_t value) {
    return __builtin_bswap32(value);
}




inline bool is_remote_IP_notify(const char *msg, size_t len){
    static const unsigned char MAGIC_PATTERN[56] = {
        0x01, 0x00, 0x00, 0xaa, 0x00, 0x00, 0x00, 0x00, 
        0x01, 0x00, 0x00, 0x09, 0x01, 0x53, 0x00, 0x05,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x06, 0x00, 0x96, 
        0x00, 0x00, 0x00, 0xa4, 0x00, 0x13, 0x00, 0x00
    };
    static const size_t PATTERN_LEN = 32;  // 56字节
    if (len < PATTERN_LEN) {
        return false;  // 报文太短，无法匹配完整特征
    }
    return std::memcmp(msg, MAGIC_PATTERN, PATTERN_LEN) == 0;
}

inline bool is_remote_str(const char *msg, size_t len){
    static const unsigned char MAGIC_PATTERN[56] = {
        0x01, 0x00, 0x01, 0xa0, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x12, 0x53, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x01, 0x8c
    };
    static const size_t PATTERN_LEN = 24;  // 56字节
    if (len < PATTERN_LEN) {
        return false;  // 报文太短，无法匹配完整特征
    }
    return std::memcmp(msg, MAGIC_PATTERN, PATTERN_LEN) == 0;
}

inline bool is_remote_date_notify(const char *msg, size_t len){
    static const unsigned char MAGIC_PATTERN[56] = {
        0x01, 0x00, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x01, 0xf2, 0x53, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0xaa,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0xbd, 0xbb, 0xd2, 0xd7, 0xcf, 0xaf,
        0xce, 0xbb, 0xb5, 0xc7, 0xc2, 0xbc, 0xb3, 0xc9,
        0xb9, 0xa6, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    static const size_t PATTERN_LEN = 56;  // 56字节
    if (len < PATTERN_LEN) {
        return false;  // 报文太短，无法匹配完整特征
    }
    return std::memcmp(msg, MAGIC_PATTERN, PATTERN_LEN) == 0;
}


inline uint16_t get_common_head_type(const char *msg, size_t len, size_t &q_len){
    // hexDumpToClog2(msg, sizeof(common_head));
    if (len < sizeof(common_head)){
        // std::clog<<__func__<<","<< __LINE__<<std::endl;
        return 0;
    }

    common_head *header = (common_head *)msg;
    // printf("%s,%d,%04X,%02X,%08X,%04X,%04X,%04X,%04X,%04X,%04X,%02X\n",__func__,__LINE__,
    //      header->mark1,header->msg_type,header->mark2,header->mark3,header->mark4,header->mark5,header->mark6,header->mark7,header->seq,header->q_len);

    //std::clog<<__func__<<","<< __LINE__<<","<<<<","<<(int)<<","<<(int)header->len<<","<<<<","<<header->mark3<<","<<<<","<<<<","<<<<","<<<<",seq:"<<<<std::endl;
    if (header->mark1 != 0x0001 || header->mark2 != 0x00000000 || header->mark3 != 0x0001 || (header->mark4 != 0x0200 && header->mark4 != 0x0800) || header->mark6 != 0x0400 || header->mark7 != 0x00) {
        // hexDumpToClog2(msg, sizeof(common_head));
        // std::clog<<__func__<<","<< __LINE__<<std::endl;
        return 0;
    }

    if ((header->msg_type == 0x07 ||header->msg_type == 0x0f || header->msg_type == 0x0e || header->msg_type == 0x05) && (header->mark5 == 0x4313 || header->mark5 == 0x4613 || header->mark5 == 0x4c13)){
        // std::clog<<__func__<<","<< __LINE__<<std::endl;
        q_len = header->q_len;
        return TYPE_INDEX;
    }

    if (header->msg_type == 0x01 && header->res1 == 0x01 && header->mark5 == 0x5333 && header->mark9 == 0x00){
        // hexDumpToClog2(msg, sizeof(common_head));
        // std::clog<<__func__<<","<< __LINE__<<std::endl;
        q_len = header->q_len + 0x100;
        return TYPE_ARBITRAGE;
    }

    if ((header->msg_type == 0x01 && header->mark5 == 0x5333) || (header->msg_type == 0x00 && header->mark5 == 0x5343)){
        // std::clog<<__func__<<","<< __LINE__<<std::endl;
        q_len = header->q_len;
        return TYPE_TIME;
    }

    if (header->msg_type == 0x00 && header->mark5 == 0x5303){
        // std::clog<<__func__<<","<< __LINE__<<std::endl;
        q_len = header->q_len;
        return TYPE_QUOTA;
    }

    // std::clog<<__func__<<","<< __LINE__<<std::endl;
    // hexDumpToClog2((char *)&(header->mark5),2);
    return 0;
}



// struct common_head{
//     uint16_t  mark1;     //0100
//     uint8_t   msg_type;  //  00   quota   01 time   0f  index
//     uint8_t   len;      
//     uint32_t  mark2;     // 00000000
//     uint16_t  mark3;     // 0100
//     uint16_t  mark4;     // time 0002   // 0008
//     uint16_t  mark5;     // 0353  1343  3353
//     uint16_t  mark6;     // 0004
//     uint8_t   mark7;     // 00
//     uint8_t   mark8;     // 00
//     uint16_t  seq; 
//     uint8_t   mark9;     //00
//     uint8_t   sub_type;
//     uint8_t   res1;
//     uint8_t   q_len; 
// };


// 0100
// 0f
// de
// 00000000
// 0100
// 0008
// 1346
// 0004
// 00
// 00
// 0000
// 00
// 2b
// 0f
// ca



class GFEXQuota : public cffex::fb::api::fb_md_plugin_api{
public:
    GFEXQuota();
    ~GFEXQuota();

private:
    inline void LoadJsonCfg();


    inline bool level2_belongTo(uint32_t ip, uint16_t port);
    inline bool level1_belongTo(uint32_t ip, uint16_t port);

    void ProcessMsg(int32_t cpu_id);
    void DispatchMessage(const Slot& slot);



    inline int32_t IsValidQuota(QuotaInfo *cache, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1);

    void OnData_lv1(const char* data, size_t size, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port);
    void OnData_lv2(const char* data, size_t size);    

    static void Lv1Handler(void *ctx, const char* data, size_t size, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port);
    static void Lv2Handler(void *ctx, const char* data, size_t size, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port);

    size_t GetCurReadMsgLength(const char* data) const ;

    void HandleLv1Quota(uint64_t t0, const char *data, size_t data_len);
    void HandleLv1Code2Index(const char *data, size_t data_len);
    void HandleLv1StaticInfo(const char *data, size_t data_len);

    bool LoadMaping();

public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;


    std::shared_ptr<RawTcpReceiver>          m_lv2_receiver;
    std::shared_ptr<RawTcpReceiver>          m_lv1_receiver;


    std::set<std::string>                    m_subscribe_insts;

    InstMap<StaticInfo>                      m_staticinfo_cache; 
    InstMap<QuotaInfo>                       m_quota_cache; 
    InstMap<Quota_t>                         m_lv1_quota;

    LockFreeRingBuffer        *m_ringbuffer;
    std::thread               *m_dispatcher;   

    uint64_t m_l1_time_err_sum = 0;
    uint64_t m_l1_vol_err_sum = 0;
    uint64_t m_l1_useless_lv1_sum = 0;

    uint64_t m_l2_time_err_sum = 0;
    uint64_t m_l2_vol_err_sum = 0;


    uint64_t m_l2_apbp_err_sum = 0;

    //febao
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;

    // lv1
    char   m_half_buffer[1024]={0};
    size_t m_half_buffer_used = 0;

    std::set<uint32_t>              m_lv1_code_index_set;
    std::map<uint32_t,std::string>  m_lv1_code_map;

    bool                   m_ser_list_recv{false};
    uint16_t               m_lv1_dst_port{0};
    std::vector<server_t>  m_server_list;


    std::unordered_map<std::string, uint32_t>    m_code_save;
    std::vector<StaticInfo>                      m_staticinfo_save;


};
