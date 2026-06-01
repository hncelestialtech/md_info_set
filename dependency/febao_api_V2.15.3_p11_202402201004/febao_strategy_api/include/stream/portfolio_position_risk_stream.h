#ifndef FB_PORTFOLIO_POSITION_RISK_STREAM_H
#define FB_PORTFOLIO_POSITION_RISK_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class portfolio_position_risk_stream :
    public i_stream_with_id<portfolio_position_risk_stream,
                            STRATEGY_STREAM_PORTFOLIO_POSITION_RISK> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual uint64_t    get_instrument_index() const   = 0;
        virtual const char *get_instrument_id() const      = 0;
        virtual int         get_portfolio_id() const       = 0;
        virtual const char *get_portfolio_name() const     = 0;
        virtual double      get_delta() const              = 0;
        virtual double      get_gamma() const              = 0;
        virtual double      get_theta() const              = 0;
        virtual double      get_vega() const               = 0;
        virtual double      get_rho() const                = 0;
        virtual double      get_charm() const              = 0;
        virtual double      get_vanna() const              = 0;
        virtual double      get_vomma() const              = 0;
        virtual double      get_speed() const              = 0;
        virtual double      get_zomma() const              = 0;
        virtual double      get_cash_delta() const         = 0;
        virtual double      get_cash_gamma() const         = 0;
        virtual double      get_position_vega() const      = 0;
        virtual double      get_position_theta() const     = 0;
        virtual double      get_skew_cash_delta() const    = 0;
        virtual double      get_td_delta() const           = 0;
        virtual double      get_td_gamma() const           = 0;
        virtual double      get_td_theta() const           = 0;
        virtual double      get_td_vega() const            = 0;
        virtual double      get_td_rho() const             = 0;
        virtual double      get_td_charm() const           = 0;
        virtual double      get_td_vanna() const           = 0;
        virtual double      get_td_vomma() const           = 0;
        virtual double      get_td_speed() const           = 0;
        virtual double      get_td_zomma() const           = 0;
        virtual double      get_td_cash_delta() const      = 0;
        virtual double      get_td_cash_gamma() const      = 0;
        virtual double      get_td_position_vega() const   = 0;
        virtual double      get_td_position_theta() const  = 0;
        virtual double      get_td_skew_cash_delta() const = 0;
        virtual double      get_yd_delta() const           = 0;
        virtual double      get_yd_gamma() const           = 0;
        virtual double      get_yd_theta() const           = 0;
        virtual double      get_yd_vega() const            = 0;
        virtual double      get_yd_rho() const             = 0;
        virtual double      get_yd_charm() const           = 0;
        virtual double      get_yd_vanna() const           = 0;
        virtual double      get_yd_vomma() const           = 0;
        virtual double      get_yd_speed() const           = 0;
        virtual double      get_yd_zomma() const           = 0;
        virtual double      get_yd_cash_delta() const      = 0;
        virtual double      get_yd_cash_gamma() const      = 0;
        virtual double      get_yd_position_vega() const   = 0;
        virtual double      get_yd_position_theta() const  = 0;
        virtual double      get_yd_skew_cash_delta() const = 0;
        virtual int8_t      get_trigger_type() const       = 0;

        virtual bool get_custom_greek(IN const char *name,
                                      OUT double    *value) const    = 0;
        virtual bool get_td_custom_greek(IN const char *name,
                                         OUT double    *value) const = 0;
        virtual bool get_yd_custom_greek(IN const char *name,
                                         OUT double    *value) const = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const i_stream_msg *get_msg(IN const char *instrument_id,
                                            IN int portfolio_id) const = 0;
        virtual const i_stream_msg *get_msg(IN uint64_t instrument_index,
                                            IN int      portfolio_id) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) const = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const  = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    portfolio_position_risk_stream(msg_callback  callback,
                                   snap_callback s_callback);
    portfolio_position_risk_stream(msg_callback callback);
    portfolio_position_risk_stream();
    ~portfolio_position_risk_stream();

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