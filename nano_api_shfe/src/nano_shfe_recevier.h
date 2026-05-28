#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "common.h"

#include "data_type_def.h"

#include "NanoShfeMdApi.h"

#include "nano_shfe_md.h"


#include "nano_shfe_typedef.h"


class CNanoMdReceiver : public CNanoShfeMdSpi
{
public:
    CNanoMdReceiver(int32_t instanceIndex);
    ~CNanoMdReceiver();

public:
    virtual void OnNanoShfeMd(const NanoShfeMdType& refSNanoShfeMd);
public:
    void RegistMdCallback(NanoShfeMdHandler handler);

private:

    NanoShfeMdHandler m_md_handler;

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

    std::unordered_map<std::string, md_interval_t>           m_md_interval;
    int32_t                                                  m_index = 0;  //对应通道
};



