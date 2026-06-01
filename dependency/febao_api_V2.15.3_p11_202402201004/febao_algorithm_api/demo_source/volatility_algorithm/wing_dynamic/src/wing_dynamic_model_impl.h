#ifndef CFFEX_MM_WING_DYNAMIC_MODEL_IMPL_H
#define CFFEX_MM_WING_DYNAMIC_MODEL_IMPL_H

#include "volatility_common.h"
#include <algorithm>

/** float defination */
#ifndef DBL_MAX
#define DBL_MAX __DBL_MAX__
#endif

#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

namespace cffex {
namespace fb {
namespace plugin {

class wing_dynamic_model_impl {
public:
	wing_dynamic_model_impl();
    virtual ~wing_dynamic_model_impl();

    void get_param_list(char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

    void get_param_value(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

    int set_param_value(const char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], const char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

    bool is_dynamic_model();
    //If the return value is true, the get_volatility API will be called because of atm_forward or option_forward changes.
    //If the return value is false, the get_volatility API will not be called because of atm_forward or option_forward changes.

    void get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]);

    bool get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward, IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]);

protected:
	bool set_single_param(char *key, float value);

protected:
    bool valid_price(double price);
    bool is_valid_atm_volatility(double atm_volatility) const;
    bool is_valid_slope(double slope) const;
    bool is_valid_call_curvature(double call_curvature) const;
    bool is_valid_call_cutoff(double call_cutoff) const;
    bool is_valid_put_curvature(double put_curvature) const;
    bool is_valid_put_cutoff(double put_cutoff) const;

private:
    // basic Wing Parameters
    double atm_volatility_;
    double slope_;
    double call_curvature_;
    double call_cutoff_;
    double put_curvature_;
    double put_cutoff_;
    double volatility_partial_;
};

}
}
}


#endif
