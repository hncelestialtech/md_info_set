#ifndef DEMO_COUNTERAPI_MD
#define DEMO_COUNTERAPI_MD

#include <stdint.h>
#include <map>
#include <string>
#include "USTPFtdcMduserApi.h"          // 这里是飞马柜台行情API的头文件
#include "fb_md_plugin_api.h"
#include "demo_counterapi_param.h"

/*
** 这里以飞马柜台行情API举例，示例了通用的柜台行情API接入飞豹的处理方式。
** 示例中并未进行完整的业务处理和错误处理，语法上也编译不通过，需要开发人员补齐
** 另外包含一个简单的Makefile示例文件
*/
class demo_counterapi_md : public cffex::fb::api::fb_md_plugin_api, public CUstpFtdcMduserSpi
{
    using INSTRUMENT_EXCHANGE_MAP = std::map<std::string, uint8_t>;

public:
    demo_counterapi_md();
    virtual ~demo_counterapi_md();

    // fb_md_plugin_api, fb_md_proxy按序调用下列API
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;
    int  init() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void connect() override;
    void release() override;


    // femas CUstpFtdcMduserSpi
    void OnFrontConnected();
    void OnFrontDisconnected(int nReason);
    void OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin,
                        CUstpFtdcRspInfoField      *pRspInfo,
                        int                         nRequestID,
                        bool                        bIsLast);
    void OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField *pMarketData);

private:
    cffex::fb::api::fb_i_md_spi                      *spi_;
    CUstpFtdcMduserApi                               *femas_api_;
    cffex::fb::api::market_data_entity               *md_;
    cffex::fb::api::inquiry_quote_entity             *inquiry_;
    cffex::fb::api::instrument_trading_status_entity *status_;
    INSTRUMENT_EXCHANGE_MAP                           instruments_;
    demo_counterapi_param                            params_;
    int                                               request_id_;
};

#endif