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
 * Date: 2020-09-22
 */

#include "fb_plugin_math_helper.h"
#include "fb_black76_custom_demo_theoretical_algorithm.h"
#include <cstring>

#ifdef __cplusplus
extern "C"
{
#endif

	void* create() {
		return new cffex::fb::plugin::fb_black76_custom_demo_theoretical_algorithm();
	}
	void destroy(void* p) {
		delete (cffex::fb::plugin::fb_black76_custom_demo_theoretical_algorithm*)p;
	}
	void identity(void* handle, register_func f) {
		f(handle, 21, "fb_black76_custom_demo_theoretical_algorithm");
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

fb_black76_custom_demo_theoretical_algorithm::fb_black76_custom_demo_theoretical_algorithm():custom_greeks_(6, 0) {
}

fb_black76_custom_demo_theoretical_algorithm::~fb_black76_custom_demo_theoretical_algorithm() {
}

void fb_black76_custom_demo_theoretical_algorithm::set_register_timer_function(register_timer_callback f) {
	f(TIMER_INTERVAL, std::bind(&fb_black76_custom_demo_theoretical_algorithm::on_timer_uncommon_greeks, this), true);
};

void  fb_black76_custom_demo_theoretical_algorithm::get_custom_greek_list(OUT char custom_greek_list[MAX_CUSTOM_GREEK_NAME_LEN]) {
	static const char custom_greeks[] = "deltaplusgamma;deltamultigamma;gammaplusvega;gammamultivega;deltaplusvega;deltamultivega";
	strncpy(custom_greek_list, custom_greeks, sizeof(custom_greeks));
	return;
}

void fb_black76_custom_demo_theoretical_algorithm::get_custom_param_list(OUT char custom_param_list[MAX_CUSTOM_NAME_LEN]) {
	static const char custom_params[] = "black76_custom1;black76_custom2;black76_custom3";
	strncpy(custom_param_list, custom_params, sizeof(custom_params));
	return;
}

int fb_black76_custom_demo_theoretical_algorithm::calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT theoretical_field* f){
	double F = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = variable_params->get_left_trading_years();
	double r = variable_params->get_rate();
	double sig = volatility_params->get_volatility();
	double volatility_partial = 0.0;
	double long_margin_rate = variable_params->get_long_margin_rate();
	double long_margin_amt  = variable_params->get_long_margin_amt();
	double short_margin_rate = variable_params->get_short_margin_rate();
	double short_margin_amt = variable_params->get_short_margin_amt();
	if (!volatility_params->get_custom_param("volatility_partial", &volatility_partial)) {
		volatility_partial = 0.0;
	}

	/*test custom_param */
	double custom1, custom2, custom3;
	bool succ = true;
	succ = succ && custom_params->get_param("black76_custom1", &custom1);
	succ = succ && custom_params->get_param("black76_custom2", &custom2);
	succ = succ && custom_params->get_param("black76_custom3", &custom3);
	if(succ) {
		PLOG("fb_black76_custom_demo_theoretical_algorithm::%s, custom_param: custom1[%lf] custom2[%lf] custom3[%lf]\n", __FUNCTION__, custom1, custom2, custom3);
	} else {
		PLOG("fb_black76_custom_demo_theoretical_algorithm::%s, get custom_param failed.\n", __FUNCTION__);
	}

	std::string instrument = variable_params->get_instrument_id();

	if (Exception(F, K, T, r, sig) == -1){
		PLOG("fb_black76_custom_demo_theoretical_algorithm::%s,there are exceptional data:F[%lf] or K[%lf] or T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n", __FUNCTION__, F, K, T, r, sig);
		return -1;
	}
	T = std::max(T, T_MIN);

	//calculate theoretical price & common greeks by black76 for european futures option directly.
	double theo_price = black76(option_type, F, K, T, r, sig);
	double delta = calc_delta(option_type, F, K, T, r, sig);
	double gamma = calc_gamma( F, K, T, r, sig);
	double vega = calc_vega(F, K, T, r, sig);

	f->set_theoretical_price(theo_price);
	f->set_delta(delta);
	f->set_gamma(gamma);
	f->set_vega(vega);
	f->set_skew_delta(delta + volatility_partial * vega);
	// auto t0 = get_current_nano_sec();
	custom_greeks_[0] = delta + gamma;
	custom_greeks_[1] = delta * gamma;
	custom_greeks_[2] = gamma + vega;
	custom_greeks_[3] = gamma * vega;
	custom_greeks_[4] = delta + vega;
	custom_greeks_[5] = delta * vega;
	// auto t1 = get_current_nano_sec();
	f->set_custom_greeks(custom_greeks_);
	// auto t2 = get_current_nano_sec();
	// PLOG("lact before: init[%ld] lact[%ld]\n", t1 - t0, t2 - t1);

	//check whether the base_map has stored the data. If it has done, read it and update it in on_timer.
	auto iter = base_map_.find(instrument);
	if (iter != base_map_.end()) {
		// previous record found
		input_params& input = iter->second.input_;
		uncommon_greeks_params& uncommon_greeks = iter->second.uncommon_greeks_;
		if (math_helper::equal(input.r, r) && math_helper::greater_equal(input.T, T) && math_helper::less(input.T-T, MATURITY_TOLERANCE) && std::fabs(input.F-F)/input.F < UNDERLYING_TOLERANCE && std::fabs(input.sig-sig) < VOLATILITY_TOLERANCE) {
			// if only T, F and sig diff, then use old uncommon greeks values instead of exact calculation
			// if diff is too big, shouldn't use old values
			f->set_theta(uncommon_greeks.theta);
			f->set_rho(uncommon_greeks.rho);
			f->set_charm(uncommon_greeks.charm);
			f->set_vanna(uncommon_greeks.vanna);
			f->set_vomma(uncommon_greeks.vomma);
			f->set_speed(uncommon_greeks.speed);
			f->set_zomma(uncommon_greeks.zomma);

			// add to updated_map, used by on_timer()
			input_params input_updated(option_type, F, K, T, r, sig);
			updated_uncommon_greeks_map_[instrument] = input_updated;
			return 0;
		}
	}

	//if the map does not store the uncommon_greeks or cannot calculate, we have to calculate them on time.
	double theta, rho, charm, vanna, vomma, speed, zomma;
	calc_uncommon_greeks(option_type, F, K, T, r, sig, &theta, &rho, &charm, &vanna, &vomma, &speed, &zomma);
	f->set_theta(theta);
	f->set_rho(rho);
	f->set_charm(charm);
	f->set_vanna(vanna);
	f->set_vomma(vomma);
	f->set_speed(speed);
	f->set_zomma(zomma);
	f->set_skew_delta(delta + volatility_partial * vega);
	// t0 = get_current_nano_sec();
	custom_greeks_[0] = delta + gamma;
	custom_greeks_[1] = delta * gamma;
	custom_greeks_[2] = gamma + vega;
	custom_greeks_[3] = gamma * vega;
	custom_greeks_[4] = delta + vega;
	custom_greeks_[5] = delta * vega;
	// t1 = get_current_nano_sec();
	f->set_custom_greeks(custom_greeks_);
	// t2 = get_current_nano_sec();
	// PLOG("lact after: init[%ld] lact[%ld]\n", t1 - t0, t2 - t1);
	// update base map after calculate
	full_params full(option_type, F, K, T, r, sig, theta, rho, charm, vanna, vomma, speed, zomma);
	base_map_[instrument] = full;
	// don't need to calcuate when on_timer_uncommon_greeks
	updated_uncommon_greeks_map_.erase(instrument);

	return 0;
}

int fb_black76_custom_demo_theoretical_algorithm::calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT double* price, OUT double* vega){
	double F = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = variable_params->get_left_trading_years();
	double r = variable_params->get_rate();
	double sig = volatility_params->get_volatility();


	if (Exception(F, K, T, r, sig) == -1){
		PLOG("fb_black76_custom_demo_theoretical_algorithm::%s,there are exceptional data:F[%lf] or K[%lf] or T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n", __FUNCTION__, F, K, T, r, sig);
		return -1;
	}
	T = std::max(T, T_MIN);

	*price = black76(option_type, F, K, T, r, sig);
	*vega = calc_vega(F, K, T, r, sig);
	return 0;
}

