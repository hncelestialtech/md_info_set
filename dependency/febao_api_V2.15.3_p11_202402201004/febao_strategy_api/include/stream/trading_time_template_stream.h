/**
 * CFFEX Confidential.
 *
 * @Copyright 2019 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: lisc
 * Date: 2019-09-06
 */
#ifndef FB_TRADING_TIME_TEMPLATE_STREAM_H
#define FB_TRADING_TIME_TEMPLATE_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {
class trading_time_template_stream :
    public i_stream_with_id<trading_time_template_stream,
                            STRATEGY_STREAM_TRADING_TIME_TEMPLATE> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual const char *get_template_name() const                      = 0;
        virtual const char *get_template_value() const                     = 0;
        virtual void        dump() const                                   = 0;
        virtual std::string to_string() const                              = 0;
        virtual int         get_curr_trading_sec(int8_t exchange_id) const = 0;
        virtual bool        is_trading_time(int8_t exchange_id) const      = 0;
        virtual int         get_curr_trading_sec() const                   = 0;
        virtual bool        is_trading_time() const                        = 0;
    };
    class i_stream_table {
    public:
        virtual const i_stream_msg *get_msg(
            IN const char *template_name) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };
    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;

public:
    trading_time_template_stream(msg_callback callback);
    trading_time_template_stream();
    virtual ~trading_time_template_stream();
    i_stream_table *get_stream_table() {
        return table_;
    }

    /** will be called by strategy after registed*/
    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }

    /** called by strategy */
    virtual void on_data(const void *data);
    void         set_msg_callback(msg_callback callback);

private:
    msg_callback   *callback_;
    i_stream_table *table_;
};
}  // namespace fb
}  // namespace cffex

#endif  // FB_TRADING_TIME_TEMPLATE_STREAM_H
