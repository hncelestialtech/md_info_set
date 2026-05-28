#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "common.h"





#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include "json.hpp"

using json = nlohmann::json;


struct Multicast {
    std::string group_ip;
    int group_port;
    std::string IfName;
};




class ConfigManager {

public:
    ConfigManager()=default;

    void Load(std::string json_file){
        std::ifstream file(json_file);
        if (!file.is_open()) {
            std::cerr << "无法打开配置文件: " << json_file << std::endl;
            return;
        }
        
        json j;
        file >> j;
        file.close();

        // 解析各个部分
        parseMulticast(j);
        parseWorkerInfo(j);
        parseFilterInfo(j);

    }




    const Multicast& getMulticast() const {
        return m_multicast;
    }
    
    std::set<std::string> & getInstrument(){
        return m_instruments;
    }
    
    int32_t getCpuId(int idx) const {
        if (idx>= m_cpuid.size()){
            return -1;
        }
        return m_cpuid[idx];
    }
    

private:



    // 解析认证信息
    void parseMulticast(const json& j) {
        if (j.contains("multicast")) {
            const auto& auth = j["multicast"];
            m_multicast.group_ip       = auth.value("group_ip", "");
            m_multicast.group_port     = auth.value("group_port", 0);
            m_multicast.IfName = auth.value("IfName", "");
        }
    }
    

 
    void parseWorkerInfo(const json& j) {
        if (j.contains("worker")) {
            const auto& worker = j["worker"];
            if (worker.contains("cpuid")) {
                m_cpuid.clear();
                for (const auto& cpu : worker["cpuid"]) {
                    m_cpuid.push_back(cpu.get<int>());
                }
            }
        }
    }
    
    void parseFilterInfo(const json& j) {
        if (j.contains("filter")) {
            const auto& filter = j["filter"];
            m_filter_instrument_info = filter.value("filter_instrument_info", "");

            std::unordered_map<std::string, std::string> underlying_inst_map;
            LoadFebaoInstrumentInfo(m_filter_instrument_info, underlying_inst_map);
            for (auto &itor : underlying_inst_map) {
                std::string underlying = itor.first;
                std::string inst = itor.second;
                m_instruments.emplace(underlying);
                m_instruments.emplace(inst);
            }
        }
    }






private:

    // 配置数据
    Multicast                            m_multicast;


    std::vector<int32_t>                 m_cpuid;
    uint32_t                             m_channel_num = 0;
    
    std::string                          m_filter_instrument_info;
    std::set<std::string>                m_instruments;

};







