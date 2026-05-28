/* Basic RX example */
#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/util.h>
#include "htf_cffex.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <cfloat>
#include <chrono>
#include <sstream>
using namespace std::chrono;
using namespace std;

#define KEYMODE_REGISTER_ADDR    0x21c  //keymode
#define KEYARG_REGISTER_ADDR     0x220  //keyarg
#define FILE_INI "./cffex_lvl2_decode.ini"

typedef	unsigned char       uchar;
typedef	unsigned short int  ushort;
typedef	unsigned int        uint;
typedef unsigned long int   uint64;
typedef long int            int64;
typedef uint64_t instrument_hash_t;

exanic_rx_t *g_exanic_rx = NULL;
unsigned int g_running = 1;
FILE *kConv_log = NULL;

std::string getDateFileName()
{
    time_t now;
    // 获取当前时间戳
    now = time(0);
    struct tm *st;
    st = localtime(&now);
    std::stringstream ss;
    //仅对此电脑配置有效
    ss << "./data/";
    ss << (st->tm_year+1900) << std::setw(2) << std::setfill('0') << (st->tm_mon+1) << std::setw(2) << std::setfill('0') << (st->tm_mday);
    ss << "data.csv";
    return ss.str();
}

void *exanic_get_md_data(void *param) {
    exanic_rx_t *rx = (exanic_rx_t *) g_exanic_rx;
    if (!rx)
    {
        fprintf(stderr, "exanic_acquire_rx_buffer: %s\n", exanic_get_last_error());
        return 1;
    }
    exanic_cycles32_t timestamp;
    ssize_t sz = 0;
    char buf[2048];

    // int num = 1;
    // if (*(char*)&num == 1) 
    //     printf("Little-Endian\n");
    // else 
    //     printf("Big-Endian\n");
    
    while(g_running)
    {

        sz = exanic_receive_frame(rx, buf, sizeof(buf), &timestamp);

        // if(sz > 0){
        //   printf("sz = %d\n",sz);
        // }
        if(sz > 280)
        {
            auto DataLog = (CFFEXIncQuotaDataT*)buf;
            // printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X\n",(unsigned char)buf[40],(unsigned char)buf[41],(unsigned char)buf[42],(unsigned char)buf[43],(unsigned char)buf[44],(unsigned char)buf[45],(unsigned char)buf[46],(unsigned char)buf[47],(unsigned char)buf[48]);
            // printf("%02X %02X %02X %02X \n",(unsigned char)buf[52],(unsigned char)buf[51],(unsigned char)buf[50],(unsigned char)buf[49]);
            // printf("%02X %02X %02X  \n",(unsigned char)buf[53],(unsigned char)buf[54],(unsigned char)buf[55]);
            // fprintf(kConv_log, "%s,%s,%u,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%lf\n",
            //         DataLog->InstrumentID,DataLog->UpdateTime,DataLog->UpdateMillisec,
            //         DataLog->OpenPrice,DataLog->HighestPrice,DataLog->LowestPrice,DataLog->ClosePrice,DataLog->UpperlimitPrice,DataLog->LowerlimitPrice,DataLog->SettlementPrice,DataLog->CurrdeltaPrice,
            //         DataLog->LastPrice,DataLog->Volume,DataLog->Turnover,DataLog->OpenInterest,
            //         DataLog->BidPrice1,DataLog->BidVolume1,DataLog->AskPrice1,DataLog->AskVolume1,
            //         DataLog->BidPrice2,DataLog->BidVolume2,DataLog->AskPrice2,DataLog->AskVolume2,
            //         DataLog->BidPrice3,DataLog->BidVolume3,DataLog->AskPrice3,DataLog->AskVolume3,
            //         DataLog->BidPrice4,DataLog->BidVolume4,DataLog->AskPrice4,DataLog->AskVolume4,
            //         DataLog->BidPrice5,DataLog->BidVolume5,DataLog->AskPrice5,DataLog->AskVolume5,
            //         DataLog->PriceBand1,DataLog->PriceBand2);
            fprintf(kConv_log, "%s,%s,%u,%lf,%d,%lf,%lf,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%d,%lf,%lf\n",
                    DataLog->InstrumentID,DataLog->UpdateTime,DataLog->UpdateMillisec,
                    DataLog->LastPrice,DataLog->Volume,DataLog->Turnover,DataLog->OpenInterest,
                    DataLog->BidPrice1,DataLog->BidVolume1,DataLog->AskPrice1,DataLog->AskVolume1,
                    DataLog->BidPrice2,DataLog->BidVolume2,DataLog->AskPrice2,DataLog->AskVolume2,
                    DataLog->BidPrice3,DataLog->BidVolume3,DataLog->AskPrice3,DataLog->AskVolume3,
                    DataLog->BidPrice4,DataLog->BidVolume4,DataLog->AskPrice4,DataLog->AskVolume4,
                    DataLog->BidPrice5,DataLog->BidVolume5,DataLog->AskPrice5,DataLog->AskVolume5,
                    DataLog->HighestPrice,DataLog->LowestPrice
                    );
            fflush(kConv_log);
        }
    }
    
}

