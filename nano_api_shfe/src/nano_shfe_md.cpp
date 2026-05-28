
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include "common.h"
#include "nano_shfe_md.h"
#include "nano_shfe_impl.h"

CNanoShfeMd::CNanoShfeMd():
    pImpl(std::make_unique<CNanoShfeMd::CNanoShfeMdImpl>())
{
}

CNanoShfeMd::~CNanoShfeMd(){
}

bool CNanoShfeMd::Init(const char * json_config_file){
    return pImpl->Init(json_config_file);
}

bool CNanoShfeMd::RegistMdCallback(NanoShfeMdHandler handler){
    pImpl->RegistMdCallback(handler);
    return true;
}

bool CNanoShfeMd::Run(){
    return pImpl->Run();
}

// std::string NanoShfeMd::ToString(){
//     std::string ret = this->inst_id;
//     ret = ret + "," + std::to_string(update_time);
//     ret = ret + "," + std::to_string(update_milli_sec);
//     ret = ret + "," + std::to_string(type);
//     ret = ret + "," + std::to_string(change_no);
//     ret = ret + "," + std::to_string(open);
//     ret = ret + "," + std::to_string(high);
//     ret = ret + "," + std::to_string(low);
//     ret = ret + "," + std::to_string(last_price);
//     ret = ret + "," + std::to_string(turn_over);
//     ret = ret + "," + std::to_string(volume);
//     ret = ret + "," + std::to_string(open_interest);
//     ret = ret + "," + std::to_string(ap[0]);
//     ret = ret + "," + std::to_string(ap[1]);
//     ret = ret + "," + std::to_string(ap[2]);
//     ret = ret + "," + std::to_string(ap[3]);
//     ret = ret + "," + std::to_string(ap[4]);
//     ret = ret + "," + std::to_string(bp[0]);
//     ret = ret + "," + std::to_string(bp[1]);
//     ret = ret + "," + std::to_string(bp[2]);
//     ret = ret + "," + std::to_string(bp[3]);
//     ret = ret + "," + std::to_string(bp[4]);
//     ret = ret + "," + std::to_string(av[0]);
//     ret = ret + "," + std::to_string(av[1]);
//     ret = ret + "," + std::to_string(av[2]);
//     ret = ret + "," + std::to_string(av[3]);
//     ret = ret + "," + std::to_string(av[4]);
//     ret = ret + "," + std::to_string(bv[0]);
//     ret = ret + "," + std::to_string(bv[1]);
//     ret = ret + "," + std::to_string(bv[2]);
//     ret = ret + "," + std::to_string(bv[3]);
//     ret = ret + "," + std::to_string(bv[4]);
//     ret = ret + "," + std::to_string(bid_volume);
//     ret = ret + "," + std::to_string(bid_amount);
//     ret = ret + "," + std::to_string(ask_volume);
//     ret = ret + "," + std::to_string(ask_amount);
//     ret = ret + "," + std::to_string(max_inst_no);
//     ret = ret + "," + std::to_string(last_tick_flag);
//     return ret;
// }


// void NanoShfeMd::Clear(){
//     this->inst_id[0] = '\n';
//     this->update_time = 0;
//     this->update_milli_sec = 0;
//     this->type = 0;
//     this->change_no = 0;
//     this->open = FLOAT64_NAN;
//     this->high = FLOAT64_NAN;
//     this->low = FLOAT64_NAN;
//     this->last_price = FLOAT64_NAN;
//     this->turn_over = FLOAT64_NAN;
//     this->volume = 0;
//     this->open_interest = 0;
//     this->ap[0] = FLOAT64_NAN;
//     this->ap[1] = FLOAT64_NAN;
//     this->ap[2] = FLOAT64_NAN;
//     this->ap[3] = FLOAT64_NAN;
//     this->ap[4] = FLOAT64_NAN;
//     this->bp[0] = FLOAT64_NAN;
//     this->bp[1] = FLOAT64_NAN;
//     this->bp[2] = FLOAT64_NAN;
//     this->bp[3] = FLOAT64_NAN;
//     this->bp[4] = FLOAT64_NAN;
//     this->av[0] = 0;
//     this->av[1] = 0;
//     this->av[2] = 0;
//     this->av[3] = 0;
//     this->av[4] = 0;
//     this->bv[0] = 0;
//     this->bv[1] = 0;
//     this->bv[2] = 0;
//     this->bv[3] = 0;
//     this->bv[4] = 0;
//     this->bid_volume = 0;
//     this->bid_amount = 0;
//     this->ask_volume = 0;
//     this->ask_amount = 0;
//     this->max_inst_no = 0;
//     this->last_tick_flag = false;
// }

