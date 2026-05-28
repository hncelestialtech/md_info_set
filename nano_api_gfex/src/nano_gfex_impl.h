#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "common.h"

#include "NanoGfexMdApi.h"

#include "nano_gfex_md.h"

#include "nano_gfex_typedef.h"

class CNanoGfexMd::CNanoGfexMdImpl{
public:
    CNanoGfexMdImpl();
    ~CNanoGfexMdImpl();

    bool Init(std::string jsonfile);

    bool Run();
    void RegistMdCallback(NanoGfexMdHandler cb);

private:
    inline void LoadJsonCfg(std::string &jsonfile);
    void Routine(int index);
    inline bool belongTo(uint32_t ip, uint16_t port);
    inline void MsgRoutine(CNanoGfexMdApi& refNanoGfexMdApi);
    void inst_stats(std::string inst);

private:
    Config_t                        m_config;
    std::vector<std::thread*>       m_worker;

    //filter
    OptionInfoFilter                m_option_info;

    NanoGfexMdHandler m_md_cb;


};



