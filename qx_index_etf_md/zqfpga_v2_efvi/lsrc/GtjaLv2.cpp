// gtjalv2.cpp: 定义应用程序的入口点。
//
#include "acc_md_sh.h"
#include "acc_md.h"
#include "tbj_gtjamcb.h"
#include "GtjaLv2.h"

#include <unordered_map>
#include <functional>
#include <cmath>
using namespace std;

const string ADDR1("addr1"), ADDR2("addr2"), ADDR3("addr3"), ADDR4("addr4");

bool belongTo(uint32_t ip, uint16_t port, vector<Conf> &confs)
{
    for (vector<Conf>::iterator iter = confs.begin(); iter != confs.end(); ++iter)
        if ((iter->ip == ip) && (iter->port == port))
            return true;
    return false;
}

bool belongTo(string section, vector<Conf> &confs)
{
    for (vector<Conf>::iterator iter = confs.begin(); iter != confs.end(); ++iter)
        if (iter->section == section)
            return true;
    return false;
}

int InitEfvi(EfviWrapper &efvi, const char *ifName, unsigned int rxBuffLen)
{
    // g_DstPort = port;
    // g_DstIp = inet_addr(ip);
    efvi.m_rxBuffLen = rxBuffLen;
    unsigned i;
    int re = 0;
    int ifindex = 0;
    int bytes;
    int vi_flags = EF_VI_FLAGS_DEFAULT | EF_VI_RX_TIMESTAMPS;

    posix_memalign((void **)&efvi.m_pkt_bufs, 8, sizeof(struct pkt_buf *) * (rxBuffLen));
    memset(efvi.m_pkt_bufs, 0, sizeof(struct pkt_buf *) * (rxBuffLen));

    ifindex = if_nametoindex(ifName);
    if (0 == ifindex)
    {
        fprintf(stderr, "can not find if with name %s,error: %d,%s\n", ifName, errno, strerror(errno));
        goto fail1;
    }

    re = ef_driver_open(&efvi.m_driver_handle);
    if (re)
    {
        fprintf(stderr, "ef_driver_open return error:%d,%s\n", errno, strerror(errno));
        goto fail1;
    }
    re = ef_pd_alloc(&efvi.m_pd, efvi.m_driver_handle, ifindex, EF_PD_MCAST_LOOP);
    if (re)
    {
        fprintf(stderr, "ef_pd_alloc for %s return error:%d,%s\n", ifName, errno, strerror(errno));
        goto fail2;
    }

    if ((re = ef_vi_alloc_from_pd(&efvi.m_vi, efvi.m_driver_handle, &efvi.m_pd, efvi.m_driver_handle, -1, -1, 0, NULL, -1,
                                  (enum ef_vi_flags)vi_flags)) < 0)
    {
        if (re == -EPERM)
        {
            fprintf(stderr, "Failed to allocate VI without event merging\n");
            vi_flags |= EF_VI_RX_EVENT_MERGE;
            if (ef_vi_alloc_from_pd(&efvi.m_vi, efvi.m_driver_handle, &efvi.m_pd, efvi.m_driver_handle, -1, -1, -1, NULL,
                                    -1, (enum ef_vi_flags)vi_flags))
            {
                fprintf(stderr, "ef_vi_alloc_from_pd fail,error:%d,%s\n", errno, strerror(errno));
                goto fail3;
            }
        }
        else
        {
            fprintf(stderr, "ef_vi_alloc_from_pd fail,error:%d,%s\n", errno, strerror(errno));
            goto fail3;
        }
    }
    // 组播地址Filter
    // ef_filter_spec fs;
    // ef_filter_spec_init(&fs, EF_FILTER_FLAG_NONE);
    // if ((re = ef_filter_spec_set_multicast_all(&fs)) < 0)
    //     cerr << "Set a Multicast All filter on the filter specification failed, error code: " << re << endl;
    // if ((re = ef_vi_filter_add(&efvi.m_vi, efvi.m_driver_handle, &fs, NULL)) < 0)
    //     cerr << "Add a filter to a virtual interface failed, error code: " << re << endl;

    // 根据 读取的 confs 添加 filter

    ef_filter_spec fs;
    for (vector<Conf>::iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        // cout << iter->ip << " " << iter->port << endl;
        ef_filter_spec_init(&fs, EF_FILTER_FLAG_NONE);
        ef_filter_spec_set_ip4_local(&fs, IPPROTO_UDP, iter->ip, iter->port);
        ef_vi_filter_add(&efvi.m_vi, efvi.m_driver_handle, &fs, NULL);
    }
    bytes = (rxBuffLen)*BUF_SIZE;
    posix_memalign(&efvi.m_pbuff, 4096, bytes);
    re = ef_memreg_alloc(&efvi.m_memreg, efvi.m_driver_handle, &efvi.m_pd, efvi.m_driver_handle, efvi.m_pbuff, bytes);
    if (re)
    {
        fprintf(stderr, "ef_memreg_alloc fail,error:%d,%s\n", errno, strerror(errno));
        goto fail4;
    }

    for (i = 0; i <= rxBuffLen; ++i)
    {
        efvi.m_pkt_bufs[i] = (struct pkt_buf *)((char *)efvi.m_pbuff + i * BUF_SIZE);
        efvi.m_pkt_bufs[i]->dma_buf_addr = ef_memreg_dma_addr(&efvi.m_memreg, i * BUF_SIZE);
        efvi.m_pkt_bufs[i]->id = i;
    }
    efvi.m_vi_recv_prefix = ef_vi_receive_prefix_len(&efvi.m_vi);
    return 0;
fail4:
    ef_vi_free(&efvi.m_vi, efvi.m_driver_handle);
fail3:
    ef_pd_free(&efvi.m_pd, efvi.m_driver_handle);
fail2:
    ef_driver_close(efvi.m_driver_handle);
fail1:
    free(efvi.m_pkt_bufs);
    free(efvi.m_pbuff);
    return errno;
}

int ReleaseEfvi(EfviWrapper &efvi)
{
    ef_memreg_free(&efvi.m_memreg, efvi.m_driver_handle);
    ef_vi_transmit_alt_free(&efvi.m_vi, efvi.m_driver_handle);
    ef_vi_free(&efvi.m_vi, efvi.m_driver_handle);
    ef_pd_free(&efvi.m_pd, efvi.m_driver_handle);
    ef_driver_close(efvi.m_driver_handle);
    free(efvi.m_pkt_bufs);
    free(efvi.m_pbuff);
    return 0;
}

