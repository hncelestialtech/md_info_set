#ifndef FB_SERIAL_THEO_CUSTOM_PARAM_STREAM_H
#define FB_SERIAL_THEO_CUSTOM_PARAM_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class serial_theo_custom_param_stream :
    public i_stream_with_id<serial_theo_custom_param_stream,
                            STRATEGY_STREAM_SERIAL_THEO_CUSTOM_PARAM> {
public:
    class i_stream_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual uint64_t    get_option_serial_index() const          = 0;
        virtual const char *get_option_serial_id() const             = 0;
        virtual bool        get_param(IN const char *param_name,
                                      OUT double    *param_value) const = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(
            IN const char *option_serial_id) const = 0;
        virtual const i_stream_msg *get_msg(
            IN uint64_t option_serial_index) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    serial_theo_custom_param_stream(msg_callback  callback,
                                    snap_callback s_callback);
    serial_theo_custom_param_stream(msg_callback callback);
    serial_theo_custom_param_stream();
    ~serial_theo_custom_param_stream();

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