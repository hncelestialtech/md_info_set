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

#include <sys/ioctl.h>  
#include <sys/socket.h> 

#include <string.h>
#include <map>
#include <iostream>
#include <mutex>

#include <functional>
#include <atomic>
#include <thread>

#include "common.h"

using UDPDataCallback = std::function<void(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port, int64_t t0)>;

class MulticastReceiver{
public:
    MulticastReceiver(std::string multicast_ip, uint16_t multicast_port, std::string interface_name, int32_t cpu_no = -1, size_t receive_buffer_size = 8196)
    {
        m_multicast_ip   = multicast_ip;
        m_multicast_port = multicast_port;
        m_interface_name = interface_name;
        m_cpu_no         = cpu_no;
        m_receive_buffer_size = receive_buffer_size;

        m_running = false;
        memset(&m_multicast_addr, 0, sizeof(m_multicast_addr));
        m_multicast_addr.sin_family = AF_INET;
        m_multicast_addr.sin_port = htons(m_multicast_port);

        if (inet_pton(AF_INET, m_multicast_ip.c_str(), &m_multicast_addr.sin_addr) <= 0) {
            std::clog <<__func__<<","<< __LINE__<< ",error: interface_name:" << m_interface_name.c_str() << ",multicast_ip:" << m_multicast_ip.c_str() << ",multicast_port:" << m_multicast_port << std::endl;
            return;
        }

        // 验证是否是组播地址（224.0.0.0 - 239.255.255.255）
        uint32_t addr = ntohl(m_multicast_addr.sin_addr.s_addr);
        if ((addr < 0xE0000000) || (addr > 0xEFFFFFFF)) {
            throw std::runtime_error("IP address is not a multicast address: " + m_multicast_ip);
        }
    }

    ~MulticastReceiver() {
        stop();
        if (m_socket_fd >= 0) {
            leaveMulticastGroup();
            close(m_socket_fd);
            m_socket_fd = -1;
        }
    }

    void registerCallback(void *ctx, UDPDataCallback callback){
        m_ctx = ctx;
        m_recv_callback = callback;
        std::clog <<__func__<<","<< __LINE__ << " success" << std::endl;
    }

    bool start(){
        if (m_running) {
            std::clog <<__func__<<","<< __LINE__ << ",Receiver is already running" << std::endl;
            return false;
        }
        
        // 初始化套接字
        if (!initializeSocket()) {
            return false;
        }
        
        // 加入组播组
        if (!joinMulticastGroup()) {
            close(m_socket_fd);
            m_socket_fd = -1;
            return false;
        }
        
        // 设置非阻塞模式（用于超时控制）
        int flags = fcntl(m_socket_fd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(m_socket_fd, F_SETFL, flags | O_NONBLOCK);
        }
        
        // 启动接收线程
        m_running = true;
        m_receive_thread = std::thread(&MulticastReceiver::receiveThread, this);
        
        std::clog <<__func__<<","<< __LINE__<< ",Multicast receiver started on:" << m_interface_name.c_str() << ", listening to " << m_multicast_ip.c_str() << ":" << m_multicast_port << std::endl;
        
        return true;
    }

    void stop(){
        std::clog <<__func__<<","<< __LINE__ << " success" << std::endl;
        if (!m_running) {
            return;
        }
        
        m_running = false;
        if (m_receive_thread.joinable()) {
            m_receive_thread.join();
        }

        std::clog <<__func__<<","<< __LINE__<< ",Multicast receiver stopped" << std::endl;
    }


    bool isRunning() const{
        return m_running;
    }

private:

    MulticastReceiver(const MulticastReceiver&) = delete;
    MulticastReceiver& operator=(const MulticastReceiver&) = delete;

    bool initializeSocket(){
        // 创建UDP套接字
        m_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_socket_fd < 0) {
            std::clog <<__func__<<","<< __LINE__<<",Failed to create socket: " << strerror(errno) << std::endl;
            return false;
        }
        


