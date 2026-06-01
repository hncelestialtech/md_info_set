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

#ifndef FB_I_STRATEGY_CALLER_H
#define FB_I_STRATEGY_CALLER_H

#include <stdint.h>

#include <typeinfo>


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {

class i_caller {
public:
    i_caller(uint16_t id) : id_(id) {}
    virtual ~i_caller() {}
    uint16_t id() const {
        return id_;
    }
    virtual const char *name() const {
        return "";
    }
    virtual void on_deleted() {}

private:
    uint16_t id_;
};

template <typename caller, uint16_t __ID__>
struct i_caller_with_id : public i_caller {
    enum { ID = __ID__ };
    i_caller_with_id() : i_caller(__ID__) {}
    virtual ~i_caller_with_id() {}
    virtual const char *name() const {
        return typeid(caller).name();
    }
};

/* caller priority */
enum { MAX_PRIORITY = 1, MIN_PRIORITY = 3 };

enum {
    STRATEGY_CALLER_TIMER = 0X0001,
    STRATEGY_CALLER_ORDER,
    STRATEGY_CALLER_QUOTE,
    STRATEGY_CALLER_INSTRUMENT_PARAM,
    STRATEGY_CALLER_CUSTOM_PARAM,
    STRATEGY_CALLER_INSTRUMENT_GROUP_PARAM,
    STRATEGY_CALLER_INSTANCE_OPERATOR,
    STRATEGY_CALLER_INSTRUMENT_PRICING_PARAM,
    STRATEGY_CALLER_SERIAL_PRICING_PARAM,
    STRATEGY_CALLER_INSTRUMENT_THEO_PRICING_PARAM,
    STRATEGY_CALLER_VOLATILITY_PARAM,
    STRATEGY_CALLER_VOLATILITY_OFFSET,
    STRATEGY_CALLER_LOG,
    STRATEGY_CALLER_TRADE,
    STRATEGY_CALLER_VOLATILITY_CONFIG,
    STRATEGY_CALLER_COMB_STOCK,
    STRATEGY_CALLER_COMB_COMMODITY,
    STRATEGY_CALLER_SERIAL_BASE_PRICE,
    STRATEGY_CALLER_SERIAL_THEO_CUSTOM_PARAM,

    STRATEGY_CALLER_ALL = 0XFFFF
};

}  // namespace fb
}  // namespace cffex

#endif
