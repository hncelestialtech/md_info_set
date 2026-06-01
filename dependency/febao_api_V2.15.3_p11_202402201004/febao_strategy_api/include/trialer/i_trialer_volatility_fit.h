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

#ifndef I_TRAILER_VOLATILITY_FIT_H
#define I_TRAILER_VOLATILITY_FIT_H

#include <stdint.h>

#include <string>

#include "i_trialer.h"

namespace cffex {
namespace fb {

class i_trialer_volatility_fit :
    public i_trialer_with_id<i_trialer_volatility_fit,
                             STRATEGY_TRIALER_VOLATILITY_FIT> {
public:
    class volatility_fit_entity {
    public:
        virtual ~volatility_fit_entity() {}
        virtual void set_serial_index(uint64_t serial_index)       = 0;
        virtual void set_serial_id(const char *serial_id)          = 0;
        virtual void set_algorithm_id(int v)                       = 0;
        virtual void set_atm_forward(double v)                     = 0;
        virtual void set_iv_points(int                     size,
                                   imply_volatility_point *points) = 0;

        virtual void reset_entity() = 0;
    };

    i_trialer_volatility_fit() {}
    virtual ~i_trialer_volatility_fit() {}

    class volatility_param_msg {
    public:
        virtual ~volatility_param_msg() {}
        virtual bool get_fix_param(IN const char *param_name,
                                   OUT int       *param_value) const      = 0;
        virtual bool get_fix_param(IN const char *param_name,
                                   OUT double    *param_value) const   = 0;
        virtual bool get_fix_param(IN const char *param_name,
                                   OUT char       param_value[512]) const = 0;
        virtual bool get_fix_param(OUT char param_value[512]) const = 0;

        virtual bool get_variable_param(IN const char *param_name,
                                        IN const int   param_size,
                                        OUT int       *param_value) const        = 0;
        virtual bool get_variable_param(IN const char *param_name,
                                        IN const int   param_size,
                                        OUT double    *param_value) const     = 0;
        virtual bool get_variable_param(IN const char *param_name,
                                        IN const int   param_size,
                                        OUT char param_value[][512]) const = 0;
        virtual bool get_variable_param(OUT char param_value[4096]) const  = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    virtual volatility_fit_entity *create_entity()                      = 0;
    virtual volatility_param_msg  *trial(volatility_fit_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
