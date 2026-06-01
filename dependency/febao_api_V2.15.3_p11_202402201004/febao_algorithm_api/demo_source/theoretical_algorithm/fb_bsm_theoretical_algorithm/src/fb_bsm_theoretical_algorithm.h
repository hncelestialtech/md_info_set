/**
 * CFFEX Confidential.
 *
 * @Copyright 2020 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty & zhengmj
 * Date: 2020-09-17
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "i_theoretical_algorithm.h"
#include <unordered_map>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif
	typedef void (*register_func)(void* handle, unsigned short id, const char* name);

	void* create();
	void destroy(void* p);
	void identity(void* handle, register_func f);

#ifdef __cplusplus
}
#endif

namespace cffex {
namespace fb {
namespace plugin {

#define T_MIN 0.0000001  // at least 30 minutes,  0.5/6/245
#define TIMER_INTERVAL  10000
#define TRADING_DAYS 245 //to calculate theta per day
#define UNDERLYING_TOLERANCE 0.0015
#define VOLATILITY_TOLERANCE 0.005
#define MATURITY_TOLERANCE 0.000004
//0.000004 * 245 day/year * 6 trading_hour/day * 60 min/trading_hour * 60 sec/min = 21.168 sec/year

class fb_bsm_theoretical_algorithm : public i_theoretical_algorithm {
public:
	class input_params {
	public:
		input_params() {}
		input_params(unsigned short option_type, double S0, double K, double T, double r, double sig, double cur_price)
			:option_type(option_type), S0(S0), K(K), T(T), r(r), sig(sig), cur_price(cur_price) {};
	public:
		unsigned short option_type;
		double S0, K, T, r, sig, cur_price;
	};

	class uncommon_greeks_params {
	public:
		uncommon_greeks_params() {}
		uncommon_greeks_params(double theta, double rho, double charm, double vanna, double vomma, double speed, double zomma)
			:theta(theta), rho(rho),charm(charm), vanna(vanna), vomma(vomma), speed(speed), zomma(zomma) {};
	public:
		double theta, rho, charm, vanna, vomma, speed, zomma;
	};

	class full_params {
	public:
		full_params() {}
		full_params(unsigned short option_type, double S0, double K, double T, double r, double sig, double cur_price, double theta, double rho, double charm, double vanna, double vomma, double speed, double zomma)
			:input_(option_type, S0, K, T, r, sig, cur_price), uncommon_greeks_(theta, rho, charm, vanna, vomma, speed, zomma) {};
	public:
		input_params input_;
		uncommon_greeks_params uncommon_greeks_;
	};

	fb_bsm_theoretical_algorithm();
	virtual ~fb_bsm_theoretical_algorithm();
	virtual void set_register_timer_function(register_timer_callback f);
	virtual int   calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT theoretical_field* f);
	virtual int   calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT double* price, OUT double* vega);
	virtual int   calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT double* price);
	virtual int   iv_derivable(IN unsigned short option_type, IN double option_price, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param *static_params);
private:
	int Exception(IN double S0, IN double K, IN double T, IN double r, IN double sig);

private:
	double bsm(IN unsigned short option_type, IN double S0, IN double K, IN double T, IN double r, IN double sig);
	double calc_delta(IN unsigned short option_type, IN double S0, IN double K, IN double T, IN double r, IN double sig);
	double calc_gamma(IN double S0, IN double K, IN double T, IN double r, IN double sig);
	double calc_vega(IN double S0, IN double K, IN double T, IN double r, IN double sig);
	int calc_uncommon_greeks(IN unsigned short option_type, IN double S0, IN double K, IN double T, IN double r, IN double sig,IN double cur_price, OUT double* theta, OUT double* rho, OUT double* charm, OUT double* vonna, OUT double* vomma, OUT double* speed, OUT double* zomma);
private:
	void on_timer_uncommon_greeks();
private:
	std::unordered_map<std::string, full_params> base_map_;
	std::unordered_map<std::string, input_params> updated_input_map_;
};

}
}
}


