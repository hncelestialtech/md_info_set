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

#ifndef FB_DERIVED_MD_STREAM_H
#define FB_DERIVED_MD_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class derived_md_stream :
    public i_stream_with_id<derived_md_stream, STRATEGY_STREAM_DERIVED_MD> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual uint64_t    get_instrument_index() const              = 0;
        virtual const char *get_instrument_id() const                 = 0;
        virtual double      get_baseprice() const                     = 0;
        virtual double      get_forward_price() const                 = 0;
        virtual double      get_raw_theoretical_price() const         = 0;
        virtual double      get_theoretical_price() const             = 0;
        virtual double      get_delta() const                         = 0;
        virtual double      get_gamma() const                         = 0;
        virtual double      get_theta() const                         = 0;
        virtual double      get_vega() const                          = 0;
        virtual double      get_rho() const                           = 0;
        virtual double      get_charm() const                         = 0;
        virtual double      get_vanna() const                         = 0;
        virtual double      get_vomma() const                         = 0;
        virtual double      get_speed() const                         = 0;
        virtual double      get_zomma() const                         = 0;
        virtual double      get_volatility() const                    = 0;
        virtual double      get_skew_delta() const                    = 0;
        virtual double      get_rate() const                          = 0;
        virtual double      get_left_trading_days() const             = 0;
        virtual double      get_left_trading_years() const            = 0;
        virtual double      get_left_natural_days() const             = 0;
        virtual double      get_left_natural_years() const            = 0;
        virtual bool        get_custom_greek(IN const char *name,
                                             OUT double    *value) const = 0;

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
    derived_md_stream(msg_callback callback, snap_callback s_callback);
    derived_md_stream(msg_callback callback);
    derived_md_stream();
    ~derived_md_stream();

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
