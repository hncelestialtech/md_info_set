#ifndef __DEMO_COUNTERAPI_PARAM_H__
#define __DEMO_COUNTERAPI_PARAM_H__

#include <set>
#include <string>

struct demo_counterapi_param
{
    std::string flow_path;
    std::string addr;
    std::string brokerid;
    std::string userid;
    std::string passwd;
    std::string investorid;
    std::set<int> topics;
    int         cpu_no;
};

#endif // __DEMO_CONFIG_H__