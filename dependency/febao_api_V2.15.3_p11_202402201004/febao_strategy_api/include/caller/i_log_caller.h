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

#ifndef FB_LOG_CALLER_H
#define FB_LOG_CALLER_H

#include <stdint.h>

#include "i_caller.h"
#include "i_stream.h"

namespace cffex {
namespace fb {

/**
 * @brief 用户继承i_log_field进行自定义异步日志的编写
 *        注意事项：派生类中只可以使用pod类型。策略内部对派生对象进行浅拷贝，所以不可在派生类中使用简单的指针传递。
 */
struct i_log_field {
public:
    virtual ~i_log_field()                                                  = default;
    virtual const char *name()                                              = 0;
    virtual void        output_to_csv(char *out, int *len, int maxlen)      = 0;
    virtual void        output_name_to_csv(char *out, int *len, int maxlen) = 0;
};

/**
 * @brief i_log_caller 接口类提供了日志指令
 *        - log接口记录的日志为本地日志，记录在服务器端本地磁盘
 *        - log_remote接口记录的日志为远程日志，通过网络发送到客户端
 *
 * @ingroup i_log_caller
 */
class i_log_caller : public i_caller_with_id<i_log_caller, STRATEGY_CALLER_LOG> {
public:
    /// 日志级别定义
    enum { ILOG_DEBUG = '0', ILOG_INFO = '1', ILOG_WARNING = '2', ILOG_ERROR = '3' };
    i_log_caller()          = default;
    virtual ~i_log_caller() = default;

    /// 记录本地日志
    virtual void log(unsigned char loglevel, const char *fmt, ...) = 0;
    /// 记录远程日志
    virtual void log_remote(unsigned char loglevel, const char *fmt, ...) = 0;
    /// 设置当前日志级别
    virtual bool set_log_level(const unsigned char loglevel) = 0;
    /// 获取当前日志级别
    virtual unsigned char get_log_level() = 0;
    /// 通过给定流格式记录异步日志
    virtual void async_log_msg(const i_stream::i_stream_base_msg *msg) = 0;
    /// 通过自定义格式记录异步日志
    virtual void async_log_msg(const i_log_field *msg, int size) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
