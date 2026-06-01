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
 * Date: 2021-02-23
 */

#ifdef _WINDOWS
#include "volatility_win_api.h"
#define CLASSNAME
#else
#include "fb_wing_static_linux_api.h"
#include "wing_fit_impl.h"
#define CLASSNAME fb_wing_static_algorithm::
static cffex::fb::plugin::wing_fit_impl g_wing_fit_impl;
#endif

#include "wing_model_impl.h"

static cffex::fb::plugin::wing_model_impl g_wing_model_impl;

#ifndef _WINDOWS
namespace cffex {
namespace fb {
namespace plugin {
#endif

void CLASSNAME get_param_list(OUT char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], OUT char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	return g_wing_model_impl.get_param_list(fixed_param_list, variable_param_list);
}

void CLASSNAME get_param_value(OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	return g_wing_model_impl.get_param_value(fixed_model_param, variable_model_param);
}

int CLASSNAME set_param_value(IN const char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], IN const char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	return g_wing_model_impl.set_param_value(fixed_model_param, variable_model_param);
}

bool CLASSNAME get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward, IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]) {
	return g_wing_model_impl.get_volatility(strike_price, atm_forward, option_forward, left_trading_years, rate, volatility, volatility_custom_param_value);
}

bool CLASSNAME is_dynamic_model(){
	return g_wing_model_impl.is_dynamic_model();
}

void CLASSNAME get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]) {
	return g_wing_model_impl.get_volatility_custom_param_name(volatility_custom_param_name);
}

#ifndef _WINDOWS
int CLASSNAME fit(IN const volatility_point *p_volatility_points,
                       			  IN const unsigned int size,
                           		  IN const double atm_forward,
								  IN const double left_trading_years,
                           		  IN const double rate,
                           		  OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
                           		  OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	int ret_code = g_wing_fit_impl.fit(p_volatility_points, size, atm_forward, left_trading_years, rate, fixed_model_param, variable_model_param);
	if (ret_code != RETURN_SUCCESS) {
		return ret_code;
	}
	set_param_value(fixed_model_param, variable_model_param);
	return ret_code;
}
#endif


#ifndef _WINDOWS
}
}
}
#endif
