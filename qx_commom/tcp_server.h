#pragma once





#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>  

#include <sys/ioctl.h>  
#include <sys/socket.h> 
#include <netinet/tcp.h>

#include <string.h>
#include <map>
#include <iostream>
#include <mutex>

#include <functional>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <queue>

#include "common.h"



struct SendItem {
    std::unique_ptr<char[]> data;
    size_t sz;
    
    SendItem() : data(nullptr), sz(0){}
    
    SendItem(const char* src_data, size_t src_sz) 
        : sz(src_sz) {
        if (src_data && src_sz > 0) {
            data = std::make_unique<char[]>(src_sz);
            memcpy(data.get(), src_data, src_sz);
        }
    }
    
    SendItem(SendItem&& other) noexcept
        : data(std::move(other.data)),
          sz(other.sz) {
        other.sz = 0;
    }
    
    SendItem& operator=(SendItem&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            sz = other.sz;
            other.sz = 0;
        }
        return *this;
    }
    
    // 禁止拷贝
    SendItem(const SendItem&) = delete;
    SendItem& operator=(const SendItem&) = delete;
};

using MsgCallback = std::function<void(void *ctx, const char* data, size_t sz, int64_t t0)>;

class TCPServer{
public:
    explicit TCPServer(size_t receive_buffer_size = 8196) {
        std::clog << "TCPServer" << std::endl;
    }

    TCPServer():TCPServer(8196) {};
    ~TCPServer(){};

    void registerCallback(void *ctx, MsgCallback callback){
        m_ctx = ctx;
        m_recv_callback = callback;
        std::clog <<__func__<<","<< __LINE__ << " success" << std::endl;
    }



    bool start(int port, const std::string& bind_ip = "0.0.0.0") {
        if (m_running.load()) {
            std::clog << "Server already running" << std::endl;
            return false;
        }
        
        m_port = port;
        m_bind_ip = bind_ip;
        
        // 创建socket
        m_server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (m_server_fd < 0) {
            handle_error("Failed to create socket: " + std::string(strerror(errno)));
            return false;
        }
        
        // 设置socket选项
        int opt = 1;
        if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            handle_error("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
            close(m_server_fd);
            return false;
        }
        
        // 绑定地址
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (bind_ip == "0.0.0.0") {
            server_addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            if (inet_pton(AF_INET, bind_ip.c_str(), &server_addr.sin_addr) <= 0) {
                handle_error("Invalid bind IP address: " + bind_ip);
                close(m_server_fd);
                return false;
            }
        }
        
        if (bind(m_server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            handle_error("Failed to bind: " + std::string(strerror(errno)));
            close(m_server_fd);
            return false;
        }
        
        // 开始监听
        if (listen(m_server_fd, 128) < 0) {
            handle_error("Failed to listen: " + std::string(strerror(errno)));
            close(m_server_fd);
            return false;
        }
        
        m_running.store(true);
        
        // 启动接收线程
        m_accept_thread = std::thread([this]() { accept_thread_func(); });
        
        std::clog << "SimpleTCPServer started on " << bind_ip << ":" << port << std::endl;
        return true;
    }
    
    void stop() {
        if (!m_running.load()) {
            return;
        }
        
        m_running.store(false);
        m_connected.store(false);
        
        // 唤醒所有等待的线程
        m_send_queue_cv.notify_all();
        
        // 关闭socket
        if (m_client_fd >= 0) {
            shutdown(m_client_fd, SHUT_RDWR);
            close(m_client_fd);
            m_client_fd = -1;
        }
        
        if (m_server_fd >= 0) {
            shutdown(m_server_fd, SHUT_RDWR);
            close(m_server_fd);
            m_server_fd = -1;
        }
        
        // 等待线程结束
        if (m_accept_thread.joinable()) {
            m_accept_thread.join();
        }
        if (m_recv_thread.joinable()) {
            m_recv_thread.join();
        }
        if (m_send_thread.joinable()) {
            m_send_thread.join();
        }
        
        // 清空发送队列
        {
            std::lock_guard<std::mutex> lock(m_send_queue_mutex);
            std::queue<SendItem> empty;
            std::swap(m_send_queue, empty);
        }
        
        std::clog << "SimpleTCPServer stopped" << std::endl;
    }
    
    bool is_running() const { return m_running.load(); }
    


    bool send(const char* data, size_t sz) {
        if (!m_connected.load() || !data || sz == 0) {
            return false;
        }
        
        SendItem item(data, sz);
        
        {
            std::lock_guard<std::mutex> lock(m_send_queue_mutex);
            
            if (m_send_queue.size() >= m_send_queue_size) {
                return false;
            }
            
            m_send_queue.push(std::move(item));
        }
        
        m_send_queue_cv.notify_one();
        return true;
    }
    
    bool send(const std::string& data) {
        return send(data.data(), data.size());
    }
    
    void set_max_send_queue_size(size_t size) { m_send_queue_size = size; }

    void accept_thread_func() {
        const int MAX_EVENTS = 10;
        struct epoll_event ev, events[MAX_EVENTS];
        
        int epoll_fd = epoll_create1(0);
        if (epoll_fd < 0) {
            handle_error("Failed to create epoll: " + std::string(strerror(errno)));
            return;
        }
        
        ev.events = EPOLLIN;
        ev.data.fd = m_server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_server_fd, &ev) < 0) {
            handle_error("Failed to add server fd to epoll: " + std::string(strerror(errno)));
            close(epoll_fd);
            return;
        }
        
