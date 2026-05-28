#include <unistd.h>
#include <string>

#include <atomic>

#include "nano_gfex_md.h"
#include "common.h"
#include "share_memery_sv.h"

std::atomic<bool> g_running;

std::shared_ptr<SharedMemoryManager>  g_shm_mgr;

void on_exit(int sig){
    std::clog <<__func__<<","<< __LINE__<<",g_running:"<<g_running.load(std::memory_order_release)<< std::endl;
    g_running.store(false, std::memory_order_release);
}

static size_t counter = 0;
void NanoGfexMdPrint(const NanoGfexMd* md){
    
    NanoGfexMd* md2 = const_cast<NanoGfexMd*>(md);
    counter ++;
    // std::string quota_str = md2->ToString();
    // LOG_TRACE(quota_str);
    if (counter%10000 == 0){
        printf("[%s][%d],[%zu][%s][%d]\n",__func__,__LINE__,counter,md->inst_id,md->extime);        
    }

    g_shm_mgr->appendData(DATA_TYPE_NANO_GFEX_MD, md, sizeof(NanoGfexMd));
}

int main(int argc, char** argv){

    if(argc!=3){
        printf("Usage: %s <filename> <cfgfile> <shm_size(unit GB)>\n", argv[0]);
        return 0;
    }

    std::string cfgfile = argv[1];
    int shm_sz = atoi(argv[2]);

    if (shm_sz < 3){
        shm_sz = 3;
    }

    LOG_TRACE(cfgfile);

    int32_t shm_key = 20250822;
    size_t  shm_size = 1024ULL*1024*1024 * shm_sz;  //3G   每条大小 253 +1 

    int32_t date = getCurrentDate();

    std::string dumpfile =  "./dump." + std::to_string(date);

    g_shm_mgr.reset(new SharedMemoryManager(shm_key, shm_size, -1, 0));

    //printf("[%s][%d],[%s]\n",__func__,__LINE__, cfgfile.c_str());
    CNanoGfexMd nano_md;
    nano_md.Init(cfgfile.c_str());
    nano_md.RegistMdCallback(NanoGfexMdPrint);
    nano_md.Run();

    int32_t lt = getCurrentTimes();

    std::clog <<__func__<<","<< __LINE__<<",g_running:"<<g_running<< std::endl;
    while (g_running.load(std::memory_order_release)){
        if (lt == 160000){
            std::clog <<__func__<<","<< __LINE__<<",lt:"<<lt<< std::endl;
            g_running.store(false, std::memory_order_release);
            break;
        }

        sleep(10);
    }

    return 0;
}