void RcvLoop(EfviWrapper &efvi)
{
    printf("start recv thread...\n");
    // 绑到11号核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(11, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        fprintf(stderr, "RcvLoop::set thread affinity failed\n");
    }

    unsigned long maxBuf = ef_vi_receive_capacity(&efvi.m_vi);
    while (0 == g_StopProcess)
    {
        // 判断是否需要post
        if (efvi.m_postIndex - efvi.m_rcvIndex < maxBuf / 2)
        {
            unsigned long total = efvi.m_rxBuffLen - (efvi.m_postIndex - efvi.m_rcvIndex);
            if (total > (maxBuf - (efvi.m_postIndex - efvi.m_rcvIndex)))
            {
                total = ((maxBuf - (efvi.m_postIndex - efvi.m_rcvIndex)) >> 3) << 3;
            }

            if (total > 0 && total <= efvi.m_rxBuffLen)
            {
                for (unsigned i = 0; i < total; ++i)
                {
                    unsigned long id = (efvi.m_postIndex + i) % efvi.m_rxBuffLen;
                    ef_vi_receive_init(&efvi.m_vi, efvi.m_pkt_bufs[id]->dma_buf_addr + RX_DMA_OFF, id);
                }
                ef_vi_receive_push(&efvi.m_vi);
                efvi.m_postIndex += total;
            }
        }

        ef_event events[POLL_BATCH_SIZE];
        ef_request_id rx_ids[EF_VI_RECEIVE_BATCH];
        int n_ev = ef_eventq_poll(&efvi.m_vi, events, POLL_BATCH_SIZE);
        int n_reqs = 0;
        int i = 0, j = 0;
        unsigned long idx = 0;
        for (i = 0; i < n_ev; ++i)
        {
            // #ifdef DEBUG
            //             printf("evt%d: %d\n", i, EF_EVENT_TYPE(events[i]));
            // #endif
            switch (EF_EVENT_TYPE(events[i]))
            {
            case EF_EVENT_TYPE_RX_DISCARD:
            case EF_EVENT_TYPE_RX_NO_DESC_TRUNC:
                // fprintf(stderr,"RX_DISCARD,type=%d",EF_EVENT_RX_DISCARD_TYPE(events[i]));
            case EF_EVENT_TYPE_RX:
                idx = efvi.m_rcvIndex % efvi.m_rxBuffLen;
                if (idx == EF_EVENT_RX_RQ_ID(events[i]))
                {
                    struct timespec sw_ts;
                    clock_gettime(CLOCK_REALTIME, &sw_ts);
                    efvi.m_pkt_bufs[idx]->len = EF_EVENT_RX_BYTES(events[i]);
                    efvi.m_pkt_bufs[idx]->timestamp = sw_ts.tv_sec * 1000000000 + sw_ts.tv_nsec;
                    OnRcvData((const char *)efvi.m_pkt_bufs[idx] + RX_DMA_OFF + efvi.m_vi_recv_prefix,
                              efvi.m_pkt_bufs[idx]->len, efvi.m_pkt_bufs[idx]->timestamp);
                    ++efvi.m_rcvIndex;
                }
                else
                {
                    fprintf(stderr, "RX error,bufIndex not right,m_recvIndex:%lu,but recevied:%d", idx,
                            EF_EVENT_RX_RQ_ID(events[i]));
                }
                break;
            case EF_EVENT_TYPE_RX_MULTI_DISCARD:
            case EF_EVENT_TYPE_RX_MULTI:
                n_reqs = ef_vi_receive_unbundle(&efvi.m_vi, &events[i], rx_ids);
                for (j = 0; j < n_reqs; ++j)
                {
                    idx = efvi.m_rcvIndex % efvi.m_rxBuffLen;
                    if (idx == (unsigned)rx_ids[j])
                    {
                        struct timespec sw_ts;
                        clock_gettime(CLOCK_REALTIME, &sw_ts);
                        uint16_t len = 0;
                        if (0 ==
                            ef_vi_receive_get_bytes(&efvi.m_vi, (const char *)efvi.m_pkt_bufs[idx] + RX_DMA_OFF, &len))
                        {
                            efvi.m_pkt_bufs[idx]->len = len;
                            efvi.m_pkt_bufs[idx]->timestamp = sw_ts.tv_sec * 1000000000 + sw_ts.tv_nsec;
                            OnRcvData((const char *)efvi.m_pkt_bufs[idx] + RX_DMA_OFF + efvi.m_vi_recv_prefix,
                                      efvi.m_pkt_bufs[idx]->len, efvi.m_pkt_bufs[idx]->timestamp);
                        }
                        ++efvi.m_rcvIndex;
                    }
                    else
                    {
                        fprintf(stderr, "RX error,bufIndex not right,m_recvIndex:%lu,but recevied:%d",
                                efvi.m_rcvIndex % efvi.m_rxBuffLen, rx_ids[j]);
                    }
                }
                break;
            case EF_EVENT_TYPE_TX:
                break;
            case EF_EVENT_TYPE_RX_PACKED_STREAM:
                fprintf(stderr, "EF_EVENT_TYPE_RX_PACKED_STREAM,but not dealed\n");
                break;
            default:
                /* Other error types */
                ;
            }
        }
    }
}

void on_exit(int sig)
{
    g_StopProcess = true;
}

