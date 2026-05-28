#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "common.h"


#include <optional>


#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include "json.hpp"

#include "XeleMd.h"


using json = nlohmann::json;


struct AuthInfo {
    std::string tcp_ipaddr;
    int tcp_port;
    std::string backup_tcp_ipaddr;
    int backup_tcp_port;
    std::string username;
    std::string password;
    std::string interface_name;
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
        parseAuthInfo(j);
        parseChannelInfo(j);
        parseWorkerInfo(j);
        parseFilterInfo(j);

    }

    const AuthInfo &getAuthInfo(){
        return m_auth_info;
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
    

    MdParam *getChannelParam(){
        return m_channel_param_vec.data();
    }


    uint32_t getChannelNum() const {
        return m_channel_num;
    }


private:

    template<size_t N>
    void safeStrCopy(char (&dest)[N], const std::string& src) {
        size_t copy_len = std::min(src.length(), N - 1);
        strncpy(dest, src.c_str(), copy_len);
        dest[copy_len] = '\0';
    }
    
    // 安全字符串拷贝函数（带默认值）
    template<size_t N>
    void safeStrCopy(char (&dest)[N], const std::string& src, const std::string& default_val) {
        if (src.empty()) {
            safeStrCopy(dest, default_val);
        } else {
            safeStrCopy(dest, src);
        }
    }

    ENicType parseNicType(int type) const {
        switch(type) {
            case 1: return E_NIC_SOLARFLARE_EFVI;
            case 2: return E_NIC_EXANIC;
            case 0:
            default: return E_NIC_NORMAL;
        }
    }

    void parseAuthInfo(const json& j) {
        if (j.contains("auth_info")) {
            const auto& auth = j["auth_info"];
            m_auth_info.tcp_ipaddr = auth.value("tcp_ipaddr", "");
            m_auth_info.tcp_port = auth.value("tcp_port", 0);
            m_auth_info.backup_tcp_ipaddr = auth.value("backup_tcp_ipaddr", "");
            m_auth_info.backup_tcp_port = auth.value("backup_tcp_port", 0);
            m_auth_info.username = auth.value("username", "");
            m_auth_info.password = auth.value("password", "");
            m_auth_info.interface_name = auth.value("interface_name", "");
        }
    }


    MdParam parseMdParam(const json& j) {
        MdParam param;
        safeStrCopy(param.m_interfaceName, j["interface_name"].get<std::string>());
        safeStrCopy(param.m_localIp, j["interface_ip"].get<std::string>());
        safeStrCopy(param.m_mcastIp, j["mcast_ip"].get<std::string>());
        param.m_mcastPort = static_cast<uint16_t>(j["mcast_port"].get<int>());
        param.m_bindCpuId = j["bind_cpu_id"].get<int>();
        param.m_nicType = parseNicType(j["nic_type"].get<int>());
        param.m_polling = j["is_polling"].get<bool>();
        param.m_cache = j["cache_size"].get<int>();
        param.m_cacheCpuId = j["cache_cpu_id"].get<int>();
        
        
        // 可选字段 - 逐笔构建快照相关
        if (j.contains("tick_to_snap")) {
            param.m_tickToSnap = j["tick_to_snap"].get<int>();
        }
        
        if (j.contains("tick_to_snap_cpu")) {
            param.m_tickSnapCpuId = j["tick_to_snap_cpu"].get<int>();
        }
        
        if (j.contains("tick_to_snap_subscribe")) {
            safeStrCopy(param.m_tickSnapSubscribe, j["tick_to_snap_subscribe"].get<std::string>());
        }
        
        if (j.contains("mark_last")) {
            int mark = j["mark_last"].get<int>();
            param.m_lastFlag = (mark != 0);
        }

        if (j.contains("back_interface_name")) {
            safeStrCopy(param.m_backupIntName, j["back_interface_name"].get<std::string>());
        }
        
        if (j.contains("back_interface_ip")) {
            safeStrCopy(param.m_backupLocalIp, j["back_interface_ip"].get<std::string>());
        }
        
        if (j.contains("back_mcast_ip")) {
            safeStrCopy(param.m_backupMcastIp, j["back_mcast_ip"].get<std::string>());
        }
        
        if (j.contains("back_mcast_port")) {
            param.m_backupMcastPort = static_cast<uint16_t>(j["back_mcast_port"].get<int>());
        }
        
        if (j.contains("back_cpu_id")) {
            param.m_backupCpuId = j["back_cpu_id"].get<int>();
        }
        
        if (j.contains("back_nic_type")) {
            param.m_backupNicType = parseNicType(j["back_nic_type"].get<int>());
        }
        
        return param;
    }


    void parseChannelInfo(const json& j) {
        if (j.contains("channel")) {
            const auto& channel = j["channel"];
            m_channel_num = channel.value("channel_num", 0);  
            m_channel_param_vec.clear();
            if (channel.contains("param") && channel["param"].is_array()) {
                for (const auto& param_json : channel["param"]) {
                    MdParam param = parseMdParam(param_json);
                    m_channel_param_vec.push_back(param);
                }
            }
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

    AuthInfo                             m_auth_info;

    uint32_t                             m_channel_num;    
    std::vector<MdParam>                 m_channel_param_vec;

    std::vector<int>                     m_cpuid;

	std::string                          m_filter_instrument_info;
    std::set<std::string>                m_instruments;

};







