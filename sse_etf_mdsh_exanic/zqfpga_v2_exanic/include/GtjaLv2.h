// gtjalv2.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#if (defined WIN32) || (defined _WINDOWS_)
#include <WinSock2.h>
#include <Windows.h>
#include <netioapi.h>
#define posix_memalign(p, v, s) (*p = malloc(s))
#else
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#endif
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "SockProto.h"
#include "cfg.h"
using namespace tcpip;
using namespace std;
#define BUF_SIZE 2048
#define POLL_BATCH_SIZE 8

#define ROUND_UP(p, align) (((p) + (align)-1u) & ~((align)-1u))

#define RX_DMA_OFF 128 // ROUND_UP(sizeof(struct pkt_buf), EF_VI_DMA_ALIGN)

#define QUEUE_LEN 819200

int volatile g_StopProcess = 0;

/*pcap file header*/
typedef struct pcapFileHeader
{
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;  /*时区修正*/
    uint32_t sigfigs;  /*精确时间戳*/
    uint32_t snaplen;  /*抓包最大长度*/
    uint32_t linktype; /*链路类型*/
} pcapFileHeader_t;

/*pcap packet header*/
typedef struct pcapPkthdr
{
    uint32_t seconds;   /*秒数*/
    uint32_t m_seconds; /*毫秒数*/
    uint32_t caplen;    /*数据包长度*/
    uint32_t len;       /*文件数据包长度*/
} pcapPkthdr_t;
#if (__cplusplus >= 201103L)
enum link_type : uint32_t
{
#else
enum link_type
{
#endif
    _bsd = 0,          // BSD loopback devices, except for later OpenBSD
    _ethernet = 1,     // Ethernet, and Linux loopback devices   以太网类型，大多数的数据包为这种类型。
    _token_ring = 6,   // 802.5 Token Ring
    _arc_net = 7,      // ARCnet
    _slip = 8,         // SLIP
    _ppp = 9,          // PPP
    _fddi = 10,        // FDDI
    _atm_snap = 100,   // LLC/SNAP-encapsulated ATM
    _no_link_ip = 101, // raw IP, with no link
    _bsd_slip = 102,   // BSD/OS SLIP
    _bsd_ppp = 103,    // BSD/OS PPP
    _cisco_hdlc = 104, // Cisco HDLC
    _802_11 = 105,     // 802.11
    _openbsd_lo = 108, // later OpenBSD loopback devices (with the AF_value in network byte order)
    _linux_cook = 113, // special Linux cooked capture
    _local_talk = 114, // LocalTalk
};
struct EvtItem
{
    uint64_t TimeStamp;
    uint32_t DataLen;
    uint8_t Data[1516];
} g_Queue[QUEUE_LEN]; // 大约800MB内存

uint32_t volatile g_WriteIdx = 0;
uint32_t volatile g_ReadIdx = 0;

struct Conf
{
    string section;
    uint32_t ip;
    uint16_t port;
};

vector<Conf> confs;
void OnRcvData(const char *data, unsigned int dataLen, uint64_t timestamp);
void *ThreadProc(void *);
// addr1
void parse_shse_snapshot(EvtItem *pItem, ofstream &outFile, ofstream &oq_bid, ofstream &oq_ask);
void parse_shse_index(EvtItem *pItem, ofstream &outFile);
void parse_shse_trade(EvtItem *pItem, ofstream &outFile);
void parse_shse_order(EvtItem *pItem, ofstream &outFile);
void parse_shse_option(EvtItem *pItem, ofstream &outFile);
void parse_shse_bond_l2(EvtItem *pItem, ofstream &outFile);
void parse_shse_bond_zb(EvtItem *pItem, ofstream &outFile);
void parse_shse_merge_tick(EvtItem *pItem, ofstream &outFile);
// addr2
void parse_szse_snapshot(EvtItem *pItem, ofstream &outFile);
void parse_szse_order_queue(EvtItem *pItem, ofstream &outFile);
void parse_szse_order(EvtItem *pItem, ofstream &outFile);
void parse_szse_trade(EvtItem *pItem, ofstream &outFile);
// addr3
void parse_szse_option(EvtItem *pItem, ofstream &outFile);
void parse_szse_index(EvtItem *pItem, ofstream &outFile);
// addr4
void parse_bse_snapshot_l1(EvtItem *pItem, ofstream &outFile);
void parse_bse_snapshot_l2(EvtItem *pItem, ofstream &outFile);
void parse_bse_index(EvtItem *pItem, ofstream &outFile);

FILE *g_Write = NULL;
// const char *g_Usage = "usage: %s [cfgFile] [outFile]\n";
// uint16_t g_DstPort = 0;
// uint32_t g_DstIp = 0;