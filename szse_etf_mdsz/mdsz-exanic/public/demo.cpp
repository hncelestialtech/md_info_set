#include "MdszRecv.h"
#include <signal.h>
#include <atomic>
#include <fstream>
#include <iostream>
#include "sstream"
#include "string.h"
#include <thread>

#define ARRLEN 150

std::string resPath;
std::ofstream marketOut;
std::ofstream indexOut;

struct PackData
{
    uint64_t LocalTimeStamp;
    uint16_t Len;
    uint8_t data[3000];
};

alignas(64) std::atomic<uint32_t> g_bStop {0};
PackData g_mdArray[ARRLEN];
alignas(64) uint32_t g_arrHeader {0};
alignas(64) uint32_t g_arrTailer {0};

inline auto MyTransU16(uint16_t a) -> uint16_t
{
    return ((a) >> 8) | ((a) << 8);
}
inline auto MyTransU32(uint32_t a) -> uint32_t
{
    return (a = (((a) >> 16) | ((a) << 16))), (((a) >> 8) & 0x00ff00ff) | (((a) << 8) & 0xff00ff00);
}

inline auto MyTransU64(uint64_t a) -> uint64_t
{
    return (a = (((a) >> 32) | ((a) << 32))), (a = ((((a) >> 16) & 0x0000ffff0000ffff) | (((a) << 16) & 0xffff0000ffff0000))), ((((a) >> 8) & 0x00ff00ff00ff00ff) | (((a) << 8) & 0xff00ff00ff00ff00));
}

static void on_exit(int sig)
{
    std::cout << "detect stop" << std::endl;
    g_bStop = 1;
}

static inline uint64_t GetLocalTime()
{
    struct timespec tp = {0};
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((uint64_t)tp.tv_sec) * 1000000000 + tp.tv_nsec;
}

void ParseRawMd_300111(uint8_t * data, uint8_t *transBuf){
    SzseSnapShot *md = (SzseSnapShot *)data;
    SampleSnapShotMd *transMd = (SampleSnapShotMd *)(transBuf);
    transMd->MsgType = MyTransU32(md->MsgType);
    transMd->MsgLen = MyTransU32(md->MsgLen);
    transMd->OrigTime = MyTransU64(md->OrigTime);
    transMd->ChannelNo = MyTransU16(md->ChannelNo);
    memcpy(transMd->MDStreamID, md->MDStreamID, 3);
    memcpy(transMd->SecurityID, md->SecurityID, 8);
    memcpy(transMd->TradingPhaseCode, md->TradingPhaseCode, 8);
    memcpy(transMd->SecurityIDSource, md->SecurityIDSource, 4);
    memcpy(transMd->TradingPhaseCode, md->TradingPhaseCode, 8);
    transMd->PrevClosePx = MyTransU64(md->PrevClosePx);
    transMd->NumTrades = MyTransU64(md->NumTrades);
    transMd->TotalValueTraded = MyTransU64(md->TotalValueTrade);
    transMd->TotalVolumeTraded = MyTransU64(md->TotalVolumeTrade);

    int readLen = sizeof(SzseSnapShot);
    for (unsigned int i = 0; i < MyTransU32(md->NoMDEntries); ++i)
    {
        SzseMDEntry_300111 *entry = (SzseMDEntry_300111 *)(data + readLen);
        if (entry->MdEntryType == 0x2030)
        {
            uint16_t level = MyTransU16(entry->MdPriceLevel) - 1;
            transMd->BidAskInfo[level].BidPrice = MyTransU64(entry->MdEntryPx);
            transMd->BidAskInfo[level].BidVolume = MyTransU64(entry->MdEntrySize);
        }
        else if (entry->MdEntryType == 0x2031)
        {
            uint16_t level = MyTransU16(entry->MdPriceLevel) - 1;
            transMd->BidAskInfo[level].AskPrice = MyTransU64(entry->MdEntryPx);
            transMd->BidAskInfo[level].AskVolume = MyTransU64(entry->MdEntrySize);
        }
        else if (entry->MdEntryType == 0x2032)
        {
            transMd->LastPx = MyTransU64(entry->MdEntryPx);
        }
        else if (entry->MdEntryType == 0x3378)
        {
            transMd->BidWeightPx = MyTransU64(entry->MdEntryPx);
            transMd->BidWeightSize = MyTransU64(entry->MdEntrySize);
        }
        else if (entry->MdEntryType == 0x3478)
        {
            transMd->AskWeightPx = MyTransU64(entry->MdEntryPx);
            transMd->AskWeightSize = MyTransU64(entry->MdEntrySize);
        }
        readLen += sizeof(SzseMDEntry_300111);
        readLen += (8 * MyTransU32(entry->NoOrders));
    }
}

