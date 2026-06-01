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
 * Date: 2020-09-18
 */

#include "fb_plugin_math_helper.h"
#include "fb_bsm_theoretical_algorithm.h"

#ifdef __cplusplus
extern "C"
{
#endif

	void* create() {
		return new cffex::fb::plugin::fb_bsm_theoretical_algorithm();
	}
	void destroy(void* p) {
		delete (cffex::fb::plugin::fb_bsm_theoretical_algorithm*)p;
	}
	void identity(void* handle, register_func f) {
		f(handle, 2, "fb_bsm_theoretical_algorithm");
	}

#ifdef __cplusplus
}
#endif


static void PLOG(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}


namespace cffex {
namespace fb {
namespace plugin {

fb_bsm_theoretical_algorithm::fb_bsm_theoretical_algorithm() {
}

fb_bsm_theoretical_algorithm::~fb_bsm_theoretical_algorithm() {
}

void fb_bsm_theoretical_algorithm::set_register_timer_function(register_timer_callback f) {
	f(TIMER_INTERVAL, std::bind(&fb_bsm_theoretical_algorithm::on_timer_uncommon_greeks, this), true);
};


int fb_bsm_theoretical_algorithm::calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT theoretical_field* f){
	double S0 = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = variable_params->get_left_trading_years();
	double r = variable_params->get_rate();
	double sig = volatility_params->get_volatility();
	double volatility_partial = 0.0;
	if (!volatility_params->get_custom_param("volatility_partial", &volatility_partial)) {
		volatility_partial = 0.0;
	}

	std::string instrument = variable_params->get_instrument_id();

	if (Exception(S0, K, T, r, sig) == -1){
		PLOG("fb_bsm_theoretical_algorithm::%s,there are exceptional data:S0[%lf] or K[%lf] or T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n", __FUNCTION__, S0, K, T, r, sig);
		return -1;
	}
	T = std::max(T, T_MIN);

	//calculate theoretical price by bsm for european futures option.
	double theo_price = bsm(option_type, S0, K, T, r, sig);
	//calculate common greeks.
	double delta = calc_delta(option_type, S0, K, T, r, sig);
	double gamma = calc_gamma(S0, K, T, r, sig);
	double vega = calc_vega(S0, K, T, r, sig);
	double skew_delta = delta + volatility_partial * vega;

	f->set_theoretical_price(theo_price);
	f->set_delta(delta);
	f->set_gamma(gamma);
	f->set_vega(vega);
	f->set_skew_delta(skew_delta);


	auto iter = base_map_.find(instrument);
	if (iter != base_map_.end()) {
		// previous record found
		input_params& input = iter->second.input_;
		uncommon_greeks_params& uncommon_greeks = iter->second.uncommon_greeks_;
		if (math_helper::equal(input.r, r) && math_helper::greater_equal(input.T, T) && math_helper::less(input.T-T, MATURITY_TOLERANCE) && std::fabs((input.S0-S0)/S0) < UNDERLYING_TOLERANCE  && std::fabs(input.sig-sig) < VOLATILITY_TOLERANCE) {
			// if only T, S0 and sig diff, then use old uncommon greeks values instead of exact calculation
			// if diff is too big, shouldn't use old values
			f->set_theta(uncommon_greeks.theta);
			f->set_rho(uncommon_greeks.rho);
			f->set_charm(uncommon_greeks.charm);
			f->set_vanna(uncommon_greeks.vanna);
			f->set_vomma(uncommon_greeks.vomma);
			f->set_speed(uncommon_greeks.speed);
			f->set_zomma(uncommon_greeks.zomma);

			// add to updated_map, used by on_timer()
			input_params input_updated(option_type, S0, K, T, r, sig, theo_price);
			updated_input_map_[instrument] = input_updated;
			return 0;
		}
	}

	//if the map does not store the high_oder_greeks or cannot calculate, we have to calculate them on time.
	double theta = 0.0;
	double rho = 0.0;
	double charm = 0.0;
	double vanna = 0.0;
	double vomma = 0.0;
	double speed = 0.0;
	double zomma = 0.0;
	calc_uncommon_greeks(option_type, S0, K, T, r, sig, theo_price, &theta, &rho, &charm, &vanna, &vomma, &speed, &zomma);

	f->set_theta(theta);
	f->set_rho(rho);
	f->set_charm(charm);
	f->set_vanna(vanna);
	f->set_vomma(vomma);
	f->set_speed(speed);
	f->set_zomma(zomma);
	// update base map after calculate
	full_params full(option_type, S0, K, T, r, sig, theo_price, theta, rho, charm, vanna, vomma, speed, zomma);
	base_map_[instrument] = full;
	// don't need to calcuate when on_timer_high_order_greeks
	updated_input_map_.erase(instrument);

	return 0;
}

int fb_bsm_theoretical_algorithm::calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT double* price, OUT double* vega){
	double S0 = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = variable_params->get_left_trading_years();
	double r = variable_params->get_rate();
	double sig = volatility_params->get_volatility();


	if (Exception(S0, K, T, r, sig) == -1){
		PLOG("fb_bsm_theoretical_algorithm::%s,there are exceptional data:S0[%lf] or K[%lf] or T[%lf] or r[%lf] or sig[%lf] or steps[%d] is invalid, failed to calculate\n", __FUNCTION__, S0, K, T, r, sig);
		return -1;
	}
	T = std::max(T, T_MIN);

	*price = bsm(option_type, S0, K, T, r, sig);
	*vega = calc_vega(S0, K, T, r, sig);
	return 0;
}

int fb_bsm_theoretical_algorithm::calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT double* price){
	double S0 = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = variable_params->get_left_trading_years();
	double r = variable_params->get_rate();
	double sig = volatility_params->get_volatility();


	if (Exception(S0, K, T, r, sig) == -1){
		PLOG("fb_bsm_theoretical_algorithm::%s,there are exceptional data:S0[%lf] or K[%lf] or T[%lf] or r[%lf] or sig[%lf] or steps[%d] is invalid, failed to calculate\n", __FUNCTION__, S0, K, T, r, sig);
		return -1;
	}
	T = std::max(T, T_MIN);

	*price = bsm(option_type, S0, K, T, r, sig);
	return 0;
}


