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

#include <etherfabric/capabilities.h>
#include <etherfabric/memreg.h>
#include <etherfabric/pd.h>
#include <etherfabric/pio.h>
#include <etherfabric/vi.h>
#include <etherfabric/ef_vi.h>

using SolarflareDataCallback = std::function<void(void *ctx, const char* data, size_t size)>;

struct ReceiveBuffer {
    ef_memreg memreg;
    uint8_t*  data;
    size_t    size;
    uint32_t  id;
};

class SolarflareReceiver{

public:
    SolarflareReceiver(std::string interface_name,const std::string& multicast_ip, uint16_t multicast_port, int32_t cpu_no = -1)
        : m_interface_name(interface_name)
        , m_multicast_ip(multicast_ip)
        , m_multicast_port(multicast_port)
        , m_cpu_no(cpu_no)
        , m_buffer_size(10*1014*1024)
        , m_driver_handle(-1)
        , m_pd()
        , m_vi()
        , m_vi_resource_id(-1)
        , m_running(false)
    {
        EfviInit(interface_name.c_str());
        if (inet_pton(AF_INET, multicast_ip.c_str(), &m_multicast_addr.sin_addr) <= 0) {
            std::clog <<__func__<<","<< __LINE__<< ",error: interface_name:" << m_interface_name.c_str()<<",multicast_ip:"<<multicast_ip.c_str()<<",multicast_port:"<<multicast_port<<",cpu_no:"<<cpu_no<< std::endl;
        }

        // 验证是否是组播地址（224.0.0.0 - 239.255.255.255）
        uint32_t addr = ntohl(m_multicast_addr.sin_addr.s_addr);
        if ((addr < 0xE0000000) || (addr > 0xEFFFFFFF)) {
            throw std::runtime_error("IP address is not a multicast address: " + m_multicast_ip);
        }

        m_multicast_addr.sin_family = AF_INET;
        m_multicast_addr.sin_port = htons(multicast_port);
    }

    ~SolarflareReceiver() {
        Stop();
        Clear();
    }


    void registerCallback(void *ctx, SolarflareDataCallback callback){
        m_ctx = ctx;
        m_recv_callback = callback;
    }

    void setBufferParams(size_t buffer_size, int buffer_count) {
        if (m_running) {
            std::clog <<__func__<<","<< __LINE__<< ",running" << std::endl;
            return;
        }
        m_buffer_size = buffer_size;
        m_buffer_count = buffer_count;
    }


    bool Start() {
        if (m_running) {
            std::clog <<__func__<<","<< __LINE__<< ",running" << std::endl;
            return false;
        }

        if (!initEFVI()) {
            std::clog <<__func__<<","<< __LINE__<< ",initEFVI error" << std::endl;
            return false;
        }

        if (!initProtectionDomain()) {
            std::clog <<__func__<<","<< __LINE__<< ",initProtectionDomain error" << std::endl;
            Clear();
            return false;
        }

        if (!initVirtualInterface()) {
            std::clog <<__func__<<","<< __LINE__<< ",initVirtualInterface error" << std::endl;
            Clear();
            return false;
        }
        
        if (!allocateReceiveBuffers()) {
            std::clog <<__func__<<","<< __LINE__<< ",allocateReceiveBuffers error" << std::endl;
            Clear();
            return false;
        }

        if (!setupMulticastFilter()) {
            std::clog <<__func__<<","<< __LINE__<< ",setupMulticastFilter error" << std::endl;
            Clear();
            return false;
        }

        m_running = true;
        m_receive_thread = std::thread(&SolarflareReceiver::receiveThread, this);
        std::clog <<__func__<<","<< __LINE__<< ",Multicast receiver started on:" << m_interface_name.c_str()<< std::endl;

        return true;
        
    }
    



