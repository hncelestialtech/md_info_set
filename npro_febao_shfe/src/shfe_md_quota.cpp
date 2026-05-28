
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include "common.h"

#include "shfe_md_quota.h"
#include "fb_md_plugin_api.h"
#include "fb_md_entity.h"
#include "fb_md_helper.h"
#include "fb_md_type.h"

// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    return new ShfeMDQuota();
}
void destroy(void *p) {
    delete (ShfeMDQuota*)p;
}
void get_md_api_version(char version[32]) {
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;

std::string  g_npro_monitor_info;


ShfeMDQuota::ShfeMDQuota(): 
    m_fb_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity()),
    m_fb_inquiry(cffex::fb::api::inquiry_quote_entity::create_entity()),
    m_fb_status(cffex::fb::api::instrument_trading_status_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    m_fb_md_interval.clear();
    m_delaystats.reserve(10000000);
}

ShfeMDQuota::~ShfeMDQuota(){
    std::clog << std::nounitbuf;
}

void ShfeMDQuota::register_spi(cffex::fb::api::fb_i_md_spi *spi)
{
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    m_fb_spi = spi;
}

inline bool ShfeMDQuota::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    try{
        cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
        std::string md_jsoncfgfile;
        parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);

        std::ifstream jsonfile(md_jsoncfgfile);
        nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

        auto& worker = json_parser["worker"];
        m_config.cpu_id = worker["cpuid"].get<uint32_t>();

        // auto& filter = json_parser["filter"];
        // for (const auto& item : filter["underlying"]) {
        //     m_option_info.AddUnderlying(item);
        // }
        // m_config.filter_path = filter["filter_instrument_info"];
        // m_option_info.Load(m_config.filter_path);
        std::clog <<__func__<<","<< __LINE__<<",cpu[" <<m_config.cpu_id<<"]"<< std::endl;
        // std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;


        //npro cfg
        auto& nprocfg = json_parser["npro"];
        m_npro_cfg.deviceName  = nprocfg["device_name"];
        std::clog <<__func__<<","<< __LINE__<<",deviceName[" <<m_npro_cfg.deviceName<<"]"<< std::endl;
        m_npro_cfg.licenseFile = nprocfg["license_file"];
        std::clog <<__func__<<","<< __LINE__<<",licenseFile[" <<m_npro_cfg.licenseFile<<"]"<< std::endl;
        m_npro_cfg.configFile  = nprocfg["config_file"];
        std::clog <<__func__<<","<< __LINE__<<",configFile[" <<m_npro_cfg.configFile<<"]"<< std::endl;
        for (auto & str : nprocfg["subscribes"]){
            m_npro_cfg.subscribes.emplace_back(str);
            std::clog <<__func__<<","<< __LINE__<<",subscribes[" <<str<<"]"<< std::endl;
        }
        m_npro_cfg.exchange = nprocfg["exchanges"];
        std::clog <<__func__<<","<< __LINE__<<",exchange[" <<m_npro_cfg.exchange<<"]"<< std::endl;

        if ((m_npro_cfg.exchange == "shfe") || (m_npro_cfg.exchange == "shfedepth")){
            m_fb_exchange_id = cffex::fb::api::FB_EXCHANGE_SHFE;
        }
        else if(m_npro_cfg.exchange == "ine" || m_npro_cfg.exchange == "inedepth"){
            m_fb_exchange_id = cffex::fb::api::FB_EXCHANGE_INE;
        }
        else{
            m_fb_exchange_id = cffex::fb::api::FB_EXCHANGE_UNKNOWN;
        }

        std::clog <<__func__<<","<< __LINE__<<",exchange_id[" <<m_fb_exchange_id<<"]"<< std::endl;

        std::string productClassStr = nprocfg["product_class"];
        if (productClassStr == "all"){
            m_npro_cfg.productClass == 0;
        }
        else{
            m_npro_cfg.productClass = nprocfg["product_class"].get<int>();
        }
  
        m_config.stats = false;
        if (json_parser.contains("stats")){
            auto& stats = json_parser["stats"];
            m_config.stats = true;
            m_config.logpath = stats["logpath"];
            m_config.logtime = stats["logtime"].get<uint32_t>();


            std::clog <<__func__<<","<< __LINE__<<",logpath[" <<m_config.logpath<<"]"<< std::endl;
            std::clog <<__func__<<","<< __LINE__<<",logtime[" <<m_config.logtime<<"]"<< std::endl;

        }

    }
    catch (std::exception &e){
        std::clog <<__func__<<","<< __LINE__<<",load cfg error." << std::endl;
        return false;
    }
 
    std::clog <<__func__<<","<< __LINE__<<",productClass[" <<m_npro_cfg.productClass<<"]"<< std::endl;

    std::clog <<__func__<<","<< __LINE__<<",deviceName[" <<m_npro_cfg.deviceName<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",licenseFile[" <<m_npro_cfg.licenseFile<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",configFile[" <<m_npro_cfg.configFile<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",exchange[" <<m_npro_cfg.exchange<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",productClass[" <<m_npro_cfg.productClass<<"]"<< std::endl;
    return true;
}