int fb_bsm_theoretical_algorithm::iv_derivable(IN unsigned short option_type, IN double option_price, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param *static_params) {
	double S0 = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = std::max(variable_params->get_left_trading_years(), T_MIN);
	double r = variable_params->get_rate();

	if ((option_type == PLUGIN_OPTION_CALL) && option_price > (S0 - K * std::exp(0-r*T)) ) {
		return 0;
	}
	if ((option_type == PLUGIN_OPTION_PUT) && option_price > (K * std::exp(0-r*T) - S0) ) {
		return 0;
	}
	return -1;
}

/******** private ********/

int fb_bsm_theoretical_algorithm::Exception(IN double S0, IN double K, IN double T, IN double r, IN double sig){
	if (!math_helper::greater(S0, 0) || !math_helper::greater(K, 0) || !math_helper::greater(T, 0) || !math_helper::greater(sig, 0) || !math_helper::greater_equal(r, 0) ||
		!math_helper::valid_price(S0) || !math_helper::valid_price(K) || !math_helper::valid_price(T) ||
		!math_helper::valid_price(r) || !math_helper::valid_price(sig)
		) {
		return -1;
	}
	return 0;
}

void fb_bsm_theoretical_algorithm::on_timer_uncommon_greeks() {
	if (updated_input_map_.empty()) {
		return;
	}

	for (auto iter = updated_input_map_.begin(); iter != updated_input_map_.end(); iter++) {
		std::string instrument = iter->first;
		input_params& input = iter->second;
		double theta, rho, charm, vanna, vomma, speed, zomma;
		calc_uncommon_greeks(input.option_type, input.S0, input.K, input.T, input.r, input.sig, input.cur_price, &theta, &rho, &charm, &vanna, &vomma, &speed, &zomma);
		full_params full(input.option_type, input.S0, input.K, input.T, input.r, input.sig, input.cur_price, theta, rho, charm, vanna, vomma, speed, zomma);
		base_map_[instrument] = full;
	}
	updated_input_map_.clear();
}

