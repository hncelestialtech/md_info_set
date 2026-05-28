#ifndef __DEMO_PROTOCOL_ADAPTER_H__
#define __DEMO_PROTOCOL_ADAPTER_H__

#include <market_data_struct.h>
#include <stddef.h>
#include <set>
#include <string>
#include <functional>

struct demo_protocol_param;
struct market_data_struct;

class demo_protocol_network;    // 处理网络细节，收发网络数据
class demo_protocol_decode;     // 根据协议解码从网络上收到的数据
class demo_protocol_encode;     // 根据协议编码发送到网络上的数据

class demo_protocol_adapter
{
    typedef std::function<void(market_data_struct*)> MARKET_DATA_CALLBACK;
    typedef std::function<void()>                    READY_CALLBACK;

public:
    explicit demo_protocol_adapter(demo_protocol_param *param);
    ~demo_protocol_adapter();

    void register_marketdata_callback(const MARKET_DATA_CALLBACK &cb) { md_cb_ = cb; }
    void register_ready_callback(const READY_CALLBACK& cb) { ready_cb_ = cb; }
    void sub_instrument(const std::string &instrument_id) { instruments_.insert(instrument_id); }

    int init();
    void start();
    void stop() { stop_ = 1; }

private:
    void handle_message(void *data, size_t len);

private:
    volatile int           stop_{0};
    demo_protocol_param   *param_;
    demo_protocol_network *net_;
    demo_protocol_decode  *decode_;
    demo_protocol_encode  *encode_;
    std::set<std::string>  instruments_;     // 已订阅合约
    READY_CALLBACK         ready_cb_;
    MARKET_DATA_CALLBACK   md_cb_;
};

#endif // __DEMO_PROTOCOL_ADAPTER_H__
