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
 * Date: 2018-09-15
 */

#ifndef FB_VOLATILITY_PARAM_CALLER_H
#define FB_VOLATILITY_PARAM_CALLER_H

#include <stdint.h>

#include "i_caller.h"
#include "volatility_param_stream.h"

namespace cffex {
namespace fb {

/**
 * @brief i_volatility_param_caller 波动率参数修改指令
 *
 * @ingroup i_volatility_param_caller
 */
class i_volatility_param_caller :
    public i_caller_with_id<i_volatility_param_caller, STRATEGY_CALLER_VOLATILITY_PARAM> {
public:
    /**
     * @brief
     * volatility_param_entity接口类抽象了期权系列波动率参数设置结构，提供其操作界面；策略代码必须调用create_entity创建实例对象
     *        volatility_param_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用modify_volatility_param完成报价后，无需删除volatility_param_entity对象
     *        - 其生命周期是两次调用create_entity之间的时间
     *        - 调用create_empty_entity返回的是重置后的对象
     *        - 调用create_entity返回的是存有当前值的对象
     */
    class volatility_param_entity {
    public:
        virtual ~volatility_param_entity() = default;
        /// 设置期权系列编号
        virtual void set_option_serial_index(uint64_t v) = 0;
        /// 设置期权系列号
        virtual void set_option_serial_id(const char *v) = 0;
        /// 设置波动率算法id
        virtual void set_volatility_algorithm_id(int16_t v) = 0;
        /// 设置拟合合约类型， 0-全部合约，1-看涨合约，2-看跌合约，3-虚值合约
        virtual void set_fit_contract_type(int8_t v) = 0;
        /**
         * @brief 设置double类型参数
         * @param param_name 参数名称
         * @param param_value 参数值
         */
        virtual void set_param(IN const char *param_name, IN double param_value) = 0;
        /**
         * @brief 设置字符串类型参数
         * @param param_name 参数名称
         * @param param_value 参数值
         */
        virtual void set_param(IN const char *param_name, IN const char *param_value) = 0;
        /**
         * @brief 设置整数类型参数
         * @param param_name 参数名称
         * @param param_value 参数值
         */
        virtual void set_param(IN const char *param_name, IN int param_value) = 0;
        /**
         * @brief 设置double类型数组参数
         * @param param_name 参数名称
         * @param size 数组长度
         * @param param_value 参数值数组
         */
        virtual bool set_param(IN const char *param_name, IN int size, IN double *param_value) = 0;
        /**
         * @brief 设置字符串类型数组参数
         * @param param_name 参数名称
         * @param size 字符数组中字符串个数
         * @param param_value 字符串数组，每个字符串最长512字节(包括结束的'\0')
         */
        virtual bool set_param(IN const char *param_name,
                               IN int         size,
                               IN const char  param_value[][512]) = 0;
        /**
         * @brief 设置整数类型数组参数
         * @param param_name 参数名称
         * @param size 数组长度
         * @param param_value 参数值数组
         */
        virtual bool set_param(IN const char *param_name, IN int size, IN int *param_value) = 0;

        virtual void reset_entity() = 0;
    };

    i_volatility_param_caller()          = default;
    virtual ~i_volatility_param_caller() = default;

    /**
     * @brief 创建空的修改结构
     * @return 空的期权系列波动率参数修改指针
     */
    virtual volatility_param_entity *create_empty_entity() = 0;

    /**
     * @brief 创建空的修改结构
     * @param option_serial_id 期权系列id
     * @param volatility_algorithm_id 波动率算法id
     * @param fit_contract_type 拟合合约类型
     * @param option_serial_index 期权系列index
     * @return 带有当前值的期权系列波动率参数修改结构指针
     */
    virtual volatility_param_entity *create_entity(IN const char   *option_serial_id,
                                                   IN const int16_t volatility_algorithm_id,
                                                   IN const int8_t  fit_contract_type) = 0;
    virtual volatility_param_entity *create_entity(IN const uint64_t option_serial_index,
                                                   IN const int16_t  volatility_algorithm_id,
                                                   IN const int8_t   fit_contract_type) = 0;

    /**
     * @brief 执行期权系列波动率参数修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_volatility_param(volatility_param_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