        // 设置地址重用，允许多个进程绑定到同一端口
        int reuse = 1;
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::clog <<__func__<<","<< __LINE__<<",Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
            return false;
        }
        
        //设置端口重用（SO_REUSEPORT），允许多个进程绑定到同一端口
        #ifdef SO_REUSEPORT
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
            std::clog <<__func__<<","<< __LINE__<<",Failed to set SO_REUSEPORT: " << strerror(errno) << std::endl;
            return false;
        }
        #endif
        
        // 设置接收缓冲区大小
        int buffer_size = static_cast<int>(m_receive_buffer_size);
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
            std::clog <<__func__<<","<< __LINE__<<",Failed to set receive buffer size: " << strerror(errno) << std::endl;
        }
        
        // 绑定到本地地址和端口
        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 接收所有接口的数据
        local_addr.sin_port = htons(m_multicast_port);
        if (bind(m_socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            std::clog<<__func__<<","<< __LINE__<<",Failed to bind socket: " << strerror(errno) << std::endl;
            close(m_socket_fd);
            return false;
        }
        
        std::clog <<__func__<<","<< __LINE__<<",bind INADDR_ANY and local port:"<< m_multicast_port<<" to m_socket_fd:"<< m_socket_fd<<",buffer_size:"<< buffer_size << std::endl;
        return true;
    }

    int setNONBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return -1;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    void receiveThread(){
        if (m_cpu_no > 0){
            BindCPU(m_cpu_no);            
        }

        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);
        char* buffer = new char[m_receive_buffer_size];


        std::clog <<__func__<<","<< __LINE__<<",m_socket_fd:"<< m_socket_fd<<",m_receive_buffer_size:"<< m_receive_buffer_size << std::endl;
        while (m_running) {
            // 接收数据
            memset(&source_addr, 0, sizeof(source_addr));
            int64_t t0 = get_nanoseconds();
            ssize_t recv_len = recvfrom(m_socket_fd, buffer, m_receive_buffer_size, 0,(struct sockaddr*)&source_addr, &addr_len);
            if (recv_len <= 0) {
                continue;
            }
  
            // 获取源IP和端口
            char source_ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &source_addr.sin_addr, source_ip_str, sizeof(source_ip_str));
            uint16_t source_port = ntohs(source_addr.sin_port);
            
            if (m_recv_callback) {
                m_recv_callback(m_ctx, buffer, recv_len, source_ip_str, source_port, t0);
            }
        }
    }

    uint32_t getInterfaceIndex(const std::string& interface_name){
        // 方法1: 使用if_nametoindex
        unsigned int ifindex = if_nametoindex(interface_name.c_str());
        if (ifindex != 0) {
            return static_cast<uint32_t>(ifindex);
        }
        
        // 方法2: 使用ioctl
        struct ifreq ifr;
        uint32_t sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            std::clog <<__func__<<","<< __LINE__ << ",Failed to create socket for interface index: "<<strerror(errno)<< std::endl;
            return -1;
        }

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);
        if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
            std::clog <<__func__<<","<< __LINE__<< ",Failed to get interface index for " << interface_name << ": " << strerror(errno) << std::endl;
            close(sock);
            return -1;
        }
        
        close(sock);
        return ifr.ifr_ifindex;
    }

    bool joinMulticastGroup(){
        // 获取网卡接口索引
        int ifindex = getInterfaceIndex(m_interface_name);
        if (ifindex < 0) {
            std::clog <<__func__<<","<< __LINE__<< ",Failed to get interface index for " << m_interface_name.c_str() << std::endl;
            return false;
        }
        
        // 设置组播数据接收接口
        struct in_addr interface_addr;
        interface_addr.s_addr = htonl(INADDR_ANY);
        if (setsockopt(m_socket_fd, IPPROTO_IP, IP_MULTICAST_IF, &interface_addr, sizeof(interface_addr)) < 0) {
            std::clog <<__func__<<","<< __LINE__<<  ",Failed to set multicast interface: " << strerror(errno) << std::endl;
        }

        // 加入组播组，指定接收接口
        struct ip_mreqn mreq;
        memset(&mreq, 0, sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr(m_multicast_ip.c_str());
        mreq.imr_address.s_addr   = htonl(INADDR_ANY);  // 使用任意本地地址
        mreq.imr_ifindex          = ifindex;  // 指定接收接口
        if (setsockopt(m_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            std::clog <<__func__<<","<< __LINE__<< ",Failed to join multicast group: " << strerror(errno) << std::endl;
            return false;
        }

        std::clog <<__func__<<","<< __LINE__<< ",IP_ADD_MEMBERSHIP, interface_name:" << m_interface_name.c_str() << ",ifindex:"<< ifindex<<",m_multicast_ip:" <<m_multicast_ip.c_str()<< std::endl;
        
        // 设置TTL
        m_ttl = 1;
        if (setsockopt(m_socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, &m_ttl, sizeof(m_ttl)) < 0) {
            std::clog <<__func__<<","<< __LINE__<< ",Failed to set TTL: " << strerror(errno) << std::endl;
        }
        
        // 禁用回环（不接收自己发送的数据）
        int loop = 0;
        if (setsockopt(m_socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
            std::clog <<__func__<<","<< __LINE__<< ",Failed to disable multicast loop: " << strerror(errno) << std::endl;
        }
        
        return true;
    }

    void leaveMulticastGroup(){
        if (m_socket_fd < 0) {
            return;
        }

        struct ip_mreqn mreq;
        memset(&mreq, 0, sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr(m_multicast_ip.c_str());
        mreq.imr_address.s_addr = htonl(INADDR_ANY);
        mreq.imr_ifindex = getInterfaceIndex(m_interface_name);
        if (setsockopt(m_socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            std::clog <<__func__<<","<< __LINE__<< "Failed to leave multicast group: " << strerror(errno) << std::endl;
        }
    }

private:
    std::string    m_multicast_ip;
    uint16_t       m_multicast_port;
    std::string    m_interface_name;
    int32_t        m_cpu_no;
    size_t         m_receive_buffer_size;

    uint32_t           m_socket_fd;        
    struct sockaddr_in m_multicast_addr;    // 组播地址结构
    std::atomic<bool>  m_running;
    std::thread        m_receive_thread;  

    void               *m_ctx;       
    UDPDataCallback    m_recv_callback;
    uint32_t           m_ttl;               // TTL值
};