double fb_bsm_theoretical_algorithm::bsm(IN unsigned short option_type, IN double S0, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(S0 / K) + (r + 0.5 * sig * sig) * T;
	d1 = d1 / sig / std::sqrt(T);
	double d2 = d1 - sig * std::sqrt(T);
	if (option_type == PLUGIN_OPTION_CALL) {
		return S0 * math_helper::normal_cdf(d1) - K * std::exp(-r * T) * math_helper::normal_cdf(d2);
	}
	else if (option_type == PLUGIN_OPTION_PUT) {
		return -S0 * math_helper::normal_cdf(-d1) + K * std::exp(-r * T) * math_helper::normal_cdf(-d2);
	}
	else {
		return -1;
	}
}

double fb_bsm_theoretical_algorithm::calc_delta(IN unsigned short option_type, IN double S0, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(S0 / K) + (r + 0.5 * sig * sig) * T;
	d1 = d1 / sig / std::sqrt(T);
	if (option_type == PLUGIN_OPTION_CALL) {
		return math_helper::normal_cdf(d1);
	}
	else if (option_type == PLUGIN_OPTION_PUT) {
		return math_helper::normal_cdf(d1) - 1;
	}
	else {
		return -1;
	}
}

double fb_bsm_theoretical_algorithm::calc_gamma(IN double S0, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(S0 / K) + (r + 0.5 * sig * sig) * T;
	d1 = d1 / sig / std::sqrt(T);
	return math_helper::normal_pdf(d1) / S0 / sig / std::sqrt(T);
}

double fb_bsm_theoretical_algorithm::calc_vega(IN double S0, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(S0 / K) + (r + 0.5 * sig * sig) * T;
	d1 = d1 / sig / std::sqrt(T);
	return S0 * math_helper::normal_pdf(d1) * sqrt(T) / 100;
}

int fb_bsm_theoretical_algorithm::calc_uncommon_greeks(IN unsigned short option_type, IN double S0, IN double K, IN double T, IN double r, IN double sig, IN double cur_price, OUT double *theta, OUT double *rho, OUT double *charm, OUT double *vanna, OUT double *vomma, OUT double *speed, OUT double *zomma) {
	const double dt = 0.00408163f; // per day,  1/245= 0.00408163
	double tomorrow_T = T - dt;

	double d1 = std::log(S0 / K) + (r + 0.5 * sig * sig) * T;
	d1 = d1 / sig / std::sqrt(T);
	double d2 = d1 - sig * std::sqrt(T);
	double d1_npdf = math_helper::normal_pdf(d1);
	double discount_factor = std::exp(-r * T);
	if (option_type == PLUGIN_OPTION_CALL) {
		if(math_helper::less_equal(tomorrow_T, T_MIN)) {
			*theta = std::max(S0 - K, 0.0) - cur_price;
		} else {
			*theta = (-S0 * d1_npdf * sig * 0.5 / std::sqrt(T) - r * K * discount_factor * math_helper::normal_cdf(d2)) / TRADING_DAYS;
		}
		*rho = T * K * discount_factor * math_helper::normal_cdf(d2) / 100;
	}
	else if (option_type == PLUGIN_OPTION_PUT) {
		if(math_helper::less_equal(tomorrow_T, T_MIN)) {
			*theta = std::max(K - S0, 0.0) - cur_price;
		} else {
			*theta = (-S0 * d1_npdf * sig * 0.5 / std::sqrt(T) + r * K * discount_factor * math_helper::normal_cdf(-d2)) / TRADING_DAYS;
		}
		*rho = -T * K * std::exp(-r * T) * math_helper::normal_cdf(-d2) / 100;
	}
	else {
		return -1;
	}

	*charm = -d1_npdf * (r / (sig * std::sqrt(T)) - d2 / (2 * T)) / TRADING_DAYS;
	*vanna = -d2 * d1_npdf / sig / 100;//d(vega)/dS per percent
	*vomma = S0 * d1_npdf * std::sqrt(T) * d1 * d2 / sig / 10000;//d(vega)/d(sigma) per percent
	*speed = -d1_npdf * (1 + d1 / (sig * std::sqrt(T))) / (S0 * S0 * sig * std::sqrt(T));//d(gamma) / dS
	*zomma = (d1 * d2 - 1) * d1_npdf / (S0 * sig * sig * std::sqrt(T)) / 100;//d(gamma)/d(sigma) per percent
	return 0;
}

}
}
}
