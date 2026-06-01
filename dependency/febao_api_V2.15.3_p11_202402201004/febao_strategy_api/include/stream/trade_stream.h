#ifndef FB_TRADE_STREAM_H
#define FB_TRADE_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class trade_stream :
    public i_stream_with_id<trade_stream, STRATEGY_STREAM_TRADE> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual int32_t     get_trade_id() const             = 0;
        virtual int8_t      get_exchange_id() const          = 0;
        virtual int64_t     get_order_id() const             = 0;
        virtual const char *get_order_sys_id() const         = 0;
        virtual const char *get_trade_sys_id() const         = 0;
        virtual uint64_t    get_instrument_index() const     = 0;
        virtual const char *get_instrument_id() const        = 0;
        virtual int8_t      get_direction() const            = 0;
        virtual int8_t      get_offset_flag() const          = 0;
        virtual int8_t      get_hedge_flag() const           = 0;
        virtual double      get_price() const                = 0;
        virtual int32_t     get_volume() const               = 0;
        virtual int64_t     get_trade_time() const           = 0;
        virtual int16_t     get_user_id() const              = 0;
        virtual int16_t     get_portfolio_id() const         = 0;
        virtual const char *get_portfolio_name() const       = 0;
        virtual int32_t     get_strategy_instance_id() const = 0;
        virtual const char *get_strategy_name() const        = 0;
        virtual int64_t     get_local_insert_time() const    = 0;
        virtual int64_t     get_local_update_time() const    = 0;
        virtual int8_t      get_trade_source() const         = 0;
        virtual const char *get_custom_flag() const          = 0;
        virtual int8_t      get_trade_status() const         = 0;
        virtual int8_t      get_price_category() const       = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(IN int32_t trade_id) const    = 0;
        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;

public:
    trade_stream(msg_callback callback);
    trade_stream();
    ~trade_stream();

    i_stream_table *get_stream_table() {
        return table_;
    }
    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }
    // virtual bool is_need_snap() { return false; }

    /** called by strategy */
    virtual void on_data(const void *data);
    void         set_msg_callback(msg_callback callback);

private:
    msg_callback   *callback_;
    i_stream_table *table_;
};

}  // namespace fb
}  // namespace cffex

#endif