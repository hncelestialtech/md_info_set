#ifndef FB_INSTRUMENT_THEO_PRICING_PARAM_CALLER_H
#define FB_INSTRUMENT_THEO_PRICING_PARAM_CALLER_H

#include <stdint.h>

#include "i_caller.h"
#include "instrument_theo_pricing_param_config_stream.h"

namespace cffex {
namespace fb {

/**
 * @brief i_instrument_theo_pricing_param_caller brief \n
 *
 * @ingroup i_instrument_theo_pricing_param_caller
 */
class i_instrument_theo_pricing_param_caller :
    public i_caller_with_id<i_instrument_theo_pricing_param_caller,
                            STRATEGY_CALLER_INSTRUMENT_THEO_PRICING_PARAM> {
public:
    class instrument_theo_pricing_param_entity {
    public:
        virtual ~instrument_theo_pricing_param_entity() = default;
        /// 设置需要修改理论价参数的合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 设置需要修改理论价参数的合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 设置需要修改理论价偏移
        virtual void set_theo_offset(double v) = 0;
        virtual void reset_entity()            = 0;
    };

    i_instrument_theo_pricing_param_caller()          = default;
    virtual ~i_instrument_theo_pricing_param_caller() = default;

    /**
     * @brief 创建空的修改结构
     * @return 空的理论价参数修改结构指针
     */
    virtual instrument_theo_pricing_param_entity *create_empty_entity() = 0;

    /**
     * @brief 创建空的修改结构
     * @return 带有当前值的理论价参数修改结构指针
     */
    virtual instrument_theo_pricing_param_entity *create_entity(IN const char *instrument_id) = 0;
    virtual instrument_theo_pricing_param_entity *create_entity(
        IN const uint64_t instrument_index) = 0;
    /**
     * @brief 执行理论价参数修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_theo_pricing_param(instrument_theo_pricing_param_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif