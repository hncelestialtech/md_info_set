#pragma once

#include <string>
#include <vector>

struct Config_t{
    std::string config_path;
    std::vector<int> cpu_id;
    std::string filter_path;
};

struct QuotaMerge_t{
    uint64_t time;
    uint64_t volume;
    QuotaMerge_t(uint64_t a, uint64_t b):time(a),volume(b){};
};
