#ifndef FB_PORTFOLIO_POSITION_STREAM_H
#define FB_PORTFOLIO_POSITION_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class portfolio_position_stream :
    public i_stream_with_id<portfolio_position_stream,
                            STRATEGY_STREAM_PORTFOLIO_POSITION> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual uint64_t    get_instrument_index() const  = 0;
        virtual const char *get_instrument_id() const     = 0;
        virtual int16_t     get_portfolio_id() const      = 0;
        virtual const char *get_portfolio_name() const    = 0;
        virtual int32_t     get_long_position() const     = 0;
        virtual int32_t     get_short_position() const    = 0;
        virtual int32_t     get_td_long_position() const  = 0;
        virtual int32_t     get_td_short_position() const = 0;
        virtual int32_t     get_yd_long_position() const  = 0;
        virtual int32_t     get_yd_short_position() const = 0;
        virtual void        dump() const                  = 0;
        virtual std::string to_string() const             = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(IN const char *instrument_id,
                                            IN const int16_t
                                                portfolio_id) const = 0;
        virtual const i_stream_msg *get_msg(IN uint64_t instrument_index,
                                            IN const int16_t
                                                portfolio_id) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    portfolio_position_stream(msg_callback callback, snap_callback s_callback);
    portfolio_position_stream(msg_callback callback);
    portfolio_position_stream();
    ~portfolio_position_stream();

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