int ShfeMDQuota::init()
{
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    if (LoadJsonCfg() == false){
        g_npro_monitor_info += "ERROR:[load json config file error,please check.]";
        return -1;
    }

    m_npro_api = CNProUserApi::CreateNProApi(m_npro_cfg.configFile.c_str());

    const char *version = m_npro_api->GetNicVersion(m_npro_cfg.deviceName.c_str());
    printf(version);
    std::clog <<__func__<<","<< __LINE__<<",:"<<version<< std::endl;

    // std::vector<std::string> instrumentIDs;
    // SplitString(m_npro_cfg.subscribes,instrumentIDs,",; ");

    std::vector<std::string> exchanges;
    SplitString(m_npro_cfg.exchange,exchanges,",; ");

    ENProUserExchangeKind exchangeType = NProUserExchangeInvalid;
    for (std::string e : exchanges)
    {
        printf("交易所:%s\n", e.c_str());
        std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) {return std::tolower(c);});
        if (e == "shfe")
        {
            exchangeType = static_cast<ENProUserExchangeKind>(exchangeType | NProUserExchangeSHFE);
        }
        if (e == "ine")
        {
            exchangeType = static_cast<ENProUserExchangeKind>(exchangeType | NProUserExchangeINE);
        }
        if (e == "shfedepth")
        {
            exchangeType = static_cast<ENProUserExchangeKind>(exchangeType | NProUserExchangeSHFEDepth);
        }
        if (e == "inedepth")
        {
            exchangeType = static_cast<ENProUserExchangeKind>(exchangeType | NProUserExchangeINEDepth);
        }
    }
    if (exchangeType == NProUserExchangeInvalid)
    {
        exchangeType = NProUserExchangeSHFE;
    }
    std::clog <<__func__<<","<< __LINE__<<",exchangeType:"<<exchangeType<<","<<m_npro_cfg.isReadOnly<<","<<m_npro_cfg.productClass<< std::endl;
    m_npro_api->SetExchangeType(exchangeType);
    m_npro_api->SetReadOnly(m_npro_cfg.isReadOnly);
    m_npro_api->SetProductClass(static_cast<ENProUserProductClassKind>(m_npro_cfg.productClass));

    std::clog <<__func__<<","<< __LINE__<<",set success"<< std::endl;

    char **subscribeList = nullptr;
    if (!m_npro_cfg.subscribes.empty())
    {
        subscribeList = new char*[m_npro_cfg.subscribes.size()];
        memset(subscribeList, 0, sizeof(char*) * m_npro_cfg.subscribes.size());
        int i = 0;
        for (auto &instrumentID : m_npro_cfg.subscribes)
        {
            subscribeList[i] = const_cast<char*>(instrumentID.c_str());
            ++i;
        }
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::clog <<__func__<<","<< __LINE__<<",deviceName:"<<m_npro_cfg.deviceName<<","<<m_npro_cfg.licenseFile<<","<<m_npro_cfg.subscribes.size()<< std::endl;

    bool init_ret = m_npro_api->Init(m_npro_cfg.deviceName.c_str(), m_npro_cfg.licenseFile.c_str(), subscribeList,m_npro_cfg.subscribes.size());
    if (!init_ret){
        g_npro_monitor_info += "ERROR:[npro init unsuccess, please check log]";
    }

    std::clog <<__func__<<","<< __LINE__<<",Init ret:"<<init_ret<< std::endl;
    if (!init_ret){
        std::clog <<__func__<<","<< __LINE__<<",error:"<<m_npro_api->GetLastError()<< std::endl;
    }

    std::clog <<__func__<<","<< __LINE__<< std::endl;

    if (subscribeList)
    {
        delete []subscribeList;
        subscribeList = nullptr;
    }

    m_npro_initialized.store(true,std::memory_order_release);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::clog <<__func__<<","<< __LINE__<<"init cost:"<<duration.count()<< std::endl;

    std::vector<TNProInstrumentItem> items;
    m_npro_api->GetInstrumentItems(exchangeType,items);
    for(auto & item: items){
        m_npro_inst_info.emplace(item.InstrumentID,item);

        std::string inst(item.InstrumentID);
        std::string product = inst.substr(0,2);

        //m_febao_inst[product].emplace(inst);
        std::clog <<__func__<<","<< __LINE__<<",InstrumentID:"<<item.InstrumentID<< std::endl;
    }
    m_fb_initialized.store(true,std::memory_order_release);
    // 
    m_worker = new std::thread(&ShfeMDQuota::Routine, this); //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    if (m_config.stats){
        m_stats_thread = new std::thread(&ShfeMDQuota::StatsRoutine, this);
    }

    m_fb_spi->on_ready();

    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    return 0;
}