int fb_black76_custom_demo_theoretical_algorithm::calculate(IN unsigned short option_type, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param* static_params, IN const theoretical_custom_param *custom_params, IN const theoretical_volatility_param *volatility_params, OUT double* price){
	double F = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = variable_params->get_left_trading_years();
	double r = variable_params->get_rate();
	double sig = volatility_params->get_volatility();
	double long_margin_rate = variable_params->get_long_margin_rate();
	double long_margin_amt  = variable_params->get_long_margin_amt();
	double short_margin_rate = variable_params->get_short_margin_rate();
	double short_margin_amt = variable_params->get_short_margin_amt();
	PLOG("fb_black76_custom_demo_theoretical_algorithm::%s, long_margin_rate[%lf] long_margin_amt[%lf] short_margin_rate[%lf] short_margin_amt[%lf]\n", __FUNCTION__, long_margin_rate, long_margin_amt, short_margin_rate, short_margin_amt);


	if (Exception(F, K, T, r, sig) == -1){
		PLOG("fb_black76_custom_demo_theoretical_algorithm::%s,there are exceptional data:F[%lf] or K[%lf] or T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n", __FUNCTION__, F, K, T, r, sig);
		return -1;
	}
	T = std::max(T, T_MIN);

	*price = black76(option_type, F, K, T, r, sig);
	return 0;
}