int main(int argc, char *argv[])
{
    g_Write = fopen("pcapout.pcap", "wb");
    pcapFileHeader_t pcapFileHeader = {0};
    pcapFileHeader.magic = 0xa1b2c3d4;
    pcapFileHeader.version_major = 0x02;
    pcapFileHeader.version_minor = 0x04;
    pcapFileHeader.thiszone = 0;
    pcapFileHeader.sigfigs = 0;
    pcapFileHeader.snaplen = 65535;
    pcapFileHeader.linktype = _ethernet;
    fwrite(&pcapFileHeader, 1, sizeof(pcapFileHeader), g_Write);

    CCfg cfg;
    if (cfg.Open("Gtjalv2.ini"))
    {
        cout << "open config file fail." << endl;
        return 0;
    }
    // 使用同一网卡
    string IfName;
    if (cfg.GetValue("eth", "IfName", IfName))
    {
        return -1;
    }
    else
    {
        cout << "IfName(single): " << IfName << endl;
    }
    if (argc == 1) // 不添加额外参数，则解析所有数据
    {
        string sections[] = {ADDR1, ADDR2, ADDR3, ADDR4};
        for (int i = 0; i != 4; ++i)
        {
            Conf conf;
            cout << "[" << sections[i] << "]" << endl;
            conf.section = sections[i];
            string ip;
            if (cfg.GetValue(sections[i], "GroupIp", ip))
            {
                return -1;
            }
            else
            {
                cout << "GroupIp: " << ip << endl;
                conf.ip = inet_addr(ip.c_str());
            }
            int32_t port = 0;
            if (cfg.GetValue(sections[i], "GroupPort", port))
            {
                return -1;
            }
            else
            {
                cout << "GroupPort: " << port << endl;
                conf.port = htons(port);
            }
            confs.push_back(conf);
        }
    }
    else
    {
        for (int i = 1; i <= argc - 1; ++i)
        {
            Conf conf;
            string section(argv[i]);
            cout << "[" << section << "]" << endl;
            conf.section = section;
            string ip;
            if (cfg.GetValue(section, "GroupIp", ip))
            {
                return -1;
            }
            else
            {
                cout << "GroupIp: " << ip << endl;
                conf.ip = inet_addr(ip.c_str());
            }
            int32_t port = 0;
            if (cfg.GetValue(section, "GroupPort", port))
            {
                return -1;
            }
            else
            {
                cout << "GroupPort: " << port << endl;
                conf.port = htons(port);
            }
            confs.push_back(conf);
        }
    }
    cfg.Close();

    EfviWrapper efvi;
    memset(&efvi, 0, sizeof(efvi));

    if (0 != InitEfvi(efvi, IfName.c_str(), 1024))
    {
        fprintf(stderr, "Init fail.\n");
        return 1;
    }

    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);

    pthread_t threadId;
    if (0 == pthread_create(&threadId, NULL, &ThreadProc, &confs))
        cout << "pthread_create::create 'ThreadProc' success" << endl;
    else
        cerr << "pthread_create::create 'ThreadProc' failed" << endl;

    RcvLoop(efvi);
    ReleaseEfvi(efvi);
    fclose(g_Write);
    printf("exit.\n");
    return 0;
}

void OnRcvData(const char *data, unsigned int dataLen, uint64_t timestamp)
{
    ip_header *pIpHeader = (ip_header *)(data + sizeof(struct ethhdr));
    uint8_t &&ip_h_len = (pIpHeader->ip_version & 0x0f) << 2;
    uint16_t &&dstPort = ntohs(((udp_header *)(data + ip_h_len + sizeof(struct ethhdr)))->dstPort);
    // if (!belongTo(pIpHeader->destination_ip_address, dstPort, confs))
    //     return;
    const char *aData;
    pcapFileHeader_t pcapFileHeader = {0};
    pcapPkthdr_t packetHeader = {0};
    pcapFileHeader.snaplen = dataLen + sizeof(pcapFileHeader_t);
    aData = (data + ip_h_len + sizeof(struct ethhdr) + sizeof(udp_header));                          // 数据包
    unsigned int &&aLen = ntohs(((udp_header *)(data + ip_h_len + sizeof(struct ethhdr)))->dataLen); // 数据包的长度
    packetHeader.caplen = dataLen;
    packetHeader.len = dataLen;
    packetHeader.m_seconds = timestamp % 1000000;
    packetHeader.seconds = (uint32_t)(timestamp / 1000000);
    if (/*aLen >= sizeof(guava_udp_normal) && */ aLen < sizeof(((EvtItem *)0)->Data))
    {
        // fwrite(&packetHeader, 1, sizeof(packetHeader), g_Write);
        // fwrite(data, 1, dataLen, g_Write);

        uint32_t &&idx = g_WriteIdx % QUEUE_LEN;
        g_Queue[idx].TimeStamp = timestamp;
        g_Queue[idx].DataLen = aLen - sizeof(udp_header);
        memcpy(g_Queue[idx].Data, aData, aLen - sizeof(udp_header));

        ++g_WriteIdx;
    }
    else
    {
        cout << "Wrong data, len:" << aLen << endl;
    }
}

// 若文件不存在，则打开文件并写入表头；若存在则仅打开文件
void open_file(ofstream &of, const char *fname, const char *header)
{
    if (access(fname, F_OK) == 0)
    {
        of.open(fname, ios::app);
    }
    else
    {
        of.open(fname, ios::app);
        of << header;
    }
}

