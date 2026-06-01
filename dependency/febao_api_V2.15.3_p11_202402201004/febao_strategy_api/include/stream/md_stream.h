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

#ifndef FB_MD_STREAM_H
#define FB_MD_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class md_stream : public i_stream_with_id<md_stream, STRATEGY_STREAM_MD> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual uint64_t    get_instrument_index() const        = 0;
        virtual const char *get_instrument_id() const           = 0;
        virtual int32_t     get_update_sec() const              = 0;
        virtual int32_t     get_update_msec() const             = 0;
        virtual double      get_pre_settlement() const          = 0;
        virtual double      get_pre_close() const               = 0;
        virtual double      get_open() const                    = 0;
        virtual double      get_close() const                   = 0;
        virtual double      get_upper_limit_price() const       = 0;
        virtual double      get_down_limit_price() const        = 0;
        virtual double      get_high_price() const              = 0;
        virtual double      get_low_price() const               = 0;
        virtual double      get_last_price() const              = 0;
        virtual int32_t     get_volume() const                  = 0;
        virtual double      get_turn_over() const               = 0;
        virtual double      get_open_interest() const           = 0;
        virtual double      get_bid1_price() const              = 0;
        virtual double      get_ask1_price() const              = 0;
        virtual double      get_bid2_price() const              = 0;
        virtual double      get_ask2_price() const              = 0;
        virtual double      get_bid3_price() const              = 0;
        virtual double      get_ask3_price() const              = 0;
        virtual double      get_bid4_price() const              = 0;
        virtual double      get_ask4_price() const              = 0;
        virtual double      get_bid5_price() const              = 0;
        virtual double      get_ask5_price() const              = 0;
        virtual double      get_bid6_price() const              = 0;
        virtual double      get_ask6_price() const              = 0;
        virtual double      get_bid7_price() const              = 0;
        virtual double      get_ask7_price() const              = 0;
        virtual double      get_bid8_price() const              = 0;
        virtual double      get_ask8_price() const              = 0;
        virtual double      get_bid9_price() const              = 0;
        virtual double      get_ask9_price() const              = 0;
        virtual double      get_bid10_price() const             = 0;
        virtual double      get_ask10_price() const             = 0;
        virtual int32_t     get_bid1_volume() const             = 0;
        virtual int32_t     get_ask1_volume() const             = 0;
        virtual int32_t     get_bid2_volume() const             = 0;
        virtual int32_t     get_ask2_volume() const             = 0;
        virtual int32_t     get_bid3_volume() const             = 0;
        virtual int32_t     get_ask3_volume() const             = 0;
        virtual int32_t     get_bid4_volume() const             = 0;
        virtual int32_t     get_ask4_volume() const             = 0;
        virtual int32_t     get_bid5_volume() const             = 0;
        virtual int32_t     get_ask5_volume() const             = 0;
        virtual int32_t     get_bid6_volume() const             = 0;
        virtual int32_t     get_ask6_volume() const             = 0;
        virtual int32_t     get_bid7_volume() const             = 0;
        virtual int32_t     get_ask7_volume() const             = 0;
        virtual int32_t     get_bid8_volume() const             = 0;
        virtual int32_t     get_ask8_volume() const             = 0;
        virtual int32_t     get_bid9_volume() const             = 0;
        virtual int32_t     get_ask9_volume() const             = 0;
        virtual int32_t     get_bid10_volume() const            = 0;
        virtual int32_t     get_ask10_volume() const            = 0;
        virtual double      get_iopv() const                    = 0;
        virtual double      get_dynamic_reference_price() const = 0;
        virtual int64_t     get_local_timestamp() const         = 0;
        virtual void        dump() const                        = 0;
        virtual std::string to_string() const                   = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(
            IN const char *instrument_id) const = 0;
        virtual const i_stream_msg *get_msg(
            IN uint64_t instrument_index) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    md_stream(msg_callback callback, snap_callback s_callback);
    md_stream(msg_callback callback);
    md_stream();
    ~md_stream();

    i_stream_table *get_stream_table() {
        return table_;
    }
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

#endif
