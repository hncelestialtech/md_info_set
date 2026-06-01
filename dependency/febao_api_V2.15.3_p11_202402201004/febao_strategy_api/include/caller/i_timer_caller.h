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

#ifndef FB_TIMER_caller_H
#define FB_TIMER_caller_H

#include <stdint.h>

#include <functional>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_timer_caller 提供了10毫秒以及1毫秒级别精度的定时器注册和注销功能
 *
 * @ingroup i_timer_caller
 */
class i_timer_caller : public i_caller_with_id<i_timer_caller, STRATEGY_CALLER_TIMER> {
public:
    /// 定时器回调原型
    typedef std::function<void()> timer_function;
    i_timer_caller()          = default;
    virtual ~i_timer_caller() = default;

    /**
     * @brief 注册定时器回调函数
     * @param milsec [IN] 定时器定时时间，精度为10毫秒
     * @param f [IN] 定时器回调函数
     * @param repeat [IN] 定时器是否重复，true表示重复，false表示不重复
     * @return int 返回定时器id
     */
    virtual int register_timer(uint32_t milsec, timer_function f, bool repeat) = 0;

    /**
     * @brief 注销定时器回调函数
     * @param timer_id [IN] 定时器id，由register_timer接口返回
     */
    virtual void unregister_timer(int timer_id) = 0;

    /**
     * @brief 注册定时器回调函数
     * @param milsec [IN] 定时器定时时间，精度为1毫秒
     * @param f [IN] 定时器回调函数
     * @param repeat [IN] 定时器是否重复，true表示重复，false表示不重复
     * @return int 返回定时器id
     */
    virtual int register_high_resolution_timer(uint32_t milsec, timer_function f, bool repeat) = 0;

    /**
     * @brief 注销定时器回调函数
     * @param timer_id [IN] 定时器id，由register_high_resolution_timer接口返回
     */
    virtual void unregister_high_resolution_timer(int timer_id) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
