#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <sched.h>

#include "qx_sz_md_etf_option_exanic.h"
#include "SockProto.h"

#include "fb_md_type.h"



// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create QxSzMdEtfOptionExanic[%s]\n");
    return new QxSzMdEtfOptionExanic();
}
void destroy(void *p) {
    printf("destroy QxSzMdEtfOptionExanic\n");
    delete (QxSzMdEtfOptionExanic*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;


inline bool QxSzMdEtfOptionExanic::belongTo(uint32_t ip, uint16_t port){
    for(auto & it : m_config.multicast){
        if((it.ip == ip) && (it.port == port)){
            return true;
        }
    }
    return false;
}




inline auto MyTransU16(uint16_t a) -> uint16_t
{
    return ((a) >> 8) | ((a) << 8);
}
inline auto MyTransU32(uint32_t a) -> uint32_t
{
    return (a = (((a) >> 16) | ((a) << 16))), (((a) >> 8) & 0x00ff00ff) | (((a) << 8) & 0xff00ff00);
}

inline auto MyTransU64(uint64_t a) -> uint64_t
{
    return (a = (((a) >> 32) | ((a) << 32))), (a = ((((a) >> 16) & 0x0000ffff0000ffff) | (((a) << 16) & 0xffff0000ffff0000))), ((((a) >> 8) & 0x00ff00ff00ff00ff) | (((a) << 8) & 0xff00ff00ff00ff00));
}



QxSzMdEtfOptionExanic::QxSzMdEtfOptionExanic(): 
    m_fb_initialized(false),
    m_exanic_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用

    // std::string testfile = "/home/febao/shenzx/test.csv";  // TODO stub
    // m_quota_stub.Load(testfile);   // TODO stub
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
}




QxSzMdEtfOptionExanic::~QxSzMdEtfOptionExanic(){
    std::clog << std::nounitbuf;
}

void QxSzMdEtfOptionExanic::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}



inline void QxSzMdEtfOptionExanic::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);

    // cfg
    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["eth"];
    m_config.ifname = eth["IfName"];

    std::clog <<__func__<<","<< __LINE__<<",ifname[" <<m_config.ifname<<"]"<< std::endl;

    for (auto& [key, addr] : eth["addresses"].items()) {
        uint32_t ip = ip_str_to_net_ip(addr["GroupIp"].get<std::string>());
        uint16_t port = htons(addr["GroupPort"].get<uint16_t>());
        m_config.multicast.emplace_back(key, ip, port);

        std::clog <<__func__<<","<< __LINE__<<",section[" <<key<<"],Group[" <<ip<<"],port["<<port<<"]"<<",["<<addr["GroupIp"].get<std::string>()<<"],["<<addr["GroupPort"].get<uint16_t>()<<"]"<< std::endl;
    }

    auto& worker = json_parser["worker"];
    m_config.cpu_id = worker["cpuid"].get<uint32_t>();

    auto& filter = json_parser["filter"];
    for (const auto& item : filter["underlying"]) {
        m_option_info.AddUnderlying(item);
    }
    m_config.filter_path = filter["filter_instrument_info"];
    m_option_info.Load(m_config.filter_path);

    std::clog <<__func__<<","<< __LINE__<<",read_cpu[" <<m_config.cpu_id<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;

}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}

