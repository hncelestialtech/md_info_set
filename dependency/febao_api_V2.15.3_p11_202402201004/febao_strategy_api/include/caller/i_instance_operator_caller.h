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

#ifndef FB_INSTANCE_OPERATOR_CALLER_H
#define FB_INSTANCE_OPERATOR_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_instance_operator_caller 在策略内部控制自身策略停止或者其它策略实例起停，成功后发送策略状态到客户端
 *
 * @ingroup i_instance_operator_caller
 */
class i_instance_operator_caller :
    public i_caller_with_id<i_instance_operator_caller,
                            STRATEGY_CALLER_INSTANCE_OPERATOR> {
public:
    i_instance_operator_caller()          = default;
    virtual ~i_instance_operator_caller() = default;

    /**
     * @brief 启动策略
     * @return bool 成功返回true，失败返回false
     */
    virtual bool start() = 0;

     /**
     * @brief 停止策略
     * @return bool 成功返回true，失败返回false
     */
    virtual bool stop()  = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
