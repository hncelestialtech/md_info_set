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

#ifndef FB_MD_ENTITY_H
#define FB_MD_ENTITY_H

#include <stddef.h>

#include <string>

#ifndef BASE_ENTITY_FUNCTIONS
#define BASE_ENTITY_FUNCTIONS                           \
    virtual void         reset_entity()            = 0; \
    virtual void         set_max_depth(uint8_t v)  = 0; \
    virtual void         set_reserved1(uint16_t v) = 0; \
    virtual void         set_reserved2(uint32_t v) = 0; \
    virtual void         set_guid(uint64_t v)      = 0; \
    virtual md_base_msg *get_base()                = 0; \
    virtual void        *get_field()               = 0; \
    virtual std::string  to_string() const         = 0; \
    virtual const char  *c_str() const             = 0;
#endif

namespace cffex {
namespace fb {
namespace api {

struct md_base_msg;
struct market_data_entity {
    static market_data_entity *create_entity();

    virtual ~market_data_entity() {}

    virtual void set_instrument_id(const char *v)      = 0;
    virtual void set_exchange_id(int8_t v)             = 0;
    virtual void set_update_sec(int32_t v)             = 0;
    virtual void set_update_msec(int32_t v)            = 0;
    virtual void set_pre_settlement(double v)          = 0;
    virtual void set_pre_close(double v)               = 0;
    virtual void set_pre_open_interest(double v)       = 0;
    virtual void set_open(double v)                    = 0;
    virtual void set_close(double v)                   = 0;
    virtual void set_upper_limit_price(double v)       = 0;
    virtual void set_down_limit_price(double v)        = 0;
    virtual void set_high_price(double v)              = 0;
    virtual void set_low_price(double v)               = 0;
    virtual void set_last_price(double v)              = 0;
    virtual void set_volume(int32_t v)                 = 0;
    virtual void set_turn_over(double v)               = 0;
    virtual void set_open_interest(double v)           = 0;
    virtual void set_bid1_price(double v)              = 0;
    virtual void set_ask1_price(double v)              = 0;
    virtual void set_bid1_volume(int32_t v)            = 0;
    virtual void set_ask1_volume(int32_t v)            = 0;
    virtual void set_bid2_price(double v)              = 0;
    virtual void set_ask2_price(double v)              = 0;
    virtual void set_bid2_volume(int32_t v)            = 0;
    virtual void set_ask2_volume(int32_t v)            = 0;
    virtual void set_bid3_price(double v)              = 0;
    virtual void set_ask3_price(double v)              = 0;
    virtual void set_bid3_volume(int32_t v)            = 0;
    virtual void set_ask3_volume(int32_t v)            = 0;
    virtual void set_bid4_price(double v)              = 0;
    virtual void set_ask4_price(double v)              = 0;
    virtual void set_bid4_volume(int32_t v)            = 0;
    virtual void set_ask4_volume(int32_t v)            = 0;
    virtual void set_bid5_price(double v)              = 0;
    virtual void set_ask5_price(double v)              = 0;
    virtual void set_bid5_volume(int32_t v)            = 0;
    virtual void set_ask5_volume(int32_t v)            = 0;
    virtual void set_bid6_price(double v)              = 0;
    virtual void set_ask6_price(double v)              = 0;
    virtual void set_bid6_volume(int32_t v)            = 0;
    virtual void set_ask6_volume(int32_t v)            = 0;
    virtual void set_bid7_price(double v)              = 0;
    virtual void set_ask7_price(double v)              = 0;
    virtual void set_bid7_volume(int32_t v)            = 0;
    virtual void set_ask7_volume(int32_t v)            = 0;
    virtual void set_bid8_price(double v)              = 0;
    virtual void set_ask8_price(double v)              = 0;
    virtual void set_bid8_volume(int32_t v)            = 0;
    virtual void set_ask8_volume(int32_t v)            = 0;
    virtual void set_bid9_price(double v)              = 0;
    virtual void set_ask9_price(double v)              = 0;
    virtual void set_bid9_volume(int32_t v)            = 0;
    virtual void set_ask9_volume(int32_t v)            = 0;
    virtual void set_bid10_price(double v)             = 0;
    virtual void set_ask10_price(double v)             = 0;
    virtual void set_bid10_volume(int32_t v)           = 0;
    virtual void set_ask10_volume(int32_t v)           = 0;
    virtual void set_iopv(double v)                    = 0;
    virtual void set_dynamic_reference_price(double v) = 0;
    virtual void set_local_timestamp(uint64_t v)       = 0;
    BASE_ENTITY_FUNCTIONS
};

struct inquiry_quote_entity {
    static inquiry_quote_entity *create_entity();

    virtual ~inquiry_quote_entity() {}

    virtual void set_inquiry_id(const char *v)      = 0;
    virtual void set_exchange_id(int8_t v)          = 0;
    virtual void set_instrument_id(const char *v)   = 0;
    virtual void set_inquiry_quote_status(int8_t v) = 0;
    virtual void set_inquiry_time(int32_t v)        = 0;
    virtual void set_response_time(int32_t v)       = 0;
    BASE_ENTITY_FUNCTIONS
};

struct instrument_trading_status_entity {
    static instrument_trading_status_entity *create_entity();

    virtual ~instrument_trading_status_entity() {}

    virtual void set_instrument_id(const char *v)        = 0;
    virtual void set_exchange_id(int8_t v)               = 0;
    virtual void set_instrument_trading_status(int8_t v) = 0;
    BASE_ENTITY_FUNCTIONS
};

}  // namespace api
}  // namespace fb
}  // namespace cffex

#endif
