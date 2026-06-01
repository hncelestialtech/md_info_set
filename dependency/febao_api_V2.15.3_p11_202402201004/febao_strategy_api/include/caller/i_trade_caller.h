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

#ifndef FB_TRADE_CALLER_H
#define FB_TRADE_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_trade_caller 接口类抽象了策略服务器提供的修改成交功能。
 *        策略代码使用本类提供的API创建成交结构，填充数据，调用接口完成功能：
 *        添加成交：使用insert_trade成员函数，传递成交结构trade_entity完成
 *        删除成交：使用delete_trade成员函数，传递删除成交结构delete_trade_entity完成
 *        迁移成交：使用transfer_trade成员函数，传递迁移成交结构transfer_trade_entity完成
 * @ingroup i_trade_caller
 */
class i_trade_caller : public i_caller_with_id<i_trade_caller, STRATEGY_CALLER_TRADE> {
public:
    /**
     * @brief
     * trade_entity接口类抽象了成交结构，提供其操作界面；策略代码必须调用create_trade_entity来创建成交结构的实例对象
     *        trade_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用insert_trade完成添加成交后，无需删除trade_entity对象
     *        - 其生命周期是两次调用create_trade_entity之间的时间
     *        -
     * 每次调用create_trade_entity返回的是干净的trade_entity对象，策略代码无需自行调用reset_entity函数
     */
    class trade_entity {
    public:
        virtual ~trade_entity() = default;
        /// 成交合约编号
        virtual void set_instrument_index(uint64_t v) = 0;
        /// 成交合约号
        virtual void set_instrument_id(const char *v) = 0;
        /// 买卖方向，0-买，1-卖
        virtual void set_direction(int8_t v) = 0;
        /// 投机套保标志，1-投机，2-套利，3-套保，4-做市商
        virtual void set_hedge_flag(int8_t v) = 0;
        /// 开平标志，1-开，2-平，3-平今，4-平昨，5-自动开平
        virtual void set_offset_flag(int8_t v) = 0;
        /// 成交价格
        virtual void set_price(double v) = 0;
        /// 成交数量
        virtual void set_volume(int v) = 0;
        /// 成交组合号。组合表达了对报单进行分类的概念
        virtual void set_portfolio_id(int v) = 0;
        /// 成交组合名称
        virtual void set_portfolio_name(const char *v) = 0;
        /// 成交时间
        virtual void set_trade_time(int v) = 0;

        virtual void reset_entity() = 0;
    };

    /**
     * @brief
     * delete_trade_entity接口类抽象了成交删除结构，提供其操作界面；策略代码必须调用create_delete_entity来创建成交删除结构的实例对象
     *        delete_trade_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用delete_trade完成添加成交后，无需删除delete_trade_entity对象
     *        - 其生命周期是两次调用create_delete_entity之间的时间
     *        -
     * 每次调用create_delete_entity返回的是干净的delete_trade_entity对象，策略代码无需自行调用reset_entity函数
     */
    class delete_trade_entity {
    public:
        virtual ~delete_trade_entity() = default;
        /// 设置成交编号
        virtual void set_trade_id(int64_t v) = 0;
        virtual void reset_entity()          = 0;
    };

    /**
     * @brief
     * transfer_trade_entity接口类抽象了成交结构，提供其操作界面；策略代码必须调用create_transfer_entity来迁移成交结构的实例对象
     *        transfer_trade_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用transfer_trade完成添加成交后，无需删除transfer_trade_entity对象
     *        - 其生命周期是两次调用create_transfer_entity之间的时间
     *        -
     * 每次调用create_transfer_entity返回的是干净的transfer_trade_entity对象，策略代码无需自行调用reset_entity函数
     */
    class transfer_trade_entity {
    public:
        virtual ~transfer_trade_entity() = default;
        /// 设置成交编号
        virtual void set_trade_id(int64_t v) = 0;
        /// 设置目标组合号
        virtual void set_dst_portfolio_id(int v) = 0;
        /// 设置目标组合名称
        virtual void set_dst_portfolio_name(const char *v) = 0;

        virtual void reset_entity() = 0;
    };

    i_trade_caller()          = default;
    virtual ~i_trade_caller() = default;

    /**
     * @brief 创建成交结构
     * @return
     * 重置后的成交结构指针；策略代码使用后无需删除此结构，其有效性持续到下一次调用create_trade_entity之前
     */
    virtual trade_entity *create_trade_entity() = 0;

    /**
     * @brief 创建删除成交结构
     * @return
     * 重置后的成交删除结构指针；策略代码使用后无需删除此结构，其有效性持续到下一次调用delete_delete_entity之前
     */
    virtual delete_trade_entity *create_delete_entity() = 0;

    /**
     * @brief 创建迁移成交结构
     * @return
     * 重置后的成交迁移结构指针；策略代码使用后无需删除此结构，其有效性持续到下一次调用transfer_trade_entity之前
     */
    virtual transfer_trade_entity *create_transfer_entity() = 0;

    /**
     * @brief 进行创建成交操作
     *        调用前策略代码填充entity报单结构参数，trade_id作为OUT参数返回本次成交编号
     *
     * @param entity [IN] 报单结构指针，只能通过调用create_trade_entity获得
     * @param order_id [OUT] 返回本次成交编号
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t insert_trade(i_trade_caller::trade_entity *entity, int64_t &trade_id) = 0;

    /**
     * @brief 进行成交删除操作
     *        调用前策略代码填充entity成交删除结构参数
     *
     * @param entity [IN] 成交删除结构指针，只能通过调用create_delete_entity获得
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t delete_trade(delete_trade_entity *entity) = 0;

    /**
     * @brief 进行成交迁移操作
     *        调用前策略代码填充entity成交迁移结构参数
     *
     * @param entity [IN] 成交迁移结构指针，只能通过调用create_transfer_entity获得
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t transfer_trade(transfer_trade_entity *entity) = 0;
};
}  // namespace fb
}  // namespace cffex

#endif
