#ifndef __DEMO_PROTOCOL_PARAM_H__
#define __DEMO_PROTOCOL_PARAM_H__

#include <set>
#include <string>

struct demo_protocol_param
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