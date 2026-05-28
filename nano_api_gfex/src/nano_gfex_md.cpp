
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include "common.h"
#include "nano_gfex_md.h"
#include "nano_gfex_impl.h"

CNanoGfexMd::CNanoGfexMd():
    pImpl(std::make_unique<CNanoGfexMd::CNanoGfexMdImpl>())
{
}

CNanoGfexMd::~CNanoGfexMd(){
}

bool CNanoGfexMd::Init(const char * json_config_file){
    return pImpl->Init(json_config_file);
}

bool CNanoGfexMd::RegistMdCallback(NanoGfexMdHandler handler){
    pImpl->RegistMdCallback(handler);
    return true;
}

bool CNanoGfexMd::Run(){
    return pImpl->Run();
}