int fb_black76_custom_demo_theoretical_algorithm::iv_derivable(IN unsigned short option_type, IN double option_price, IN const theoretical_variable_param* variable_params, IN const theoretical_static_param *static_params) {
	double F = variable_params->get_forward();
	double K = variable_params->get_strike();
	double T = std::max(variable_params->get_left_trading_years(), T_MIN);
	double r = variable_params->get_rate();
	double long_margin_rate = variable_params->get_long_margin_rate();
	double long_margin_amt  = variable_params->get_long_margin_amt();
	double short_margin_rate = variable_params->get_short_margin_rate();
	double short_margin_amt = variable_params->get_short_margin_amt();
	PLOG("fb_black76_custom_demo_theoretical_algorithm::%s, long_margin_rate[%lf] long_margin_amt[%lf] short_margin_rate[%lf] short_margin_amt[%lf]\n", __FUNCTION__, long_margin_rate, long_margin_amt, short_margin_rate, short_margin_amt);

	if ((option_type == PLUGIN_OPTION_CALL) && option_price > ((F - K) * std::exp(0 - r * T))) {
		return 0;
	}
	if ((option_type == PLUGIN_OPTION_PUT) && option_price > ((K - F) * std::exp(0 - r * T))) {
		return 0;
	}
	return -1;
}

/******** private ********/
int fb_black76_custom_demo_theoretical_algorithm::Exception(IN double F, IN double K, IN double T, IN double r, IN double sig){
	if (!math_helper::greater(F, 0) || !math_helper::greater(K, 0) || !math_helper::greater(T, 0) || !math_helper::greater(sig, 0) || !math_helper::greater_equal(r, 0) ||
		!math_helper::valid_price(F) || !math_helper::valid_price(K) || !math_helper::valid_price(T) ||
		!math_helper::valid_price(r) || !math_helper::valid_price(sig)
		) {
		return -1;
	}
	return 0;
}

void fb_black76_custom_demo_theoretical_algorithm::on_timer_uncommon_greeks() {
	if (updated_uncommon_greeks_map_.empty()) {
		return;
	}
	for (auto iter = updated_uncommon_greeks_map_.begin(); iter != updated_uncommon_greeks_map_.end(); iter++) {
		std::string instrument = iter->first;
		input_params& input = iter->second;
		double theta, rho, charm, vanna, vomma, speed, zomma;
		calc_uncommon_greeks(input.option_type, input.F, input.K, input.T, input.r, input.sig, &theta, &rho, &charm, &vanna, &vomma, &speed, &zomma);
		full_params full(input.option_type, input.F, input.K, input.T, input.r, input.sig, theta, rho, charm, vanna, vomma, speed, zomma);
		base_map_[instrument] = full;
	}
	updated_uncommon_greeks_map_.clear();
}

