/**
 * CFFEX Confidential.
 *
 * @Copyright 2021 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty&zhengmj
 * Date: 2021-03-11
 */

#ifndef FB_VOLATILITY_ALGORITHM_H
#define FB_VOLATILITY_ALGORITHM_H

#include "volatility_common.h"

namespace cffex {
namespace fb {

// double value for null
#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

class i_volatility_algorithm {
public:
    virtual ~i_volatility_algorithm() {}

    /**
     * @brief 获取波动率参数列表（参数名和参数类型）
     * @param fixed_param_list [OUT] 静态参数列表
     * @param variable_param_list [OUT] 动态参数列表
     */
    virtual void get_param_list(OUT char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH],
                                OUT char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]) = 0;

    /**
     * @brief 获取波动率参数（参数值）
     * @param fixed_model_param [OUT] 静态模型参数
     * @param variable_model_param [OUT] 动态模型参数
     */
    virtual void get_param_value(
        OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
        OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) = 0;

    /**
     * @brief 设置波动率参数
     * @param fixed_model_param [IN] 静态模型参数
     * @param variable_model_param [IN] 动态模型参数
     */
    virtual int set_param_value(
        IN const char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
        IN const char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) = 0;

    /**
     * @brief 判断是否动态波动率模型
     */
    virtual bool is_dynamic_model() = 0;

    /**
     * @brief 获取波动率自定义参数
     * @param volatility_custom_param_name [OUT] 波动率自定义参数
     */
    virtual void get_volatility_custom_param_name(
        OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]) = 0;

    /**
     * @brief 理论波动率计算
     * @param strike_price [IN] 执行价
     * @param atm_forward [IN] 平值价
     * @param option_forward [IN] 远期价
     * @param left_trading_years [IN] 上一个交易年
     * @param rate [IN] 比率
     * @param volatility [OUT] 波动率
     * @param volatility_custom_param_value [OUT] 波动率自定义参数
     * @return 计算是否成功，false 计算失败，true 计算成功
     */
    virtual bool get_volatility(
        IN const double strike_price,
        IN const double atm_forward,
        IN const double option_forward,
        IN const double left_trading_years,
        IN const double rate,
        OUT double     *volatility,
        OUT double      volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]) = 0;

    /**
     * @brief 波动率拟合，用于中台波动率服务
     * @param p_volatility_points 波动率点
     * @param size [IN] 数量
     * @param atm_forward [IN] 平值价
     * @param left_trading_years [IN] 上一个交易年
     * @param rate [IN] 比率
     * @param fixed_model_param [IN] 静态模型参数
     * @param variable_model_param [OUT] 动态模型参数
     * @return 计算是否成功，-1 拟合失败，0 拟合成功
     */
    virtual int fit(IN const volatility_point *p_volatility_points,
                    IN const unsigned int      size,
                    IN const double            atm_forward,
                    IN const double            left_trading_years,
                    IN const double            rate,
                    OUT char                   fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
                    OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
