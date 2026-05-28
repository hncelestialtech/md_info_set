
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include "common.h"
#include "nano_gfex_impl.h"
#include "nano_gfex_typedef.h"

#include "nano_gfex_recevier.h"



std::atomic<bool> g_running = false;

CNanoGfexMd::CNanoGfexMdImpl::CNanoGfexMdImpl(){
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    return;
}


bool CNanoGfexMd::CNanoGfexMdImpl::Init(std::string jsonfile){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    LoadJsonCfg(jsonfile);

    for (int i = 0; i< 1; ++i){
        m_worker.emplace_back(new std::thread(&CNanoGfexMd::CNanoGfexMdImpl::Routine, this, i)); 
    }


    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    return true;
}

bool CNanoGfexMd::CNanoGfexMdImpl::Run(){
    g_running.store(true,std::memory_order_release);
    return true;
}

CNanoGfexMd::CNanoGfexMdImpl::~CNanoGfexMdImpl(){
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

void CNanoGfexMd::CNanoGfexMdImpl::RegistMdCallback(NanoGfexMdHandler cb){
    m_md_cb = cb;
}

inline void CNanoGfexMd::CNanoGfexMdImpl::LoadJsonCfg(std::string &jsoncfgfile){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    std::ifstream jsonfile(jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["nano"];
    m_config.config_path = eth["config_path"];

    std::clog <<__func__<<","<< __LINE__<<",config_path[" <<m_config.config_path<<"]"<< std::endl;

    auto& worker = json_parser["worker"];
    m_config.cpu_id = worker["cpuid"].get<std::vector<int>>();

    for (auto & id : m_config.cpu_id){
        std::clog << __func__ << "," << __LINE__ << ",cpu[" << id << "]" << std::endl; 
    }
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;

}

void CNanoGfexMd::CNanoGfexMdImpl::Routine(int32_t index){
    BindCPU(m_config.cpu_id[index]);

    CNanoMdReceiver mdSpi(index);

    mdSpi.RegistMdCallback(m_md_cb);

    CNanoGfexMdApi& refNanoGfexMdApi = CNanoGfexMdApi::CreateNanoGfexMdApi();

    std::string cfgname = m_config.config_path + "/config_" + std::to_string(index) + ".ini";
    std::clog <<__func__<<","<< __LINE__<<",cfgname:"<<cfgname<< std::endl;
    int32_t r1 = refNanoGfexMdApi.NanoStart(mdSpi, cfgname.c_str());
    if (r1){
        std::clog <<__func__<<","<< __LINE__<<",failed"<< std::endl;
        return;
    }
    //获取合约静态信息
    mdSpi.SaveInstStaticInfo(refNanoGfexMdApi);

    int32_t ret = 0;
    while(g_running.load(std::memory_order_relaxed) && (-1 != (ret = refNanoGfexMdApi.NanoRecv()))) {
        if (ret ==-1){
            break;
        }
    }

    CNanoGfexMdApi::DestroyNanoGfexMdApi(refNanoGfexMdApi);
    std::clog <<__func__<<","<< __LINE__<<",finished,index:"<<index << std::endl;
}


