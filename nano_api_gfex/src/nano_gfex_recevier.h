#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "common.h"

#include "data_type_def.h"

#include "NanoGfexMdApi.h"

#include "nano_gfex_md.h"


#include "nano_gfex_typedef.h"


class CNanoMdReceiver : public CNanoGfexMdSpi
{
public:
    CNanoMdReceiver(int32_t instanceIndex);
    ~CNanoMdReceiver();

    void RegistSender();
    void RegistReceiver();

public:
    //一档行情回调接口
    virtual void OnNanoGfexL1Md(const NanoGfexL1MdType& refNanoGfexL1Md);

    //五档行情回调接口
    virtual void OnNanoGfexL2Md(const NanoGfexL2MdType& refNanoGfexL2Md);

public:

    void RegistMdCallback(NanoGfexMdHandler handler);

    void SaveInstStaticInfo(CNanoGfexMdApi& refNanoGfexMdApi);

    inline bool IsValidQuota(const std::string &inst, uint64_t extime, uint64_t vol);

private:

    NanoGfexMdHandler m_md_handler;

    struct md_interval_t{
        double open;
        double high;
        double low;
        void clear(){
            open = FLOAT64_NAN;
            high = FLOAT64_NAN;
            low = FLOAT64_NAN;
        };

        md_interval_t(double a, double b, double c):open(a),high(b),low(c){};
    };

    std::unordered_map<std::string, QuotaMerge_t>            m_quota_merge;
    std::unordered_map<std::string, std::set<std::string>>   m_febao_inst;
    std::unordered_map<std::string, md_interval_t>           m_md_interval;
    std::unordered_map<std::string, NanoGfexInstStaticInfo>  m_static_info;
    int32_t                                                  m_index = 0;  //对应通道
};



