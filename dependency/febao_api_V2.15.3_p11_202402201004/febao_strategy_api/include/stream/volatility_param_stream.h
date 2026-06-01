#ifndef FB_VOLATILITY_PARAM_STREAM_H
#define FB_VOLATILITY_PARAM_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class volatility_param_stream :
    public i_stream_with_id<volatility_param_stream,
                            STRATEGY_STREAM_VOLATILITY_PARAM> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual uint64_t    get_option_serial_index() const     = 0;
        virtual const char *get_option_serial_id() const        = 0;
        virtual int16_t     get_volatility_algorithm_id() const = 0;
        virtual int8_t      get_fit_contract_type() const       = 0;

        virtual bool get_param(IN const char *param_name,
                               OUT int       *param_value) const        = 0;
        virtual bool get_param(IN const char *param_name,
                               OUT double    *param_value) const     = 0;
        virtual bool get_param(IN const char *param_name,
                               OUT char       param_value[512]) const   = 0;
        virtual bool get_param(IN const char *param_name,
                               IN const int   param_size,
                               OUT int       *param_value) const        = 0;
        virtual bool get_param(IN const char *param_name,
                               IN const int   param_size,
                               OUT double    *param_value) const     = 0;
        virtual bool get_param(IN const char *param_name,
                               IN const int   param_size,
                               OUT char       param_value[][512]) const = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(
            IN const char   *option_serial_id,
            IN const int16_t volatility_algorithm_id,
            IN const int8_t  fit_contract_type) const = 0;
        virtual const i_stream_msg *get_msg(
            IN uint64_t      option_serial_index,
            IN const int16_t volatility_algorithm_id,
            IN const int8_t  fit_contract_type) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    volatility_param_stream(msg_callback callback, snap_callback s_callback);
    volatility_param_stream(msg_callback callback);
    volatility_param_stream();
    ~volatility_param_stream();

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
