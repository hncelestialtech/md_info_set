#ifndef FB_ORDER_STREAM_H
#define FB_ORDER_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class order_stream :
    public i_stream_with_id<order_stream, STRATEGY_STREAM_ORDER> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual int64_t     get_order_id() const             = 0;
        virtual const char *get_order_sys_id() const         = 0;
        virtual uint64_t    get_instrument_index() const     = 0;
        virtual const char *get_instrument_id() const        = 0;
        virtual int8_t      get_direction() const            = 0;
        virtual int8_t      get_offset_flag() const          = 0;
        virtual int8_t      get_hedge_flag() const           = 0;
        virtual double      get_price() const                = 0;
        virtual int32_t     get_volume() const               = 0;
        virtual int8_t      get_price_category() const       = 0;
        virtual int8_t      get_volume_condition() const     = 0;
        virtual int8_t      get_time_condition() const       = 0;
        virtual int8_t      get_order_status() const         = 0;
        virtual int16_t     get_user_id() const              = 0;
        virtual int16_t     get_action_user_id() const       = 0;
        virtual int16_t     get_portfolio_id() const         = 0;
        virtual const char *get_portfolio_name() const       = 0;
        virtual int64_t     get_insert_time() const          = 0;
        virtual int64_t     get_update_time() const          = 0;
        virtual int64_t     get_local_create_time() const    = 0;
        virtual int64_t     get_local_insert_time() const    = 0;
        virtual int64_t     get_local_update_time() const    = 0;
        virtual int8_t      get_exchange_id() const          = 0;
        virtual int32_t     get_traded_volume() const        = 0;
        virtual int32_t     get_cancel_volume() const        = 0;
        virtual int8_t      get_order_source() const         = 0;
        virtual int32_t     get_strategy_instance_id() const = 0;
        virtual const char *get_strategy_name() const        = 0;
        virtual int32_t     get_error_id() const             = 0;
        virtual int16_t     get_priority() const             = 0;
        virtual const char *get_custom_flag() const          = 0;
        virtual const char *get_seat_no() const              = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(IN int64_t order_id) const    = 0;
        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    class i_stream_notify_cancel_msg {
    public:
        virtual ~i_stream_notify_cancel_msg() {}

        virtual int64_t     get_order_id() const         = 0;
        virtual int32_t     get_error_id() const         = 0;
        virtual uint64_t    get_instrument_index() const = 0;
        virtual const char *get_instrument_id() const    = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_notify_cancel_msg *msg)>
        notify_cancel_msg_callback;

public:
    order_stream(msg_callback callback);
    order_stream();
    ~order_stream();

    i_stream_table *get_stream_table() {
        return table_;
    }
    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }
    // virtual bool is_need_snap() { return false; }

    /** called by strategy */
    virtual void on_data(const void *data);
    virtual void on_notify_cancel_data(const void *data);
    void         set_msg_callback(msg_callback callback);
    void         set_notify_cancel_callback(
                notify_cancel_msg_callback notify_cancel_callback);

private:
    msg_callback               *callback_;
    notify_cancel_msg_callback *notify_cancel_callback_;
    i_stream_table             *table_;
};

}  // namespace fb
}  // namespace cffex

#endif