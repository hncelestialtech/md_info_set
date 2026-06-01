/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2021-03-15
 */

#ifndef I_TRIALER_H
#define I_TRIALER_H

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

struct imply_volatility_point {
    double strike;
    double volatility;
    double forward_price;
};

class i_trialer {
public:
    i_trialer(uint16_t id) : id_(id) {}
    virtual ~i_trialer() {}

    uint16_t id() const {
        return id_;
    }
    virtual const char *name() const {
        return "";
    }

private:
    uint16_t id_;
};

template <typename trialer, uint16_t __ID__>
struct i_trialer_with_id : public i_trialer {
    enum { ID = __ID__ };
    i_trialer_with_id() : i_trialer(__ID__) {}
    virtual ~i_trialer_with_id() {}

    virtual const char *name() const {
        return typeid(trialer).name();
    }
};

enum {
    STRATEGY_TRIALER_THEORETICAL_PRICE = 0X0001,
    STRATEGY_TRIALER_IV                = 0X0002,
    STRATEGY_TRIALER_VOLATILITY_FIT    = 0X0003,
    STRATEGY_TRIALER_VOLATILITY_CALC   = 0X0004,

    STRATEGY_TRIALER_ALL = 0XFFFF
};

}  // namespace fb
}  // namespace cffex

#endif