double fb_black76_custom_demo_theoretical_algorithm::black76(IN unsigned short option_type, IN double F, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(F / K) + 0.5 * sig * sig * T;
	d1 = d1 / sig / std::sqrt(T);
	double d2 = d1 - sig * std::sqrt(T);
	double discount_factor = std::exp(-r * T);
	if (option_type == PLUGIN_OPTION_CALL) {
		return discount_factor * (F * math_helper::normal_cdf(d1) - K * math_helper::normal_cdf(d2));
	}
	else if (option_type == PLUGIN_OPTION_PUT) {
		return discount_factor * (K * math_helper::normal_cdf(-d2) - F * math_helper::normal_cdf(-d1));
	}
	else {
		return -1;
	}
}

double fb_black76_custom_demo_theoretical_algorithm::calc_delta(IN unsigned short option_type, IN double F, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(F / K) + 0.5 * sig * sig * T;
	d1 = d1 / sig / std::sqrt(T);
	if (option_type == PLUGIN_OPTION_CALL) {
		return std::exp(-r * T) * math_helper::normal_cdf(d1);
	}
	else if (option_type == PLUGIN_OPTION_PUT) {
		return std::exp(-r * T) * (math_helper::normal_cdf(d1) - 1);
	}
	else {
		return -1;
	}
}

double fb_black76_custom_demo_theoretical_algorithm::calc_gamma(IN double F, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(F / K) + 0.5 * sig * sig * T;
	d1 = d1 / sig / std::sqrt(T);
	return math_helper::normal_pdf(d1) * std::exp(-r * T) / F / sig / std::sqrt(T);
}

double fb_black76_custom_demo_theoretical_algorithm::calc_vega(IN double F, IN double K, IN double T, IN double r, IN double sig) {
	double d1 = std::log(F / K) + 0.5 * sig * sig * T;
	d1 = d1 / sig / std::sqrt(T);
	return F * std::exp(-r * T) * math_helper::normal_pdf(d1) * sqrt(T) / 100;
}

int fb_black76_custom_demo_theoretical_algorithm::calc_uncommon_greeks(IN unsigned short option_type, IN double F, IN double K, IN double T, IN double r, IN double sig, OUT double *theta, OUT double *rho, OUT double *charm, OUT double *vanna, OUT double *vomma, OUT double *speed, OUT double *zomma) {
	double d1 = std::log(F / K) + 0.5 * sig * sig * T;
	d1 = d1 / sig / std::sqrt(T);
	double d2 = d1 - sig * std::sqrt(T);
	double d1_ncdf = math_helper::normal_cdf(d1);
	double d1_npdf = math_helper::normal_pdf(d1);
	double discount_factor = std::exp(-r * T);
	if (option_type == PLUGIN_OPTION_CALL) {
		*theta = (-F * discount_factor * d1_npdf * sig * 0.5 / std::sqrt(T) + r * F * discount_factor * d1_ncdf - r * K * discount_factor * math_helper::normal_cdf(d2)) / TRADING_DAYS;
		*rho = -T * discount_factor * (F * d1_ncdf - K * math_helper::normal_cdf(d2)) / 100;
		*charm = -discount_factor * (-d2 * d1_npdf / 2 / T - r * d1_ncdf) / TRADING_DAYS;
		//d(delta)/dt per day
	}
	else if (option_type == PLUGIN_OPTION_PUT) {
		*theta = (-F * discount_factor * d1_npdf * sig * 0.5 / std::sqrt(T) - r * F * discount_factor * math_helper::normal_cdf(-d1) + r * K * discount_factor * math_helper::normal_cdf(-d2)) / TRADING_DAYS;
		*rho = -T * discount_factor * (K * math_helper::normal_cdf(-d2) - F * math_helper::normal_cdf(-d1)) / 100;
		*charm = -discount_factor * (-d2 * d1_npdf / 2 / T + r * d1_ncdf) / TRADING_DAYS;
	}
	else {
		return -1;
	}
	*vanna = -discount_factor * d2 * d1_npdf / sig / 100;//d(vega)/dS per percent
	*vomma = F * discount_factor * d1_npdf * std::sqrt(T) * d1 * d2 / sig / 10000;//d(vega)/d(sigma) per percent
	*speed = -d1_npdf * discount_factor * (1 + d1 / (sig * std::sqrt(T))) / (F * F * sig * std::sqrt(T)); //d(gamma) / dS
	*zomma = (d1 * d2 - 1) * d1_npdf * discount_factor / (F * sig * sig * std::sqrt(T)) / 100;//d(gamma)/d(sigma) per percent

	return 0;
}

}//plugin
}//cffex
}//mm