void ShfeMDQuota::StatsRoutine(){
    int cut = 0;

    std::clog <<__func__<<","<< __LINE__<<","<<m_fb_initialized.load(std::memory_order_relaxed)<<","<< m_npro_initialized.load(std::memory_order_relaxed) <<","<< g_running.load(std::memory_order_relaxed) << std::endl;
   
    if (m_fb_initialized.load(std::memory_order_relaxed) && m_npro_initialized.load(std::memory_order_relaxed)){
        while(g_running.load(std::memory_order_relaxed)) {
            cut = getCurrentTimes();
            std::clog <<__func__<<","<<__LINE__<<",logtime:"<<m_config.logtime<<",cut:"<<cut<< std::endl;
            if(cut > m_config.logtime){
                std::string logfile = m_config.logpath + "/delaystats.log";
                std::ofstream logstream(logfile, std::ios::binary|std::ios::out|std::ios::app);
                for (auto & it : m_delaystats){
                    std::string line = std::to_string(it.extime)+ ","+ std::to_string(it.extime_ms)+ ","+ std::to_string(it.local_time) + "\n";
                    logstream.write(line.c_str(),line.size());
                }
                logstream.close();
                break;
            }
            else{
                // sched_yield();
                sleep(30);
                // std::clog <<__func__<<","<<__LINE__<<",logtime:"<<m_config.logtime<<",cut:"<<cut<< std::endl;
            }
        }
    }
}

void ShfeMDQuota::Routine(){
    BindCPU(m_config.cpu_id);

    if (g_npro_monitor_info.size() > 0){
        std::clog <<g_npro_monitor_info<< std::endl;
    }
    else{
        std::clog <<"SUCCESS:[OK]"<< std::endl;
    }

    std::clog <<__func__<<","<< __LINE__<<","<<m_fb_initialized.load(std::memory_order_relaxed)<<","<< m_npro_initialized.load(std::memory_order_relaxed) <<","<< g_running.load(std::memory_order_relaxed)<< std::endl;

    if (m_fb_initialized.load(std::memory_order_relaxed) && m_npro_initialized.load(std::memory_order_relaxed)){
        while(g_running.load(std::memory_order_relaxed)) {
            MsgRoutine();
        }
    }
}

void ShfeMDQuota::inst_stats(std::string inst){
    static std::unordered_map<std::string, std::set<std::string>>  product_option;
    std::string product = inst.substr(0, 2);
    if(product_option.count(product) == 0){
        product_option.emplace(product,std::set<std::string>{inst});
    }
    else{
        if (product_option[product].count(inst) == 0){
            product_option[product].emplace(inst);
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],product size[%zu], product[%s], inst size[%zu], inst[%s]\n",__LINE__, product_option.size(), product.c_str(), product_option[product].size(), inst.c_str());
        
            std::string line;
            for(auto &it : m_febao_inst){
                std::string product = it.first;
                line +=product;
                line +="[";
                line +=std::to_string(it.second.size());
                line +="],";
            }
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],febao[%s]\n",__LINE__, line.c_str());


            std::string line2;
            for(auto &it : product_option){
                std::string product = it.first;
                line2 += product;
                line2 +="[";
                line2 += std::to_string(it.second.size());
                line2 +="],";
            }
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],npro[%s]\n",__LINE__, line2.c_str());        
        
        }
    }
 
}