bool writeKEYMODE(exanic_t* exanic)
{
    volatile uint32_t *registers;
    if ((registers = exanic_get_devkit_registers(exanic)) == NULL) {
        fprintf(stderr, "[ERROR] %s: %s\n", __FUNCTION__, exanic_get_last_error());
        return false;
    }
    unsigned char codeData[118];
    int keyMode = -1;
    FILE* fp = fopen(FILE_INI, "rb");
    if (fp)
    {
        int retSize = fread(codeData, 1, sizeof(codeData), fp);
        if (retSize != 118)
        {
            fprintf(stderr, "Ini File is wrong\n");
            fclose(fp);
            return false;
        }
        switch (codeData[117])
        {
        case 0x83:
            /* code */
            keyMode = 0;
            break;
        case 0x31:
            /* code */
            keyMode = 1;
            break;
        case 0xeb:
            /* code */
            keyMode = 2;
            break;
        
        default:
            break;
        }

        registers[KEYMODE_REGISTER_ADDR] = keyMode;
        registers[KEYARG_REGISTER_ADDR] = codeData[8];
        fprintf(stdout,"mode:%d,arg:0x%02X\n",keyMode,codeData[8]);
        return true;
    }
    else
    {
        fprintf(stderr, "can't open cffex_lvl2_decode.ini\n");
        return false;
    }

}

/**signal handle function
 */
void exanic_signal_handle(int sig) {
  printf("\r\nReceived sig[%d]\r\n", sig);
  g_running = 0;
}

typedef void (Sigfunc)(int);
Sigfunc *exanic_signal_intr(int signo, Sigfunc *func) {
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
  
    #ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
    #endif

    int ret = sigaction(signo, &act, &oact);
    if ( ret< 0) {
        return (SIG_ERR);
    }

    return oact.sa_handler;
}

int exanic_exit_data_file() {
  if(kConv_log != NULL)
  {
    fclose(kConv_log);
    kConv_log = NULL;
  }

  return 0;
}

int main(void)
{
    char *device = "exanic0";
    int port = 0;
    std::string fileName = getDateFileName();
    kConv_log = fopen(fileName.c_str(),"wb+");
    // fprintf(kConv_log, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
    //     "InstrumentID", "UpdateTime", "UpdateMillisec", 
    //     "OpenPrice", "HighestPrice","LowestPrice","ClosePrice","UpperlimitPrice","LowerlimitPrice","SettlementPrice","CurrdeltaPrice",
    //     "LastPrice", "Volume", "Turnover","OpenInterest",
    //     "BidPrice1", "BidVolume1", "AskPrice1","AskVolume1",
    //     "BidPrice2", "BidVolume2", "AskPrice2","AskVolume2",
    //     "BidPrice3", "BidVolume3", "AskPrice3","AskVolume3",
    //     "BidPrice4", "BidVolume4", "AskPrice4","AskVolume4",
    //     "BidPrice5", "BidVolume5", "AskPrice5","AskVolume5"
    //     ,"PriceBand1", "PriceBand2"
    // );
     fprintf(kConv_log, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
        "InstrumentID", "UpdateTime", "UpdateMillisec", 
        "LastPrice", "Volume", "Turnover","OpenInterest",
        "BidPrice1", "BidVolume1", "AskPrice1","AskVolume1",
        "BidPrice2", "BidVolume2", "AskPrice2","AskVolume2",
        "BidPrice3", "BidVolume3", "AskPrice3","AskVolume3",
        "BidPrice4", "BidVolume4", "AskPrice4","AskVolume4",
        "BidPrice5", "BidVolume5", "AskPrice5","AskVolume5",
        "HighestPrice","LowestPrice"
    );

    //static struct bmddepth kConv;
    /* init signal*/
    exanic_signal_intr(SIGINT,  exanic_signal_handle);
    exanic_signal_intr(SIGTERM, exanic_signal_handle);
    exanic_signal_intr(SIGABRT, exanic_signal_handle);
    exanic_signal_intr(SIGSTOP, exanic_signal_handle);
    exanic_t *exanic = exanic_acquire_handle(device);
    if (!exanic)
    {
        fprintf(stderr, "exanic_acquire_handle: %s\n", exanic_get_last_error());
        return 1;
    }

    g_exanic_rx = exanic_acquire_rx_buffer(exanic, port, 0);
    if (!g_exanic_rx)
    {
        fprintf(stderr, "exanic_acquire_rx_buffer: %s\n", exanic_get_last_error());
        return 1;
    }

    if (writeKEYMODE(exanic)==false)
    {
        fprintf(stderr, "set key mode failed\n");
        return -1;        
    }
    fprintf(stdout, "set key mode success\n");

    /*start get dma data*/
    printf("Receiving mkdata...\n");
    thread md_rcv_thread(exanic_get_md_data, g_exanic_rx);
    
    printf("Input 'ctrl + c' to quit:\n");
    while (g_running) {
      sleep(1);
    }
    md_rcv_thread.join();

    exanic_release_rx_buffer(g_exanic_rx);
    exanic_release_handle(exanic);
    return 0;
}
