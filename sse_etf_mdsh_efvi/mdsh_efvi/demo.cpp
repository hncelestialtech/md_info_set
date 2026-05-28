#include <signal.h>
#include <atomic>
#include <fstream>
#include <iostream>
#include "include/MdshReceiveEfvi.h"
#include "sstream"
#include "string.h"

#define ARRLEN 24

std::string resPath;
std::ofstream resFile;

struct PackData
{
    uint64_t LocalTimeStamp;
    CMdshMd Md;
};

alignas(64) std::atomic<uint32_t> g_bStop {0};
PackData g_mdArray[ARRLEN];
alignas(64) uint32_t g_arrHeader {0};
alignas(64) uint32_t g_arrTailer {0};


static void on_exit(int sig)
{
    g_bStop = 1;
}

static inline uint64_t GetLocalTime()
{
    struct timespec tp = {0};
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((uint64_t)tp.tv_sec) * 1000000000 + tp.tv_nsec;
}

static void *RecordThread(void *)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(6, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);
    
    while (!g_bStop)
    {
        if (g_arrTailer < g_arrHeader)
        {
            
            PackData &item = g_mdArray[g_arrTailer % ARRLEN];

            std::stringstream ss;
            ss.setf(std::ios::fixed);
            ss.unsetf(std::ios::showpoint);

            ss << item.LocalTimeStamp << ","
                << std::hex << int(item.Md.verAndExchange) << ","
                << std::dec << int(item.Md.length) << ","
                << std::hex << int(item.Md.tradeModeAndSecurityType) << ","
                << item.Md.mdStreamId[0] << item.Md.mdStreamId[1] << item.Md.mdStreamId[2]
                << item.Md.mdStreamId[3] << item.Md.mdStreamId[4] << ","
                << std::dec << item.Md.tradingPhaseCode[0] << item.Md.tradingPhaseCode[1] << item.Md.tradingPhaseCode[2]
                << item.Md.tradingPhaseCode[3] << item.Md.tradingPhaseCode[4] << item.Md.tradingPhaseCode[5]
                << item.Md.tradingPhaseCode[6] << item.Md.tradingPhaseCode[7] << ","
                << std::dec << int(item.Md.timestamp) << ","
                << item.Md.securityId[0] << item.Md.securityId[1] << item.Md.securityId[2] << item.Md.securityId[3]
                << item.Md.securityId[4] << item.Md.securityId[5] << item.Md.securityId[6] << item.Md.securityId[7] << ","
                << std::dec << long(item.Md.totalValue) << ","
                << std::dec << long(item.Md.totalVolume) << ","
                << item.Md.lastPrice << ","
                << item.Md.openInterest;
            
            if (item.Md.length == 60)
            {
                resFile << ss.str() << std::endl;
                resFile.flush();
                ++g_arrTailer;
                continue;
            }

            ss << "," << item.Md.bidPrice[0] << ","
                << item.Md.bidPrice[1] << ","
                << item.Md.bidPrice[2] << ","
                << item.Md.bidPrice[3] << ","
                << item.Md.bidPrice[4] << ","
                << item.Md.bidVolume[0] << ","
                << item.Md.bidVolume[1] << ","
                << item.Md.bidVolume[2] << ","
                << item.Md.bidVolume[3] << ","
                << item.Md.bidVolume[4] << ","
                << item.Md.askPrice[0] << ","
                << item.Md.askPrice[1] << ","
                << item.Md.askPrice[2] << ","
                << item.Md.askPrice[3] << ","
                << item.Md.askPrice[4] << ","
                << item.Md.askVolume[0] << ","
                << item.Md.askVolume[1] << ","
                << item.Md.askVolume[2] << ","
                << item.Md.askVolume[3] << ","
                << item.Md.askVolume[4];
            
            resFile << ss.str() << "\n";
            resFile.flush();
            ++g_arrTailer;
        }
    }
    return NULL;
}

void ReceiveData(uint8_t *data, int len)
{
    PackData &item = g_mdArray[g_arrHeader % ARRLEN];
    item.LocalTimeStamp = GetLocalTime();
    memcpy(&item.Md, data, len);
    ++g_arrHeader;
}

void beginRecord(void)
{
    resFile << "LocalTimeStamp"
            << ","
            << "verAndExchange"
            << ","
            << "length"
            << ","
            << "tradeModeAndSecurityType"
            << ","
            << "mdStreamId"
            << ","
            << "tradingPhaseCode"
            << ","
            << "timestamp"
            << ","
            << "securityId"
            << ","
            << "totalValue"
            << ","
            << "totalVolume"
            << ","
            << "lastPrice"
            << ","
            << "openInterest"
            << ","
            << "bidPrice[0]"
            << ","
            << "bidPrice[1]"
            << ","
            << "bidPrice[2]"
            << ","
            << "bidPrice[3]"
            << ","
            << "bidPrice[4]"
            << ","
            << "bidVolume[0]"
            << ","
            << "bidVolume[1]"
            << ","
            << "bidVolume[2]"
            << ","
            << "bidVolume[3]"
            << ","
            << "bidVolume[4]"
            << ","
            << "askPrice[0]"
            << ","
            << "askPrice[1]"
            << ","
            << "askPrice[2]"
            << ","
            << "askPrice[3]"
            << ","
            << "askPrice[4]"
            << ","
            << "askVolume[0]"
            << ","
            << "askVolume[1]"
            << ","
            << "askVolume[2]"
            << ","
            << "askVolume[3]"
            << ","
            << "askVolume[4]"
            << "\n";
    resFile.flush();
}

int main(int argc, char **argv)
{
    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);
    
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(5, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);

    if (argc < 2)
    {
        std::cout << "no ini file input" << std::endl;
        return 0;
    }

    const char *configFile = argv[1];
    CMdshReceiver *rev;
    if (MdshReceiverCreate(&rev, configFile) < 0)
    {
        std::cout << "init error!" << std::endl;
        return 0;
    }

    const char *resPath = MdshGetOutputPath(rev);
    resFile.open(resPath, std::ios_base::out);
    if (!resFile)
    {
        std::cout << "ResFile open failed!" << std::endl;
        return 0;
    }

    beginRecord();

    pthread_t thread;
    pthread_create(&thread, NULL, &RecordThread, NULL);

    while (!g_bStop)
    {
        MdshReceiverRun(rev, ReceiveData);
    }

    std::cout << "finish" << std::endl;
    MdshReceiverClose(rev);
    return 0;
}