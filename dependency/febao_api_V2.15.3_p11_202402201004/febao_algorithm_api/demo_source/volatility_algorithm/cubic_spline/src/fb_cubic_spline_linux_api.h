/**
 * CFFEX Confidential.
 *
 * @Copyright 2021 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty&zhengmj
 * Date: 2021-02-26
 */

#include "i_volatility_algorithm.h"

namespace cffex {
namespace fb {
namespace plugin {

class fb_cubic_spline_algorithm : public i_volatility_algorithm {
public:
    fb_cubic_spline_algorithm() {}
    virtual ~fb_cubic_spline_algorithm() {}
    virtual void get_param_list(OUT char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], OUT char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
    virtual void get_param_value(OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
    virtual int set_param_value(IN const char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], IN const char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
    virtual bool is_dynamic_model();
    virtual void get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]);
    virtual bool get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward, IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]);
    virtual int fit(IN const volatility_point *p_volatility_points,
                           IN const unsigned int size,
                           IN const double atm_forward,
                           IN const double left_trading_years,
                           IN const double rate,
                           OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
                           OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
};


}
}
}


#ifdef __cplusplus
extern "C"
{
#endif

void* create() {
    return new cffex::fb::plugin::fb_cubic_spline_algorithm();
}
void destroy(void* p) {
    delete (cffex::fb::plugin::fb_cubic_spline_algorithm *)p;
}

typedef void (*register_func)(void *handle, unsigned short id, const char *name);
void identity(void* handle,register_func f) {
    f(handle,0x0002,"cubic_spline_volatility_algorithm");
}

#ifdef __cplusplus
}
#endif

