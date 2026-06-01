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

#ifndef FB_SERIAL_PRICING_PARAM_CALLER_H
#define FB_SERIAL_PRICING_PARAM_CALLER_H

#include <stdint.h>

#include "i_caller.h"
#include "serial_pricing_param_config_stream.h"

namespace cffex {
namespace fb {

/**
 * @brief i_serial_pricing_param_caller 设置期权系列定价参数
 *
 * @ingroup i_serial_pricing_param_caller
 */
class i_serial_pricing_param_caller :
    public i_caller_with_id<i_serial_pricing_param_caller, STRATEGY_CALLER_SERIAL_PRICING_PARAM> {
public:
    /**
     * @brief
     * serial_pricing_param_entity接口类抽象了期权系列基准价设置结构，提供其操作界面；策略代码必须调用create_entity创建实例对象
     *        serial_pricing_param_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用modify_pricing_param完成报价后，无需删除serial_pricing_param_entity对象
     *        - 其生命周期是两次调用create_entity之间的时间
     *        - 调用create_empty_entity返回的是重置后的对象
     *        - 调用create_entity返回的是存有当前值的对象
     */
    class serial_pricing_param_entity {
    public:
        virtual ~serial_pricing_param_entity() = default;
        /// 设置期权系列编号
        virtual void set_option_serial_index(uint64_t v) = 0;
        /// 设置期权系列
        virtual void set_option_serial_id(const char *v) = 0;
        /// 设置基差值
        virtual void set_basis(double v) = 0;
        /// 设置价格乘数
        virtual void set_base_multi(double v) = 0;
        /// 设置价格偏移
        virtual void set_base_offset(double v) = 0;
        virtual void reset_entity()            = 0;
    };

    i_serial_pricing_param_caller()          = default;
    virtual ~i_serial_pricing_param_caller() = default;

    /**
     * @brief 创建空的修改结构
     * @return 空的期权系列定价参数修改结构指针
     */
    virtual serial_pricing_param_entity *create_empty_entity() = 0;

    /**
     * @brief 创建空的修改结构
     * @return 带有当前值的期权系列定价参数修改结构指针
     */
    virtual serial_pricing_param_entity *create_entity(IN const char *option_serial_id)       = 0;
    virtual serial_pricing_param_entity *create_entity(IN const uint64_t option_serial_index) = 0;

    /**
     * @brief 执行期权系列定价参数修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_pricing_param(serial_pricing_param_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