void *ThreadProc(void *arg)
{
    vector<Conf> *pconfs = (vector<Conf> *)arg;
    unsigned int len_s = 0; // loggerBuf总长度
    unsigned int len_c = 0; // loggerBuf当前长度
    cpu_set_t mask;
    cpu_set_t get;
    double t1, t2;
    int count = 1;
    double totalDelay = 0;
    // 绑到12号核
    CPU_ZERO(&mask);
    CPU_SET(12, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        fprintf(stderr, "ThreadProc_shse::set thread affinity failed\n");
    }
    ofstream f_shse_snapshot, f_shse_index, f_shse_trade, f_shse_order, f_shse_orderq_bid, f_shse_orderq_ask, f_shse_option, f_shse_bond_l2, f_shse_bond_zb, f_shse_merge_tick;
    ofstream f_szse_snapshot, f_szse_index, f_szse_trade, f_szse_order, f_szse_orderq, f_szse_option;
    ofstream f_bse_snapshot_l1, f_bse_snapshot_l2, f_bse_index;
    if (belongTo(ADDR1, *pconfs))
    {
        string dir = "../output_" + ADDR1;
        open_file(f_shse_snapshot, (dir + "/shse_snapshot.csv").c_str(),
                  "产品代码,行情时间(时分秒),昨收盘价,开盘价,最高价,最低价,最新价,成交笔数,成交总量,成交总金额,"
                  "委托买入总量,委托卖出总量,加权平均委买价格,加权平均委卖价格,当前产品状态,涨停价,跌停价,"
                  "买一价,买二价,买三价,买四价,买五价,买六价,买七价,买八价,买九价,买十价,"
                  "买一量,买二量,买三量,买四量,买五量,买六量,买七量,买八量,买九量,买十量,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,卖六价,卖七价,卖八价,卖九价,卖十价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,卖六量,卖七量,卖八量,卖九量,卖十量\n");
        open_file(f_shse_orderq_bid, (dir + "/shse_order_bid_queue.csv").c_str(),
                  "产品代码,行情时间(时分秒),买一价,买一总委托笔数,买一揭示委托笔数\n");
        open_file(f_shse_orderq_ask, (dir + "/shse_order_ask_queue.csv").c_str(),
                  "产品代码,行情时间(时分秒),卖一价,卖一总委托笔数,卖一揭示委托笔数\n");
        open_file(f_shse_index, (dir + "/shse_index.csv").c_str(),
                  "产品代码,行情时间(时分秒),前收盘,开盘,成交金额,最高,最低,最新,成交时间(时分0.01秒),成交数量,收盘\n");
        open_file(f_shse_trade, (dir + "/shse_trade.csv").c_str(),
                  "时间戳,产品代码,成交通道,成交序号,成交时间(时分0.01秒),成交价格,成交数量,成交金额,买方订单号,卖方订单号,内/外盘,业务序列号\n");
        open_file(f_shse_order, (dir + "/shse_order.csv").c_str(),
                  "时间戳,产品代码,委托通道,委托序号,委托时间(时分0.01秒),订单类型,原始订单号,委托价格,剩余委托量,委托标识,业务序列号\n");
        open_file(f_shse_bond_l2, (dir + "/shse_bond_l2.csv").c_str(),
                  "产品代码,最新订单时间,昨收盘价,开盘价,最高价,最低价,最新价,成交笔数,成交总量,成交总金额,委托买入总量,委托卖出总量,"
                  "债券加权平均委买价格,债券加权平均委卖价格,当前产品状态,"
                  "买一价,买二价,买三价,买四价,买五价,买六价,买七价,买八价,买九价,买十价,"
                  "买一量,买二量,买三量,买四量,买五量,买六量,买七量,买八量,买九量,买十量,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,卖六价,卖七价,卖八价,卖九价,卖十价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,卖六量,卖七量,卖八量,卖九量,卖十量,"
                  "买一总委托笔数,买一揭示委托笔数,卖一总委托笔数,卖一揭示委托笔数\n");
        open_file(f_shse_bond_zb, (dir + "/shse_bond_zb.csv").c_str(),
                  "时间戳,产品代码,通道,逐笔序号,订单或成交时间,类型,买方订单,卖方订单,价格,数量,成交金额,标志\n");
        open_file(f_shse_merge_tick, (dir + "/shse_merge_tick.csv").c_str(),
                  "时间戳,产品代码,通道,逐笔序号,时间(时分0.01秒),订单类型,买方订单,卖方订单,价格,数量,已成交数量或成交金额,标志\n");
    }
    if (belongTo(ADDR2, *pconfs))
    {
        string dir = "../output_" + ADDR2;
        open_file(f_szse_snapshot, (dir + "/szse_snapshot.csv").c_str(),
                  "证券代码,时间戳(年月日时分秒毫秒),前收盘价,开盘价,最高价,最低价,最新价,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,卖六价,卖七价,卖八价,卖九价,卖十价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,卖六量,卖七量,卖八量,卖九量,卖十量,"
                  "买一价,买二价,买三价,买四价,买五价,买六价,买七价,买八价,买九价,买十价,"
                  "买一量,买二量,买三量,买四量,买五量,买六量,买七量,买八量,买九量,买十量,"
                  "成交总金额,成交笔数,成交总量,涨停价,跌停价,阶段码,行情类别,"
                  "加权平均委买价格,委托买入总量,加权平均委卖价格,委托卖出总量,"
                  "匹配成交最近价,匹配成交成交量,匹配成交成交金额,细分交易阶段个数,债券细分阶段码\n");
        open_file(f_szse_orderq, (dir + "/szse_order_queue.csv").c_str(),
                  "证券代码,时间戳(年月日时分秒毫秒),买卖方向,一档委托价格,订单数量,明细个数\n");
        open_file(f_szse_order, (dir + "/szse_order.csv").c_str(),
                  "时间戳,证券代码,消息记录号,委托价格,委托数量,时间戳(年月日时分秒毫秒),委托方向,委托类型,行情类别,通道号\n");
        open_file(f_szse_trade, (dir + "/szse_trade.csv").c_str(),
                  "时间戳,证券代码,消息记录号,买方委托索引,卖方委托索引,成交价格,成交数量,时间戳(年月日时分秒毫秒),成交类型,行情类别,通道号\n");
        open_file(f_szse_index, (dir + "/szse_index.csv").c_str(),
                  "证券代码,时间戳(年月日时分秒毫秒),开盘,最高,最低,最新,昨收,交易数量,成交金额\n");
    }
    if (belongTo(ADDR3, *pconfs))
    {
        string dir = "../output_" + ADDR3;
        open_file(f_szse_option, (dir + "/szse_option.csv").c_str(),
                  "证券代码,时间戳(年月日时分秒毫秒),前收盘价,开盘价,最高价,最低价,最新价,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,"
                  "买一价,买二价,买三价,买四价,买五价,"
                  "买一量,买二量,买三量,买四量,买五量,"
                  "成交总金额,成交总量,涨停价,跌停价,持仓数量,成交笔数,阶段码,行情类别\n");
        open_file(f_shse_option, (dir + "/shse_option.csv").c_str(),
                  "产品代码,时间戳,昨日结算价,今日结算价,今开盘价,最高价,最低价,现价,动态参考价格,虚拟匹配数量,当前合约未平仓数量,"
                  "买一量,买二量,买三量,买四量,买五量,"
                  "买一价,买二价,买三价,买四价,买五价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,"
                  "成交数量,成交金额,成交阶段代码\n");
    }
    if (belongTo(ADDR4, *pconfs))
    {
        string dir = "../output_" + ADDR4;
        open_file(f_bse_snapshot_l1, (dir + "/bse_snapshot_l1.csv").c_str(),
                  "证券代码,时间戳(年月日时分秒毫秒),状态,前收盘价,开盘价,最高价,最低价,最新价,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,"
                  "买一价,买二价,买三价,买四价,买五价,"
                  "买一量,买二量,买三量,买四量,买五量,"
                  "成交笔数,成交总量,成交总金额,涨停价,跌停价,"
                  "前IOPV,IOPV\n");
        open_file(f_bse_snapshot_l2, (dir + "/bse_snapshot_l2.csv").c_str(),
                  "证券代码,时间戳(时分秒毫秒),状态,前收盘价,开盘价,最高价,最低价,最新价,"
                  "卖一价,卖二价,卖三价,卖四价,卖五价,卖六价,卖七价,卖八价,卖九价,卖十价,"
                  "卖一量,卖二量,卖三量,卖四量,卖五量,卖六量,卖七量,卖八量,卖九量,卖十量,"
                  "买一价,买二价,买三价,买四价,买五价,买六价,买七价,买八价,买九价,买十价,"
                  "买一量,买二量,买三量,买四量,买五量,买六量,买七量,买八量,买九量,买十量,"
                  "成交笔数,成交总量,成交总金额,委托买入总量,委托卖出总量,"
                  "加权平均委买价格,加权平均委卖价格,IOPV净值估值,到期收益率,涨停价,跌停价,"
                  "证券信息前缀,市盈率1,市盈率2,升跌2（对比上一笔）,前IOPV\n");
        open_file(f_bse_index, (dir + "/bse_index.csv").c_str(),
                  "证券代码,行情时间(时分秒毫秒),今开盘指数,最高,最低,最新,参与计算相应指数的交易数量,参与计算相应指数的成交金额,前盘指数\n");
    }

    std::unordered_map<decltype(MD_TYPE_SH_L2), std::function<void(EvtItem * p, uint64_t)>> const parse_market_data_dict = {
        {MD_TYPE_SH_L2, [&](EvtItem *p, uint64_t)
         { parse_shse_snapshot(p, f_shse_snapshot, f_shse_orderq_bid, f_shse_orderq_ask); }},
        {MD_TYPE_SH_L2_ZK, [&](EvtItem *p, uint64_t)
         { parse_shse_index(p, f_shse_index); }},
        {MD_TYPE_SH_ZC, [&](EvtItem *p, uint64_t ts)
         { f_shse_trade << ts << ",";  parse_shse_trade(p, f_shse_trade); }},
        {MD_TYPE_SH_ZW, [&](EvtItem *p, uint64_t ts)
         { f_shse_order << ts << ",";  parse_shse_order(p, f_shse_order); }},
        {MD_TYPE_SH_L1_OPTION, [&](EvtItem *p, uint64_t)
         { parse_shse_option(p, f_shse_option); }},
        {MD_TYPE_SHZQ_L2, [&](EvtItem *p, uint64_t)
         { parse_shse_bond_l2(p, f_shse_bond_l2); }},
        {MD_TYPE_SHZQ_ZB, [&](EvtItem *p, uint64_t ts)
         { f_shse_bond_zb << ts << ",";  parse_shse_bond_zb(p, f_shse_bond_zb); }},
        {MD_TYPE_SHMERGE_ZB, [&](EvtItem *p, uint64_t ts)
         { f_shse_merge_tick<< ts << ",";  parse_shse_merge_tick(p, f_shse_merge_tick); }},
        {MD_TYPE_SZ_300111_L2, [&](EvtItem *p, uint64_t)
         { parse_szse_snapshot(p, f_szse_snapshot); }},
        {MD_TYPE_SZ_300192, [&](EvtItem *p, uint64_t ts)
         { f_szse_order << ts << ",";  parse_szse_order(p, f_szse_order); }},
        {MD_TYPE_SZ_300191, [&](EvtItem *p, uint64_t ts)
         { f_szse_trade << ts << ",";  parse_szse_trade(p, f_szse_trade); }},
        {MD_TYPE_SZ_ORDER_QUE, [&](EvtItem *p, uint64_t)
         { parse_szse_order_queue(p, f_szse_orderq); }},
        {MD_TYPE_SZ_300111_L1_OPTION, [&](EvtItem *p, uint64_t)
         { parse_szse_option(p, f_szse_option); }},
        {MD_TYPE_SZ_309011, [&](EvtItem *p, uint64_t)
         { parse_szse_index(p, f_szse_index); }},
        {MD_TYPE_BJ_L1, [&](EvtItem *p, uint64_t)
         { parse_bse_snapshot_l1(p, f_bse_snapshot_l1); }},
        {MD_TYPE_BJ_L2, [&](EvtItem *p, uint64_t)
         { parse_bse_snapshot_l2(p, f_bse_snapshot_l2); }},
        {MD_TYPE_BJ_INDEX, [&](EvtItem *p, uint64_t ts)
         { parse_bse_index(p, f_bse_index); }},
    };

    while (0 == g_StopProcess)
    {
        if ((int32_t)(g_WriteIdx - g_ReadIdx) > 0)
        {
            EvtItem *pItem = &g_Queue[g_ReadIdx % QUEUE_LEN]; // payload为pItem->data
            auto timestamp = pItem->TimeStamp;
            int msgtype = 0;
            memcpy(&msgtype, pItem->Data, sizeof(int));
#ifdef DEBUG
            cout << "msgtype: " << msgtype << " length: " << pItem->DataLen << endl;
#endif
            if (auto parse_func = parse_market_data_dict.find(msgtype); parse_func != parse_market_data_dict.end())
            {
                parse_func->second(pItem, timestamp);
            }
            // else
            // {
            //     std::cerr << "unknown type: " << msgtype << std::endl;
            // }
            ++g_ReadIdx;
        }
    }
    if (belongTo(ADDR1, *pconfs))
    {
        f_shse_snapshot.close();
        f_shse_orderq_ask.close();
        f_shse_orderq_bid.close();
        f_shse_index.close();
        f_shse_trade.close();
        f_shse_order.close();
        f_shse_merge_tick.close();
    }
    if (belongTo(ADDR2, *pconfs))
    {
        f_szse_snapshot.close();
        f_szse_orderq.close();
        f_szse_order.close();
        f_szse_trade.close();
    }
    if (belongTo(ADDR3, *pconfs))
    {
        f_szse_option.close();
        f_szse_index.close();
    }
    if (belongTo(ADDR4, *pconfs))
    {
        f_bse_snapshot_l1.close();
        f_bse_snapshot_l2.close();
        f_bse_index.close();
    }

    return nullptr;
}