        while (m_running.load()) {
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
            
            if (nfds < 0) {
                if (errno == EINTR) {
                    continue;
                }
                handle_error("epoll_wait error: " + std::string(strerror(errno)));
                break;
            }
            
            for (int i = 0; i < nfds; i++) {
                if (events[i].data.fd == m_server_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    
                    int client_fd = accept(m_server_fd, 
                                          (struct sockaddr*)&client_addr, 
                                          &client_len);
                    
                    if (client_fd < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            handle_error("Accept error: " + std::string(strerror(errno)));
                        }
                        continue;
                    }
                    
                    // 关闭之前的连接
                    if (m_connected.load()) {
                        close_connection();
                    }
                    
                    handle_connection(client_fd);
                    
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                    std::clog << "New connection from " << client_ip << ":" 
                             << ntohs(client_addr.sin_port) << std::endl;
                }
            }
        }
        
        close(epoll_fd);
    }
    
    void handle_connection(int client_fd) {
        m_client_fd = client_fd;
        m_connected.store(true);
        
        // 设置TCP_NODELAY
        int flag = 1;
        setsockopt(m_client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
        
        // 启动收发线程
        m_recv_thread = std::thread([this]() { recv_thread_func(); });
        m_send_thread = std::thread([this]() { send_thread_func(); });
    }
    
    void close_connection() {
        m_connected.store(false);
        
        if (m_client_fd >= 0) {
            shutdown(m_client_fd, SHUT_RDWR);
            close(m_client_fd);
            m_client_fd = -1;
        }
        
        if (m_recv_thread.joinable()) {
            m_recv_thread.join();
        }
        
        if (m_send_thread.joinable()) {
            m_send_thread.join();
        }
        
        {
            std::lock_guard<std::mutex> lock(m_send_queue_mutex);
            std::queue<SendItem> empty;
            std::swap(m_send_queue, empty);
        }
    }
    
    void recv_thread_func() {
        // 使用栈上的小缓冲区，避免内存分配
        char buffer[4096];
        
        while (m_running.load() && m_connected.load()) {
            if (m_client_fd < 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            
            ssize_t n = ::recv(m_client_fd, buffer, sizeof(buffer), 0);
            
            if (n > 0) {
                if (m_recv_callback) {
                    m_recv_callback(m_ctx, buffer, n, 0);
                }
            } else if (n == 0) {
                std::clog << "Connection closed by client" << std::endl;
                close_connection();
                break;
            } else {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    handle_error("Recv error: " + std::string(strerror(errno)));
                    close_connection();
                    break;
                }
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }
    
    void send_thread_func() {
        while (m_running.load() && m_connected.load()) {
            SendItem item;
            
            {
                std::unique_lock<std::mutex> lock(m_send_queue_mutex);
                
                m_send_queue_cv.wait(lock, [this]() {
                    return !m_running.load() || 
                           !m_connected.load() || 
                           !m_send_queue.empty();
                });
                
                if (!m_running.load() || !m_connected.load()) {
                    break;
                }
                
                if (m_send_queue.empty()) {
                    continue;
                }
                
                item = std::move(m_send_queue.front());
                m_send_queue.pop();
            }
            
            if (item.data && item.sz > 0) {
                internal_send(item.data.get(), item.sz);
            }
        }
    }
    
    bool internal_send(const char* data, size_t sz) {
        if (m_client_fd < 0) {
            return false;
        }
        
        size_t total_sent = 0;
        
        while (total_sent < sz && m_running.load() && m_connected.load()) {
            ssize_t n = ::send(m_client_fd, 
                              data + total_sent,
                              sz - total_sent,
                              MSG_NOSIGNAL);
            
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                } else {
                    handle_error("Send error: " + std::string(strerror(errno)));
                    return false;
                }
            }
            
            total_sent += n;
        }
        
        return total_sent == sz;
    }
    
    void handle_error(const std::string& error_msg) {
        std::clog << "[Error] " << error_msg << std::endl;
    }

private:


    int m_server_fd{-1};
    int m_client_fd{-1};


    std::mutex m_send_queue_mutex;
    std::condition_variable m_send_queue_cv;
    std::queue<SendItem>    m_send_queue;
    size_t                  m_send_queue_size{10000};

    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_running{false};

    std::thread m_accept_thread;
    std::thread m_recv_thread;
    std::thread m_send_thread;


    MsgCallback  m_recv_callback;
    void         *m_ctx;

    std::string m_bind_ip;
    int m_port{0};


};





