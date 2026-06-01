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

#ifndef FB_THEORETICAL_ALGORITHM_H
#define FB_THEORETICAL_ALGORITHM_H

// #include "mm_plugin_struct.h"
#include <stdint.h>

#include <functional>
#include <vector>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {

#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

#define MAX_CUSTOM_GREEK_NAME_LEN 256
#define MAX_CUSTOM_NAME_LEN 512

class i_theoretical_algorithm {
public:
    enum name { PLUGIN_OPTION_CALL = 'c', PLUGIN_OPTION_PUT = 'p' };
    class theoretical_variable_param {
    public:
        virtual ~theoretical_variable_param() {}
        /// 获取远期价
        virtual double get_forward() const = 0;
        /// 获取行权价
        virtual double get_strike() const = 0;
        /// 获取剩余自然日
        virtual double get_left_natural_years() const = 0;
        /// 获取剩余交易日
        virtual double get_left_trading_years() const = 0;
        /// 获取利率
        virtual double get_rate() const = 0;
        /// 是否是交易日
        virtual bool is_tradingday() const = 0;
        /// 获取总的交易日
        virtual int get_total_tradingday() const = 0;
        /// 获取合约编号
        virtual uint64_t get_instrument_index() const = 0;
        /// 获取合约号
        virtual const char *get_instrument_id() const = 0;
        /// 获取多头保证金费率按比例
        virtual double get_long_margin_rate() const = 0;
        /// 获取多头保证金费率按手数
        virtual double get_long_margin_amt() const = 0;
        /// 空头保证金费率按比例
        virtual double get_short_margin_rate() const = 0;
        /// 空头保证金费率按手数
        virtual double get_short_margin_amt() const = 0;
    };

    class theoretical_volatility_param {
    public:
        virtual ~theoretical_volatility_param() {}
        /// 获取波动率
        virtual double get_volatility() const = 0;
        /// 设置隐含波动率
        virtual void set_imply_volatility(double iv) {}
        /// 获取自定义参数
        virtual bool get_custom_param(IN const char *param_name, OUT double *param_value) const = 0;
    };

    class theoretical_static_param {
    public:
        virtual ~theoretical_static_param() {}
        /// 获取参数，参数值为int型
        virtual bool get_param(IN const char *param_name, OUT int *param_value) const = 0;
        /// 获取参数，参数值为double型
        virtual bool get_param(IN const char *param_name, OUT double *param_value) const = 0;
        /// 获取参数，参数值为字符串型
        virtual bool get_param(IN const char *param_name, OUT char param_value[512]) const = 0;
    };

    class theoretical_custom_param {
    public:
        virtual ~theoretical_custom_param() {}
        /// 获取参数，参数值为double型
        virtual bool get_param(IN const char *param_name, OUT double *param_value) const = 0;
    };

    class theoretical_field {
    public:
        virtual ~theoretical_field() {}
        // PriceEngine will use last updated value if not set this time
        /// 设置理论价
        virtual void set_theoretical_price(double v) = 0;
        /// 设置delta
        virtual void set_delta(double v) = 0;
        /// 设置gamma
        virtual void set_gamma(double v) = 0;
        /// 设置theta
        virtual void set_theta(double v) = 0;
        /// 设置vega
        virtual void set_vega(double v) = 0;
        /// 设置rho
        virtual void set_rho(double v) = 0;
        /// 设置charm
        virtual void set_charm(double v) = 0;
        /// 设置vanna
        virtual void set_vanna(double v) = 0;
        /// 设置vomma
        virtual void set_vomma(double v) = 0;
        /// 设置speed
        virtual void set_speed(double v) = 0;
        /// 设置zomma
        virtual void set_zomma(double v) = 0;
        /// 设置skew_delta
        virtual void set_skew_delta(double v) = 0;
        /// 设置自定义希腊字母
        virtual void set_custom_greeks(std::vector<double> &custom_greeks) = 0;
    };

