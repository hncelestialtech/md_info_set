/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: jinfy
 * Date: 2022-04-14
 */

#ifndef FB_MD_PLUGIN_API_H
#define FB_MD_PLUGIN_API_H

#include "fb_md_entity.h"
#include "fb_md_helper.h"
#include "fb_md_version.h"

#ifdef __cplusplus
extern "C" {
#endif

void *create();

void destroy(void *p);

void get_md_api_version(char version[32]);

#ifdef __cplusplus
}
#endif

namespace cffex {
namespace fb {
namespace api {

/**
 * @brief 为满足客户自研行情接入的需求，febao提供了行情api。客户使用行情api将行情相关数据发送给飞豹，以完成对接过程。
 * @details 行情api主要功能由两个类实现，包括fb_i_md_spi和fb_md_plugin_api
 *        - fb_i_md_spi: 自研程序向飞豹系统发送行情相关数据。自研程序无需重载此类，直接调用接口即可。
 *        - fb_md_plugin_api: 飞豹系统向自研程序进行回调。自研程序需要重载此类，根据自身需求加以实现。
 * @ingroup fb_md_api
 */
class fb_i_md_spi {
public:
    virtual ~fb_i_md_spi() {}
    /**
     * @brief 向飞豹推送行情消息
     * @param entity [IN] 当接入程序收到行情后，调用接口将数据发送给飞豹
     */
    virtual void on_msg(market_data_entity *entity)               = 0;
    /**
     * @brief 向飞豹推送市场询价消息
     * @param entity [IN] 当接入程序收到市场询价请求后，调用接口将数据发送给飞豹
     */
    virtual void on_msg(inquiry_quote_entity *entity)             = 0;
    /**
     * @brief 向飞豹推送合约交易状态消息
     * @param entity [IN] 当接入程序收到合约交易状态请求后，调用接口将数据发送给飞豹
     */
    virtual void on_msg(instrument_trading_status_entity *entity) = 0;
    /**
     * @brief 向飞豹推送初始化完成消息。此消息在初始化完成后必须发送
     */
    virtual void on_ready() { }
    /**
     * @brief 获得飞豹提供的xml配置解析功能
     */
    virtual fb_md_config_helper *get_config_helper() = 0;
    /**
     * @brief 获得飞豹提供的日志打印功能
     */
    virtual fb_md_xlog_helper   *get_xlog_helper()   = 0;
};

/**
 * @brief fb_md_plugin_api由用户重写实现功能定制，之后运行时由飞豹调用
 * @details 客户进行自研行情接入时，需要以动态库的形式提供结果，最终由框架程序fb_md_proxy控制加载启动。
            在fb_md_proxy加载动态库的时候，会对动态库的执行进行回调。fb_md_plugin_api就是回调的基类。
            客户需要从fb_md_plugin_api派生出自己的受控接口，并进行功能实现。
 * @ingroup fb_md_api
 */
class fb_md_plugin_api {
public:
    virtual ~fb_md_plugin_api() {}

    /**
     * @brief 飞豹fb_md_proxy启动后回调此接口，用户重写接口完成自身初始化
     */
    virtual int  init()                                                                = 0;
    /**
     * @brief 飞豹fb_md_proxy关闭前回调此接口，用户重写接口完成自身资源清理
     */
    virtual void release()                                                             = 0;
    /**
     * @brief 飞豹fb_md_proxy启动后回调此接口，用户重写接口启动行情等接收功能
     */
    virtual void connect()                                                             = 0;
    /**
     * @brief 飞豹fb_md_proxy启动后回调此接口，用户重写接口接收配置中订阅的产品合约
     */
    virtual void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) = 0;
    /**
     * @brief 飞豹fb_md_proxy启动后回调此接口，用户重写接口接收spi对象
     */
    virtual void register_spi(fb_i_md_spi *spi)                                        = 0;
};

}  // namespace api
}  // namespace fb
}  // namespace cffex

#endif