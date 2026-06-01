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

#ifndef I_TRAILER_VOLATILITY_CALC_H
#define I_TRAILER_VOLATILITY_CALC_H

#include <stdint.h>
#include <string>
#include "i_trialer.h"

namespace cffex {
namespace fb {

class i_trialer_volatility_calc :
    public i_trialer_with_id<i_trialer_volatility_calc,
                             STRATEGY_TRIALER_VOLATILITY_CALC> {
public:
    class volatility_param_entity {
    public:
        virtual ~volatility_param_entity() {}
        virtual void set_instrument_index(uint64_t instrument_index) = 0;
        virtual void set_instrument_id(const char *instrument_id)    = 0;
        virtual void set_forward_price(double forward_price)         = 0;
        virtual void set_atm_forward(double v)                       = 0;
        virtual void set_volatility_algorithm_id(int16_t v)          = 0;
        virtual void set_fix_param(IN const char *param_name,
                                   IN double      param_value)            = 0;

        virtual void set_fix_param(IN const char *param_name,
                                   IN const char *param_value) = 0;

        virtual void set_fix_param(IN const char *param_name,
                                   IN int         param_value) = 0;

        virtual void set_fix_param(IN const char param_value[512]) = 0;

        virtual void set_variable_param(IN const char *param_name,
                                        IN int         size,
                                        IN double     *param_value) = 0;

        virtual void set_variable_param(IN const char *param_name,
                                        IN int         size,
                                        IN const char  param_value[][512]) = 0;

        virtual void set_variable_param(IN const char *param_name,
                                        IN int         size,
                                        IN int        *param_value) = 0;

        virtual void set_variable_param(IN const char param_value[4096]) = 0;

        virtual void reset_entity() = 0;
    };

    i_trialer_volatility_calc() {}
    virtual ~i_trialer_volatility_calc() {}

    class volatility_msg {
    public:
        virtual ~volatility_msg() {}
        virtual double get_volatility()                                   = 0;
        virtual bool   get_custom_param(IN const char *param_name,
                                        OUT double    *param_value) const    = 0;
        virtual bool   get_custom_param(OUT char param_value[4096]) const = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    virtual volatility_param_entity *create_entity(
        IN const char *instrument_id, IN uint64_t instrument_index = 0) = 0;
    virtual volatility_msg *trial(volatility_param_entity *entity)      = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
