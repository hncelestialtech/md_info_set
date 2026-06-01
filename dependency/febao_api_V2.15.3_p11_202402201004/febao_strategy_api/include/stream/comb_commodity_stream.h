#ifndef FB_COMB_COMMODITY_STREAM_H
#define FB_COMB_COMMODITY_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class comb_commodity_stream :
    public i_stream_with_id<comb_commodity_stream,
                            STRATEGY_STREAM_COMB_COMMODITY> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual int8_t      get_comb_strategy() const     = 0;
        virtual uint64_t    get_instrument_index1() const = 0;
        virtual uint64_t    get_instrument_index2() const = 0;
        virtual const char *get_instrument_id1() const    = 0;
        virtual const char *get_instrument_id2() const    = 0;
        virtual int8_t      get_comb_direction() const    = 0;
        virtual int32_t     get_position() const          = 0;
        virtual int8_t      get_exchange_id() const       = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}

        virtual const i_stream_msg *get_msg(IN int8_t comb_strategy,
                                            IN char  *instrument_id1,
                                            IN char  *instrument_id2,
                                            IN int8_t comb_direction) const = 0;
        virtual const i_stream_msg *get_msg(IN int8_t   comb_strategy,
                                            IN uint64_t instrument_index1,
                                            IN uint64_t instrument_index2,
                                            IN int8_t comb_direction) const = 0;
        virtual const i_stream_msg *first(const i_filter *f = NULL) const   = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const    = 0;
    };

    class i_stream_notify_comb_commodity_msg {
    public:
        virtual ~i_stream_notify_comb_commodity_msg() {}

        virtual int64_t     get_comb_id() const           = 0;
        virtual int32_t     get_error_id() const          = 0;
        virtual int8_t      get_comb_strategy() const     = 0;
        virtual uint64_t    get_instrument_index1() const = 0;
        virtual uint64_t    get_instrument_index2() const = 0;
        virtual const char *get_instrument_id1() const    = 0;
        virtual const char *get_instrument_id2() const    = 0;
        virtual int8_t      get_comb_direction() const    = 0;
        virtual int32_t     get_position() const          = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;
    typedef std::function<void(
        IN const i_stream_notify_comb_commodity_msg *msg)>
        notify_comb_commodity_msg_callback;

public:
    comb_commodity_stream(msg_callback callback, snap_callback s_callback);
    comb_commodity_stream(msg_callback callback);
    comb_commodity_stream();
    ~comb_commodity_stream();

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
    void set_msg_callback(msg_callback callback);
    void set_snap_callback(snap_callback s_callback);
    void set_notify_comb_commodity_callback(
        notify_comb_commodity_msg_callback notify_comb_commodity_callback);

    virtual void on_data(const void *data);
    virtual void on_snap(const void *data, const bool is_last);
    virtual void on_notify_comb_commodity_data(const void *data);

private:
    msg_callback                       *callback_;
    snap_callback                      *snap_callback_;
    notify_comb_commodity_msg_callback *notify_comb_commodity_callback_;
    i_stream_table                     *table_;
};

}  // namespace fb
}  // namespace cffex

#endif