bool QxSzMdEtfOptionExanic::InitExanic(const char *ifName){
    std::clog <<__func__<<","<< __LINE__<<",ifName["<< ifName<<"]" << std::endl;
    char exaName[46];
    int portNum;
    
    if (0 != exanic_find_port_by_interface_name(ifName, exaName, sizeof(exaName) - 1, &portNum))
    {
        std::cerr << "exanic find port error:" <<ifName <<",error:"<< strerror(errno)<< std::endl;
        return false;
    }
    std::clog <<__func__<<","<< __LINE__<<",exaName["<< exaName<<"]"<<",portNum["<< portNum<<"]" << std::endl;
    
    m_exanic = exanic_acquire_handle(exaName);

    if (!m_exanic)
    {
        std::cerr << "exanic nullptr" << std::endl;
        return false;
    }

    std::clog <<__func__<<","<< __LINE__<<",m_exanic["<< reinterpret_cast<int64_t>(m_exanic) <<"]" << std::endl;

    int port = 0;
    m_exanic_rx = exanic_acquire_rx_buffer(m_exanic, portNum, 0);
    if (!m_exanic_rx)
    {
        std::cerr << "exanic acquire rx buffer error: " << exanic_get_last_error() << std::endl;
        return false;
    }
    std::clog <<__func__<<","<< __LINE__<<",m_exanic_rx["<< reinterpret_cast<int64_t>(m_exanic_rx)<<"]" << std::endl;
    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);

    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;

    return true;
}

void QxSzMdEtfOptionExanic::ReleaseExanic(){
    exanic_release_rx_buffer(m_exanic_rx);
    exanic_release_handle(m_exanic);
}



