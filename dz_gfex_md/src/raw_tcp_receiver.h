#pragma once

#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <iostream>
#include <functional>
#include <thread>
#include <atomic>
#include <cstring>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <linux/filter.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <iomanip>




#include "SockProto.h"



// 配置结构体
struct TcpConfig {
    std::string interface;      // 网卡名称，如 "ens8191"
    std::string     src_ip;         // 源IP地址，空字符串表示不过滤
    uint16_t        src_port;       // 源端口，0表示不过滤    
    uint32_t    net_src_ip;   
    uint16_t    net_src_port;

    std::string     dst_ip;         // 目标IP地址，空字符串表示不过滤
    uint16_t        dst_port;       // 目标端口，0表示不过滤
    uint32_t    net_dst_ip;
    uint16_t    net_dst_port;
};

using TcpCallback = std::function<void(void *ctx, const char* data, size_t size, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port)>;


class RawTcpReceiver {
public:
    RawTcpReceiver(const TcpConfig& config, int32_t cpu_no = -1) 
        : m_config(config), m_cpu_no(cpu_no), m_running(false), m_sock_fd(-1), m_ctx(nullptr) {
    }

    ~RawTcpReceiver() {
        stop();
    }

    void setConfig(const TcpConfig& config){
        m_config = config;
    }

    TcpConfig &getConfig(){
        return m_config;
    }


    void registerCallback(void *ctx, TcpCallback callback) {
        m_ctx = ctx;
        m_recv_callback = callback;
    }

    bool start() {
        if (m_running) {
            return false;
        }

        m_sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
        if (m_sock_fd < 0) {
            perror("socket creation failed");
            return false;
        }

        int val = 1;
        if (setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
            perror("setsockopt failed");
        }

        // 绑定到指定网卡
        struct sockaddr_ll saddr;
        memset(&saddr, 0, sizeof(saddr));
        saddr.sll_family = AF_PACKET;
        saddr.sll_protocol = htons(ETH_P_IP);
        
        struct ifreq ifr;
        strncpy(ifr.ifr_name, m_config.interface.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(m_sock_fd, SIOCGIFINDEX, &ifr) < 0) {
            perror("ioctl SIOCGIFINDEX failed");
            close(m_sock_fd);
            m_sock_fd = -1;
            return false;
        }
        
        saddr.sll_ifindex = ifr.ifr_ifindex;
        
        if (bind(m_sock_fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
            perror("bind failed");
            close(m_sock_fd);
            m_sock_fd = -1;
            return false;
        }

        struct packet_mreq mr;
        memset(&mr, 0, sizeof(mr));
        mr.mr_ifindex = ifr.ifr_ifindex;
        mr.mr_type = PACKET_MR_PROMISC;
        
        if (setsockopt(m_sock_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,  &mr, sizeof(mr)) < 0) {
            perror("setsockopt PACKET_ADD_MEMBERSHIP failed");
            // 非致命错误，继续运行但可能收不到非本机MAC的包
            std::clog << "Warning: Failed to set promiscuous mode, "<< "may not capture all packets" << std::endl;
        } else {
            std::clog << "Promiscuous mode enabled on "  << m_config.interface << std::endl;
        }

        // if (!isPromiscuousMode()) {
        //     std::clog << "Warning: " << m_config.interface << " is not in promiscuous mode!" << std::endl;
        // }
        // else{
        //     std::clog << "success: " << m_config.interface << " is in promiscuous mode!" << std::endl;
        // }

        m_running = true;
        
        // 启动接收线程
        m_recv_thread = std::thread(&RawTcpReceiver::recvLoop, this);
        
        return true;
    }


    bool isPromiscuousMode() {
        struct ifreq ifr;
        strncpy(ifr.ifr_name, m_config.interface.c_str(), IFNAMSIZ - 1);
        
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) return false;
        
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
            close(sock);
            return false;
        }
        
        close(sock);
        return (ifr.ifr_flags & IFF_PROMISC) != 0;
    }


    // 停止接收
    void stop() {
        if (!m_running) {
            return;
        }
        
        m_running = false;
        
        if (m_sock_fd >= 0) {
            close(m_sock_fd);
            m_sock_fd = -1;
        }
        
        if (m_recv_thread.joinable()) {
            m_recv_thread.join();
        }
    }

    // 是否正在运行
    bool isRunning() const {
        return m_running;
    }