inline void ShfeMDQuota::MsgRoutine(){
    uint64_t local_time_ns = get_nanoseconds();
    //将npro读到的msg转发到febao
    const TNProUserMarketData *msgdata = m_npro_api->Read(); //非阻塞,读不到返回空指针，暂时不做读写分离

    // msgdata = &fakedata;

    if (!msgdata) return;

    // std::clog <<__func__<<","<< __LINE__<<","<<msgdata<< std::endl;

    // auto &inst_info = m_npro_inst_info.at(msgdata->InstrumentID);

    int64_t local_time_ns_0 = get_nanoseconds();

    std::string inst(msgdata->InstrumentID);
    if (m_fb_md_interval.count(inst)==0){
        m_fb_md_interval.emplace(inst,fb_md_interval_t(msgdata->LastPrice,msgdata->LastPrice,msgdata->LastPrice));
        //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],size[%zu],inst[%s]\n",__LINE__,m_fb_md_interval.size(),inst.c_str());
    }

    //inst_stats(inst);

    auto &md_interval = m_fb_md_interval.at(inst);
    if (std::isnan(md_interval.open) && (msgdata->LastPrice>0)){
        md_interval.open = msgdata->LastPrice;
    }

    if (msgdata->LastPrice > md_interval.high){
        md_interval.high = msgdata->LastPrice;
    }

    if (msgdata->LastPrice < md_interval.low){
        md_interval.low = msgdata->LastPrice;
    }

    int64_t local_time_ns_1 = get_nanoseconds();
    uint32_t east_8_offset  = 28800;
    // std::string underlying(std::string(msgdata->InstrumentID, 6));

    // if (underlying == "au2509"){

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],Version[%u],InstrumentID[%s],InstrumentNo[%hu],TimestampSecond[%d],TimestampMillisecond[%d],LastPrice[%.4f][%.4f],Volume[%d],Turnover[%f],oi[%d]\n",__LINE__,
    //                                         msgdata->Version,
    //                                         msgdata->InstrumentID,
    //                                         msgdata->InstrumentNo,
    //                                         msgdata->TimestampSecond ,
    //                                         msgdata->TimestampMillisecond,
    //                                         msgdata->LastPrice,
    //                                         convert_price_if_zero(msgdata->LastPrice),
    //                                         msgdata->Volume,
    //                                         msgdata->Turnover,
    //                                         msgdata->OpenInterest);


    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],open[%.4f],high[%.4f],low[%.4f]\n",__LINE__,
    //                                         md_interval.open,
    //                                         md_interval.high,
    //                                         md_interval.low);



    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap1[%.4f],ap2[%.4f],ap3[%.4f],ap4[%.4f],ap5[%.4f]\n",__LINE__,
    //                                                                 convert_price_if_volume_zero(msgdata->AskPrice[0],msgdata->AskVolume[0]),
    //                                                                 convert_price_if_volume_zero(msgdata->AskPrice[1],msgdata->AskVolume[1]),
    //                                                                 convert_price_if_volume_zero(msgdata->AskPrice[2],msgdata->AskVolume[2]),
    //                                                                 convert_price_if_volume_zero(msgdata->AskPrice[3],msgdata->AskVolume[3]),
    //                                                                 convert_price_if_volume_zero(msgdata->AskPrice[4],msgdata->AskVolume[4]));

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],av1[%d],av2[%d],av3[%d],av4[%d],av5[%d]\n",__LINE__,
    //         msgdata->AskVolume[0],msgdata->AskVolume[1],msgdata->AskVolume[2],msgdata->AskVolume[3],msgdata->AskVolume[4]);

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bp1[%.4f],bp2[%.4f],bp3[%.4f],bp4[%.4f],bp5[%.4f]\n",__LINE__,
    //                                                                 convert_price_if_volume_zero(msgdata->BidPrice[0],msgdata->BidVolume[0]),
    //                                                                 convert_price_if_volume_zero(msgdata->BidPrice[1],msgdata->BidVolume[1]),
    //                                                                 convert_price_if_volume_zero(msgdata->BidPrice[2],msgdata->BidVolume[2]),
    //                                                                 convert_price_if_volume_zero(msgdata->BidPrice[3],msgdata->BidVolume[3]),
    //                                                                 convert_price_if_volume_zero(msgdata->BidPrice[4],msgdata->BidVolume[4]));

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bv1[%d],bv2[%d],bv3[%d],bv4[%d],bv5[%d]\n",__LINE__,
    //         msgdata->BidVolume[0],msgdata->BidVolume[1],msgdata->BidVolume[2],msgdata->BidVolume[3],msgdata->BidVolume[4]);

    // }

    int64_t local_time_ns_2 = get_nanoseconds();

    m_fb_md->reset_entity();                                // 重置飞豹行情结构
    m_fb_md->set_local_timestamp(local_time_ns); //utc ns
    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(msgdata->InstrumentID);
    m_fb_md->set_exchange_id(m_fb_exchange_id);
    m_fb_md->set_update_sec(msgdata->TimestampSecond + east_8_offset);
    m_fb_md->set_update_msec(msgdata->TimestampMillisecond);
    m_fb_md->set_pre_open_interest(msgdata->OpenInterest);
    // m_fb_md->set_pre_settlement();
    // m_fb_md->set_pre_close();
    // m_fb_md->set_pre_open_interest();
    m_fb_md->set_open(convert_price_if_zero(md_interval.open));
    m_fb_md->set_close(convert_price_if_zero(msgdata->LastPrice));

    if (m_npro_inst_info.count(inst) >0){
        auto &inst_item = m_npro_inst_info.at(inst);
        m_fb_md->set_upper_limit_price(inst_item.UpperLimitPrice);
        m_fb_md->set_down_limit_price(inst_item.LowerLimitPrice);
    }

    m_fb_md->set_high_price(convert_price_if_zero(md_interval.high));
    m_fb_md->set_low_price(convert_price_if_zero(md_interval.low));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata->LastPrice));
    m_fb_md->set_volume(msgdata->Volume);
    m_fb_md->set_turn_over(msgdata->Turnover);
    m_fb_md->set_open_interest(msgdata->OpenInterest);
    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->AskPrice[0],msgdata->AskVolume[0]));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->AskPrice[1],msgdata->AskVolume[1]));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->AskPrice[2],msgdata->AskVolume[2]));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->AskPrice[3],msgdata->AskVolume[3]));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->AskPrice[4],msgdata->AskVolume[4]));
    m_fb_md->set_ask1_volume(msgdata->AskVolume[0]);
    m_fb_md->set_ask2_volume(msgdata->AskVolume[1]);
    m_fb_md->set_ask3_volume(msgdata->AskVolume[2]);
    m_fb_md->set_ask4_volume(msgdata->AskVolume[3]);
    m_fb_md->set_ask5_volume(msgdata->AskVolume[4]);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->BidPrice[0],msgdata->BidVolume[0]));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->BidPrice[1],msgdata->BidVolume[1]));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->BidPrice[2],msgdata->BidVolume[2]));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->BidPrice[3],msgdata->BidVolume[3]));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->BidPrice[4],msgdata->BidVolume[4]));
    m_fb_md->set_bid1_volume(msgdata->BidVolume[0]);
    m_fb_md->set_bid2_volume(msgdata->BidVolume[1]);
    m_fb_md->set_bid3_volume(msgdata->BidVolume[2]);
    m_fb_md->set_bid4_volume(msgdata->BidVolume[3]);
    m_fb_md->set_bid5_volume(msgdata->BidVolume[4]);
    // m_fb_md->set_iopv();
    // m_fb_md->set_dynamic_reference_price();


    int64_t local_time_ns_3 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t local_time_ns_end = get_nanoseconds();
    m_delaystats.emplace_back(DelaySummary(msgdata->TimestampSecond,msgdata->TimestampMillisecond,local_time_ns));

    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 1-2
    static uint64_t delay_sum_3 = 0; // 2-3
    static uint64_t delay_sum_4 = 0; // 3-end

    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns);
    delay_sum_0 += (local_time_ns_0 - local_time_ns);
    delay_sum_1 += (local_time_ns_1 - local_time_ns_0);
    delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
    delay_sum_3 += (local_time_ns_3 - local_time_ns_2);
    delay_sum_4 += (local_time_ns_end - local_time_ns_3);


    int cpu = sched_getcpu();
    static uint64_t  period = 10000;
    count_stats ++;
    if (count_stats%period == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],decode[%llu],filter[%llu],fill[%llu],send[%llu][%.4f]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, delay_sum_4/period,delay_sum_4/(delay_sum*1.0));
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;
        delay_sum_4 = 0;
    }
}

void ShfeMDQuota::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    if(m_stats_thread){
        if(m_stats_thread->joinable()){
            m_stats_thread->join();
        }
        delete m_stats_thread;
        m_stats_thread = nullptr;        
    }
}

void ShfeMDQuota::connect(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(true,std::memory_order_relaxed);
}

void ShfeMDQuota::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id)
{
    std::clog <<__func__<<","<< __LINE__<<",instrument_id["<< instrument_id <<"],exchange_id["<<exchange_id <<"]" << std::endl;
    printf("subscribe_inst:instrument_id[%s],exchange_id[%hhu]\n",instrument_id.c_str(),exchange_id);
}



