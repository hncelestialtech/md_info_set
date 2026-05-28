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


#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include "common.h"




using ExanicDataCallback = std::function<void(void *ctx, const char* data, size_t size)>;

class ExanicReceiver{

public:

    ExanicReceiver(std::string interface_name, int32_t cpu_no = -1, size_t receive_buffer_size = 8196)
    {
        m_interface_name = interface_name;
        m_cpu_no         = cpu_no;
        m_receive_buffer_size = receive_buffer_size;
        m_running = false;
  
        InitExanic(interface_name.c_str());

    }

    ~ExanicReceiver() {
        exanic_release_rx_buffer(m_exanic_rx);
        exanic_release_handle(m_exanic);
    }


    void registerCallback(void *ctx, ExanicDataCallback callback){
        m_ctx = ctx;
        m_recv_callback = callback;
    }

    bool start(){
        if (m_running) {
            std::cerr << "Receiver is already running" << std::endl;
            return false;
        }

        // 启动接收线程
        m_running = true;
        m_receive_thread = std::thread(&ExanicReceiver::receiveThread, this);
        
        std::clog <<__func__<<","<< __LINE__<< ",Multicast receiver started on:" << m_interface_name.c_str()<< std::endl;
        
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

    ExanicReceiver(const ExanicReceiver&) = delete;
    ExanicReceiver& operator=(const ExanicReceiver&) = delete;

    bool InitExanic(const char *ifName){
        std::clog <<__func__<<","<< __LINE__<<"],ifName["<< ifName<<"]" << std::endl;

        char exaName[16] = "exanic0";
        int portNum = 0;

        if (0 != exanic_find_port_by_interface_name(ifName, exaName, sizeof(exaName) - 1, &portNum))
        {
            std::cerr << "exanic find port error:" <<ifName <<",error:"<< strerror(errno)<< std::endl;
            return false;
        }
        std::clog <<__func__<<","<< __LINE__<<",exaName["<< exaName<<"]"<<",portNum["<< portNum<<"]" << std::endl;
        
        m_exanic = exanic_acquire_handle(exaName);
        if (!m_exanic)
        {
            std::cerr << "exanic nullptr" << std::endl;
            return false;
        }

        std::clog <<__func__<<","<< __LINE__<<",m_exanic["<< reinterpret_cast<int64_t>(m_exanic) <<"]" << std::endl;

        int port = 0;
        m_exanic_rx = exanic_acquire_rx_buffer(m_exanic, portNum, 0);
        if (!m_exanic_rx)
        {
            std::cerr << "exanic acquire rx buffer error: " << exanic_get_last_error() << std::endl;
            return false;
        }
        std::clog <<__func__<<","<< __LINE__<<",m_exanic_rx["<< reinterpret_cast<int64_t>(m_exanic_rx)<<"]" << std::endl;


        std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;

        return true;
    }

    void ReleaseExanic(){
        exanic_release_rx_buffer(m_exanic_rx);
        exanic_release_handle(m_exanic);
    }



    void receiveThread(){
        sleep(2);
        BindCPU(m_cpu_no);
        std::clog <<__func__<<","<< __LINE__<< ",m_running:" << m_running << ",m_receive_buffer_size:" << m_receive_buffer_size << std::endl;

        char* buffer = new char[m_receive_buffer_size];
        while (m_running) {
 
            exanic_cycles32_t timestamp;
            auto sz = exanic_receive_frame(m_exanic_rx, buffer, m_receive_buffer_size, &timestamp);
            // std::clog <<__func__<<","<< __LINE__<< ",sz:" << sz << std::endl;
            if (sz <= 0){
                continue;
            }
            // std::clog <<__func__<<","<< __LINE__<< ",sz:" << sz << std::endl;
            if (m_recv_callback) {
                m_recv_callback(m_ctx, buffer, sz);
            }

        }
    }




private:

    std::string    m_interface_name;
    int32_t        m_cpu_no;
    size_t         m_receive_buffer_size;


    std::atomic<bool>  m_running;
    std::thread        m_receive_thread;  


    exanic_t           *m_exanic;
    exanic_rx_t        *m_exanic_rx;


    void               *m_ctx;       
    ExanicDataCallback m_recv_callback;

};





















