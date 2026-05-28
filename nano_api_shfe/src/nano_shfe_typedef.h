#pragma once

#include <string>
#include <vector>

struct Config_t{
    std::vector<std::string> config_path;
    std::vector<int>         cpu_id;
    std::string              filter_path;
};