    typedef std::function<void()> timer_function;
    typedef std::function<void(uint32_t milsec, timer_function f, bool repeat)>
        register_timer_callback;

public:
    virtual ~i_theoretical_algorithm() {}
    /**
     * @brief 设置定时器回调函数
     * @param f [IN] 定时器回调函数
     */
    virtual void set_register_timer_function(register_timer_callback f) {}
    /**
     * @brief 加载回调函数
     */
    virtual void on_loaded() {}
    // for setting custom greek list
    virtual void get_custom_greek_list(OUT char custom_greek_list[MAX_CUSTOM_GREEK_NAME_LEN]) {}
    // for setting custom param list
    virtual void get_custom_param_list(OUT char custom_param_list[MAX_CUSTOM_NAME_LEN]) {}
    // for theoPrice & full greeks calculation
    /**
     * @brief 用于计算理论价
     * @param option_type [IN] 期权类型，'c' 认购期权，'p'认估期权
     * @param variable_params [IN] 动态参数
     * @param static_params [IN] 静态参数（模型参数）
     * @param custom_params [IN] 自定义参数
     * @param volatility_params [IN] 波动率参数
     * @param f [OUT] 理论价及希腊字母
     * @return 计算是否成功，-1 计算失败，0 计算成功
     */
    virtual int calculate(IN unsigned short                      option_type,
                          IN const theoretical_variable_param   *variable_params,
                          IN const theoretical_static_param     *static_params,
                          IN const theoretical_custom_param     *custom_params,
                          IN const theoretical_volatility_param *volatility_params,
                          OUT theoretical_field                 *f) = 0;

    // for iv calculation only, output vega is used by NewTon iteration
    /**
     * @brief 用于计算理论价，用于策略试算
     * @param option_type [IN] 期权类型，'c' 认购期权，'p'认估期权
     * @param variable_params [IN] 动态参数
     * @param static_params [IN] 静态参数（模型参数）
     * @param custom_params [IN] 自定义参数
     * @param volatility_params [IN] 波动率参数
     * @param price [OUT] 理论价
     * @param vega [OUT] 希腊字母vega
     * @return 计算是否成功，-1 计算失败，0 计算成功
     */
    virtual int calculate(IN unsigned short                      option_type,
                          IN const theoretical_variable_param   *variable_params,
                          IN const theoretical_static_param     *static_params,
                          IN const theoretical_custom_param     *custom_params,
                          IN const theoretical_volatility_param *volatility_params,
                          OUT double                            *price,
                          OUT double                            *vega) = 0;

    // for theoretical price trial in strategy
    /**
     * @brief 用于计算理论价，用于策略试算
     * @param option_type [IN] 期权类型，'c' 认购期权，'p'认估期权
     * @param variable_params [IN] 动态参数
     * @param static_params [IN] 静态参数（模型参数）
     * @param custom_params [IN] 自定义参数
     * @param volatility_params [IN] 波动率参数
     * @param price [OUT] 理论价
     * @return 计算是否成功，-1 计算失败，0 计算成功
     */
    virtual int calculate(IN unsigned short                      option_type,
                          IN const theoretical_variable_param   *variable_params,
                          IN const theoretical_static_param     *static_params,
                          IN const theoretical_custom_param     *custom_params,
                          IN const theoretical_volatility_param *volatility_params,
                          OUT double                            *price) = 0;

    /**
     * @brief 计算iv(bid1.iv, ask1.iv, last.iv, mid.iv) 用于中台波动率服务
     * @param option_type [IN] 期权类型，'c' 认购期权，'p'认估期权
     * @param option_price [IN] 期权价格
     * @param variable_params [IN] 动态参数
     * @param static_params [IN] 静态参数（模型参数）
     * @return 计算是否成功，-1 计算失败，0 计算成功
     */
    virtual int iv_derivable(IN unsigned short                    option_type,
                             IN double                            option_price,
                             IN const theoretical_variable_param *variable_params,
                             IN const theoretical_static_param   *static_params) = 0;

    // TODO 2.6.1
    // virtual int   iv_derivable(IN unsigned short option_type, IN double
    // option_price, IN const theoretical_variable_param *variable_params, IN
    // const theoretical_static_param *static_params) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
