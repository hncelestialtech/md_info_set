/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: renjh
 * Date: 2022-05-16
 */

#ifndef CFFEX_FB_MD_MSG_H
#define CFFEX_FB_MD_MSG_H

#include <string>

namespace cffex {
namespace fb {

#ifndef  BASE_MSG_FUNCTIONS
#define  BASE_MSG_FUNCTIONS                                                             \
    virtual const char  *get_md_name() const = 0;                                       \
    virtual int8_t       get_max_depth() const = 0;                                     \
    virtual void *get_field() = 0;                                                      \
    virtual void *get_header() = 0;                                                     \
    virtual void dump() const = 0;                                                      \
    virtual std::string to_string() const = 0;
#endif

struct market_data_msg {
    virtual ~market_data_msg() { }

    virtual const char   *get_instrument_id() const = 0;
    virtual int32_t       get_update_sec() const = 0;
    virtual int32_t       get_update_msec() const = 0;
    virtual double        get_pre_settlement() const = 0;
    virtual double        get_pre_close() const = 0;
    virtual double        get_open() const = 0;
    virtual double        get_close() const = 0;
    virtual double        get_upper_limit_price() const = 0;
    virtual double        get_down_limit_price() const = 0;
    virtual double        get_high_price() const = 0;
    virtual double        get_low_price() const = 0;
    virtual double        get_last_price() const = 0;
    virtual int32_t       get_volume() const = 0;
    virtual double        get_turn_over() const = 0;
    virtual double        get_open_interest() const = 0;

    // depths: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
    virtual double        get_bid_price(int8_t depth) const = 0; // not int depths: return DOUBLE_MAX
    virtual double        get_ask_price(int8_t depth) const = 0;
    virtual int32_t       get_bid_volume(int8_t depth) const = 0; // not int depths: return -1
    virtual int32_t       get_ask_volume(int8_t depth) const = 0;

    virtual void          set_bid_price(int8_t depth, double v) = 0;
    virtual void          set_ask_price(int8_t depth, double v) = 0;
    virtual void          set_bid_volume(int8_t depth, int32_t v) = 0;
    virtual void          set_ask_volume(int8_t depth, int32_t v) = 0;

    virtual double        get_iopv() const = 0;
    virtual double        get_dynamic_reference_price() const = 0;
    BASE_MSG_FUNCTIONS
};

struct inquiry_quote_msg {
    virtual ~inquiry_quote_msg() { }

    virtual const char* get_inquiry_id() const = 0;
    virtual int8_t get_exchange_id() const = 0;
    virtual const char* get_instrument_id() const = 0;
    virtual int8_t get_inquiry_quote_status() const = 0;
    virtual int32_t get_inquiry_time() const = 0;
    virtual int32_t get_response_time() const = 0;
    BASE_MSG_FUNCTIONS
};

struct instrument_trading_status_msg {
    virtual ~instrument_trading_status_msg() { }

    virtual const char* get_instrument_id() const = 0;
    virtual int8_t get_exchange_id() const = 0;
    virtual int8_t get_instrument_trading_status() const = 0;
    BASE_MSG_FUNCTIONS
};




}
}

#endif
