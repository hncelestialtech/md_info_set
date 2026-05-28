#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "acc_md_sh.h"
#include "acc_md.h"
#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()
#define MSG_MAX_LEN     1516  //所有类型的行情数据msg的最大长度，不包含协议头
#define QUEUE_MAX_LEN   (1024*1024)  //环形队列的长度

using fb_market_data_t = cffex::fb::api::market_data_entity;

struct MultiCastConf
{
    std::string section;
    uint32_t ip;   
    uint16_t port;

    MultiCastConf(std::string a,uint32_t b, uint16_t c):section(a),ip(b),port(c){}

};

struct Config_t{
    std::string ifname;
    std::vector<MultiCastConf> multicast;
    int read_cpu;
    int write_cpu;
    std::string filter_path;
};



struct Element {
    uint32_t  msg_type;
    uint32_t  used;
    uint64_t  timestamp;
    char      data[MSG_MAX_LEN];
};



const std::string ADDR1("addr1");
const std::string ADDR2("addr2"); 
const std::string ADDR3("addr3");
const std::string ADDR4("addr4");


std::set<std::string> g_filter_code = {"159915.SZ","588000.SH"}; //只要创业板ETF和科创板ETF





class OptionInfoFilter{
public:

    OptionInfoFilter():m_underlying{"588000","159915","IO"}{
    }
    void Load(std::string instfile)
    {
        printf("instfile[%s]\n",instfile.c_str());
        std::ifstream if_inst(instfile,std::ios::in);
        std::string line;
        while (std::getline(if_inst, line)) {

            std::string_view line_view(line);
            size_t start = 0;
            size_t end = 0;
            int currentCol = 0;
            std::string_view underlying;
            std::string_view inst;
            while ((end = line_view.find(',', start)) != line_view.npos) {

                if (currentCol == 7) {
                    inst = line_view.substr(start, end - start);
                }

                if (currentCol == 16) {
                    std::string_view underlying2 = line_view.substr(start, end - start);
                    size_t pos = underlying2.find('_');
                    underlying = underlying2.substr(pos + 1);
                }

                start = end + 1;
                currentCol ++;
            }

            if (!m_underlying.count((std::string)underlying)){
                continue;
            }
            m_underlying_inst.emplace((std::string)(inst), (std::string)(underlying));

            std::cout << inst <<","<< underlying<< '\n';
        }
    }

    bool Filter(std::string inst){
        if (m_underlying.count(inst) > 0) {
            return true;
        }

        return m_underlying_inst.count(inst)>0;
    }

private:
    std::unordered_map<std::string, std::string>   m_underlying_inst;
    std::set<std::string>                          m_underlying;
};











class QxIndexEtfMd : public cffex::fb::api::fb_md_plugin_api{
public:
    QxIndexEtfMd();
    ~QxIndexEtfMd();

private:
    // inline void LoadCfg();
    inline void LoadJsonCfg();
    void Routine();
    inline void MsgRoutine();
    void EfviInit();

    void ReaderRoutine();
    void WriterRoutine();
    bool InitExanic(const char *ifName);

    inline bool belongTo(std::string &section);

    inline bool belongTo(uint32_t ip, uint16_t port);

    // void handle_shse_snapshot(Element &msgdata, fb_market_data_t *targetdata);
    // void handle_shse_option_snapshot(Element &elem, fb_market_data_t *targetdata);
    // void handle_szse_snapshot(Element &msgdata, fb_market_data_t *targetdata);
    // void handle_szse_option_snapshot(Element &elem, fb_market_data_t *targetdata);

    void handle_default(Element &msgdata, fb_market_data_t *targetdata);

    void ReleaseExanic();
    void OnRcvData(const char *data, unsigned int dataLen, uint64_t timestamp);


    inline void SendMsg();


    std::string GetInstrument(int msgtype, const char *aData);


public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;


    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;

private:
    //exanic 
    
    Config_t    m_config;

    
    exanic_t *m_exanic;
    exanic_rx_t *m_exanic_rx;
    
    std::vector<std::thread*>  m_reader;
    std::atomic<bool> m_exanic_initialized;
    std::atomic<bool> m_reader_running;

    //febao
    std::atomic<bool> m_fb_initialized;  //先ready发送，再ready接收



    fb_market_data_t                                 *m_fb_md;

    struct fb_md_interval_t{
        double open;
        double high;
        double low;
        void clear(){
            open = FLOAT64_NAN;
            high = FLOAT64_NAN;
            low = FLOAT64_NAN;
        }
    };

    fb_md_interval_t                                 m_fb_md_interval;

    cffex::fb::api::inquiry_quote_entity             *m_fb_inquiry;
    cffex::fb::api::instrument_trading_status_entity *m_fb_status;

    std::vector<std::thread*>  m_writer;


    volatile int64_t m_delay_stats;
    volatile uint64_t m_count_stats;




    OptionInfoFilter  m_option_info;


  



};




using MsgHander = void (*)(QxIndexEtfMd *pthis, Element & data, fb_market_data_t *);