void ParseRawMd_309011(uint8_t * data, uint8_t *transBuf)
{
    SzseSnapShot *md = (SzseSnapShot *)data;
    SampleIndexMd *transMd = (SampleIndexMd *)(transBuf);
    transMd->MsgType = MyTransU32(md->MsgType);
    transMd->MsgLen = MyTransU32(md->MsgLen);
    transMd->OrigTime = MyTransU64(md->OrigTime);
    transMd->ChannelNo = MyTransU16(md->ChannelNo);
    memcpy(transMd->MDStreamID, md->MDStreamID, 3);
    memcpy(transMd->TradingPhaseCode, md->TradingPhaseCode, 8);
    memcpy(transMd->SecurityIDSource, md->SecurityIDSource, 4);
    memcpy(transMd->TradingPhaseCode, md->TradingPhaseCode, 8);
    transMd->PrevClosePx = MyTransU64(md->PrevClosePx);
    transMd->NumTrades = MyTransU64(md->NumTrades);
    transMd->TotalValueTraded = MyTransU64(md->TotalValueTrade);
    transMd->TotalVolumeTraded = MyTransU64(md->TotalVolumeTrade);
    int readLen = sizeof(SzseSnapShot);
    for (unsigned int i = 0; i < MyTransU32(md->NoMDEntries); ++i)
    {
        SzseMDEntry_309011 *entry = (SzseMDEntry_309011 *)(data + readLen);
        if (entry->MdEntryType == 0x2033)
        {
            transMd->LastPx = MyTransU64(entry->MdEntryPx);
            break;
        }
        readLen += sizeof(SzseMDEntry_309011);
    }
}

static void *RecordThread(void *)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(6, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);

    uint8_t transBuf[500];

    while (!g_bStop)
    {
        if (g_arrTailer < g_arrHeader)
        {
            PackData &item = g_mdArray[g_arrTailer % ARRLEN];
            uint8_t *data = item.data + 46;
            uint16_t msgNum = MyTransU16(*(uint16_t *)(data));
            data += 2;

            std::stringstream ss;
            ss.setf(std::ios::fixed);
            ss.unsetf(std::ios::showpoint);
            for (int i = 0; i < msgNum; ++i){
                SzseMsgHead *msgHead = (SzseMsgHead *)(data);
                uint32_t msgType = MyTransU32(msgHead->MsgType);
                uint32_t msgLen = MyTransU32(msgHead->MsgLen);
                if (msgType == 300111)
                {
                    ParseRawMd_300111(data, transBuf);
                    auto *md = (SampleSnapShotMd *)transBuf;
                    ss << std::dec << md->MsgType << ","
                        << md->MsgLen << ","
                        << md->OrigTime << ","
                        << md->ChannelNo << ","
                        << md->MDStreamID << ","
                        << md->SecurityID << ","
                        << md->SecurityIDSource << ","
                        << md->TradingPhaseCode << ","
                        << md->PrevClosePx << ","
                        << md->LastPx << ","
                        << md->NumTrades << ","
                        << md->TotalVolumeTraded << ","
                        << md->TotalValueTraded << ","
                        << md->BidWeightPx << ","
                        << md->AskWeightPx << ","
                        << md->BidWeightSize << ","
                        << md->AskWeightSize << ","
                        << md->BidAskInfo[0].BidPrice << ","
                        << md->BidAskInfo[0].BidVolume << ","
                        << md->BidAskInfo[0].AskPrice << ","
                        << md->BidAskInfo[0].AskVolume;
                    marketOut << ss.str() << std::endl;
                    marketOut.flush();
                    ss.str("");
                }
                else if (msgType == 309011)
                {
                    ParseRawMd_309011(data, transBuf);
                    auto *md = (SampleIndexMd *)transBuf;
                    ss << std::dec << md->MsgType << ","
                        << md->MsgLen << ","
                        << md->OrigTime << ","
                        << md->ChannelNo << ","
                        << md->MDStreamID << ","
                        << md->SecurityID << ","
                        << md->SecurityIDSource << ","
                        << md->TradingPhaseCode << ","
                        << md->PrevClosePx << ","
                        << md->LastPx << ","
                        << md->NumTrades << ","
                        << md->TotalVolumeTraded << ","
                        << md->TotalValueTraded << ","
                        << md->TradingPhaseCode;
                    indexOut << ss.str() << std::endl;
                    indexOut.flush();
                    ss.str("");
                }
                data += msgLen;
                data += (8 - msgLen % 8);
            }
            
            ++g_arrTailer;
        }
    }
    return NULL;
}

void ReceiveData(uint8_t *data, int len)
{
    PackData &item = g_mdArray[g_arrHeader % ARRLEN];
    item.LocalTimeStamp = GetLocalTime();
    item.Len = len;
    memcpy(&item.data, data, len);
    ++g_arrHeader;
}

int main(int argc, char **argv)
{
    const char *configFile = argv[1];
    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);
    
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(5, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);

    MdszL1Receiver *rev;
    if (MdszReceiveCreate(&rev, configFile) < 0)
    {
        std::cout << "init error!" << std::endl;
        return 0;
    }
    
    std::string resPath = MdszGetOutputPath(rev);
    marketOut.open(resPath + "market.csv", std::ios_base::out);
    indexOut.open(resPath + "index.csv", std::ios_base::out);

    pthread_t Rthread;
    pthread_create(&Rthread, NULL, &RecordThread, NULL);

    while (!g_bStop)
    {
        MdszReceiveRun(rev, ReceiveData);
    }

    pthread_join(Rthread, NULL);
    std::cout << "finish" << std::endl;
    MdszReceiverClose(rev);

    return 0;
}