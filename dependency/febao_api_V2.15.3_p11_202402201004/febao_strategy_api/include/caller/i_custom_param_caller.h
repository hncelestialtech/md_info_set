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

#ifndef FB_CUSTOM_PARAM_CALLER_H
#define FB_CUSTOM_PARAM_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_custom_param_caller接口类提供了自定义参数修改指令
 *        - 自定义参数指的是在客户端 交易 -> 策略参数 -> 自定义参数 中设置的参数
 *        - 调用指令设置自定义参数，必须是已经通过客户端设置过在系统中已经存在的参数
 *        - 调用指令设置自定义参数，必须是符合自定义参数的类型
 *
 * @ingroup i_custom_param_caller
 */
class i_custom_param_caller :
    public i_caller_with_id<i_custom_param_caller, STRATEGY_CALLER_CUSTOM_PARAM> {
public:
    i_custom_param_caller()          = default;
    virtual ~i_custom_param_caller() = default;

    /**
     * @brief 设置整数类型的自定义参数
     * @param custom_id 参数级别id
     * @param param_key 参数名
     * @param param_value 参数值
     * @return bool 成功返回true，失败返回false
     */
    virtual bool set_param(IN const char *custom_id,
                           IN const char *param_key,
                           IN int         param_value) = 0;

    /**
     * @brief 设置双精度浮点类型的自定义参数
     * @param custom_id 参数级别id
     * @param param_key 参数名
     * @param param_value 参数值
     * @return bool 成功返回true，失败返回false
     */
    virtual bool set_param(IN const char *custom_id,
                           IN const char *param_key,
                           IN double      param_value) = 0;

    /**
     * @brief 设置字符串类型的自定义参数
     * @param custom_id 参数级别id
     * @param param_key 参数名
     * @param param_value 参数值
     * @return bool 成功返回true，失败返回false
     */
    virtual bool set_param(IN const char *custom_id,
                           IN const char *param_key,
                           IN const char *param_value) = 0;

    /**
     * @brief 设置布尔类型的自定义参数
     * @param custom_id 参数级别id
     * @param param_key 参数名
     * @param param_value 参数值
     * @return bool 成功返回true，失败返回false
     */
    virtual bool set_param(IN const char *custom_id,
                           IN const char *param_key,
                           IN bool        param_value) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
