#ifndef FB_MARGIN_FEE_STREAM_H
#define FB_MARGIN_FEE_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

/*
期货市场：
open_fee(开仓手续费)
offset_fee(平或者平昨)
ot_fee(平金)
exec_fee(行权)

证券市场：
open_fee(买开)
open_sell_fee(卖开)
offset_fee(买平)
offset_sell_fee(卖平)
*/

class margin_fee_stream :
    public i_stream_with_id<margin_fee_stream, STRATEGY_STREAM_MARGIN_FEE> {
public:
    class i_stream_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual uint64_t     get_instrument_index() const  = 0;
        virtual const char  *get_instrument_id() const     = 0;
        virtual const double get_long_margin_rate() const  = 0;
        virtual const double get_long_margin_amt() const   = 0;
        virtual const double get_short_margin_rate() const = 0;
        virtual const double get_short_margin_amt() const  = 0;
        virtual const double get_open_fee_rate() const     = 0;
        virtual const double get_open_fee_amt() const      = 0;
        virtual const double get_open_sell_fee_rate() const= 0;
        virtual const double get_open_sell_fee_amt() const = 0;
        virtual const double get_offset_fee_rate() const   = 0;
        virtual const double get_offset_fee_amt() const    = 0;
        virtual const double get_offset_sell_fee_rate() const= 0;
        virtual const double get_offset_sell_fee_amt() const = 0;
        virtual const double get_ot_fee_rate() const       = 0;
        virtual const double get_ot_fee_amt() const        = 0;
        virtual const double get_exec_fee_rate() const     = 0;
        virtual const double get_exec_fee_amt() const      = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
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
    margin_fee_stream(msg_callback callback, snap_callback s_callback);
    margin_fee_stream(msg_callback callback);
    margin_fee_stream();
    ~margin_fee_stream();

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
