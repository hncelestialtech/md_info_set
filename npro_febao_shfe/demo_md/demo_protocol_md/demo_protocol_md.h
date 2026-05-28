#ifndef DEMO_PROTOCOL_MD
#define DEMO_PROTOCOL_MD

#include <stdint.h>
#include <map>
#include <string>
#include "demo_protocol_adapter.h"          // 这里是飞马柜台行情API的头文件
#include "fb_md_plugin_api.h"
#include "demo_protocol_param.h"

/*
** 这个示例不依赖于柜台行情API，而是需要插件自己完成从网络到协议再到行情的整个解析过程
** demo_protocol_md: 处理行情插件和febao的交互
** demo_protocol_adapter: 处理行情插件与交易所行情前置的交互
** 另外包含一个简单的Makefile示例文件
*/

struct market_data_struct;
class demo_protocol_adapter;
class demo_protocol_md : public cffex::fb::api::fb_md_plugin_api
{
public:
    demo_protocol_md();
    virtual ~demo_protocol_md();

    // fb_md_plugin_api, fb_md_proxy按序调用下列API
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;
    int  init() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void connect() override;
    void release() override;

    // 这里只展示如何处理market_data_entity，inquiry和status一样
    void handle_marketdata(market_data_struct *md);

private:
    cffex::fb::api::fb_i_md_spi                      *spi_;
    std::thread                                      *thread_;   // adapter的运行线程，也可以将thread内置到adapter内部
    demo_protocol_adapter                            *adapter_;
    cffex::fb::api::market_data_entity               *md_;
    cffex::fb::api::inquiry_quote_entity             *inquiry_;
    cffex::fb::api::instrument_trading_status_entity *status_;
    demo_protocol_param                               params_;
};

#endif