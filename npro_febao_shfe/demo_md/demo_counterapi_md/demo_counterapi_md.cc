#include <float.h>
#include <string.h>
#include <vector>
#include <limits>
#include "fb_md_type.h"
#include "fb_md_entity.h"
#include "demo_counterapi_md.h"


// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    return new demo_counterapi_md();
}
void destroy(void *p) {
    delete (demo_counterapi_md*)p;
}
void get_md_api_version(char version[32]) {
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif


using namespace cffex::fb::api;

demo_counterapi_md::demo_counterapi_md() :
    spi_(nullptr),
    femas_api_(nullptr),
    md_(market_data_entity::create_entity()),
    inquiry_(inquiry_quote_entity::create_entity()),
    status_(instrument_trading_status_entity::create_entity()),
    request_id_(0)
{
}

demo_counterapi_md::~demo_counterapi_md()
{
    delete status_;
    delete inquiry_;
    delete md_;
}

void demo_counterapi_md::register_spi(fb_i_md_spi *spi)
{
    spi_ = spi;
}

int demo_counterapi_md::init()
{
    /*
    <demo_counterapi_md>
        <flow_path path="./flow/"/>
        <md_xlog loglevel="all" />
        <addr addr="tcp://172.31.194.19:8005"/>
        <thread_bind cpu_no="1" />
        <account brokerid="0001"  userid="renjh"  passwd="111111" encrypt="false" investorid="2032004">
            <item topicid="111"/>
        </account>
    </demo_counterapi_md>
    */
    // 从server_config.xml中解析出插件自定义的配置, 这里以femas柜台api插件的配置为例
    fb_md_config_helper *parser = spi_->get_config_helper();
    parser->get_attribute("path", "/flow_path", params_.flow_path);
    parser->get_attribute("addr", "/addr", params_.addr);
    parser->get_attribute("brokerid",   "/account", params_.brokerid);
    parser->get_attribute("userid",     "/account", params_.userid);
    parser->get_attribute("investorid", "/account", params_.investorid);
    parser->parse_passwd(parser->get_attribute("passwd", "/account"),
                         parser->get_attribute("encrypt", "/account"),
                         params_.passwd);
    if (params_.addr.empty()) {
        fprintf(stderr, "parse config file failed, no addr info\n");
        return -1;
    }

    // parse topic
    std::vector<std::string> svec;
    parser->get_attribute_elements("topicid", "/account/item", svec);
    for(uint32_t i = 0; i < svec.size(); ++i) {
        params_.topics.insert(atoi(svec[i].c_str()));
    }

    // parse thread_bind
    params_.cpu_no = -1;
    char *p_cpu_no = parser->get_attribute("cpu_no", "/thread_bind");
    if (strlen(p_cpu_no) > 0) {
        params_.cpu_no = atoi(p_cpu_no);
    }

    // print result
    spi_->get_xlog_helper()->xlog(FB_XLOG_INFO,
        "flow_path=%s, addr=%s, brokerid=%s, userid=%s, investorid=%s, cpu_no=%d",
        params_.flow_path.c_str(),
        params_.addr.c_str(),
        params_.brokerid.c_str(),
        params_.userid.c_str(),
        params_.investorid.c_str(),
        params_.cpu_no);

    return 0;
}

void demo_counterapi_md::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id)
{
    instruments_.insert(std::make_pair(instrument_id, exchange_id));
    spi_->get_xlog_helper()->xlog(FB_XLOG_INFO,
                                  "demo_counterapi_md::%s, %s exchange_id[%u]\n",
                                  __FUNCTION__,
                                  instrument_id.c_str(),
                                  exchange_id);
}

void demo_counterapi_md::connect()
{
    // 创建并初始化柜台行情API
    femas_api_ = CUstpFtdcMduserApi::CreateFtdcMduserApi(params_.flow_path.c_str());
    femas_api_->RegisterSpi(this);

    for (auto topicid : params_.topics) {
        femas_api_->SubscribeMarketDataTopic(topicid, USTP_TERT_QUICK);
        spi_->get_xlog_helper()->xlog(FB_XLOG_DEBUG,
            "demo_counterapi_md::%s, sub topic_id[%d]\n", __FUNCTION__, topicid);
    }

    femas_api_->RegisterFront((char *)params_.addr.c_str());
    femas_api_->Init();
}

void demo_counterapi_md::release()
{
    // 销毁柜台行情API
    if (femas_api_) {
        femas_api_->Release();
        femas_api_ = nullptr;
    }
}

// 这里需要注意，有些柜台断线会自动重连，有些则需要插件去处理重连
void demo_counterapi_md::OnFrontConnected()
{
    // 与柜台连接成功后进行登录
    CUstpFtdcReqUserLoginField reqUserLogin;
    strcpy(reqUserLogin.BrokerID, params_.brokerid.c_str());
    strcpy(reqUserLogin.UserID, params_.userid.c_str());
    strcpy(reqUserLogin.Password, params_.passwd.c_str());
    int r = femas_api_->ReqUserLogin(&reqUserLogin, ++request_id_);
    // 对登录请求返回值进行错误处理
}

void demo_counterapi_md::OnFrontDisconnected(int nReason)
{
    spi_->get_xlog_helper()->xlog(FB_XLOG_WARNING,
        "demo_counterapi_md::%s, OnFrontDisconnected\n", __FUNCTION__);
}