template <size_t N>
std::string string_strip(const char (&arr)[N])
{
    int end_pos = N - 1;
    while (end_pos >= 0 && (arr[end_pos] == ' ' || arr[end_pos] == '\0'))
    {
        end_pos--;
    }

    if (end_pos < 0)
    {
        return "";
    }
    else
    {
        return std::string(arr, arr + end_pos + 1);
    }
}

void parse_shse_snapshot(EvtItem *pItem, ofstream &outFile, ofstream &oq_bid, ofstream &oq_ask)
{
    Lev2WholeData *snap = (Lev2WholeData *)pItem->Data;
    outFile << snap->security_id << ","
            << snap->time_stamp << ","
            << (double)snap->pre_close_price / 1000 << ","
            << (double)snap->open_price / 1000 << ","
            << (double)snap->high_price / 1000 << ","
            << (double)snap->low_price / 1000 << ","
            << (double)snap->match_price / 1000 << ","
            << snap->num_trade << ","
            << (double)snap->total_volume_trade / 1000 << ","
            << (double)snap->total_value_trade / 100000 << ","
            << (double)snap->total_bid_qty / 1000 << ","
            << (double)snap->total_offer_qty / 1000 << ","
            << (double)snap->weighted_avg_bid_price / 1000 << ","
            << (double)snap->weighted_avg_offer_price / 1000 << ","
            << snap->trading_phase_code << ","
            << (double)snap->high_limit_price / 10000 << ","
            << (double)snap->low_limit_price / 10000 << ",";
    for (const auto &price : snap->bid_price)
        outFile << (double)price / 1000 << ",";
    for (const auto &qty : snap->bid_qty)
        outFile << (double)qty / 1000 << ",";
    for (const auto &price : snap->offer_price)
        outFile << (double)price / 1000 << ",";
    for (const auto &qty : snap->offer_qty)
        outFile << (double)qty / 1000 << ",";
    outFile << endl;

    oq_bid << snap->security_id << ","
           << snap->time_stamp << ","
           << (double)snap->bid_price[0] / 1000 << ","
           << snap->bid_one_num_order << ","
           << snap->bid_one_pos << ",||"; // ||分隔，后面是揭示委买量
    for (int i = 0; i != snap->bid_one_pos; ++i)
        oq_bid << "," << (double)snap->bid_order[i] / 1000;
    oq_bid << endl;
    oq_ask << snap->security_id << ","
           << snap->time_stamp << ","
           << (double)snap->offer_price[0] / 1000 << ","
           << snap->offer_one_num_order << ","
           << snap->offer_one_pos << ",||"; // ||分隔，后面是揭示委卖量
    for (int i = 0; i != snap->offer_one_pos; ++i)
        oq_ask << "," << (double)snap->offer_order[i] / 1000;
    oq_ask << endl;
}

void parse_shse_index(EvtItem *pItem, ofstream &outFile)
{
    Lev2Index *index = (Lev2Index *)pItem->Data;
    outFile << index->trade_security_id << ","
            << index->index_data_time_stamp << ","
            << (double)index->index_pre_close / 100000 << ","
            << (double)index->index_open / 100000 << ","
            << (double)index->index_turnover / 10 << ","
            << (double)index->index_high / 100000 << ","
            << (double)index->index_low / 100000 << ","
            << (double)index->index_last / 100000 << ","
            << index->index_trade_time << ","
            << (double)index->index_total_volume / 100000 << ","
            << (double)index->index_close / 100000;
    outFile << endl;
}

void parse_shse_trade(EvtItem *pItem, ofstream &outFile)
{
    Lev2Trade *trade = (Lev2Trade *)pItem->Data;
    outFile << trade->trade_security_id << ","
            << trade->trade_channel << ","
            << trade->trade_index << ","
            << trade->trade_time << ","
            << (double)trade->trade_price / 1000 << ","
            << (double)trade->trade_qty / 1000 << ","
            << (double)trade->trade_money / 100000 << ","
            << trade->trade_buy_no << ","
            << trade->trade_sell_no << ","
            << trade->trade_bs_flag << ","
            << trade->trade_biz_index << endl;
}

void parse_shse_order(EvtItem *pItem, ofstream &outFile)
{
    Lev2Order *order = (Lev2Order *)pItem->Data;
    outFile << order->order_security_id << ","
            << order->order_channel << ","
            << order->order_index << ","
            << order->order_time << ","
            << order->order_type << ","
            << order->order_no << ","
            << (double)order->order_price / 1000 << ","
            << (double)order->order_balance / 1000 << ","
            << order->order_bs_flag << ","
            << order->order_biz_index << endl;
}

void parse_shse_option(EvtItem *pItem, ofstream &outFile)
{
    Lev1Option *option = (Lev1Option *)pItem->Data;
    outFile << string_strip(option->security_id) << ","
            << option->data_timestamp << ","
            << (double)option->pre_settle_price / 100000 << ","
            << option->today_settle_price << ","
            << (double)option->open_price / 100000 << ","
            << (double)option->high_price / 100000 << ","
            << (double)option->low_price / 100000 << ","
            << (double)option->last_price / 100000 << ","
            << (double)option->auction_price / 100000 << ","
            << (double)option->auction_qty << ","
            << option->total_long_position << ",";

    for (int i = 0; i < 5; i++)
        outFile << option->bid_qty[i] << ",";

    for (int i = 0; i < 5; i++)
        outFile << (double)option->bid_price[i] / 100000 << ",";

    for (int i = 0; i < 5; i++)
        outFile << option->offer_qty[i] << ",";

    for (int i = 0; i < 5; i++)
        outFile << (double)option->offer_price[i] / 100000 << ",";

    outFile << option->total_volume_trade << ","
            << option->total_value_trade << ","
            << std::string(option->trading_phase_code, 4) << endl;
}

void parse_shse_bond_l2(EvtItem *pItem, ofstream &outFile)
{
    Lev2BondWholeData *bond = (Lev2BondWholeData *)pItem->Data;
    outFile << string_strip(bond->security_id) << ","
            << bond->time_stamp << ","
            << (double)bond->pre_close_price / 1000 << ","
            << (double)bond->open_price / 1000 << ","
            << (double)bond->high_price / 1000 << ","
            << (double)bond->low_price / 1000 << ","
            << (double)bond->match_price / 1000 << ","
            << bond->num_trade << ","
            << (double)bond->total_volume_trade / 1000 << ","
            << (double)bond->total_value_trade / 100000 << ","
            << (double)bond->total_bid_qty / 1000 << ","
            << (double)bond->total_offer_qty / 1000 << ","
            << (double)bond->alt_weighted_avg_bid_price / 1000 << ","
            << (double)bond->alt_weighted_avg_offer_price / 1000 << ","
            << bond->trading_phase_code << ",";

    for (int i = 0; i < 10; i++)
        outFile << (double)bond->bid_price[i] / 1000 << ",";

    for (int i = 0; i < 10; i++)
        outFile << (double)bond->bid_qty[i] / 1000 << ",";

    for (int i = 0; i < 10; i++)
        outFile << (double)bond->offer_price[i] / 1000 << ",";

    for (int i = 0; i < 10; i++)
        outFile << (double)bond->offer_qty[i] / 1000 << ",";

    outFile << bond->bid_one_num_order << ","
            << bond->bid_one_pos << ","
            << bond->offer_one_num_order << ","
            << bond->offer_one_pos << ",||"; // ||分隔，后面是揭示委买量和揭示委卖量

    for (int i = 0; i < bond->bid_one_pos; i++)
        outFile << "," << (double)bond->bid_order[i] / 1000;

    for (int i = 0; i < bond->offer_one_pos; i++)
        outFile << "," << (double)bond->offer_order[i] / 1000;
    outFile << endl;
}

void parse_shse_bond_zb(EvtItem *pItem, ofstream &outFile)
{
    Lev2BondTick *bondtick = (Lev2BondTick *)pItem->Data;
    outFile << string_strip(bondtick->tick_security_id) << ","
            << bondtick->tick_channel << ","
            << bondtick->tick_index << ","
            << bondtick->tick_time << ","
            << bondtick->tick_type << ","
            << bondtick->tick_buy_no << ","
            << bondtick->tick_sell_no << ","
            << (double)bondtick->tick_price / 1000 << ","
            << (double)bondtick->tick_qty / 1000 << ","
            << (double)bondtick->tick_money / 100000 << ","
            << bondtick->tick_bs_flag << endl;
}

void parse_shse_merge_tick(EvtItem *pItem, ofstream &outFile)
{
    Lev2MergeTick *order = (Lev2MergeTick *)pItem->Data;
    outFile << order->tick_security_id << ","
            << order->tick_channel << ","
            << order->tick_biz_index << ","
            << order->tick_time << ","
            << order->tick_type << ","
            << order->tick_buy_no << ","
            << order->tick_sell_no << ","
            << (double)order->tick_price / 1000 << ","
            << (double)order->tick_qty / 1000 << ","
            << ((order->tick_type == 'T') ? (double)order->tick_money / 100000 : (double)order->tick_money / 1000) << ","
            << order->tick_bs_flag
            << endl;
}

void parse_szse_snapshot(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    MDSnapShotL2 *snapshot = (MDSnapShotL2 *)pappData->data;
    outFile << string_strip(snapshot->security_id) << ","
            << snapshot->md_time << ","
            << (double)snapshot->pre_close / 10000 << ","
            << (double)snapshot->open / 1000000 << ","
            << (double)snapshot->high / 1000000 << ","
            << (double)snapshot->low / 1000000 << ","
            << (double)snapshot->match / 1000000 << ",";
    for (int i = 0; i != 10; ++i)
        outFile << (double)snapshot->ask_price[i] / 1000000 << ",";
    for (int i = 0; i != 10; ++i)
        outFile << (double)snapshot->ask_vol[i] / 100 << ",";
    for (int i = 0; i != 10; ++i)
        outFile << (double)snapshot->bid_price[i] / 1000000 << ",";
    for (int i = 0; i != 10; ++i)
        outFile << (double)snapshot->bid_vol[i] / 100 << ",";
    outFile << (double)snapshot->total_value / 10000 << ","
            << snapshot->num_trades << ","
            << (double)snapshot->total_volume / 100 << ","
            << (double)snapshot->upper_limit / 1000000 << ","
            << (double)snapshot->lower_limit / 1000000 << ",";
    for (int i = 0; i != 8; ++i)
        outFile << snapshot->phase_code[i];
    outFile << ",";
    for (int i = 0; i != 3; ++i)
        outFile << snapshot->mdstream_id[i];
    outFile << "," << (double)snapshot->weighted_avg_bid_price / 1000000 << ","
            << (double)snapshot->total_bid_vol / 100 << ","
            << (double)snapshot->weighted_avg_ask_price / 1000000 << ","
            << (double)snapshot->total_ask_vol / 100 << ","
            << (double)snapshot->auction_match << ","
            << (double)snapshot->auction_volume_trade / 100 << ","
            << (double)snapshot->auction_value_trade / 10000 << ","
            << snapshot->num_sub_trading_phasecodes << ",||,"; // ||分隔，后面是债券细分阶段码
    for (int i = 0; i < snapshot->num_sub_trading_phasecodes; i++)
    {
        for (int j = 0; j < sizeof(snapshot->sub_pcs->phase_code); j++)
        {
            outFile << snapshot->sub_pcs[i].phase_code[j];
        }
        outFile << "," << (int)snapshot->sub_pcs[i].trading_type << ",";
    }
    outFile << endl;
}

void parse_szse_order_queue(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    OrderQueue *orderq = (OrderQueue *)pappData->data;
    outFile << string_strip(orderq->security_id) << ","
            << orderq->md_time << ","
            << (char)orderq->side << ","
            << (double)orderq->price / 1000000 << ","
            << orderq->num_orders << ","
            << orderq->num_items << ",||"; // ||分隔，后面是揭示委托量
    for (int i = 0; i != orderq->num_items; ++i)
        outFile << "," << (double)orderq->items[i] / 100;
    outFile << endl;
}

void parse_szse_index(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    StockIndex *index = (StockIndex *)pappData->data;
    outFile << string_strip(index->security_id) << ","
            << index->md_time << ","
            << (double)index->open_index / 1000000 << ","
            << (double)index->high_index / 1000000 << ","
            << (double)index->low_index / 1000000 << ","
            << (double)index->last_index / 1000000 << ","
            << (double)index->pre_close_index / 1000000 << ","
            << index->total_volume << ","
            << (double)index->total_value / 10000 << endl;
}

