#ifndef FB_POSITION_FUND_STREAM_H
#define FB_POSITION_FUND_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class position_fund_stream :
    public i_stream_with_id<position_fund_stream,
                            STRATEGY_STREAM_POSITION_FUND> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual uint64_t    get_instrument_index() const      = 0;
        virtual const char *get_instrument_id() const         = 0;
        virtual int32_t     get_position() const              = 0;
        virtual int32_t     get_td_position() const           = 0;
        virtual int32_t     get_yd_position() const           = 0;
        virtual double      get_cur_close_profit() const      = 0;
        virtual double      get_his_close_profit() const      = 0;
        virtual double      get_position_avg_price() const    = 0;
        virtual double      get_td_position_avg_price() const = 0;
        virtual double      get_yd_position_avg_price() const = 0;
        virtual double      get_td_position_amount() const    = 0;
        virtual double      get_yd_position_amount() const    = 0;

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
    position_fund_stream(msg_callback callback, snap_callback s_callback);
    position_fund_stream(msg_callback callback);
    position_fund_stream();
    ~position_fund_stream();

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