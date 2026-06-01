/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: lisc
 * Date: 2020-03-09
 */

#ifndef FB_TRADING_PROXY_API_H
#define FB_TRADING_PROXY_API_H

#include <set>
#include <functional>

#include "fb_api_helper.h"
#include "fb_trading_proxy_entity.h"
#include "fb_trading_proxy_msg.h"

typedef enum {
    FB_XLOG_FATAL   = 0X01,
    FB_XLOG_ERROR   = 0X02,
    FB_XLOG_WARNING = 0X04,
    FB_XLOG_INFO    = 0X08,
    FB_XLOG_DEBUG   = 0X10,
    FB_XLOG_TRACE   = 0X20,
    FB_XLOG_ALL     = 0XFF
} FB_XLOG_LEVEL;

#define FB_XLOG(level, fmt, ...)                                                    \
    do {                                                                            \
        if (level == (level & cffex::fb::fb_trading_proxy_api::get_xlog_level())) { \
            cffex::fb::fb_trading_proxy_api::xlog(0X00, level, fmt, ##__VA_ARGS__); \
        }                                                                           \
    } while (0)

#ifndef OUT
#define OUT
#endif

static const int MAX_TRADING_PROXY_LEN = 128;

namespace cffex {
namespace fb {

class fb_trading_proxy_api_base_caller {
public:
    virtual ~fb_trading_proxy_api_base_caller() {}
    /**
     * @brief 报单请求
     * @param msg 报单请求指针
     * @return 成功返回0，失败返回-1
     */
    virtual int insert_order(const req_order_msg *msg) = 0;

    /**
     * @brief 撤单请求
     * @param msg 撤单请求指针
     * @return 成功返回0，失败返回-1
     */
    virtual int cancel_order(const req_order_msg *msg) = 0;

    /**
     * @brief 报价请求
     * @param msg 报价请求指针
     * @return 成功返回0，失败返回-1
     */
    virtual int insert_quote(const req_quote_msg *msg) = 0;

    /**
     * @brief 报价撤销请求
     * @param msg 报价撤销请求指针
     * @return 成功返回0，失败返回-1
     */
    virtual int cancel_quote(const req_quote_msg *msg) = 0;

    /**
     * @brief 组合保证金组合请求
     * @param msg 组合保证金组合请求指针
     * @return 成功返回0，失败返回-1
     */
    virtual int comb_combine(const req_comb_msg *msg) = 0;

    /**
     * @brief 组合保证金拆分请求
     * @param msg 组合保证金拆分请求指针
     * @return 成功返回0，失败返回-1
     */
    virtual int comb_split(const req_comb_msg *msg) = 0;

    /// 请求持仓查询
    ///@param user_manage_request_id 出参，用户管理的请求ID，实现时需要赋值
    ///@return  0 查询成功
    ///@return -3 查询失败
    virtual int req_position(OUT int &user_manage_request_id) = 0;

    /// 请求证券持仓查询
    ///@param user_manage_request_id 出参，用户管理的请求ID，实现时需要赋值
    ///@return  0 查询成功
    ///@return -3 查询失败
    virtual int req_security_position(OUT int &user_manage_request_id) = 0;

    /// 请求成交查询
    ///@param user_manage_request_id 出参，用户管理的请求ID，实现时需要赋值
    ///@return  0 查询成功
    ///@return -3 查询失败
    virtual int req_qry_trade(OUT int &user_manage_request_id) = 0;

    /// 请求组合持仓明细查询
    ///@param user_manage_request_id 出参，用户管理的请求ID，实现时需要赋值
    ///@return  0 查询成功
    ///@return -3 查询失败
    virtual int req_qry_comb_detail(OUT int &user_manage_request_id) = 0;

    /**
     * @brief 请求组合保证金组合持仓查询
     * @param user_manage_request_id [OUT] 用户管理的请求ID，实现时需要赋值
     * @return 成功返回0，失败返回-3
     */
    virtual int req_qry_comb_strategy_position(OUT int &user_manage_request_id) = 0;

    /**
     * @brief 请求账户资金
     * @return 成功返回0，失败返回-3
     */
    virtual int  req_qry_account_fund()     = 0;
    virtual void reset_request_flow_count() = 0;
};

class fb_trading_proxy_api {
public:
    enum { MM_MAX_ELEMENT_COUNT = 0XFFFF };
    enum { MM_MAX_ELEMENT_LEN = 0XFF };

    static fb_trading_proxy_api *create();
    static void                  destroy(fb_trading_proxy_api *api);

    virtual ~fb_trading_proxy_api() {}
    /* call init first, should call config_parser::parse before init */
    virtual int init(const char *exe, uint8_t node_id, int argc, char *argv[]) = 0;
    /* set callback adapter */
    virtual void set_adapter(fb_trading_proxy_api_base_caller *cb) = 0;

    virtual void start() = 0;
    virtual void wait()  = 0;
    virtual void stop()  = 0;

    /* query interface */
    /**
     * @brief 获取首条合约信息
     * @return 合约信息
     */
    virtual instrument_msg *get_first_instrument() = 0;
    
    /**
     * @brief 获取下条合约信息
     * @return 合约信息
     */
    virtual instrument_msg *get_next_instrument() = 0;

    /**
     * @brief 获取合约信息
     * @param instrument_id [IN] 合约号
     * @param exchange_id [IN] 交易所编号
     * @return 合约信息
     */
    virtual instrument_msg *get_instrument_msg(const char  *instrument_id,
                                               const int8_t exchange_id) = 0;
    virtual int             get_node_id()                                = 0;

    /**
     * @brief 获取组合外部标志号
     * @param portfolio_id [IN] 组合号
     * @return 组合外部标志号
     */
    virtual const char *get_portfolio_flag_id(int16_t portfolio_id) = 0;

    /**
     * @brief 获取组合号
     * @param portfolio_flag_id [IN] 组合外部标志号
     * @return 组合号
     */
    virtual int16_t get_portfolio_id(const char *portfolio_flag_id) = 0;

    /**
     * @brief 获取交易账号
     * @return 交易账号
     */
    virtual int     get_trading_account_id() = 0;
    virtual int64_t get_local_id_prefix()    = 0;

    /* publish interface */
    /**
     * @brief 发布登录信息
     */
    virtual void publish_login() = 0;

    /**
     * @brief 发布交易所信息
     * @param en [IN] 交易所信息指针
     */
    virtual void publish_exchange(exchange_entity *en) = 0;

    /**
     * @brief 发布报单响应
     * @param en [IN] 报单响应信息指针
     */
    virtual void publish_rsp_order_insert(rsp_order_entity *en) = 0;

    /**
     * @brief 发布撤单响应
     * @param en [IN] 撤单响应信息指针
     */
    virtual void publish_rsp_order_cancel(rsp_order_entity *en) = 0;

    /**
     * @brief 发布报价响应
     * @param en [IN] 报价响应信息指针
     */
    virtual void publish_rsp_quote_insert(rsp_quote_entity *en) = 0;

    /**
     * @brief 发布报价撤销响应
     * @param en [IN] 报价撤销响应信息指针
     */
    virtual void publish_rsp_quote_cancel(rsp_quote_entity *en) = 0;

    /**
     * @brief 发布报单回报
     * @param en [IN] 报单回报指针
     */
    virtual void publish_rtn_order(rtn_order_entity *en) = 0;

    /**
     * @brief 发布报价回报
     * @param en [IN] 报价回报指针
     */
    virtual void publish_rtn_quote(rtn_quote_entity *en) = 0;

    /**
     * @brief 发布成交回报
     * @param en [IN] 成交回报指针
     */
    virtual void publish_rtn_trade(rtn_trade_entity *en) = 0;

    /**
     * @brief 发布询价回报
     * @param en [IN] 询价回报指针
     */
    virtual void publish_rtn_inquiry(inquiry_quote_entity *en) = 0;

    /**
     * @brief 发布合约状态回报
     * @param en [IN] 合约状态信息指针
     */
    virtual void publish_rtn_instrument_status(instrument_trading_status_entity *en, const bool arbi_extend = true) = 0;
    /**
     * @brief 发布组保/拆分成功回报
     * @param en [IN] 组保/拆分信息指针
     */
    virtual void publish_rtn_comb(rtn_comb_entity *en) = 0;
    /**
     * @brief 发布组保失败回报
     * @param en [IN] 组保失败信息指针
     */
    virtual void publish_rsp_comb_combine(rsp_comb_entity *en) = 0;
    /**
     * @brief 发布拆分失败回报
     * @param en [IN] 拆分信息信息指针
     */
    virtual void publish_rsp_comb_split(rsp_comb_entity *en) = 0;
    /**
     * @brief 发布组保单腿信息回报（证券交易所）
     * @param en [IN] 单腿信息指针
     */
    virtual void publish_comb_single_position(comb_single_position_entity *en) = 0;
    /**
     * @brief 发布组保单腿信息回报（期货交易所）
     * @param en [IN] 单腿信息指针
     */
    virtual void publish_comb_strategy_position(comb_strategy_position_entity *en) = 0;

    /**
     * @brief 发布投资者资金账户信息
     * @param en [IN] 资金账户信息指针
     */
    virtual void publish_investor_account(investor_account_fund_entity *en) = 0;

    /* response interface */
    /**
     * @brief 持仓查询响应
     * @param request_id [IN] 请求号
     * @param msg [IN] 持仓回报信息指针
     * @param is_last [IN] 是否是最后一个响应
     */
    virtual void response_qry_position(int                  request_id,
                                       rtn_position_entity *en      = NULL,
                                       bool                 is_last = true) = 0;

    /**
     * @brief 现货持仓查询响应
     * @param request_id [IN] 请求号
     * @param msg [IN] 持仓回报信息指针
     * @param is_last [IN] 是否是最后一个响应
     */
    virtual void response_qry_security_position(int                           request_id,
                                                rtn_security_position_entity *en      = NULL,
                                                bool                          is_last = true) = 0;

    /**
     * @brief 成交查询响应
     * @param request_id [IN] 请求号
     * @param msg [IN] 成交回报信息指针
     * @param is_last [IN] 是否是最后一个响应
     */
    virtual void response_qry_trade(int               request_id,
                                    rtn_trade_entity *en      = NULL,
                                    bool              is_last = true)                  = 0;
    virtual void response_qry_comb_detail(int              request_id,
                                          rtn_comb_entity *en      = NULL,
                                          bool             is_last = true)            = 0;
    virtual void response_qry_comb_single_position(int                          request_id,
                                                   comb_single_position_entity *en      = NULL,
                                                   bool                         is_last = true)   = 0;
    virtual void response_qry_comb_strategy_position(int                            request_id,
                                                     comb_strategy_position_entity *en = NULL,
                                                     bool is_last                      = true) = 0;

    /* async event*/
    virtual void post_void_function(const std::function<void ()>& f) = 0;

    // to delete, async dump move to fb_data_service
    // virtual void post_counter_biz_event(const char *buf, int len)  = 0;
    // virtual void post_local_order_event(uint64_t    order_id,
    //                                     uint64_t    local_id,
    //                                     bool        is_order,
    //                                     const char *sys_id,
    //                                     uint8_t     exchange_id,
    //                                     const char *insturment_id) = 0;

    // static void WRITE_ROUNDTRIP(rtn_order_entity *en, uint64_t node_id);
    // static void WRITE_ROUNDTRIP(rtn_quote_entity *en, uint64_t node_id);

    /*获取xml的item*/
    // virtual std::string get_attribute(const char *name, const char *path) = 0;
    /**
     * @brief 获取xml的item
     * @param name [IN] item名称
     * @param path [IN] xml文件路径
     * @return item配置信息
     */
    virtual char *get_attribute(const char *name, const char *path) = 0;
    virtual int   get_attribute_elements(const char *name,
                                         const char *path,
                                         OUT char (*elements)[MM_MAX_ELEMENT_LEN],
                                         OUT int &nCount)           = 0;

    static void    xlog(uint8_t log_model, uint8_t loglevel, const char *fmt, ...);
    static uint8_t get_xlog_level();

    virtual void set_curr_exchange_sec(const char &exchange_id,
                                       const int  &cur_sec,
                                       const int  &cur_msec = 0) = 0;
    virtual int  get_curr_exchange_sec(const char &exchange_id) = 0;
    virtual bool get_receive_outside()                          = 0;
    virtual bool get_system_status_running()                    = 0;

    virtual void    local_order_dump(uint64_t    order_id,
                                     uint64_t    local_id,
                                     bool        is_order,
                                     const char *sys_id,
                                     uint8_t     exchange_id,
                                     const char *insturment_id)     = 0;
    virtual void    counter_biz_dump(const char *buf, int len)      = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
