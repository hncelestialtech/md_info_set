/**
 * CFFEX Confidential.
 *
 * @Copyright 2019 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: Wang FengNing
 * Date: 2019-02-26
 */
#ifndef FB_SPREAD_TEMPLATE_H
#define FB_SPREAD_TEMPLATE_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {
class spread_template_stream :
    public i_stream_with_id<spread_template_stream,
                            STRATEGY_STREAM_SPREAD_TEMPLATE> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual const char *get_template_name() const      = 0;
        virtual const char *get_template_value() const     = 0;
        virtual void        dump() const                   = 0;
        virtual std::string to_string() const              = 0;
        virtual double      get_spread(double price) const = 0;
    };
    class i_stream_table {
    public:
        virtual const i_stream_msg *get_msg(
            IN const char *template_name) const                           = 0;
        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };
    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    spread_template_stream(msg_callback callback, snap_callback snap_callback);
    spread_template_stream(msg_callback callback);
    spread_template_stream();
    virtual ~spread_template_stream();
    i_stream_table *get_stream_table() {
        return table_;
    }

    /** will be called by strategy after registed*/
    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }
    virtual bool is_need_snap() {
        return snap_callback_ != NULL;
    }

    /** called by strategy */
    virtual void on_data(const void *data);
    virtual void on_snap(const void *data, const bool is_last);
    void         set_msg_callback(msg_callback callback);
    void         set_snap_callback(snap_callback s_callback);

private:
    msg_callback   *callback_;
    snap_callback  *snap_callback_;
    i_stream_table *table_;
};
}  // namespace fb
}  // namespace cffex

#endif  // FB_SPREAD_TEMPLATE_H
