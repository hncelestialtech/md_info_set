#ifndef FB_COMB_COMMODITY_CALLER_H
#define FB_COMB_COMMODITY_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_combmargin_commodity_caller接口类抽象了策略服务器提供的商品期货组合保证金业务
 *        - 组合：使用combmargin_combine成员函数完成组保组合功能
 *        - 拆分：使用combmargin_split成员函数完成组保拆分功能
 * @details description
 * @ingroup i_combmargin_commodity_caller
 */
class i_comb_commodity_caller :
    public i_caller_with_id<i_comb_commodity_caller, STRATEGY_CALLER_COMB_COMMODITY> {
public:
    /**
     * @brief
     * combmargin_commodity_entity接口类抽象了商品组合保证金组合和拆分结构；策略代码必须调用create_combmargin_commodity_entity来创建其实例对象
     *        combmargin_commodity_entity对象的生命周期由策略进程管理：
     *        -
     * 策略代码调用combmargin_combine或者combmargin_split完成功能后，无需删除combmargin_commodity_entity对象
     *        - 其生命周期是两次调用combmargin_combine或者combmargin_split之间的时间
     *        -
     * 每次调用create_combmargin_commodity_entity返回的是重置后的对象，策略代码无需自行调用reset_entity函数
     */
    class comb_commodity_entity {
    public:
        virtual ~comb_commodity_entity() = default;
        /// 设置组保使用的策略
        /// 组合保证金策略，0-认购牛市价差，1-认购熊市价差，2-认沽牛市价差，3-认沽熊市价差，4-跨式空头，5-宽跨式空头，6-期货跨期，7-期货跨品种，8-期货对锁，9-期权对锁，a-期权跨式，b-期权宽跨式，c-买入垂直价差，d-卖出垂直价差，e-买入期权期货组合，f-卖出期权期货组合，n-未知
        virtual void set_comb_strategy(int8_t v) = 0;
        /// 设置组保腿1合约编号
        virtual void set_instrument_index1(uint64_t v) = 0;
        /// 设置组保腿2合约编号
        virtual void set_instrument_index2(uint64_t v) = 0;
        /// 设置组保腿1合约号
        virtual void set_instrument_id1(const char *v) = 0;
        /// 设置组保腿2合约号
        virtual void set_instrument_id2(const char *v) = 0;
        /// 设置组保交易方向
        virtual void set_comb_direction(int8_t v) = 0;
        /// 设置组保仓位
        virtual void set_position(int v) = 0;
        /// 设置交易所
        virtual void set_exchange_id(int8_t v) = 0;
        /// 重置结构，客户策略代码无需调用
        virtual void reset_entity() = 0;
    };

    i_comb_commodity_caller()          = default;
    virtual ~i_comb_commodity_caller() = default;

    /**
     * @brief 创建组保报单结构
     * @return 重置后的组保报单结构指针
     */
    virtual comb_commodity_entity *create_comb_commodity_entity() = 0;

    /**
     * @brief 组合保证金组合
     * @param entity [IN] 组保报单结构指针，只能通过调用create_combmargin_commodity_entity获得
     * @param combmargin_id [OUT] 报单ID
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t comb_combine(comb_commodity_entity *entity, int64_t &comb_id) = 0;
    /**
     * @brief 组合保证金拆分
     * @param entity [IN] 组保报单结构指针，只能通过调用create_combmargin_commodity_entity获得
     * @param combmargin_id [OUT] 报单ID
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t comb_split(comb_commodity_entity *entity, int64_t &comb_id) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif