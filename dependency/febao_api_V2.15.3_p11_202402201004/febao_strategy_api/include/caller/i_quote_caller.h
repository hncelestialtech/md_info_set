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

#ifndef FB_QUOTE_CALLER_H
#define FB_QUOTE_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_quote_caller 接口类抽象了策略服务器提供的报价及撤报价功能。
 *        策略代码使用本类提供的API创建报价结构或者撤报价结构，填充数据，调用接口完成功能：
 *        报价：使用insert_quote成员函数，传递报价结构quote_entity完成报价
 *        撤报价：使用cancel_quote成员函数，传递撤报价结构cancel_quote_entity完成撤报价
 *
 * @ingroup i_quote_caller
 */
class i_quote_caller : public i_caller_with_id<i_quote_caller, STRATEGY_CALLER_QUOTE> {
public:
    /**
     * @brief
     * quote_entity接口类抽象了报价结构，提供其操作界面；策略代码必须调用create_quote_entity来创建报价结构的实例对象
     *        quote_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用insert_quote完成报价后，无需删除quote_entity对象
     *        - 其生命周期是两次调用create_quote_entity之间的时间
     *        -
     * 每次调用create_quote_entity返回的是重置的quote_entity对象，策略代码无需自行调用reset_entity函数
     */
    class quote_entity {
    public:
        virtual ~quote_entity() = default;
        /// 报价组合号。组合表达了对报价进行分类的概念
        virtual void set_portfolio_id(int v) = 0;
        /// 报价组合名称
        virtual void set_portfolio_name(const char *v) = 0;
        /// 报价合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 报价合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 买向开平标志
        virtual void set_bid_offset_flag(int8_t v) = 0;
        /// 卖向开平标志
        virtual void set_ask_offset_flag(int8_t v) = 0;
        /// 买向投机套保标志
        virtual void set_bid_hedge_flag(int8_t v) = 0;
        /// 卖向投机套保标志
        virtual void set_ask_hedge_flag(int8_t v) = 0;
        /// 报价买价价格
        virtual void set_bid_price(double v) = 0;
        /// 报价卖价价格
        virtual void set_ask_price(double v) = 0;
        /// 报价买量数量
        virtual void set_bid_volume(double v) = 0;
        /// 报价卖量数量
        virtual void set_ask_volume(double v) = 0;
        /// 报单时间条件
        virtual void set_time_condition(int8_t v) = 0;
        /// 报单数量条件
        virtual void set_volume_condition(int8_t v) = 0;
        /// 询价编号
        virtual void set_inquiry_id(const char *v) = 0;
        /// 设置报价号
        virtual void set_quote_id(int64_t v) = 0;
        /// 设置自定义标记
        virtual void set_custom_flag(const char *v) = 0;
        /// 设置报价优先级
        virtual void set_priority(int16_t v) = 0;
        virtual void set_quote_sys_id(const char *v) = 0;
        /// 设置席位号
        virtual void set_seat_no(const char *v) = 0;

        virtual void reset_entity() = 0;
    };

    /**
     * @brief
     * cancel_quote_entity接口类抽象了撤报价结构，提供其操作界面；策略代码必须调用create_cancel_entity来创建撤报价结构的实例对象
     *        cancel_quote_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用cancel_quote完成报价后，无需删除cancel_quote_entity对象
     *        - 其生命周期是两次调用create_cancel_entity之间的时间
     *        -
     * 每次调用create_cancel_entity返回的是干净的cancel_quote_entity对象，策略代码无需自行调用reset_entity函数
     */
    class cancel_quote_entity {
    public:
        virtual ~cancel_quote_entity() = default;
        /// 撤报价订单编号；由insert_quote的quote_id参数返回
        virtual void set_quote_id(int64_t v) = 0;
        /// 撤报价合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 撤报价合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 撤报价优先级
        virtual void set_priority(int16_t v) = 0;
        /// 撤单类型，0-撤双边，1-撤买，2-撤卖
        virtual void set_cancel_type(int8_t v) = 0;
        /// 重置撤报价结构
        virtual void reset_entity() = 0;
    };

    i_quote_caller()          = default;
    virtual ~i_quote_caller() = default;

    /**
     * @brief 创建报价结构
     * @return
     * 重置后的报价结构指针；策略代码使用后无需删除此结构，其有效性持续到下一次调用create_quote_entity之前
     */
    virtual quote_entity *create_quote_entity() = 0;

    /**
     * @brief 创建撤报价结构
     * @return
     * 重置后的撤报价结构；策略代码使用后无需删除此结构，其有效性持续到下一次调用create_cancel_entity之前
     */
    virtual cancel_quote_entity *create_cancel_entity() = 0;

    /**
     * @brief 进行报价操作
     *        调用前策略代码填充entity报价结构参数，quote_id作为OUT参数返回本次报价编号
     *
     * @param entity [IN] 报价结构指针，只能通过调用create_quote_entity获得
     * @param quote_id [OUT] 返回本次报价编号
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t insert_quote(i_quote_caller::quote_entity *entity, int64_t &quote_id) = 0;

    /**
     * @brief 进行撤报价操作
     *        调用前策略代码填充entity撤报价结构参数
     *
     * @param entity [IN] 撤报价结构指针，只能通过调用create_cancel_entity获得
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t cancel_quote(cancel_quote_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
