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

#ifndef FB_ORDER_CALLER_H
#define FB_ORDER_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_order_caller 接口类抽象了策略服务器提供的报单及撤单功能。
 *        策略代码使用本类提供的API创建报单结构或者撤单结构，填充数据，调用接口完成功能：
 *        报单：使用insert_order成员函数，传递报单结构order_entity完成报单
 *        撤单：使用cancel_order成员函数，传递撤单结构cancel_order_entity完成撤单
 * @ingroup i_order_caller
 */
class i_order_caller : public i_caller_with_id<i_order_caller, STRATEGY_CALLER_ORDER> {
public:
    /**
     * @brief
     * order_entity接口类抽象了报单结构，提供其操作界面；策略代码必须调用create_order_entity来创建报单结构的实例对象
     *        order_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用insert_order完成报单后，无需删除order_entity对象
     *        - 其生命周期是两次调用create_order_entity之间的时间
     *        -
     * 每次调用create_order_entity返回的是干净的order_entity对象，策略代码无需自行调用reset_entity函数
     */
    class order_entity {
    public:
        virtual ~order_entity() = default;
        /// 报单合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 报单合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 报单方向，0-买，1-卖
        virtual void set_direction(int8_t v) = 0;
        /// 报单对冲标记，1-投机，2-套利，3-套保，4-做市商
        virtual void set_hedge_flag(int8_t v) = 0;
        /// 开平标记，1-开，2-平，3-平今，4-平昨，5-自动开平
        virtual void set_offset_flag(int8_t v) = 0;
        /// 报单价格
        virtual void set_price(double v) = 0;
        /// 报单价格类型价格类型，1-市价，2-限价，3-对手方最优价，4-最优五档，5-套利，6-互换，7-报价衍生，8-其他，9-本方最优价
        virtual void set_price_category(int8_t v) = 0;
        /// 报单数量
        virtual void set_volume(int v) = 0;
        /// 报单组合号。组合表达了对报单进行分类的概念
        virtual void set_portfolio_id(int v) = 0;
        /// 报单组合名称
        virtual void set_portfolio_name(const char *v) = 0;
        /// 报单成交数量类型，1-部分成交，2-全部成交
        virtual void set_volume_condition(int8_t v) = 0;
        /// 报单时间条件，1-IOC，2-GFD
        virtual void set_time_condition(int8_t v) = 0;
        /// 报单号
        virtual void set_order_id(int64_t v) = 0;
        /// 报单优先级
        virtual void set_priority(int16_t v) = 0;
        /// 自定义标识
        virtual void set_custom_flag(const char *v) = 0;
        /// 设置席位号
        virtual void set_seat_no(const char *v) = 0;
        /// 报单结构重置
        virtual void reset_entity() = 0;
    };

    /**
     * @brief
     * cancel_order_entity接口类抽象了撤单结构，提供其操作界面；策略代码必须调用create_cancel_entity来创建撤单结构的实例对象
     *        cancel_order_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用cancel_order完成报单后，无需删除cancel_order_entity对象
     *        - 其生命周期是两次调用create_cancel_entity之间的时间
     *        -
     * 每次调用create_cancel_entity返回的是干净的cancel_order_entity对象，策略代码无需自行调用reset_entity函数
     */
    class cancel_order_entity {
    public:
        virtual ~cancel_order_entity() = default;
        /// 撤单合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 撤单合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 撤单订单编号；由insert_order的order_id参数返回
        virtual void set_order_id(int64_t v) = 0;
        /// 撤单优先级
        virtual void set_priority(int16_t v) = 0;
        /// 重置撤单结构
        virtual void reset_entity() = 0;
    };

    i_order_caller()          = default;
    virtual ~i_order_caller() = default;

    /**
     * @brief 创建报单结构
     * @return
     * 重置后的报单结构指针；策略代码使用后无需删除此结构，其有效性持续到下一次调用create_order_entity之前
     */
    virtual order_entity *create_order_entity() = 0;

    /**
     * @brief 创建撤单结构
     * @return
     * 重置后的撤单结构；策略代码使用后无需删除此结构，其有效性持续到下一次调用create_cancel_entity之前
     */
    virtual cancel_order_entity *create_cancel_entity() = 0;

    /**
     * @brief 进行报单操作
     *        调用前策略代码填充entity报单结构参数，order_id作为OUT参数返回本次报单编号
     *
     * @param entity [IN] 报单结构指针，只能通过调用create_order_entity获得
     * @param order_id [OUT] 返回本次报单编号
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t insert_order(i_order_caller::order_entity *entity, int64_t &order_id) = 0;

    /**
     * @brief 进行撤单操作
     *        调用前策略代码填充entity撤单结构参数
     *
     * @param entity [IN] 撤单结构指针，只能通过调用create_cancel_entity获得
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t cancel_order(cancel_order_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