void parse_szse_order(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    StepOrder *order = (StepOrder *)pappData->data;
    outFile << string_strip(order->security_id) << ","
            << order->seq_num << ","
            << (double)order->price / 10000 << ","
            << (double)order->qty / 100 << ","
            << order->md_time << ","
            << order->side << ","
            << order->order_type << ",";
    for (int i = 0; i != 3; ++i)
        outFile << order->mdstream_id[i];
    outFile << "," << order->channel_no << endl;
}

void parse_szse_trade(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    StepTrade *trade = (StepTrade *)pappData->data;
    outFile << string_strip(trade->security_id) << ","
            << trade->seq_num << ","
            << trade->bid_seq_num << ","
            << trade->offer_seq_num << ","
            << (double)trade->price / 10000 << ","
            << (double)trade->qty / 100 << ","
            << trade->md_time << ","
            << trade->exec_type << ",";
    for (int i = 0; i != 3; ++i)
        outFile << trade->mdstream_id[i];
    outFile << "," << trade->channel_no << endl;
}

void parse_szse_option(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    OptionSnapShotL1 *option_l1 = (OptionSnapShotL1 *)pappData->data;
    outFile << string_strip(option_l1->security_id) << ","
            << option_l1->md_time << ","
            << (double)option_l1->pre_close / 10000 << ","
            << (double)option_l1->open / 1000000 << ","
            << (double)option_l1->high / 1000000 << ","
            << (double)option_l1->low / 1000000 << ","
            << (double)option_l1->match / 1000000 << ",";
    for (int i = 0; i != 5; ++i)
        outFile << (double)option_l1->ask_price[i] / 1000000 << ",";
    for (int i = 0; i != 5; ++i)
        outFile << (double)option_l1->ask_vol[i] / 100 << ",";
    for (int i = 0; i != 5; ++i)
        outFile << (double)option_l1->bid_price[i] / 1000000 << ",";
    for (int i = 0; i != 5; ++i)
        outFile << (double)option_l1->bid_vol[i] / 100 << ",";
    outFile << (double)option_l1->total_value / 10000 << ","
            << (double)option_l1->total_volume / 100 << ","
            << (double)option_l1->upper_limit / 1000000 << ","
            << (double)option_l1->lower_limit / 1000000 << ","
            << (double)option_l1->position_volume / 100 << ","
            << option_l1->num_trades << ",";
    for (int i = 0; i != 8; ++i)
        outFile << option_l1->phase_code[i];
    outFile << ",";
    for (int i = 0; i != 3; ++i)
        outFile << option_l1->mdstream_id[i];
    outFile << endl;
}

void parse_bse_snapshot_l1(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    auto *snapshot = (t_bj_mdL1_mcb *)pappData->data;
    outFile << string_strip(snapshot->sSecurityCode) << ","
            << snapshot->nTime << ","
            << snapshot->nStatus << ","
            << (double)snapshot->uPreClose / 10000 << ","
            << (double)snapshot->uOpen / 10000 << ","
            << (double)snapshot->uHigh / 10000 << ","
            << (double)snapshot->uLow / 10000 << ","
            << (double)snapshot->uMatch / 10000 << ",";
    for (const auto &price : snapshot->uAskPrice)
        outFile << (double)price / 10000 << ",";
    for (const auto &vol : snapshot->uAskVol)
        outFile << (double)vol << ",";
    for (const auto &price : snapshot->uBidPrice)
        outFile << (double)price / 10000 << ",";
    for (const auto &vol : snapshot->uBidVol)
        outFile << (double)vol << ",";
    outFile << snapshot->uNumTrades << ","
            << snapshot->iVolume << ","
            << (double)snapshot->iTurnover / 1000 << ","
            << (double)snapshot->uHighLimited / 10000 << ","
            << (double)snapshot->uLowLimited / 10000 << ","
            << (double)snapshot->nPreIOPV / 10000 << ","
            << (double)snapshot->nIOPV / 10000 << endl;
}

void parse_bse_snapshot_l2(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    auto *snapshot = (t_bj_mdL2_mcb *)pappData->data;
    outFile << string_strip(snapshot->sSecurityCode) << ","
            << snapshot->nTime << ","
            << snapshot->nStatus << ","
            << (double)snapshot->uPreClose / 10000 << ","
            << (double)snapshot->uOpen / 10000 << ","
            << (double)snapshot->uHigh / 10000 << ","
            << (double)snapshot->uLow / 10000 << ","
            << (double)snapshot->uMatch / 10000 << ",";
    for (const auto &price : snapshot->uAskPrice)
        outFile << (double)price / 10000 << ",";
    for (const auto &vol : snapshot->uAskVol)
        outFile << (double)vol << ",";
    for (const auto &price : snapshot->uBidPrice)
        outFile << (double)price / 10000 << ",";
    for (const auto &vol : snapshot->uBidVol)
        outFile << (double)vol << ",";
    outFile << snapshot->uNumTrades << ","
            << snapshot->iVolume << ","
            << (double)snapshot->iTurnover / 1000 << ","
            << snapshot->iTotalBidVol << ","
            << snapshot->iTotalAskVol << ","
            << (double)snapshot->uWeightedAvgBidPrice / 10000 << ","
            << (double)snapshot->uWeightedAvgAskPrice / 10000 << ","
            << (double)snapshot->nIOPV / 10000 << ","
            << (double)snapshot->nYieldToMaturity / 10000 << ","
            << (double)snapshot->uHighLimited / 10000 << ","
            << (double)snapshot->uLowLimited / 10000 << ","
            << string_strip(snapshot->sPrefix) << ","
            << (double)snapshot->nSyl1 / 10000 << ","
            << (double)snapshot->nSyl2 / 10000 << ","
            << (double)snapshot->nSD2 / 10000 << ","
            << (double)snapshot->nPreIOPV / 10000 << endl;
}

void parse_bse_index(EvtItem *pItem, ofstream &outFile)
{
    StaticAccMD *pappData = (StaticAccMD *)pItem->Data;
    auto *index = (bj_mdindex_mcb *)pappData->data;
    outFile << string_strip(index->sSecurityCode) << ","
            << index->nTime << ","
            << (double)index->nOpenIndex / 10000 << ","
            << (double)index->nHighIndex / 10000 << ","
            << (double)index->nLowIndex / 10000 << ","
            << (double)index->nLastIndex / 10000 << ","
            << index->iTotalVolume << ","
            << (double)index->iTurnover / 1000 << ","
            << (double)index->nPreCloseIndex / 10000 << endl;
}