void demo_counterapi_md::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin,
                                        CUstpFtdcRspInfoField      *pRspInfo,
                                        int                         nRequestID,
                                        bool                        bIsLast)
{
    if (pRspInfo->ErrorID != 0) {
        // 端登失败错误处理
        return;
    }

    // 向柜台订阅合约行情
    femas_api_->SubMarketData();

    // 重要！！！行情插件准备好后必须通知febao后台，否则视为启动失败
    // 重要！！！行情插件准备好后必须通知febao后台，否则视为启动失败
    // 重要！！！行情插件准备好后必须通知febao后台，否则视为启动失败
    spi_->on_ready();
}

// febao行情中，和价格相关的值使用DML_MAX表示空值
static inline double convert_price_if_zero(double price) {
    return (price == 0.0f) ? DBL_MAX : price;
}

// febao行情中，板价和板量的关系
static inline double convert_price_if_volume_zero(double price, uint32_t volume) {
    if (volume > 0 && price > std::numeric_limits<double>::lowest() && price < std::numeric_limits<double>::max())
        return price;
    return DBL_MAX;
}

// 处理柜台行情API的行情回调，将柜台行情结构转换为febao行情结构
void demo_counterapi_md::OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField *pMarketData)
{
    if (params_.cpu_no > 0) {
        // 这里可以将柜台行情回调线程绑核，可参考如下算法：
        static uint64_t n_mds = 0;
        n_mds++;
        if (n_mds % 1000 == 0) {
            // 检查是否已经绑核，如果未绑核则进行绑核处理
        }
    }

    md_->reset_entity();        // 重置飞豹行情结构
    md_->set_max_depth(1);      // 设置行情深度
    md_->set_guid(FB_SET_GUID_TAG());   // 用于性能统计

    md_->set_instrument_id(pMarketData->InstrumentID);
    md_->set_exchange_id(FB_EXCHANGE_CFFEX);

    // 将柜台行情API的时间格式转换为febao时间格式
    // febao的时间格式为：时*3600 + 分*60 + 秒
    int sec = convert_femas_updatesec_to_febao_updatesec(pMarketData->UpdateTime);
    md_->set_update_sec(sec);
    md_->set_update_msec(pMarketData->UpdateMillisec);

    md_->set_pre_settlement(pMarketData->PreSettlementPrice);
    md_->set_pre_close(pMarketData->PreClosePrice);
    md_->set_pre_open_interest(pMarketData->PreOpenInterest);
    md_->set_open(convert_price_if_zero(pMarketData->OpenPrice));
    md_->set_close(convert_price_if_zero(pMarketData->ClosePrice));
    md_->set_upper_limit_price(pMarketData->UpperLimitPrice);
    md_->set_down_limit_price(pMarketData->LowerLimitPrice);
    md_->set_high_price(convert_price_if_zero(pMarketData->HighestPrice));
    md_->set_low_price(convert_price_if_zero(pMarketData->LowestPrice));

    md_->set_last_price(pMarketData->LastPrice);
    md_->set_volume(pMarketData->Volume);
    md_->set_turn_over(pMarketData->Turnover);
    md_->set_open_interest(pMarketData->OpenInterest);

    md_->set_bid1_price(convert_price_if_volume_zero(pMarketData->BidPrice1, pMarketData->BidVolume1));
    md_->set_ask1_price(convert_price_if_volume_zero(pMarketData->AskPrice1, pMarketData->AskVolume1));
    md_->set_bid1_volume(pMarketData->BidVolume1);
    md_->set_ask1_volume(pMarketData->AskVolume1);

    md_->set_bid2_price(convert_price_if_volume_zero(pMarketData->BidPrice2, pMarketData->BidVolume2));
    md_->set_ask2_price(convert_price_if_volume_zero(pMarketData->AskPrice2, pMarketData->AskVolume2));
    md_->set_bid2_volume(pMarketData->BidVolume2);
    md_->set_ask2_volume(pMarketData->AskVolume2);

    md_->set_bid3_price(convert_price_if_volume_zero(pMarketData->BidPrice3, pMarketData->BidVolume3));
    md_->set_ask3_price(convert_price_if_volume_zero(pMarketData->AskPrice3, pMarketData->AskVolume3));
    md_->set_bid3_volume(pMarketData->BidVolume3);
    md_->set_ask3_volume(pMarketData->AskVolume3);

    md_->set_bid4_price(convert_price_if_volume_zero(pMarketData->BidPrice4, pMarketData->BidVolume4));
    md_->set_ask4_price(convert_price_if_volume_zero(pMarketData->AskPrice4, pMarketData->AskVolume4));
    md_->set_bid4_volume(pMarketData->BidVolume4);
    md_->set_ask4_volume(pMarketData->AskVolume4);

    md_->set_bid5_price(convert_price_if_volume_zero(pMarketData->BidPrice5, pMarketData->BidVolume5));
    md_->set_ask5_price(convert_price_if_volume_zero(pMarketData->AskPrice5, pMarketData->AskVolume5));
    md_->set_bid5_volume(pMarketData->BidVolume5);
    md_->set_ask5_volume(pMarketData->AskVolume5);

    // 将行情发送到febao内部
    spi_->on_msg(md_);
}
