
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include <signal.h>
#include "common.h"
#include "nano_shfe_impl.h"
#include "nano_shfe_typedef.h"
#include "nano_shfe_recevier.h"

std::atomic<bool> g_running = false;

CNanoShfeMd::CNanoShfeMdImpl::CNanoShfeMdImpl(){
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    return;
}




bool CNanoShfeMd::CNanoShfeMdImpl::Init(std::string jsonfile){
    LOG_TRACE("init begin");
    std::clog <<__CLASS__<<","<<__func__<<","<< __LINE__<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<< std::endl;
    LoadJsonCfg(jsonfile);
    g_running.store(true,std::memory_order_release);

    for (int i = 0; i < m_config.config_path.size(); i++){
        std::string cfgname = m_config.config_path[i];
        LOG_TRACE(cfgname);
        if (false == isFileExist(cfgname)){
            std::clog <<__func__<<","<< __LINE__<<",not found cfgname:"<<cfgname<< std::endl;        
            continue;
        }
        m_worker.emplace_back(new std::thread(&CNanoShfeMd::CNanoShfeMdImpl::Routine, this, i)); 
    }

    // std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    std::clog <<__CLASS__<<","<<__func__<<","<< __LINE__<< std::endl;
    LOG_TRACE("Init finished");
    return true;
}

bool CNanoShfeMd::CNanoShfeMdImpl::Run(){
    g_running.store(true,std::memory_order_release);
    std::clog <<__CLASS__<<","<<__func__<<","<< __LINE__<< std::endl;
    LOG_TRACE(" start");
    return true;
}

CNanoShfeMd::CNanoShfeMdImpl::~CNanoShfeMdImpl(){
    std::clog << std::nounitbuf;
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);

    for (int i = 0; i< m_worker.size(); ++i){
        if(m_worker[i]->joinable()){
            m_worker[i]->join();
        }
        delete m_worker[i];
        m_worker[i] = nullptr;
    }
    m_worker.clear();
}

void CNanoShfeMd::CNanoShfeMdImpl::RegistMdCallback(NanoShfeMdHandler cb){
    m_md_cb = cb;
}

inline void CNanoShfeMd::CNanoShfeMdImpl::LoadJsonCfg(std::string &jsoncfgfile){
    // std::clog <<__func__<<","<< __LINE__<< std::endl;
    std::ifstream jsonfile(jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["nano"];
    for (const auto& item : eth["config_path"]) {
        m_config.config_path.emplace_back(item);
        std::clog <<__func__<<","<< __LINE__<<",config_path[" <<item<<"]"<< std::endl;
    }

    auto& worker = json_parser["worker"];
    m_config.cpu_id = worker["cpuid"].get<std::vector<int>>();

    auto& filter = json_parser["filter"];
    for (const auto& item : filter["underlying"]) {
        m_option_info.AddUnderlying(item);
    }
    // m_config.filter_path = filter["filter_instrument_info"];
    // m_option_info.Load(m_config.filter_path);
    for (auto & id : m_config.cpu_id){
        LOG_TRACE(std::string(",CPU ID:") + std::to_string(id));
        std::clog << __func__ << "," << __LINE__ << ",cpu[" << id << "]" << std::endl; 
    }
    // LOG_TRACE(m_config.filter_path);
    // std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;

}

void on_exit(int sig){
    std::clog <<__func__<<","<< __LINE__<<",g_running:"<<g_running<< std::endl;
    g_running.store(false, std::memory_order_release);
}

void CNanoShfeMd::CNanoShfeMdImpl::Routine(int32_t index){
    pthread_setname_np(pthread_self(), "nano_shfe_md");
    BindCPU(m_config.cpu_id[index]);

    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);

    CNanoMdReceiver mdSpi(index);

    mdSpi.RegistMdCallback(m_md_cb);

    CNanoShfeMdApi& refNanoShfeMdApi = CNanoShfeMdApi::CreateNanoShfeMdApi();

    std::string cfgname = m_config.config_path[index];
    LOG_TRACE(cfgname);
    std::clog <<__func__<<","<< __LINE__<<",cfgname:"<<cfgname<< std::endl;

    int32_t r1 = refNanoShfeMdApi.NanoStart(mdSpi, cfgname.c_str());
    LOG_TRACE(std::string("refNanoShfeMdApi.NanoStart:") + std::to_string(r1));
    if (r1){
        std::clog <<__func__<<","<< __LINE__<<",failed"<< std::endl;
        return;
    }

    int32_t ret = 0;
    while(g_running.load(std::memory_order_relaxed) && (-1 != (ret = refNanoShfeMdApi.NanoRecv()))) {
        uint64_t local_time_ns = get_nanoseconds();
        // std::clog <<"NanoRecv ret:"<< ret<< std::endl;
        if (ret ==-1){
            g_running.store(false, std::memory_order_release);
            break;
        }
    }

    CNanoShfeMdApi::DestroyNanoShfeMdApi(refNanoShfeMdApi);
    LOG_TRACE(std::string("finished,index:")+std::to_string(index) + ",g_running:"+std::to_string(g_running) + ",ret:"+std::to_string(ret));
    std::clog <<__func__<<","<< __LINE__<<",finished,index:"<<index<<",g_running:"<<g_running <<",NanoRecv ret:"<<ret<< std::endl;
}