void QxSzMdEtfOptionExanic::OnData(){

    char databuffer[2048];
    exanic_cycles32_t timestamp;
    auto sz = exanic_receive_frame(m_exanic_rx, databuffer, sizeof(databuffer), &timestamp);
    if (sz <= 0){
        return;
    }

    

    // static int32_t idx = 0;
    // static size_t quota_sz = g_quota_stub->GetSize();
    // size_t sz = 0;
    // if (idx >= quota_sz) {
    //     return;
    // }
    // //std::clog <<__func__<<","<< __LINE__<<",quota_sz[" <<quota_sz<<"],idx:" << idx << std::endl;
    // char * stubdata = g_quota_stub->GetNextQuota(idx, sz, 100);
    // uint64_t local_time_ns_start = get_nanoseconds();
    // idx ++;
    // if (stubdata == nullptr || sz == 0){
    //     return ;
    // }
    // inputdata = (uint8_t *)stubdata;   //TODO stub
    // len = (int)sz;

    uint64_t local_time_ns_start = get_nanoseconds();

    tcpip::ip_header *ip_ptr = (tcpip::ip_header *)(databuffer + sizeof(tcpip::ethhdr));
    uint16_t ip_header_data_len = TransU16(ip_ptr->ip_len);

    tcpip::udp_header *udp_ptr = (tcpip::udp_header *)(databuffer + sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header));
    uint16_t udp_header_data_len = TransU16(udp_ptr->dataLen);

    std::string ip_str = uint32_to_ip_safe(ip_ptr->destination_ip_address);
    uint16_t dstPort_local = ntohs(udp_ptr->dstPort);

    // std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local << std::endl;

    if (!belongTo(ip_ptr->destination_ip_address, udp_ptr->dstPort)){
        return;
    }

    size_t offset = sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header) + sizeof(tcpip::udp_header);

    //std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local <<",data_len:" <<udp_header_data_len<<",offset:" <<offset+4 << std::endl;

    uint8_t *data = (uint8_t *)(databuffer);

    data += 46;
    // if ( idx == 98){
    //     hexDumpToClog((char *)data, udp_header_data_len); 
    // }

    uint16_t msgNum = MyTransU16(*(uint16_t *)(data));
    data += sizeof(uint16_t);

    //std::clog <<__func__<<","<< __LINE__<<",msgNum[" <<msgNum<<"]" << std::endl;

    uint64_t local_time_ns_0 = 0;
    uint64_t local_time_ns_1 = 0;
    uint64_t local_time_ns_2 = 0;

    uint64_t local_time_ns_3 = 0;
    uint64_t local_time_ns_4 = 0;
    uint64_t local_time_ns_5 = 0;


    uint64_t local_time_ns_end = 0;

    local_time_ns_0 = get_nanoseconds();

    bool count_flag = false;

    for (int i = 0; i < msgNum; ++i){
        local_time_ns_0 = get_nanoseconds();

        SzseMsgHead *msgHead = (SzseMsgHead *)(data);
        uint32_t msgType = MyTransU32(msgHead->MsgType);
        uint32_t msgLen  = MyTransU32(msgHead->MsgLen);
        if (msgType != 300111) {
            continue;
        }

        SzseSnapShot *md = (SzseSnapShot *)data;
        uint16_t ChannelNo = MyTransU16(md->ChannelNo);
        if(ChannelNo/10 != 105){
            continue;
        }


        //std::clog <<__func__<<","<< __LINE__<<",msgType[" <<msgType<<"],msgLen[" <<msgLen<<"]"<< std::endl;

        //Quotation quotation = {};

        //20250813131334897
        std::string inst(md->SecurityID, 8);
        if (!(inst.size()>0 && m_option_info.Filter(inst))){
            return;
        }

        // if (md->MDStreamID == ""){

        // }


        // std::clog <<__func__<<","<< __LINE__<<",MDStreamID[" <<md->MDStreamID<<"],ChannelNo[" <<MyTransU16(md->ChannelNo)<<"]"<< std::endl;

        local_time_ns_1 = get_nanoseconds();

        m_fb_md->reset_entity();
        m_fb_md->set_local_timestamp(local_time_ns_start); //utc ns
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计

        int32_t origtime  = (MyTransU64(md->OrigTime))%1000000000;
        m_fb_md->set_instrument_id(inst.c_str());
        m_fb_md->set_update_sec(hhmmssToUtcSeconds2(origtime/1000));
        m_fb_md->set_update_msec(origtime/1000);

        m_fb_md->set_pre_close((MyTransU64(md->PrevClosePx))/10000.0);
        m_fb_md->set_volume((MyTransU64(md->TotalVolumeTrade))/100);
        m_fb_md->set_turn_over((MyTransU64(md->TotalValueTrade))/10000.0);

        // quotation.update_sec = hhmmssToUtcSeconds2(origtime/1000);
        // quotation.update_msec = origtime%1000;
        // quotation.instrument_id = inst;
        // quotation.pre_close = (MyTransU64(md->PrevClosePx))/10000.0;     //N13(4)
        // quotation.turn_over  = (MyTransU64(md->TotalValueTrade))/10000.0; //N18(4)
        // quotation.volume     = (MyTransU64(md->TotalVolumeTrade))/100;    //N15(2)

        int readLen = sizeof(SzseSnapShot);
        uint32_t md_no = MyTransU32(md->NoMDEntries);

        for (unsigned int i = 0; i < md_no; ++i){
            SzseMDEntry_300111 *entry = (SzseMDEntry_300111 *)(data + readLen);

            //std::clog <<__func__<<","<< __LINE__<<",MdEntryType[" <<MdEntryType<<"]"<< std::endl;

            if (entry->MdEntryType == MDENTRYTYPE_0){
                uint16_t level = MyTransU16(entry->MdPriceLevel);
                // quotation.bp[level-1] = (MyTransU64(entry->MdEntryPx))/1000000.0;   //N18(6)
                // quotation.bv[level-1] = (MyTransU64(entry->MdEntrySize))/100;       //N15(2)
                // quotation.level ++;

                if( level == 1){
                    m_fb_md->set_bid1_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_bid1_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 2){
                    m_fb_md->set_bid2_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_bid2_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 3){
                    m_fb_md->set_bid3_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_bid3_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 4){
                    m_fb_md->set_bid4_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_bid4_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 5){
                    m_fb_md->set_bid5_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_bid5_volume((MyTransU64(entry->MdEntrySize))/100);
                }else{
                    std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                        <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;                    
                }

            }
            else if (entry->MdEntryType == MDENTRYTYPE_1){
                uint16_t level = MyTransU16(entry->MdPriceLevel);
                // quotation.ap[level-1] = (MyTransU64(entry->MdEntryPx))/1000000.0;
                // quotation.av[level-1] = (MyTransU64(entry->MdEntrySize))/100;


                if( level == 1){
                    m_fb_md->set_ask1_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_ask1_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 2){
                    m_fb_md->set_ask2_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_ask2_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 3){
                    m_fb_md->set_ask3_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_ask3_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 4){
                    m_fb_md->set_ask4_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_ask4_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else if( level == 5){
                    m_fb_md->set_ask5_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                    m_fb_md->set_ask5_volume((MyTransU64(entry->MdEntrySize))/100);
                }
                else{
                    std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                        <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
                }

            }
            else if (entry->MdEntryType == MDENTRYTYPE_2){
                double last_price = (MyTransU64(entry->MdEntryPx))/1000000.0;
                m_fb_md->set_last_price(last_price);
                m_fb_md->set_close(last_price);
                // quotation.last_price = last_price;
                // quotation.close = last_price;
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;

            }
            else if (entry->MdEntryType == MDENTRYTYPE_4){
                // quotation.open = (MyTransU64(entry->MdEntryPx))/1000000.0;
                m_fb_md->set_open((MyTransU64(entry->MdEntryPx))/1000000.0);
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;

            }
            else if (entry->MdEntryType == MDENTRYTYPE_7){
                // quotation.high = (MyTransU64(entry->MdEntryPx))/1000000.0;
                m_fb_md->set_high_price((MyTransU64(entry->MdEntryPx))/1000000.0);

                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;

            }
            else if (entry->MdEntryType == MDENTRYTYPE_8){
                // quotation.low = (MyTransU64(entry->MdEntryPx))/1000000.0;
                m_fb_md->set_low_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;

            }
            else if (entry->MdEntryType == MDENTRYTYPE_x1){
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
            }
            else if (entry->MdEntryType == MDENTRYTYPE_x2){
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
            }
            else if (entry->MdEntryType == MDENTRYTYPE_xe){
                // quotation.upper = (MyTransU64(entry->MdEntryPx))/1000000.0;
                m_fb_md->set_upper_limit_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
            }
            else if (entry->MdEntryType == MDENTRYTYPE_xf){
                // quotation.lower = (MyTransU64(entry->MdEntryPx))/1000000.0;
                m_fb_md->set_down_limit_price((MyTransU64(entry->MdEntryPx))/1000000.0);
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
            }
            else if (entry->MdEntryType == MDENTRYTYPE_xg){
                // quotation.open_interest = (MyTransU64(entry->MdEntrySize))/100.0;
                m_fb_md->set_open_interest((MyTransU64(entry->MdEntrySize))/100.0);
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
            }
            else if (entry->MdEntryType == MDENTRYTYPE_xi){
                // std::clog <<__func__<<","<< __LINE__<<",MdEntryPx[" <<MyTransU64(entry->MdEntryPx)<<"],MdEntrySize[" <<MyTransU64(entry->MdEntrySize)
                // <<"],MdPriceLevel[" <<MyTransU16(entry->MdPriceLevel)<<"],NumberOfOrders[" <<MyTransU64(entry->NumberOfOrders)<<"]"<< std::endl;
            }
            else{
                std::clog <<__func__<<","<< __LINE__<<",unexpect MdEntryType[0x" <<std::hex<<entry->MdEntryType<<std::oct<<"]"<< std::endl;
            }


            local_time_ns_2 = get_nanoseconds();


            readLen += sizeof(SzseMDEntry_300111);
            readLen += (8 * MyTransU32(entry->NoOrders));

            count_flag = true;
        }

        //SampleSnapShotMd transMd = {};

        // std::clog <<__func__<<","<< __LINE__<<",OrigTime[" <<MyTransU64(md->OrigTime)<<"],ChannelNo[" << MyTransU16(md->ChannelNo)<<"],inst[" <<inst<<"]"<< std::endl;
        // std::clog <<__func__<<","<< __LINE__<<",origtime[" <<origtime<<"]" << std::endl;    
        // std::clog <<__func__<<","<< __LINE__<<",hhmmss[" <<hhmmss<<"]" << std::endl;           
        // std::clog <<__func__<<","<< __LINE__<<",update_sec[" <<quotation.update_sec<<"]" << std::endl;
        // std::clog <<__func__<<","<< __LINE__<<",update_msec[" <<quotation.update_msec<<"]" << std::endl;
        // std::clog <<__func__<<","<< __LINE__<<",SecurityIDSource[" <<md->SecurityIDSource<<"],TradingPhaseCode[" << md->TradingPhaseCode<<"]"<< std::endl;



        //std::clog <<__func__<<","<< __LINE__<<",NumTrades[" <<md->NumTrades<<"]" << std::endl;

        //std::clog <<__func__<<","<< __LINE__<<",last_price[" <<quotation.last_price<<"],turn_over[" <<quotation.turn_over<<"],volume[" <<quotation.volume<<"]"<< std::endl;


        //std::clog <<__func__<<","<< __LINE__<<",NoMDEntries[" <<md_no<<"],readLen[" <<readLen<<"]"<< std::endl;

        data += msgLen;
        data += (8 - msgLen % 8);


        local_time_ns_3 = get_nanoseconds();

        //std::clog <<__func__<<","<< __LINE__<<","<<origtime<<",[" <<quotation.to_string()<<"]"<< std::endl;

        if (count_flag){


            // g_fb_spi->on_msg(g_fb_md);

            local_time_ns_end = get_nanoseconds();

            static uint64_t delay_sum = 0;
            static uint64_t delay_sum_0 = 0; //  -0
            static uint64_t delay_sum_1 = 0; // 0-1
            static uint64_t delay_sum_2 = 0; // 1-2
            static uint64_t delay_sum_3 = 0; // 2-3
            static uint64_t delay_sum_4 = 0; // 3-end

            static uint64_t count_stats = 0;
            delay_sum += (local_time_ns_end - local_time_ns_start);
            delay_sum_0 += (local_time_ns_0 - local_time_ns_start);
            delay_sum_1 += (local_time_ns_1 - local_time_ns_0);
            delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
            delay_sum_3 += (local_time_ns_3 - local_time_ns_2);
            delay_sum_4 += (local_time_ns_end - local_time_ns_3);


            int cpu = sched_getcpu();

            static uint64_t period = 10000;

            count_stats ++;
            if (count_stats%period == 0){
                //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],count sum [%llu], per msg routine cost[%llu]\n",cpu,period, delay_sum/period);

                m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],filter[%llu],fill[%llu],fill_gap[%llu],send[%llu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, delay_sum_4/period);
                delay_sum = 0;        
                delay_sum_0 = 0;
                delay_sum_1 = 0;
                delay_sum_2 = 0;
                delay_sum_3 = 0;
                delay_sum_4 = 0;
            }
        }
    }
}

void QxSzMdEtfOptionExanic::Routine(){
    BindCPU(m_config.cpu_id);
    exanic_cycles32_t timestamp;
    std::clog <<__func__<<","<< __LINE__<<","<<g_running<<","<<m_exanic_initialized.load(std::memory_order_acquire) << std::endl;
    if (m_exanic_initialized.load(std::memory_order_acquire) && m_fb_initialized.load(std::memory_order_acquire)){
        while(g_running.load(std::memory_order_relaxed)) {
            OnData();
        }
    }
}

int QxSzMdEtfOptionExanic::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();
    InitExanic(m_config.ifname.c_str());   // TODO stub

    m_worker = new std::thread(&QxSzMdEtfOptionExanic::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_exanic_initialized.store(true,std::memory_order_release);
    m_fb_initialized.store(true,std::memory_order_release);
    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void QxSzMdEtfOptionExanic::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    ReleaseExanic();
}

void QxSzMdEtfOptionExanic::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void QxSzMdEtfOptionExanic::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}






