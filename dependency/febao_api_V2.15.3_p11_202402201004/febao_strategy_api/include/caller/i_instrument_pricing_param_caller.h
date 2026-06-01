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

#ifndef FB_INSTRUMENT_PRICING_PARAM_CALLER_H
#define FB_INSTRUMENT_PRICING_PARAM_CALLER_H

#include <stdint.h>

#include "i_caller.h"
#include "instrument_pricing_param_config_stream.h"

namespace cffex {
namespace fb {

/**
 * @brief i_instrument_pricing_param_caller 接口类提供了理论价定价参数修改指令
 *        - 理论价定价参数指的是在客户端 定价 -> 定价参数 中设置的参数
 *
 * 理论价定价公式如下，以BSM定价模型为例：
 * raw.forward.p = baseprice * basemulti + basis
 * forward.p = raw.forward.p + baseoffset
 * rawtheoprice = BSM(forward.p, strike, theo.vol, rate, natural.days)
 * theoprice = rawtheoprice + theooffset
 *
 * @ingroup i_instrument_pricing_param_caller
 */
class i_instrument_pricing_param_caller :
    public i_caller_with_id<i_instrument_pricing_param_caller,
                            STRATEGY_CALLER_INSTRUMENT_PRICING_PARAM> {
public:
    /**
     * @brief
     * instrument_pricing_param_entity接口类抽象了合约理论价定价参数修改结构；策略代码必须调用create_empty_entity或者create_entity来创建其实例对象
     *        instrument_pricing_param_entity对象的生命周期由策略进程管理：
     *        -
     * 策略代码调用modify_pricing_param完成功能后，无需删除instrument_pricing_param_entity对象
     *        - 其生命周期是两次调用modify_pricing_param之间的时间
     *        - 调用create_empty_entity返回的是重置后的对象
     *        - 调用create_entity返回的是存有当前值的对象
     */
    class instrument_pricing_param_entity {
    public:
        virtual ~instrument_pricing_param_entity() = default;
        /// 设置需要修改定价参数的合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 设置需要修改定价参数的合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 设置基差值
        virtual void set_basis(double v) = 0;
        /// 设置价格乘数
        virtual void set_base_multi(double v) = 0;
        /// 设置价格偏移
        virtual void set_base_offset(double v) = 0;
        /// 重置基差值
        virtual void reset_basis() = 0;
        /// 重置价格乘数
        virtual void reset_base_multi() = 0;
        /// 重置价格偏移
        virtual void reset_base_offset() = 0;
        virtual void reset_entity()      = 0;
    };

    i_instrument_pricing_param_caller()          = default;
    virtual ~i_instrument_pricing_param_caller() = default;

    /**
     * @brief 创建空的修改结构
     * @return 空的定价参数修改结构指针
     */
    virtual instrument_pricing_param_entity *create_empty_entity() = 0;

    /**
     * @brief 创建空的修改结构
     * @return 带有当前值的定价参数修改结构指针
     */
    virtual instrument_pricing_param_entity *create_entity(IN const char *instrument_id)       = 0;
    virtual instrument_pricing_param_entity *create_entity(IN const uint64_t instrument_index) = 0;

    /**
     * @brief 执行定价参数修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_pricing_param(instrument_pricing_param_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
