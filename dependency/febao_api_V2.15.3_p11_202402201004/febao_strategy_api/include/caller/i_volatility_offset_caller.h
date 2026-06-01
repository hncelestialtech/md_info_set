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

#ifndef FB_VOLATILITY_OFFSET_CALLER_H
#define FB_VOLATILITY_OFFSET_CALLER_H

#include <stdint.h>

#include "i_caller.h"
#include "volatility_offset_stream.h"

namespace cffex {
namespace fb {

/**
 * @brief i_volatility_offset_caller 波动率偏移配置修改指令
 *
 * @ingroup i_volatility_offset_caller
 */
class i_volatility_offset_caller :
    public i_caller_with_id<i_volatility_offset_caller, STRATEGY_CALLER_VOLATILITY_OFFSET> {
public:
    /**
     * @brief
     * volatility_offset_entity接口类抽象了期权系列波动率偏移设置结构，提供其操作界面；策略代码必须调用create_entity创建实例对象
     *        volatility_offset_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用modify_volatility_offset完成报价后，无需删除volatility_offset_entity对象
     *        - 其生命周期是两次调用create_entity之间的时间
     *        - 调用create_empty_entity返回的是重置后的对象
     *        - 调用create_entity返回的是存有当前值的对象
     */
    class volatility_offset_entity {
    public:
        virtual ~volatility_offset_entity() = default;
        /// 设置期权系列编号
        virtual void set_option_serial_index(uint64_t v) = 0;
        /// 设置期权系列
        virtual void set_option_serial_id(const char *v) = 0;
        /// 设置波动率算法
        virtual void set_volatility_algorithm_id(int16_t v) = 0;
        /// 设置拟合合约类型， 0-全部合约，1-看涨合约，2-看跌合约，3-虚值合约
        virtual void set_fit_contract_type(int8_t v) = 0;
        /// 设置执行价格
        virtual void set_strike_price(double v) = 0;
        /// 设置波动率偏移
        virtual void set_volatility_offset(double v) = 0;

        virtual void reset_entity() = 0;
    };

    i_volatility_offset_caller()          = default;
    virtual ~i_volatility_offset_caller() = default;

    /**
     * @brief 创建空的修改结构
     * @return 空的期权系列波动率偏移参数修改指针
     */
    virtual volatility_offset_entity *create_empty_entity() = 0;

    /**
     * @brief 创建空的修改结构
     * @param option_serial_id 期权系列id
     * @param volatility_algorithm_id 波动率算法id
     * @param fit_contract_type 拟合合约类型
     * @param strike_price 执行价格
     * @param option_serial_index 期权系列index
     * @return 带有当前值的期权系列波动率偏移参数修改结构指针
     */
    virtual volatility_offset_entity *create_entity(IN const char   *option_serial_id,
                                                    IN const int16_t volatility_algorithm_id,
                                                    IN const int8_t  fit_contract_type,
                                                    IN const double  strike_price) = 0;
    virtual volatility_offset_entity *create_entity(IN const uint64_t option_serial_index,
                                                    IN const int16_t  volatility_algorithm_id,
                                                    IN const int8_t   fit_contract_type,
                                                    IN const double   strike_price) = 0;

    /**
     * @brief 执行期权系列波动率偏移参数修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_volatility_offset(volatility_offset_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
