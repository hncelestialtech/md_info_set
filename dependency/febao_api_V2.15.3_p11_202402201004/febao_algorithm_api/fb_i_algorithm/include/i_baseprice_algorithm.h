/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: zhr
 * Date: 2018-03-08
 */

#ifndef FB_I_BASEPRICE_ALGORITHM_H
#define FB_I_BASEPRICE_ALGORITHM_H

#include <cstdint>
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {

// double value for null
#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

/* instrument_type */
enum {
    // 期货合约
    FB_INSTRUMENT_FUTURE = '0',
    // 期权合约
    FB_INSTRUMENT_OPTION = '1',
    // 现货合约
    FB_INSTRUMENT_SECURITY = '2',
    // 套利合约
    FB_INSTRUMENT_ARBITRAGE = '3',
    FB_INSTRUMENT_TYPE_ALL  = '9'
};

/* option_type */
enum {
    // 看涨期权
    FB_OPTION_CALL = 'c',
    // 看跌期权
    FB_OPTION_PUT = 'p',
};

/* exercise_style */
enum {
    // 美式
    FB_EXERCISE_STYLE_AMERICAN = '0',
    // 欧式
    FB_EXERCISE_STYLE_EUROPEAN = '1'
};

/* sub_stitute_flag */
enum {
    // 沪市禁止现金替代
    PLUGIN_SH_NO_CASH = '0',
    // 沪市可以进行现金替代
    PLUGIN_SH_CASH_ALLOW = '1',
    // 沪市必须用现金替代
    PLUGIN_SH_CASH_ONLY = '2',
    // 深市退补现金替代
    PLUGIN_SZ_CASH_ALLOW = '3',
    // 深市必须用现金替代
    PLUGIN_SZ_CASH_ONLY = '4',
    // 非深沪市退补现金替代
    PLUGIN_ELSE_CASH_ALLOW = '5',
    // 非深沪市必须用现金替代
    PLUGIN_ELSE_CASH_ONLY = '6',
    // 港市退补现金替代
    PLUGIN_HK_CASH_ALLOW = '7',
    // 港市必须用现金替代
    PLUGIN_HK_CASH_ONLY = '8',
    // 未配置
    PLUGIN_CASH_UNKNOWN = 'n'
};

/* arbi_type */
enum {
    // 期货跨期
    FB_COMB_STRATEGY_SP = '6',
    // 期货跨品种
    FB_COMB_STRATEGY_SPC = '7',
};

class i_baseprice_algorithm {
public:
    class md_field {
    public:
        virtual ~md_field() {}
        /// 获取合约编号
        virtual const uint64_t get_instrument_index() const = 0;
        /// 获取合约号
        virtual const char *get_instrument_id() const = 0;
        /// 获取更新时间：秒
        virtual const int32_t get_update_sec() const = 0;
        /// 获取更新时间：毫秒
        virtual const int32_t get_update_msec() const = 0;
        /// 获取昨收盘价
        virtual const double get_pre_close() const = 0;
        /// 获取开盘价
        virtual const double get_open() const = 0;
        /// 获取收盘价
        virtual const double get_close() const = 0;
        /// 获取涨停价
        virtual const double get_upper_limit_price() const = 0;
        /// 获取跌停价
        virtual const double get_down_limit_price() const = 0;
        /// 获取最新价
        virtual const double get_last_price() const = 0;
        /// 获取数量
        virtual const int32_t get_volume() const = 0;
        /// 获取成交额
        virtual const double get_turn_over() const = 0;
        /// 获取持仓量
        virtual const double get_open_interest() const = 0;
        /// 获取买一价
        virtual const double get_bid1_price() const = 0;
        /// 获取卖一价
        virtual const double get_ask1_price() const = 0;
        /// 获取买一量
        virtual const int32_t get_bid1_volume() const = 0;
        /// 获取卖一量
        virtual const int32_t get_ask1_volume() const = 0;
        /// 获取买二价
        virtual const double get_bid2_price() const = 0;
        /// 获取卖二价
        virtual const double get_ask2_price() const = 0;
        /// 获取买二量
        virtual const int32_t get_bid2_volume() const = 0;
        /// 获取卖二量
        virtual const int32_t get_ask2_volume() const = 0;
        /// 获取买三价
        virtual const double get_bid3_price() const = 0;
        /// 获取卖三价
        virtual const double get_ask3_price() const = 0;
        /// 获取买三量
        virtual const int32_t get_bid3_volume() const = 0;
        /// 获取卖三量
        virtual const int32_t get_ask3_volume() const = 0;
        /// 获取买四价
        virtual const double get_bid4_price() const = 0;
        /// 获取卖四价
        virtual const double get_ask4_price() const = 0;
        /// 获取买四量
        virtual const int32_t get_bid4_volume() const = 0;
        /// 获取卖四量
        virtual const int32_t get_ask4_volume() const = 0;
        /// 获取买五价
        virtual const double get_bid5_price() const = 0;
        /// 获取卖五价
        virtual const double get_ask5_price() const = 0;
        /// 获取买五量
        virtual const int32_t get_bid5_volume() const = 0;
        /// 获取卖五量
        virtual const int32_t get_ask5_volume() const = 0;
        /// 获取买六价
        virtual const double get_bid6_price() const = 0;
        /// 获取卖六价
        virtual const double get_ask6_price() const = 0;
        /// 获取买六量
        virtual const int32_t get_bid6_volume() const = 0;
        /// 获取卖六量
        virtual const int32_t get_ask6_volume() const = 0;
        /// 获取买七价
        virtual const double get_bid7_price() const = 0;
        /// 获取卖七价
        virtual const double get_ask7_price() const = 0;
        /// 获取买七量
        virtual const int32_t get_bid7_volume() const = 0;
        /// 获取卖七量
        virtual const int32_t get_ask7_volume() const = 0;
        /// 获取买八价
        virtual const double get_bid8_price() const = 0;
        /// 获取卖八价
        virtual const double get_ask8_price() const = 0;
        /// 获取买八量
        virtual const int32_t get_bid8_volume() const = 0;
        /// 获取卖八量
        virtual const int32_t get_ask8_volume() const = 0;
        /// 获取买九价
        virtual const double get_bid9_price() const = 0;
        /// 获取卖九价
        virtual const double get_ask9_price() const = 0;
        /// 获取买九量
        virtual const int32_t get_bid9_volume() const = 0;
        /// 获取卖九量
        virtual const int32_t get_ask9_volume() const = 0;
        /// 获取买十价
        virtual const double get_bid10_price() const = 0;
        /// 获取卖十价
        virtual const double get_ask10_price() const = 0;
        /// 获取买十量
        virtual const int32_t get_bid10_volume() const = 0;
        /// 获取卖十量
        virtual const int32_t get_ask10_volume() const = 0;
        /// 获取基金净值
        virtual const double get_iopv() const = 0;
        /// 获取本地时间
        virtual const int64_t get_local_timestamp() const = 0;
    };

    class instrument_field {
    public:
        virtual ~instrument_field() {}
        /// 获取合约编号
        virtual const uint64_t get_instrument_index() const = 0;
        /// 获取合约号
        virtual const char *get_instrument_id() const = 0;
        /// 获取合约类型
        virtual const int8_t get_instrument_type() const = 0;
        /// 获取期权类型
        virtual const int8_t get_option_type() const = 0;
        /// 获取行权方式
        virtual const int8_t get_exercise_style() const = 0;
        /// 获取合约乘数
        virtual const double get_instrument_multiple() const = 0;
        /// 获取合约tick
        virtual const double get_instrument_tick() const = 0;
        /// 获取合约行权价
        virtual const double get_instrument_strike_price() const = 0;
        /// 获取品种编号
        virtual const uint64_t get_product_index() const = 0;
        /// 获取品种号
        virtual const char *get_product_id() const = 0;
        /// 获取期权系列编号
        virtual const uint64_t get_option_serial_index() const = 0;
        /// 获取期权系列号
        virtual const char *get_option_serial_id() const = 0;
        /// 获取标的合约编号
        virtual const uint64_t get_underlying_instrument_index() const = 0;
        /// 获取标的合约号
        virtual const char *get_underlying_instrument_id() const = 0;
        /// 获取行权日
        virtual const char *get_expire_date() const = 0;
        /// 获取到期日剩余时间
        virtual const int32_t get_left_trading_day() const = 0;
        // 获取最小申购/赎回单位
        virtual const double get_creation_redemption_unit() const = 0;
        // 获取预估现金差额
        virtual const double get_estimate_cash_component() const = 0;
        // 获取套利策略，0-认购牛市价差，1-认购熊市价差，2-认沽牛市价差，3-认沽熊市价差，4-跨式空头，5-宽跨式空头，6-期货跨期，7-期货跨品种，8-期货对锁，9-期权对锁，a-期权跨式，b-期权宽跨式，c-买入垂直价差，d-卖出垂直价差，e-买入期权期货组合，f-卖出期权期货组合，n-未知
        virtual const int8_t get_arbi_type() const = 0;
        /// 获取腿一合约编号
        virtual const uint64_t get_leg1_instrument_index() const = 0;
        /// 获取腿一合约
        virtual const char *get_leg1_instrument_id() const = 0;
        /// 获取腿二合约编号
        virtual const uint64_t get_leg2_instrument_index() const = 0;
        /// 获取腿二合约
        virtual const char *get_leg2_instrument_id() const = 0;
    };

    class serial_field {
    public:
        /// 获取产品编号
        virtual const uint64_t get_product_index() const               = 0;
        /// 获取产品号
        virtual const char    *get_product_id() const                  = 0;
        /// 获取期权系列编号
        virtual const uint64_t get_option_serial_index() const         = 0;
        /// 获取期权系列
        virtual const char    *get_option_serial_id() const            = 0;
        /// 获取系列乘数
        virtual const double   get_serial_multiple() const             = 0;
        /// 获取系列tick
        virtual const double   get_serial_tick() const                 = 0;
        /// 获取行权类型
        virtual const int8_t   get_exercise_style() const              = 0;
        /// 获取标的合约编号
        virtual const uint64_t get_underlying_instrument_index() const = 0;
        /// 获取标的合约号
        virtual const char    *get_underlying_instrument_id() const    = 0;
        /// 获取行权日
        virtual const char    *get_expire_date() const                 = 0;
    };

    class fund_component_field {
    public:
        virtual ~fund_component_field() {}
        /// 获取合约编号
        virtual const uint64_t get_instrument_index() const           = 0;
        /// 获取合约号
        virtual const char    *get_instrument_id() const              = 0;
        /// 获取现金替代标志，0-沪市禁止现金替代，1-沪市可以进行现金替代，2-沪市必须用现金替代，3-深市退补现金替代，4-深市必须用现金替代，5-非深沪市退补现金替代，6-非深沪市必须用现金替代，7-港市退补现金替代，8-港市必须用现金替代，n-未配置
        virtual const int8_t   get_sub_stitute_flag() const           = 0;
        /// 获取成份证券数
        virtual const int64_t  get_component_share() const            = 0;
        /// 获取申购替代金额
        virtual const double   get_creation_cash_substitute() const   = 0;
        /// 获取赎回替代金额
        virtual const double   get_redemption_cash_substitute() const = 0;
    };

    class instrument_md_field {
    public:
        virtual ~instrument_md_field() {}
        /// 获取行情结构体
        virtual const md_field         *get_md_field() const          = 0;
        /// 获取合约结构体
        virtual const instrument_field *get_instrument_field() const  = 0;
        /// 获取剩余交易时间
        virtual const double            get_left_trading_time() const = 0;
    };

    class component_md_field {
    public:
        virtual ~component_md_field() {}
        /// 获取行情结构体
        virtual const md_field             *get_md_field() const             = 0;
        /// 获取合约结构体
        virtual const instrument_field     *get_instrument_field() const     = 0;
        /// 获取基金结构体
        virtual const fund_component_field *get_fund_component_field() const = 0;
    };

    class instrument_md_field_container {
    public:
        virtual ~instrument_md_field_container() {}
        /// 根据合约编号查询合约行情结构体
        virtual const instrument_md_field *get(IN const uint64_t instrument_index) const = 0;
        /// 根据合约号查询合约行情结构体
        virtual const instrument_md_field *get(IN const char *instrument_id) const       = 0;
        /// 遍历合约行情结构体
        virtual const instrument_md_field *first() const                                 = 0;
        virtual const instrument_md_field *next() const                                  = 0;
        virtual const instrument_md_field *get() const                                   = 0;
        /// 获取当前行情结构体
        virtual const md_field            *get_update_md_field() const                   = 0;
        /// 获取标的合约个数
        virtual unsigned int               size() const                                  = 0;
    };

    class component_md_field_container {
    public:
        virtual ~component_md_field_container() {}
        /// 根据合约编号查询基金成分合约行情结构体
        virtual const component_md_field *get(IN const uint64_t instrument_index) const = 0;
        /// 根据合约号查询基金成分合约行情结构体
        virtual const component_md_field *get(IN const char *instrument_id) const       = 0;
        /// 遍历基金成分合约行情结构体
        virtual const component_md_field *first() const                                 = 0;
        virtual const component_md_field *next() const                                  = 0;
        /// 获取基金标的合约个数
        virtual unsigned int              size() const                                  = 0;
    };

public:
    virtual ~i_baseprice_algorithm() {}
    /**
     * @brief 计算期货、期权，现货（除基金）合约的基准价，可通过algo_id选择客户自定义的一种的基准价算法进行计算。
     * @param algo_id [IN] 算法id,由客户自己定义，可以通过该参数选择对应的基准价算法
     * @param target_instrument [IN] 目标合约
     * @param mds [IN] 基准合约行情表，每条记录包含了一条基准合约的行情数据及基准合约数据
     * @param baseprice [OUT] 基准价，计算出的基准价通过该参数输出
     * @return 计算是否成功，-1计算失败，0计算成功
     */
    virtual int calculate(unsigned short                          algo_id,
                          IN instrument_field                    *target_instrument,
                          IN const instrument_md_field_container *mds,
                          OUT double                             *baseprice) {
        return 0;
    };

    /**
     * @brief 计算期货、期权，现货（除基金）合约的基准价，可通过algo_id选择客户自定义的一种的基准价算法进行计算。
     * @param algo_id [IN] 算法id,由客户自己定义，可以通过该参数选择对应的基准价算法
     * @param target_serial [IN] 目标系列
     * @param mds [IN] 基准合约行情表，每条记录包含了一条基准合约的行情数据及基准合约数据
     * @param baseprice [OUT] 基准价，计算出的基准价通过该参数输出
     * @return 计算是否成功，-1计算失败，0计算成功
     */
    virtual int calculate(unsigned short                          algo_id,
                          IN serial_field                        *target_serial,
                          IN const instrument_md_field_container *mds,
                          OUT double                             *baseprice) {
        return 0;
    };

    /**
     * @brief 计算基金合约的基准价，可通过algo_id选择客户自定义的一种基准价算法进行计算。
     * @param algo_id [IN] 算法id,由客户自己定义，可以通过该参数选择对应的基准价算法
     * @param target_fund_instrument [IN] 基金合约信息
     * @param mds [IN] 基准合约行情表，每条记录包含了一条基准合约的行情数据及基准合约数据
     * @param component_mds [IN] 成分股合约行情表，每条记录包含了一条成分股合约的行情数据及基准合约数据
     * @param baseprice [OUT] 基准价，计算出的基准价通过该参数输出
     * @return 计算是否成功，-1计算失败，0计算成功
     */
    virtual int calculate(unsigned short                          algo_id,
                          IN instrument_field                    *target_fund_instrument,
                          IN const instrument_md_field_container *mds,
                          IN const component_md_field_container  *component_mds,
                          OUT double                             *baseprice) {
        return 0;
    };
};

}  // namespace fb
}  // namespace cffex

#endif
