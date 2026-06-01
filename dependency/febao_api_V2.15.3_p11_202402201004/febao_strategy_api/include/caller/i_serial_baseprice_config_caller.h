/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: zhanjh
 * Date: 2022-01-19
 */

#ifndef FB_SERIAL_BASE_PRICE_CALLER_H
#define FB_SERIAL_BASE_PRICE_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_serial_baseprice_config_caller 设置期权系列基准价命令
 *
 * @ingroup i_serial_baseprice_config_caller
 */
class i_serial_baseprice_config_caller :
    public i_caller_with_id<i_serial_baseprice_config_caller, STRATEGY_CALLER_SERIAL_BASE_PRICE> {
public:
    /**
     * @brief
     * serial_baseprice_config_entity接口类抽象了期权系列基准价设置结构，提供其操作界面；策略代码必须调用create_entity创建实例对象
     *        serial_baseprice_config_entity对象的生命周期由策略进程管理：
     *        - 策略代码调用modify_base_price完成报价后，无需删除serial_baseprice_config_entity对象
     *        - 其生命周期是两次调用create_entity之间的时间
     *        -
     * 每次调用create_entity返回的是重置的serial_baseprice_config_entity对象，策略代码无需自行调用reset_entity函数
     */
    class serial_baseprice_config_entity {
    public:
        virtual ~serial_baseprice_config_entity() = default;
        /// 设置期权系列编号
        virtual void set_option_serial_index(uint64_t v) = 0;
        /// 设置期权系列
        virtual void set_option_serial_id(const char *v) = 0;
        /// 设置基准价
        virtual void set_base_price(double v) = 0;
        virtual void reset_entity()           = 0;
    };

    i_serial_baseprice_config_caller()          = default;
    virtual ~i_serial_baseprice_config_caller() = default;

    /**
     * @brief 创建期权系列基准价设置结构
     * @return 重置后的设置结构指针
     */
    virtual serial_baseprice_config_entity *create_entity() = 0;

    /**
     * @brief 进行期权系列基准价设置
     *        调用前策略代码填充entity撤报价结构参数
     *
     * @param entity [IN] 期权系列基准价设置结构指针，只能通过调用create_entity获得
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_base_price(serial_baseprice_config_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
