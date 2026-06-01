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

#ifndef I_TRAILER_IV_H
#define I_TRAILER_IV_H

#include <stdint.h>

#include "i_trialer.h"

namespace cffex {
namespace fb {

class i_trialer_iv :
    public i_trialer_with_id<i_trialer_iv, STRATEGY_TRIALER_IV> {
public:
    class iv_entity {
    public:
        enum { NEWTON = 1, BINARY = 2 };
        virtual ~iv_entity() {}
        virtual void set_instrument_index(uint64_t instrument_index) = 0;
        virtual void set_instrument_id(const char *instrument_id)    = 0;
        virtual void set_option_price(double option_price)           = 0;
        virtual void set_forward_price(double forward_price)         = 0;
        // if not set! newton is default.
        virtual void set_iteration_algorithm_module(int module_id) = 0;

        // if not set! newtow default is 100, binary default is 1000
        virtual void set_iteration_max_times(int max_times) = 0;

        // if not set! newton default is 0.5，binary default is 1
        virtual void set_iteration_beginning(double iv) = 0;

        // if not set! default is tick/5
        virtual void set_iteration_epsilon(double epsilon) = 0;
        virtual void reset_entity()                        = 0;
    };

    i_trialer_iv() {}
    virtual ~i_trialer_iv() {}

    virtual iv_entity *create_entity(IN const char *instrument_id,
                                     IN uint64_t    instrument_index = 0) = 0;
    virtual int32_t    trial(IN iv_entity *entity,
                             OUT double   &imply_volatility)             = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
