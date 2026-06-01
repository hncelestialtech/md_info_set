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

#ifndef I_TRAILER_THEORETICAL_PRICE_H
#define I_TRAILER_THEORETICAL_PRICE_H

#include <stdint.h>
#include <string>
#include "i_trialer.h"

namespace cffex {
namespace fb {

class i_trialer_theoretical_price :
    public i_trialer_with_id<i_trialer_theoretical_price,
                             STRATEGY_TRIALER_THEORETICAL_PRICE> {
public:
    class theoretical_entity {
    public:
        virtual ~theoretical_entity() {}
        virtual void set_instrument_index(uint64_t v)                   = 0;
        virtual void set_instrument_id(const char *v)                   = 0;
        virtual void set_option_forward(double v)                       = 0;
        virtual void set_volatility(double v)                           = 0;
        virtual void set_volatility_custom_param(IN const char *param_name,
                                                 IN double param_value) = 0;
        virtual void set_volatility_custom_param(
            IN const char param_value[4096]) = 0;

        virtual void reset_entity() = 0;
    };

    i_trialer_theoretical_price() {}
    virtual ~i_trialer_theoretical_price() {}

    class theoretical_price_msg {
    public:
        virtual ~theoretical_price_msg() {}
        virtual double get_theoretical_price() const = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    class greeks_msg {
    public:
        virtual ~greeks_msg() {}
        virtual double get_delta() const                         = 0;
        virtual double get_gamma() const                         = 0;
        virtual double get_theta() const                         = 0;
        virtual double get_vega() const                          = 0;
        virtual double get_rho() const                           = 0;
        virtual double get_charm() const                         = 0;
        virtual double get_vanna() const                         = 0;
        virtual double get_vomma() const                         = 0;
        virtual double get_speed() const                         = 0;
        virtual double get_zomma() const                         = 0;
        virtual bool   get_custom_greek(IN const char *name,
                                        OUT double    *value) const = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    virtual theoretical_entity *create_entity(
        IN const char *instrument_id, IN uint64_t instrument_index = 0) = 0;
    virtual theoretical_price_msg *trial_theoretical_price(
        theoretical_entity *entity)                              = 0;
    virtual greeks_msg *trial_greeks(theoretical_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