private:


    // 设置BPF过滤器，在内核层面过滤TCP包
    bool setupBPFFilter() {
        struct sock_filter bpf_code[32];
        struct sock_fprog bpf_prog;
        int filter_index = 0;
        
        // 只捕获IPv4 TCP包
        // 检查以太网协议类型是否为IPv4 (0x0800)
        bpf_code[filter_index++] = { 0x28, 0, 0, 0x0000000c };  // ld [12] (以太网协议类型偏移12)
        bpf_code[filter_index++] = { 0x15, 0, 3, 0x00000800 };  // jeq #0x800, 跳过3条指令
        
        // 检查IP协议类型是否为TCP (6)
        bpf_code[filter_index++] = { 0x30, 0, 0, 0x00000017 };  // ld [23] (IP协议字段偏移23)
        bpf_code[filter_index++] = { 0x15, 0, 1, 0x00000006 };  // jeq #6, 跳过1条指令
        bpf_code[filter_index++] = { 0x06, 0, 0, 0x00000000 };  // ret #0 (丢弃)
        
        // 如果需要IP和端口过滤，继续添加
        if (!m_config.src_ip.empty() || !m_config.dst_ip.empty() || 
            m_config.src_port > 0 || m_config.dst_port > 0) {
            // IP和端口过滤比较复杂，这里简化处理，在用户态过滤
            // 如果确实需要内核过滤，可以添加更多的BPF指令
        }
        
        bpf_code[filter_index++] = { 0x06, 0, 0, 0x0000ffff };  // ret #0xffff (接受)
        
        bpf_prog.len = filter_index;
        bpf_prog.filter = bpf_code;
        
        if (setsockopt(m_sock_fd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf_prog, sizeof(bpf_prog)) < 0) {
            perror("setsockopt SO_ATTACH_FILTER failed");
            return false;
        }
        
        return true;
    }


    // 接收循环
    void recvLoop() {
        BindCPU(m_cpu_no);
        const int BUFFER_SIZE = 2048;
        char buffer[BUFFER_SIZE];

        while (m_running) {
            ssize_t recv_len = recv(m_sock_fd, buffer, BUFFER_SIZE, 0);
            if (unlikely(recv_len < 54)) {
                continue;
            }

            struct ethhdr* eth = reinterpret_cast<struct ethhdr*>(buffer);
            if (unlikely(ntohs(eth->h_proto) != ETH_P_IP)) {
                continue;
            }

            tcpip::ip_header *pIpHeader = (tcpip::ip_header *)(buffer + sizeof(ethhdr));
            if (unlikely(pIpHeader->ip_protocol != 6)) {   //只收TCP
                continue;
            }

            uint8_t &&ip_h_len = (pIpHeader->ip_version & 0x0f) << 2;
            uint16_t src_port =  ((tcpip::tcp_header *)(buffer + ip_h_len + sizeof(ethhdr)))->srcPort;
            uint16_t dst_port =  ((tcpip::tcp_header *)(buffer + ip_h_len + sizeof(ethhdr)))->dstPort;

            // uint16_t srcPort_local = ntohs(src_port);
            // uint16_t dstPort_local = ntohs(dst_port);

            // std::string src_ip_str = uint32_to_ip_safe(pIpHeader->source_ip_address);
            // std::string dst_ip_str = uint32_to_ip_safe(pIpHeader->destination_ip_address);

            uint16_t ip_total_len= ntohs(pIpHeader->ip_len);

            tcpip::tcp_header * tcp = (tcpip::tcp_header *)(buffer + sizeof(ethhdr) + sizeof(tcpip::ip_header));


            // hexDumpToClog((char *)&(tcp->header_len), 1);

            int32_t tcp_header_len = (tcp->header_len >>4) * 4;

            size_t offset = sizeof(ethhdr) + sizeof(tcpip::ip_header) + tcp_header_len;
            size_t content_len = ip_total_len - sizeof(tcpip::ip_header) - tcp_header_len;

            //40642 34068 53452 均自己与自己有重复报文，一模一样的发两条，包括半包，无法区分

            //  60796  59528 57376  40616 41016 34062 
            // if (dstPort_local == 53452){
            //     continue; 
            // }

            // if (dstPort_local != 51064){
            //     continue; 
            // }

            // std::clog<<"seqnum:"<<tcp->seqnum<<",acknum:"<<tcp->acknum<<",checksum:"<<ntohs(tcp->checksum)<<",src ip:"<<src_ip_str<<",srcPort_local:"<<srcPort_local<<",dst ip:"<<dst_ip_str<<",dstPort_local:"<<dstPort_local<<",ip_len:"<<ip_total_len<<",recv_len:"<<recv_len<<",offset:"<<offset<<",tcp_header_len:"<<tcp_header_len<<",content_len:"<<content_len<<std::endl;

            // if (m_config.net_src_ip != 0 && pIpHeader->source_ip_address != m_config.net_src_ip){
            //     continue; 
            // }

            // if (m_config.net_src_port != 0 && src_port != m_config.net_src_port){
            //     continue; 
            // }

            // if (m_config.net_dst_ip != 0 && pIpHeader->destination_ip_address != m_config.net_dst_ip){
            //     continue;
            // }

            // if (m_config.net_dst_port !=0 && dst_port != m_config.net_dst_port){
            //     continue; 
            // }

            // std::clog<<__func__<<","<< __LINE__<<",seqnum:"<<tcp->seqnum<<",acknum:"<<tcp->acknum<<",checksum:"<<ntohs(tcp->checksum)<<",src ip:"<<src_ip_str<<",srcPort_local:"<<srcPort_local<<",dst ip:"<<dst_ip_str<<",dstPort_local:"<<dstPort_local<<",ip_len:"<<ip_total_len<<",recv_len:"<<recv_len<<",offset:"<<offset<<",tcp_header_len:"<<tcp_header_len<<",content_len:"<<content_len<<std::endl;


            // std::clog<<__func__<<","<< __LINE__<<",recvLoop recv msg:"<<std::endl;
            // hexDumpToClog2(buffer + offset, content_len);

            // std::clog<<"src ip:"<<src_ip_str<<",srcPort_local:"<<srcPort_local<<",dst ip:"<<dst_ip_str<<",dstPort_local:"<<dstPort_local<<",ip_len:"<<ip_total_len<<",recv_len:"<<recv_len<<",offset:"<<offset<<",tcp_header_len:"<<tcp_header_len<<",content_len:"<<content_len<<std::endl;


            if (m_recv_callback && content_len>0) {
                m_recv_callback(m_ctx, buffer + offset, content_len,pIpHeader->source_ip_address, src_port, pIpHeader->destination_ip_address, dst_port);
            }
        }
    }


private:
    TcpConfig               m_config;
    int32_t                 m_cpu_no;
    std::atomic<bool>       m_running;
    int                     m_sock_fd;
    std::thread             m_recv_thread;
    void*                   m_ctx;
    TcpCallback             m_recv_callback;




};

