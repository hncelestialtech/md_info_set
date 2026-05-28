#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "common.h"

#include "NanoShfeMdApi.h"

#include "nano_shfe_md.h"

#include "nano_shfe_typedef.h"

class CNanoShfeMd::CNanoShfeMdImpl{
public:
    CNanoShfeMdImpl();
    ~CNanoShfeMdImpl();

    bool Init(std::string jsonfile);

    bool Run();
    void RegistMdCallback(NanoShfeMdHandler cb);

private:
    inline void LoadJsonCfg(std::string &jsonfile);
    void Routine(int index);
    inline void MsgRoutine(CNanoShfeMdApi& refNanoShfeMdApi);
    void inst_stats(std::string inst);

private:
    Config_t                        m_config;
    std::vector<std::thread*>       m_worker;

    //filter
    OptionInfoFilter                m_option_info;

    NanoShfeMdHandler m_md_cb;


};



