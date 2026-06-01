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
 * Date: 2018-08-17
 */

#ifndef FB_I_STRATEGY_STREAM_H
#define FB_I_STRATEGY_STREAM_H

#include <stdint.h>

#include <functional>
#include <string>
#include "strategy_instance_filter.h"
#include "i_filter.h"

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {

class i_stream {
    typedef std::function<void(i_stream *stream)> register_stream_caller;

public:
    class i_stream_base_msg {
    public:
        virtual int16_t get_field_id() const = 0;
        virtual void   *get_field() const    = 0;
    };
    i_stream(uint16_t id) : id_(id), filter_(NULL) {}

    virtual ~i_stream() {
        if (filter_ != NULL) {
            delete filter_;
            filter_ = NULL;
        }
    }

    uint16_t id() const {
        return id_;
    }
    virtual const char *name() const {
        return "";
    }
    virtual void *field() const {
        return NULL;
    }
    virtual void on_data(const void *data) {}
    virtual void on_snap(const void *data, const bool is_last) {}
    virtual bool is_need_snap() {
        return false;
    }

    void set_filter(const i_filter *f) {
        if(filter_ == NULL){
            filter_ = f->clone();
        }else{
            const i_filter* temp_f = &(*f & *filter_);
            delete filter_;
            filter_ = temp_f->clone();
        }
    }

    /** called by strategy engine */
    const i_filter *get_filter() {
        return filter_;
    }
    /** called reset_filter before set_filter */
    void reset_filter() {
        if(filter_ != nullptr) {
            delete filter_;
            filter_ = nullptr;
        }
    }
    /** called by stream_category */
    int get_state() {
        return state_;
    }
    void set_state(int s) {
        state_ = s;
    }

private:
    uint16_t        id_;
    const i_filter *filter_;
    int             state_;
};

template <typename STREAM, uint16_t __ID__>
struct i_stream_with_id : public i_stream {
    enum { ID = __ID__ };
    i_stream_with_id() : i_stream(__ID__) {}
    virtual ~i_stream_with_id() {}
    virtual const char *name() const {
        return typeid(STREAM).name();
    }
};

enum {
    STRATEGY_STREAM_INSTRUMENT_PARAM = 0X0001,
    STRATEGY_STREAM_STRATEGY_INSTANCE_PARAM,
    STRATEGY_STREAM_MD,
    STRATEGY_STREAM_DERIVED_MD,
    STRATEGY_STREAM_POSITION,
    STRATEGY_STREAM_SECURITY_POSITION,
    STRATEGY_STREAM_FUND_POSITION,
    STRATEGY_STREAM_ORDER,
    STRATEGY_STREAM_QUOTE,
    STRATEGY_STREAM_TRADE,
    STRATEGY_STREAM_CUSTOM_PARAM,
    STRATEGY_STREAM_INQUIRY_QUOTE,
    STRATEGY_STREAM_INSTRUMENT,
    STRATEGY_STREAM_INSTRUMENT_TRADING_STATUS,
    STRATEGY_STREAM_VOLATILITY_CONFIG,
    STRATEGY_STREAM_VOLATILITY_PARAM,
    STRATEGY_STREAM_VOLATILITY_OFFSET,
    STRATEGY_STREAM_PORTFOLIO_POSITION_RISK,
    STRATEGY_STREAM_EXCHANGE_TIME,
    STRATEGY_STREAM_TRADING_DAY,
    STRATEGY_STREAM_PORTFOLIO,
    STRATEGY_STREAM_VIRTUAL_PORTFOLIO_CONFIG,
    STRATEGY_STREAM_SPREAD_TEMPLATE,
    STRATEGY_STREAM_TRADING_TIME_TEMPLATE,
    STRATEGY_STREAM_PORTFOLIO_POSITION,
    STRATEGY_STREAM_PORTFOLIO_POSITION_PROFIT,
    STRATEGY_STREAM_INSTRUMENT_PRICING_PARAM_CONFIG,
    STRATEGY_STREAM_SERIAL_PRICING_PARAM_CONFIG,
    STRATEGY_STREAM_INSTRUMENT_THEO_PRICING_PARAM_CONFIG,
    STRATEGY_STREAM_INVESTOR_ACCOUNT_FUND,
    STRATEGY_STREAM_PORTFOLIO_RISK,
    STRATEGY_STREAM_INSTRUMENT_PRICING_CONFIG,
    STRATEGY_STREAM_INSTRUMENT_GROUP_PARAM_VALUE,
    STRATEGY_STREAM_INSTRUMENT_GROUP_CONFIG,
    STRATEGY_STREAM_FUND_INSTRUMENT_INFO,
    STRATEGY_STREAM_FUND_COMPONENT,
    STRATEGY_STREAM_LOCK_POSITION_CONFIG,
    STRATEGY_STREAM_POSITION_FUND,
    STRATEGY_STREAM_VOLATILITY_FITTING_PARAM_CONFIG,
    STRATEGY_STREAM_ATM_FORWARD_MD,
    STRATEGY_STREAM_COMB_STOCK,
    STRATEGY_STREAM_COMB_COMMODITY,
    STRATEGY_STREAM_MARGIN_FEE,
    STRATEGY_STREAM_SERIAL_THEO_CUSTOM_PARAM,

    STRATEGY_STREAM_ALL = 0XFFFF
};

}  // namespace fb
}  // namespace cffex

#endif
