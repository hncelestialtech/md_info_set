#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>  

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include <string.h>
#include <map>
#include <iostream>
#include <mutex>

#include <functional>
#include <atomic>
#include <thread>


#include "common.h"

struct UdpConfig{
    std::string interface_name;
    uint32_t    cpu_no;
    size_t      receive_buffer_size = 1024;

    std::string src_ip;
    uint16_t    src_port;
    uint32_t    net_src_ip;   
    uint16_t    net_src_port;

    std::string dst_ip;
    uint16_t    dst_port;
    uint32_t    net_dst_ip;   
    uint16_t    net_dst_port;

};


using UDPDataCallback = std::function<void(void *ctx, const char* data, size_t size)>;

class RawUdpReceiver{

public:

    RawUdpReceiver(UdpConfig &config)
    {
        m_running = false;

        m_socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (m_socket_fd < 0) {
            std::cerr << "socket error: " << strerror(errno) << std::endl;
            return;
        }

        int ifindex = if_nametoindex(m_config.interface_name.c_str());
        if (ifindex == 0) {
            std::cerr << "con not find interface:"<<m_config.interface_name.c_str() << std::endl;
            return;
        }

        // 设置地址重用，允许多个进程绑定到同一端口
        int reuse = 1;
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::clog <<__func__<<","<< __LINE__<<",Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
            return;
        }
        
        //设置端口重用（SO_REUSEPORT），允许多个进程绑定到同一端口
        #ifdef SO_REUSEPORT
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
            std::clog <<__func__<<","<< __LINE__<<",Failed to set SO_REUSEPORT: " << strerror(errno) << std::endl;
            return;
        }
        #endif

        struct sockaddr_ll addr;
        memset(&addr, 0, sizeof(addr));
        addr.sll_family = AF_PACKET;
        addr.sll_protocol = htons(ETH_P_ALL);
        addr.sll_ifindex = ifindex;
        
        if (bind(m_socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "bind error: " << strerror(errno) << std::endl;
            return;
        }
        
        struct packet_mreq mr;
        memset(&mr, 0, sizeof(mr));
        mr.mr_ifindex = ifindex;
        mr.mr_type = PACKET_MR_PROMISC;
        
        if (setsockopt(m_socket_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
            std::cerr << "PACKET_MR_PROMISC error: " << strerror(errno)<< " try sudo again" << std::endl;
        } else {
            std::cout << "PACKET_MR_PROMISC success" << std::endl;
        }
        
        struct ifreq ifr;
        strcpy(ifr.ifr_name, m_config.interface_name.c_str());
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
            std::cerr << "SO_BINDTODEVICE error:"<< strerror(errno) << std::endl;
        }
    }

    ~RawUdpReceiver() {
        stop();
        if (m_socket_fd >= 0) {
            close(m_socket_fd);
            m_socket_fd = -1;
        }
    }


    void registerCallback(void *ctx, UDPDataCallback callback){
        m_ctx = ctx;
        m_recv_callback = callback;
    }

    bool start(){
        if (m_running) {
            std::cerr << "Receiver is already running" << std::endl;
            return false;
        }
        
        // 设置非阻塞模式（用于超时控制）
        int flags = fcntl(m_socket_fd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(m_socket_fd, F_SETFL, flags | O_NONBLOCK);
        }
        
        // 启动接收线程
        m_running = true;
        m_receive_thread = std::thread(&RawUdpReceiver::receiveThread, this);
        
        std::clog <<__func__<<","<< __LINE__<< ",Multicast receiver started on:" << m_config.interface_name.c_str() << ", listening to " << m_config.interface_name.c_str()  << std::endl;
        
        return true;
    }

    void stop(){
        if (!m_running) {
            return;
        }
        
        m_running = false;

        // 等待接收线程结束
        if (m_receive_thread.joinable()) {
            m_receive_thread.join();
        }
        
        std::clog <<__func__<<","<< __LINE__<< "Multicast receiver stopped" << std::endl;
    }


    bool isRunning() const{
        return m_running;
    }

private:

    RawUdpReceiver(const RawUdpReceiver&) = delete;
    RawUdpReceiver& operator=(const RawUdpReceiver&) = delete;


    void receiveThread(){
        sleep(1);
        BindCPU(m_config.cpu_no);
        std::clog <<__func__<<","<< __LINE__<< ",m_running:" << m_running << ",m_receive_buffer_size:" << m_config.receive_buffer_size << std::endl;
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        char* buffer = new char[m_config.receive_buffer_size];
        while (m_running) {
            // 接收数据
            memset(&source_addr, 0, sizeof(source_addr));
            ssize_t recv_len = recv(m_socket_fd, buffer, m_config.receive_buffer_size, 0);

            // EAGAIN
            // EWOULDBLOCK
            if (recv_len <= 0) {
                if (!m_running) {
                    break;
                }
                continue;
            }

            if (m_recv_callback) {
                m_recv_callback(m_ctx, buffer, recv_len);
            }
        }
    }



    bool joinMulticastGroup(){
        struct ifreq ifr;
        strcpy(ifr.ifr_name, m_config.interface_name.c_str());
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
            std::cerr << "SO_BINDTODEVICE error:"<< strerror(errno) << std::endl;
        }
        
        return true;
    }



private:

    UdpConfig  m_config;

    int32_t        m_interface_index;
    uint32_t           m_socket_fd;        

    std::atomic<bool>  m_running;
    std::thread        m_receive_thread;  


    void               *m_ctx;       
    UDPDataCallback    m_recv_callback;
};





















