#include <unistd.h>
#include <string>

#include "nano_gfex_md.h"

void NanoGfexMdPrint(const NanoGfexMd* md){
    printf("[%s][%hu][%llu][%d]\n",md->inst_id,md->contract_no,md->extime,md->md_level);
}

int main(int argc, char** argv){
    std::string cfgfile = argv[1];
    printf("[%s][%d],[%s]\n",__func__,__LINE__, cfgfile.c_str());
    CNanoGfexMd nano_md;
    nano_md.Init(cfgfile.c_str());
    nano_md.RegistMdCallback(NanoGfexMdPrint);
    nano_md.Run();
    
    sleep(100000);

    return 0;
}