    void Stop(){
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

    bool initEFVI() {
         std::clog <<__func__<<","<< __LINE__ << std::endl;
        int rc = ef_driver_open(&m_driver_handle);
        if (rc != 0) {
            std::clog <<__func__<<","<< __LINE__<< ",strerror:" <<strerror(rc)<< std::endl;
            return false;
        }
        
        return true;
    }
  
    bool initProtectionDomain() {
        std::clog <<__func__<<","<< __LINE__ << std::endl;
        int rc = ef_pd_alloc_by_name(&m_pd, m_driver_handle,m_interface_name.c_str(), EF_PD_DEFAULT);
        if (rc != 0) {
            std::clog <<__func__<<","<< __LINE__<< ",ef_pd_alloc_by_name strerror:" <<strerror(rc)<< std::endl;
            return false;
        }
        
        return true;
    }

    bool initVirtualInterface() {
        std::clog <<__func__<<","<< __LINE__ << std::endl;

        unsigned vi_flags = EF_VI_FLAGS_DEFAULT;
        int rc = ef_vi_alloc_from_pd(&m_vi, m_driver_handle, &m_pd, m_driver_handle,vi_flags, m_evq_capacity, m_rxq_capacity,-1, -1, -1, nullptr, 0);
        if (rc != 0) {
            std::clog <<__func__<<","<< __LINE__<< ",ef_vi_alloc_from_pd strerror:" <<strerror(rc)<< std::endl;
            return false;
        }

        m_vi_resource_id = ef_vi_resource_id(&m_vi);
        std::clog <<__func__<<","<< __LINE__<< ",m_vi_resource_id:" <<m_vi_resource_id<< std::endl;
        std::clog <<__func__<<","<< __LINE__<< ",m_evq_capacity:" <<m_evq_capacity<< std::endl;
        std::clog <<__func__<<","<< __LINE__<< ",m_rxq_capacity:" <<m_rxq_capacity<< std::endl;
        return true;
    }


    bool allocateReceiveBuffers() {
        std::clog <<__func__<<","<< __LINE__ << std::endl;
        size_t aligned_buffer_size = (m_buffer_size + 63) & ~63;  // 64字节对齐
        

        for (int i = 0; i < m_buffer_count; ++i) {
            ReceiveBuffer buf;

            buf.size = aligned_buffer_size;
            buf.data = static_cast<uint8_t*>(aligned_alloc(64, buf.size));
            if (!buf.data) {
                std::clog <<__func__<<","<< __LINE__<< ",aligned_alloc error:" << strerror(errno) << std::endl;
                return false;
            }
            
            // 清零内存
            memset(buf.data, 0, buf.size);
            
            // 注册内存到EFVI
            int rc = ef_memreg_alloc(&buf.memreg, m_driver_handle, &m_pd, m_driver_handle, buf.data, buf.size);
            if (rc != 0) {
                std::clog <<__func__<<","<< __LINE__<< ",ef_memreg_alloc strerror:" << strerror(rc) << std::endl;
                free(buf.data);
                return false;
            }
            
            buf.id = i;
            m_receive_buffers.push_back(buf);
            
            ef_addr memreg_addr = ef_memreg_dma_addr(&buf.memreg, 0);
            ef_vi_receive_init(&m_vi, memreg_addr, buf.size);
        }
        
        std::clog <<__func__<<","<< __LINE__<< "m_receive_buffers.size:" << m_receive_buffers.size() << " aligned_buffer_size:" << aligned_buffer_size  << std::endl;
        
        return true;

    }


    bool setupMulticastFilter() {
        std::clog <<__func__<<","<< __LINE__ << std::endl;
        ef_filter_spec filter_spec;
        ef_filter_spec_init(&filter_spec, EF_FILTER_FLAG_NONE);
        
        uint32_t mc_addr = m_multicast_addr.sin_addr.s_addr;
        int rc = ef_filter_spec_set_ip4_local(&filter_spec, IPPROTO_UDP, mc_addr, htons(m_multicast_port));
        if (rc != 0) {
            std::clog <<__func__<<","<< __LINE__ << ",ef_filter_spec_set_ip4_local: " << strerror(rc) << std::endl;
            
            // 尝试使用另一种方法
            ef_filter_spec_init(&filter_spec, EF_FILTER_FLAG_NONE);
            rc = ef_filter_spec_set_ip4_full(&filter_spec, 
                                            IPPROTO_UDP,
                                            htonl(INADDR_ANY), htons(m_multicast_port),
                                            mc_addr, 0);
            if (rc != 0) {
                std::cerr << "设置完整IP过滤器失败: " << strerror(rc) << std::endl;
                return false;
            }
        }
        
        // 应用过滤器到VI
        rc = ef_vi_filter_add(&m_vi, m_driver_handle, &filter_spec, nullptr);
        if (rc != 0) {
            std::cerr << "添加过滤器失败: " << strerror(rc) << std::endl;
            return false;
        }
        
        std::cout << "组播过滤器设置成功" << std::endl;
        return true;
    }

    // 接收线程主循环
    void receiveThread() {
        BindCPU(m_cpu_no);
        std::clog <<__func__<<","<< __LINE__<< ",m_running:" << m_running << ",m_receive_buffer_size:" << m_receive_buffer_size << std::endl;
        
        // 预分配事件数组
        const int MAX_EVENTS = 32;
        ef_event events[MAX_EVENTS];
        
        while (m_running) {
            // 轮询事件
            int n_events = ef_eventq_poll(&m_vi, events, MAX_EVENTS);
            
            if (n_events > 0) {
                for (int i = 0; i < n_events; ++i) {
                    if (EF_EVENT_TYPE_RX(events[i])) {
                        // 接收事件
                        uint32_t desc = EF_EVENT_RX_RQ_ID(events[i]);
                        uint32_t bytes = EF_EVENT_RX_BYTES(events[i]);
                        
                        if (desc < m_receive_buffers.size()) {
                            ReceiveBuffer& buf = m_receive_buffers[desc];

                            // 调用回调函数处理数据
                            if (m_recv_callback) {
                                try {
                                    // 跳过以太网头（14字节）、IP头（至少20字节）获取UDP数据
                                    // 实际应用中需要根据协议解析
                                    size_t eth_header_size = 14;
                                    size_t ip_header_size = 20;
                                    size_t udp_header_size = 8;
                                    size_t data_offset = eth_header_size + ip_header_size + udp_header_size;
                                    
                                    if (bytes > data_offset) {
                                        size_t data_size = bytes - data_offset;
                                        const char* data = reinterpret_cast<const char*>(buf.data + data_offset);
                                        
                                        m_recv_callback(m_ctx, data, data_size);
                                    } else if (bytes > 0) {
                                        // 如果包太小，传递整个包
                                        const char* data = reinterpret_cast<const char*>(buf.data);
                                        m_recv_callback(m_ctx, data, bytes);
                                    }
                                } catch (const std::exception& e) {
                                    std::cerr << "回调函数异常: " << e.what() << std::endl;
                                }
                            }
                            
                            // 重新发布缓冲区
                            ef_addr memreg_addr = ef_memreg_dma_addr(&buf.memreg, 0);
                            ef_vi_receive_init(&m_vi, memreg_addr, buf.size, desc);
                        }
                    } else if (EF_EVENT_TYPE_RX_MULTI(events[i])) {
                        // 处理多个接收事件
                        uint32_t desc = EF_EVENT_RX_MULTI_RQ_ID(events[i]);
                        uint32_t n_descs = EF_EVENT_RX_MULTI_COUNT(events[i]);
                        
                        for (uint32_t j = 0; j < n_descs; ++j) {
                            uint32_t current_desc = desc + j;
                            if (current_desc < m_receive_buffers.size()) {
                                ReceiveBuffer& buf = m_receive_buffers[current_desc];
                                
                                // 注意：这里不知道包大小，需要从描述符中获取或使用默认值
                                // 实际应用中需要更精确的处理
                                
                                // 重新发布缓冲区
                                ef_addr memreg_addr = ef_memreg_dma_addr(&buf.memreg, 0);
                                ef_vi_receive_init(&m_vi, memreg_addr, buf.size, current_desc);
                            }
                        }
                    }
                }
            } else if (n_events == 0) {
                // 没有事件，短暂休眠避免忙等待
                usleep(10);  // 10微秒
            } else {
                // 错误
                if (errno != EINTR) {
                    std::cerr << "ef_eventq_poll 错误: " << strerror(errno) << std::endl;
                }
                break;
            }
        }
        
        std::cout << "接收线程结束" << std::endl;
    }
    
    // 清理资源
    void Clear() {
        std::cout << "清理资源..." << std::endl;
        

        for (auto& buf : m_receive_buffers) {
            if (buf.data) {
                // 取消内存注册
                ef_memreg_free(&buf.memreg, m_driver_handle);
                
                // 释放内存
                free(buf.data);
                buf.data = nullptr;
            }
        }
        m_receive_buffers.clear();
        
        // 释放虚拟接口
        if (m_vi_resource_id >= 0) {
            ef_vi_free(&m_vi, m_driver_handle);
            m_vi_resource_id = -1;
        }
        
        // 释放保护域
        ef_pd_free(&m_pd, m_driver_handle);
        
        // 关闭驱动句柄
        if (m_driver_handle >= 0) {
            ef_driver_close(m_driver_handle);
            m_driver_handle = -1;
        }
        
        std::cout << "资源清理完成" << std::endl;
    }
    




private:

    SolarflareReceiver(const SolarflareReceiver&) = delete;
    SolarflareReceiver& operator=(const SolarflareReceiver&) = delete;

    bool EfviInit(const char *ifName);

    void EFVIRelease(){

    }



    void receiveThread(){
        BindCPU(m_cpu_no);
        std::clog <<__func__<<","<< __LINE__<< ",m_running:" << m_running << ",m_receive_buffer_size:" << m_receive_buffer_size << std::endl;


    }




private:

    std::string        m_interface_name;
    std::string        m_multicast_ip;
    uint16_t           m_multicast_port;
    struct sockaddr_in m_multicast_addr;

    int32_t        m_cpu_no;
    size_t         m_buffer_size;
    int32_t        m_buffer_count;

    int m_evq_capacity;
    int m_rxq_capacity;

    std::atomic<bool>  m_running;
    std::thread        m_receive_thread;  

    void                   *m_ctx;       
    SolarflareDataCallback m_recv_callback;

    // EFVI 
    ef_driver_handle m_driver_handle;
    ef_pd            m_pd;
    ef_vi            m_vi;
    int32_t          m_vi_resource_id;
    uint32_t         m_vi_flags;

    std::vector<ReceiveBuffer> m_receive_buffers;


};
















