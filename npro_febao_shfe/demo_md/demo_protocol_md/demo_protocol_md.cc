#include <float.h>
#include <market_data_struct.h>
#include <string.h>
#include <thread>
#include <vector>
#include <limits>
#include "fb_md_plugin_api.h"
#include "fb_md_type.h"
#include "fb_md_entity.h"
#include "demo_protocol_adapter.h"
#include "demo_protocol_md.h"


// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    return new demo_protocol_md();
}
void destroy(void *p) {
    delete (demo_protocol_md*)p;
}
void get_md_api_version(char version[32]) {
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif


using namespace cffex::fb::api;

demo_protocol_md::demo_protocol_md() :
    spi_(nullptr),
    adapter_(nullptr),
    md_(market_data_entity::create_entity()),
    inquiry_(inquiry_quote_entity::create_entity()),
    status_(instrument_trading_status_entity::create_entity())
{
}

demo_protocol_md::~demo_protocol_md()
{
    delete adapter_;
    delete thread_;
    delete status_;
    delete inquiry_;
    delete md_;
}

void demo_protocol_md::register_spi(fb_i_md_spi *spi)
{
    spi_ = spi;
}

int demo_protocol_md::init()
{
    // 配置文件解析参考demo_counterapi_md

    // 初始化adapter
    adapter_ = new demo_protocol_adapter(&params_);
    adapter_->register_marketdata_callback(std::bind(&demo_protocol_md::handle_marketdata, this, std::placeholders::_1));
    adapter_->register_ready_callback(std::bind(&cffex::fb::api::fb_i_md_spi::on_ready, spi_));
    return adapter_->init();
}

void demo_protocol_md::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id)
{
    adapter_->sub_instrument(instrument_id);
}

void demo_protocol_md::connect()
{
    // 启动adapter开始接收行情
    // start函数不能阻塞回调线程，否则fb_md_proxy不工作
    thread_ = new std::thread(std::bind(&demo_protocol_adapter::start, adapter_));

    // 这里可以利用future和promise获取adapter执行状态
}

void demo_protocol_md::release()
{
    // 销毁柜台行情API
    if (adapter_) {
        adapter_->stop();
        thread_->join();
    }
}

// 处理柜台行情API的行情回调，将柜台行情结构转换为febao行情结构
void demo_protocol_md::handle_marketdata(market_data_struct *pmd)
{
    md_->reset_entity();        // 重置飞豹行情结构
    md_->set_max_depth(1);      // 设置行情深度
    md_->set_guid(FB_SET_GUID_TAG());   // 用于性能统计

    // 填充md_的其他字段
    // 可以参考demo_counterapi_md

    // 将行情发送到febao内部
    spi_->on_msg(md_);
}
