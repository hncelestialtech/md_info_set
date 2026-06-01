#include "stdio.h"
#include "wing_model_impl.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "volatility_common.h"

#ifdef  _WINDOWS
#include <string.h>
#endif
#ifdef  _WINDOWS
	#define STRTOK strtok_s
#else
	#define STRTOK strtok_r
#endif

namespace cffex {
namespace fb {
namespace plugin {

wing_model_impl::wing_model_impl()
{
    atm_forward_ = 0;
    atm_volatility_ = 0;
    slope_ = 0;
    call_curvature_ = 0;
    call_cutoff_ = 0;
    put_curvature_ = 0;
    put_cutoff_ = 0;
	volatility_partial_ = CFFEX_DOUBLE_NULL;
}

wing_model_impl::~wing_model_impl() {};


void wing_model_impl::get_param_list(char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	static const char wing_param[] = "atm_forward:double;atm_volatility:double;slope:double;call_curvature:double;call_cutoff:double;put_curvature:double;put_cutoff:double";
	strncpy(fixed_param_list, wing_param, sizeof(wing_param));
	variable_param_list[0] = '\0';
	return;
}

void wing_model_impl::get_param_value(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	sprintf(fixed_model_param, "atm_forward:%.6f;atm_volatility:%.6f;slope:%.6f;call_curvature:%.6f;call_cutoff:%.6f;put_curvature:%.6f;put_cutoff:%.6f",
						atm_forward_, atm_volatility_, slope_, call_curvature_, call_cutoff_, put_curvature_, put_cutoff_);
	variable_model_param[0] = '\0';
	return;
}

int wing_model_impl::set_param_value(const char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], const char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH])
{
	static const int wing_param_num = 7;
	if (fixed_model_param == NULL) {
		return INPUT_OUTPUT_PARAM_IS_NULL;
	}

	char param[MAX_FIXED_MODEL_PARAM_LENGTH];
	strncpy(param, fixed_model_param, sizeof(char)*MAX_FIXED_MODEL_PARAM_LENGTH);
	const char delim[] = ":;";
	char *key;
	char *value;
	char *last = NULL;
	int num = 1;
	key = STRTOK(param, delim, &last);
	value = STRTOK(NULL, delim, &last);
	while (key!=NULL && value!=NULL) {
		if (!set_single_param(key, atof(value))) {
			return INPUT_PARAM_IS_INVALID;
		}
		if (num++ == wing_param_num && strlen(variable_model_param) == 0) {
			return RETURN_SUCCESS;
		}
		key = STRTOK(NULL, delim, &last);
		if (key == NULL) {
			return INPUT_PARAM_IS_INVALID;
		}
		value = STRTOK(NULL, delim, &last);
		if (value == NULL) {
			return INPUT_PARAM_IS_INVALID;
		}
	}
	return INPUT_PARAM_IS_INVALID;
}

bool wing_model_impl::is_dynamic_model(){
	return false;
}

void wing_model_impl::get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]) {
	static const char wing_volatility_custom_param[] = "volatility_partial";
	strcpy(volatility_custom_param_name, wing_volatility_custom_param);
	return;
}

bool wing_model_impl::get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward, IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]) {
	if (volatility == NULL){
		return false;
	}
	volatility_custom_param_value[0] = volatility_partial_;
	double x = log(strike_price / atm_forward_);
    if (x <= put_cutoff_){
		*volatility = atm_volatility_ + slope_*x + put_curvature_* (2*put_cutoff_*x - put_cutoff_*put_cutoff_);
		return true;
    }
	if (x > put_cutoff_ && x <= 0){
		*volatility = atm_volatility_ + slope_*x + put_curvature_*x*x;
		return true;
    }
	if ( x > 0 && x <= call_cutoff_){
		*volatility = atm_volatility_ + slope_*x + call_curvature_*x*x;
		return true;
    }
	if (x > call_cutoff_){
		*volatility = atm_volatility_ + slope_*x + call_curvature_* (2*call_cutoff_*x - call_cutoff_*call_cutoff_);
		return true;
    }
	return false;
}

bool wing_model_impl::set_single_param(char *key, float value) {
	if (strcmp(key, "atm_forward") == 0) {
		if (!is_valid_atm_forward(value)) {
			return false;
		}
		atm_forward_ = value;
	}
	else if (strcmp(key, "atm_volatility") == 0) {
		if (!is_valid_atm_volatility(value)) {
			return false;
		}
		atm_volatility_ = value;
	}
	else if (strcmp(key, "slope") == 0) {
		if (!is_valid_slope(value)) {
			return false;
		}
		slope_ = value;
	}
	else if (strcmp(key, "call_curvature") == 0) {
		if (!is_valid_call_curvature(value)) {
			return false;
		}
		call_curvature_ = value;
	}
	else if (strcmp(key, "call_cutoff") == 0) {
		if (!is_valid_call_cutoff(value)) {
			return false;
		}
		call_cutoff_ = value;
	}
	else if (strcmp(key, "put_curvature") == 0) {
		if (!is_valid_put_curvature(value)) {
			return false;
		}
		put_curvature_ = value;
	}
	else if (strcmp(key, "put_cutoff") == 0) {
		if (!is_valid_put_cutoff(value)) {
			return false;
		}
		put_cutoff_ = value;
	}
	else {
		return false;
	}
	return true;
}

/* is_valid */
bool wing_model_impl::valid_price(double price) { return (price > -DBL_MAX && price < DBL_MAX);}

bool wing_model_impl::is_valid_atm_forward(double atm_forward) const { return atm_forward > 0; }

bool wing_model_impl::is_valid_atm_volatility(double atm_volatility) const { return atm_volatility >= 0; }

bool wing_model_impl::is_valid_slope(double slope) const { return true; }

bool wing_model_impl::is_valid_call_curvature(double call_curvature) const { return true;}

bool wing_model_impl::is_valid_call_cutoff(double call_cutoff) const { return call_cutoff >=0;  }

bool wing_model_impl::is_valid_put_curvature(double put_curvature) const { return true; }

bool wing_model_impl::is_valid_put_cutoff(double put_cutoff) const { return put_cutoff <=0; }

}
}
}
