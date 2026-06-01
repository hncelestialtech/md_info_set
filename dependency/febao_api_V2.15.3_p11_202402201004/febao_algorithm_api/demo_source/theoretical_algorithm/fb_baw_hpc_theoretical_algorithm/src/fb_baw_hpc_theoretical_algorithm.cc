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
 * Date: 2020-09-21
 */

#include "fb_baw_hpc_theoretical_algorithm.h"

#include "fb_plugin_math_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    return new cffex::fb::plugin::fb_baw_hpc_theoretical_algorithm();
}
void destroy(void *p) {
    delete (cffex::fb::plugin::fb_baw_hpc_theoretical_algorithm *)p;
}
void identity(void *handle, register_func f) {
    f(handle, 13, "fb_baw_hpc_theoretical_algorithm");
}

#ifdef __cplusplus
}
#endif

static void PLOG(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

namespace cffex {
// using cffex::algorithm::math;
namespace fb {
namespace plugin {

fb_baw_hpc_theoretical_algorithm::fb_baw_hpc_theoretical_algorithm() {}
fb_baw_hpc_theoretical_algorithm::~fb_baw_hpc_theoretical_algorithm() {}

void fb_baw_hpc_theoretical_algorithm::set_register_timer_function(register_timer_callback f) {
    f(TIMER_INTERVAL_PRICING,
      std::bind(&fb_baw_hpc_theoretical_algorithm::on_timer_pricing, this),
      true);
    f(TIMER_INTERVAL_UNCOMMON_GREEKS,
      std::bind(&fb_baw_hpc_theoretical_algorithm::on_timer_uncommon_greeks, this),
      true);
};

int fb_baw_hpc_theoretical_algorithm::Exception(
    IN double F, IN double K, IN double T, IN double r, IN double sig) {
    if (!math_helper::greater(F, 0) || !math_helper::greater(K, 0) || !math_helper::greater(T, 0) ||
        !math_helper::greater(sig, 0) || !math_helper::greater(r, 0) ||
        !math_helper::valid_price(F) || !math_helper::valid_price(K) ||
        !math_helper::valid_price(T) || !math_helper::valid_price(r) ||
        !math_helper::valid_price(sig)) {
        return -1;
    }
    return 0;
}

void fb_baw_hpc_theoretical_algorithm::on_timer_pricing() {
    if (updated_pricing_params_map_.empty()) {
        return;
    }

    for (auto iter = updated_pricing_params_map_.begin(); iter != updated_pricing_params_map_.end();
         iter++) {
        std::string   instrument  = iter->first;
        input_params &input       = iter->second;
        full_params  &full_origin = base_map_[instrument];

        double theo_price, delta, gamma, vega;
        pricing_baw(input.option_type,
                    input.F,
                    input.K,
                    input.T,
                    input.r,
                    input.sig,
                    &theo_price,
                    &delta,
                    &gamma,
                    &vega);

        full_params full_updated(input.option_type,
                                 input.F,
                                 input.K,
                                 input.T,
                                 input.r,
                                 input.sig,
                                 theo_price,
                                 delta,
                                 gamma,
                                 vega,
                                 full_origin.uncommon_greeks_.theta,
                                 full_origin.uncommon_greeks_.rho,
                                 full_origin.uncommon_greeks_.charm,
                                 full_origin.uncommon_greeks_.vanna,
                                 full_origin.uncommon_greeks_.vomma,
                                 full_origin.uncommon_greeks_.speed,
                                 full_origin.uncommon_greeks_.zomma);
        base_map_[instrument] = full_updated;
    }
    updated_pricing_params_map_.clear();
}

void fb_baw_hpc_theoretical_algorithm::on_timer_uncommon_greeks() {
    if (updated_uncommon_greeks_map_.empty()) {
        return;
    }

    for (auto iter = updated_uncommon_greeks_map_.begin();
         iter != updated_uncommon_greeks_map_.end();
         iter++) {
        std::string   instrument  = iter->first;
        input_params &input       = iter->second;
        full_params  &full_origin = base_map_[instrument];

        double theta, rho, charm, vanna, vomma, speed, zomma;
        uncommon_greeks_baw(input.option_type,
                            input.F,
                            input.K,
                            input.T,
                            input.r,
                            input.sig,
                            &theta,
                            &rho,
                            &charm,
                            &vanna,
                            &vomma,
                            &speed,
                            &zomma);
        full_params full_updated(full_origin.input_.option_type,
                                 full_origin.input_.F,
                                 full_origin.input_.K,
                                 full_origin.input_.T,
                                 full_origin.input_.r,
                                 full_origin.input_.sig,
                                 full_origin.pricing_.theo_price,
                                 full_origin.pricing_.delta,
                                 full_origin.pricing_.gamma,
                                 full_origin.pricing_.vega,
                                 theta,
                                 rho,
                                 charm,
                                 vanna,
                                 vomma,
                                 speed,
                                 zomma);
        base_map_[instrument] = full_updated;
    }
    updated_uncommon_greeks_map_.clear();
}

int fb_baw_hpc_theoretical_algorithm::calculate(
    IN unsigned short                      option_type,
    IN const theoretical_variable_param   *variable_params,
    IN const theoretical_static_param     *static_params,
    IN const theoretical_custom_param     *custom_params,
    IN const theoretical_volatility_param *volatility_params,
    OUT theoretical_field                 *f) {
    double F                  = variable_params->get_forward();
    double K                  = variable_params->get_strike();
    double T                  = variable_params->get_left_trading_years();
    double r                  = variable_params->get_rate();
    double sig                = volatility_params->get_volatility();
    double volatility_partial = 0.0;
    if (!volatility_params->get_custom_param("volatility_partial", &volatility_partial)) {
        volatility_partial = 0.0;
    }

    if (Exception(F, K, T, r, sig) == -1) {
        PLOG(
            "fb_baw_hpc_theoretical_algorithm::%s,there are exceptional data:F[%lf] or K[%lf] or "
            "T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n",
            __FUNCTION__,
            F,
            K,
            T,
            r,
            sig);
        return -1;
    }
    T = std::max(T, T_MIN);

    std::string instrument = variable_params->get_instrument_id();
    auto        iter       = base_map_.find(instrument);
    if (iter != base_map_.end()) {
        // previous record found
        input_params           &input           = iter->second.input_;
        pricing_params         &pricing         = iter->second.pricing_;
        uncommon_greeks_params &uncommon_greeks = iter->second.uncommon_greeks_;
        if (math_helper::equal(input.r, r) && math_helper::greater_equal(input.T, T) &&
            math_helper::less(input.T - T, MATURITY_TOLERANCE) &&
            std::fabs((input.F - F) / F) < UNDERLYING_TOLERANCE &&
            std::fabs(input.sig - sig) < VOLATILITY_TOLERANCE) {
            // if only T, F and sig diff, then use Taylor approximate method instead of exact
            // calculation if diff is too big, shouldn't use approximate method
            f->set_theoretical_price(pricing.theo_price + pricing.delta * (F - input.F) +
                                     0.5 * pricing.gamma * (F - input.F) * (F - input.F) +
                                     pricing.vega * 100 * (sig - input.sig));
            f->set_delta(pricing.delta);
            f->set_gamma(pricing.gamma);
            f->set_vega(pricing.vega);
            f->set_theta(uncommon_greeks.theta);
            f->set_rho(uncommon_greeks.rho);
            f->set_charm(uncommon_greeks.charm);
            f->set_vanna(uncommon_greeks.vanna);
            f->set_vomma(uncommon_greeks.vomma);
            f->set_speed(uncommon_greeks.speed);
            f->set_zomma(uncommon_greeks.zomma);
            f->set_skew_delta(pricing.delta + volatility_partial * pricing.vega);

            // add to updated_ map, used by on_timer()
            input_params input_updated(option_type, F, K, T, r, sig);
            updated_pricing_params_map_[instrument]  = input_updated;
            updated_uncommon_greeks_map_[instrument] = input_updated;
            return 0;
        }
    }

    double theo_price, delta, gamma, vega, theta, rho, charm, vanna, vomma, speed, zomma;
    pricing_baw(option_type, F, K, T, r, sig, &theo_price, &delta, &gamma, &vega);
    uncommon_greeks_baw(
        option_type, F, K, T, r, sig, &theta, &rho, &charm, &vanna, &vomma, &speed, &zomma);

    f->set_theoretical_price(theo_price);
    f->set_delta(delta);
    f->set_gamma(gamma);
    f->set_vega(vega);
    f->set_theta(theta);
    f->set_rho(rho);
    f->set_charm(charm);
    f->set_vanna(vanna);
    f->set_vomma(vomma);
    f->set_speed(speed);
    f->set_zomma(zomma);
    f->set_skew_delta(delta + volatility_partial * vega);

    // update base map when full_iteration finished
    full_params full(option_type,
                     F,
                     K,
                     T,
                     r,
                     sig,
                     theo_price,
                     delta,
                     gamma,
                     vega,
                     theta,
                     rho,
                     charm,
                     vanna,
                     vomma,
                     speed,
                     zomma);
    base_map_[instrument] = full;
    // don't need to calcuate when on_timer
    updated_pricing_params_map_.erase(instrument);
    updated_uncommon_greeks_map_.erase(instrument);
    return 0;
}

int fb_baw_hpc_theoretical_algorithm::calculate(
    IN unsigned short                      option_type,
    IN const theoretical_variable_param   *variable_params,
    IN const theoretical_static_param     *static_params,
    IN const theoretical_custom_param     *custom_params,
    IN const theoretical_volatility_param *volatility_params,
    OUT double                            *price,
    OUT double                            *vega) {
    double F   = variable_params->get_forward();
    double K   = variable_params->get_strike();
    double T   = variable_params->get_left_trading_years();
    double r   = variable_params->get_rate();
    double sig = volatility_params->get_volatility();

    if (Exception(F, K, T, r, sig) == -1) {
        PLOG(
            "fb_baw_hpc_theoretical_algorithm::%s,there are exceptional data:F[%lf] or K[%lf] or "
            "T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n",
            __FUNCTION__,
            F,
            K,
            T,
            r,
            sig);
        return -1;
    }
    T = std::max(T, T_MIN);

    single_baw(option_type, F, K, T, r, sig, price);
    *vega = calc_vega(option_type, F, K, T, r, sig);
    return 0;
}

int fb_baw_hpc_theoretical_algorithm::calculate(
    IN unsigned short                      option_type,
    IN const theoretical_variable_param   *variable_params,
    IN const theoretical_static_param     *static_params,
    IN const theoretical_custom_param     *custom_params,
    IN const theoretical_volatility_param *volatility_params,
    OUT double                            *price) {
    double F   = variable_params->get_forward();
    double K   = variable_params->get_strike();
    double T   = variable_params->get_left_trading_years();
    double r   = variable_params->get_rate();
    double sig = volatility_params->get_volatility();

    if (Exception(F, K, T, r, sig) == -1) {
        PLOG(
            "fb_baw_hpc_theoretical_algorithm::%s,there are exceptional data:F[%lf] or K[%lf] or "
            "T[%lf] or r[%lf] or sig[%lf] is invalid, failed to calculate\n",
            __FUNCTION__,
            F,
            K,
            T,
            r,
            sig);
        return -1;
    }
    T = std::max(T, T_MIN);

    single_baw(option_type, F, K, T, r, sig, price);
    return 0;
}

int fb_baw_hpc_theoretical_algorithm::iv_derivable(
    IN unsigned short                    option_type,
    IN double                            option_price,
    IN const theoretical_variable_param *variable_params,
    IN const theoretical_static_param   *static_params) {
    double F = variable_params->get_forward();
    double K = variable_params->get_strike();
    double T = std::max(variable_params->get_left_trading_years(), T_MIN);
    double r = variable_params->get_rate();
    if ((option_type == PLUGIN_OPTION_CALL) && option_price > ((F - K) * std::exp(0 - r * T))) {
        return 0;
    }
    if ((option_type == PLUGIN_OPTION_PUT) && option_price > ((K - F) * std::exp(0 - r * T))) {
        return 0;
    }
    return -1;
}

/******** private ********/
int fb_baw_hpc_theoretical_algorithm::pricing_baw(IN unsigned short option_type,
                                                  IN double         F,
                                                  IN double         K,
                                                  IN double         T,
                                                  IN double         r,
                                                  IN double         sig,
                                                  OUT double       *theo_price,
                                                  OUT double       *delta,
                                                  OUT double       *gamma,
                                                  OUT double       *vega) {
    single_baw(option_type, F, K, T, r, sig, theo_price);
    if (delta != NULL && gamma != NULL) {
        calc_delta_gamma(option_type, F, K, T, r, sig, *theo_price, delta, gamma);
    }

    if (vega != NULL) {
        *vega = calc_vega(option_type, F, K, T, r, sig);
    }
    return 0;
}

int fb_baw_hpc_theoretical_algorithm::uncommon_greeks_baw(IN unsigned short option_type,
                                                          IN double         F,
                                                          IN double         K,
                                                          IN double         T,
                                                          IN double         r,
                                                          IN double         sig,
                                                          OUT double       *theta,
                                                          OUT double       *rho,
                                                          OUT double       *charm,
                                                          OUT double       *vanna,
                                                          OUT double       *vomma,
                                                          OUT double       *speed,
                                                          OUT double       *zomma) {
    double theo_price = 0.0;
    single_baw(option_type, F, K, T, r, sig, &theo_price);
    if (theta != NULL) {
        *theta = calc_theta(option_type, F, K, T, r, sig, theo_price);
    }
    if (rho != NULL) {
        *rho = calc_rho(option_type, F, K, T, r, sig, theo_price);
    }
    if (charm != NULL && vanna != NULL && vomma != NULL && speed != NULL && zomma != NULL) {
        calc_high_order_greeks(option_type, F, K, T, r, sig, charm, vanna, vomma, speed, zomma);
    }
    return 0;
}

int fb_baw_hpc_theoretical_algorithm::single_baw(IN unsigned short option_type,
                                                 IN double         F,
                                                 IN double         K,
                                                 IN double         T,
                                                 IN double         r,
                                                 IN double         sig,
                                                 OUT double       *theo_price) {
    if (theo_price == NULL) {
        return -1;
    }
    if (option_type == PLUGIN_OPTION_CALL) {
        *theo_price = baw_call(option_type, F, K, T, r, sig);
        return 0;
    } else if (option_type == PLUGIN_OPTION_PUT) {
        *theo_price = baw_put(option_type, F, K, T, r, sig);
        return 0;
    } else {
        return -1;
    }
}

double fb_baw_hpc_theoretical_algorithm::baw_call(IN unsigned short option_type,
                                                  IN double         F,
                                                  IN double         K,
                                                  IN double         T,
                                                  IN double         r,
                                                  IN double         sig) {
    double alpha = 2.0 * r / sig / sig;
    double h     = 1.0 - std::exp(-r * T);
    double d1, b, eur_val_iter, rhs, ERR, eur_option_val;
    double discount    = std::exp(-r * T);
    double S_star      = K;
    bool   iterate     = true;  // allow to iterate
    int    flag        = 0;     // control the iteration numbers
    double gamma_2     = std::sqrt(1 + (4.0 * alpha / h));
    gamma_2            = (1 + gamma_2) / 2.0;
    double gamma_2_inv = 1 / gamma_2;
    while (iterate) {
        d1 = (std::log(S_star / K) + 0.5 * sig * sig * T) / sig / std::sqrt(T);
        b  = discount * math_helper::normal_cdf(d1) * (1.0 - gamma_2_inv) +
            gamma_2_inv * (1.0 - discount * math_helper::normal_pdf(d1) / sig / std::sqrt(T));
        // calculate the Black Scholes value of a European call
        black_scholes_merton(option_type, S_star, K, T, r, sig, &eur_val_iter);
        rhs = eur_val_iter +
              (1.0 - std::exp(-r * T) * math_helper::normal_cdf(d1)) * S_star * gamma_2_inv;
        S_star = (K + rhs - b * S_star) / (1.0 - b);
        ERR    = std::fabs((S_star - K) - rhs) / K;
        flag   = flag + 1;
        if (ERR < TOL) {
            iterate = false;
        }
        if (flag > MAX_ITER) {
            PLOG(
                "fb_baw_hpc_theoretical_algorithm::%s, the calculation excessed the maximum "
                "iteration limit %lf.\n",
                __FUNCTION__,
                MAX_ITER);
            return -1;
        }
    }
    double A_2 = S_star * gamma_2_inv * (1.0 - std::exp(-r * T) * math_helper::normal_cdf(d1));
    if (F < S_star) {
        black_scholes_merton(option_type, F, K, T, r, sig, &eur_option_val);
        return eur_option_val + A_2 * std::pow((F / S_star), gamma_2);
    } else {
        return F - K;
    }
}

double fb_baw_hpc_theoretical_algorithm::baw_put(IN unsigned short option_type,
                                                 IN double         F,
                                                 IN double         K,
                                                 IN double         T,
                                                 IN double         r,
                                                 IN double         sig) {
    double alpha = 2.0 * r / sig / sig;
    double h     = 1.0 - std::exp(-r * T);
    double d1, b, eur_val_iter, rhs, ERR, eur_option_val;
    double discount    = std::exp(-r * T);
    double S_star      = K;
    bool   iterate     = true;  // allow to iterate
    int    flag        = 0;     // control the iteration numbers
    double gamma_1     = std::sqrt(1 + (4.0 * alpha / h));
    gamma_1            = (1 - gamma_1) / 2.0;
    double gamma_1_inv = 1 / gamma_1;
    while (iterate) {
        d1 = (std::log(S_star / K) + ((sig * sig / 2.0)) * T) / (sig * std::sqrt(T));
        b  = discount * (math_helper::normal_cdf(d1) * (1.0 - gamma_1_inv) - 1.0) +
            gamma_1_inv *
                (discount - 1.0 - ((discount * math_helper::normal_pdf(d1)) / sig / std::sqrt(T)));
        // calculate the Black Scholes value of a European put
        black_scholes_merton(option_type, S_star, K, T, r, sig, &eur_val_iter);
        rhs = eur_val_iter -
              (1.0 - std::exp(-r * T) + std::exp(-r * T) * math_helper::normal_cdf(d1)) * S_star *
                  gamma_1_inv;
        S_star = (K - rhs + b * S_star) / (1.0 + b);
        ERR    = std::fabs((K - S_star) - rhs) / K;
        flag   = flag + 1;
        if (ERR < TOL) {
            iterate = false;
        }
        if (flag > MAX_ITER) {
            PLOG(
                "fb_baw_hpc_theoretical_algorithm::%s, the calculation excessed the maximum "
                "iteration limit %lf.\n",
                __FUNCTION__,
                MAX_ITER);
            return -1;
        }
    }
    double A_1 = -S_star * gamma_1_inv * (1.0 - std::exp(-r * T) * math_helper::normal_cdf(-d1));
    if (F > S_star) {
        black_scholes_merton(option_type, F, K, T, r, sig, &eur_option_val);
        return eur_option_val + A_1 * std::pow((F / S_star), gamma_1);
    } else {
        return K - F;
    }
}

int fb_baw_hpc_theoretical_algorithm::black_scholes_merton(IN unsigned short option_type,
                                                           IN double         F,
                                                           IN double         K,
                                                           IN double         T,
                                                           IN double         r,
                                                           IN double         sig,
                                                           OUT double       *eur_val) {
    double d1              = (std::log(F / K) + ((sig * sig / 2)) * T) / (sig * std::sqrt(T));
    double d2              = d1 - sig * std::sqrt(T);
    double discount_factor = std::exp(-r * T);
    // evaluate the option price
    if (option_type == PLUGIN_OPTION_CALL) {
        *eur_val =
            discount_factor * (F * math_helper::normal_cdf(d1) - K * math_helper::normal_cdf(d2));
    } else if (option_type == PLUGIN_OPTION_PUT) {
        *eur_val = discount_factor *
                   (-F * math_helper::normal_cdf(-d1) + K * math_helper::normal_cdf(-d2));
    }
    return 0;
}

int fb_baw_hpc_theoretical_algorithm::calc_delta_gamma(IN unsigned short option_type,
                                                       IN double         F,
                                                       IN double         K,
                                                       IN double         T,
                                                       IN double         r,
                                                       IN double         sig,
                                                       IN double         cur_price,
                                                       OUT double       *delta,
                                                       OUT double       *gamma) {
    const double DIST           = 0.001;
    double       FUp            = F + DIST;
    double       FDn            = F - DIST;
    double       option_priceUp = 0.0;
    double       option_priceDn = 0.0;
    single_baw(option_type, FUp, K, T, r, sig, &option_priceUp);
    single_baw(option_type, FDn, K, T, r, sig, &option_priceDn);
    *delta = (option_priceUp - option_priceDn) / (2 * DIST);
    *gamma = (option_priceUp - 2 * cur_price + option_priceDn) / (DIST * DIST);
    return 0;
}

double fb_baw_hpc_theoretical_algorithm::calc_vega(IN unsigned short option_type,
                                                   IN double         F,
                                                   IN double         K,
                                                   IN double         T,
                                                   IN double         r,
                                                   IN double         sig) {
    double dv = std::fmin(0.01f, (double)sig * 0.1);
    double v1 = sig + dv;
    double v2 = sig - dv;
    double p1 = 0.0;
    double p2 = 0.0;
    single_baw(option_type, F, K, T, r, v1, &p1);
    single_baw(option_type, F, K, T, r, v2, &p2);
    const int factor = 100;  // per 1%
    return (p1 - p2) / (2 * dv * factor);
}

double fb_baw_hpc_theoretical_algorithm::calc_theta(IN unsigned short option_type,
                                                    IN double         F,
                                                    IN double         K,
                                                    IN double         T,
                                                    IN double         r,
                                                    IN double         sig,
                                                    IN double         cur_price) {
    const double dt         = 0.00408163f;  // per day,  1/245= 0.00408163
    double       tomorrow_T = T - dt;
    double       tomorrow_p = 0.0;
    if (math_helper::less_equal(tomorrow_T, T_MIN)) {
        tomorrow_p = std::fmax(0, payoff(F, K, option_type));
    } else {
        single_baw(option_type, F, K, tomorrow_T, r, sig, &tomorrow_p);
    }
    return (tomorrow_p - cur_price);
}

double fb_baw_hpc_theoretical_algorithm::calc_rho(IN unsigned short option_type,
                                                  IN double         F,
                                                  IN double         K,
                                                  IN double         T,
                                                  IN double         r,
                                                  IN double         sig,
                                                  IN double         cur_price) {
    // calculate single side only for rho
    const double dr = 0.01f;
    double       r1 = r + dr;
    double       p1 = 0.0;
    single_baw(option_type, F, K, T, r1, sig, &p1);
    return (p1 - cur_price);
}

int fb_baw_hpc_theoretical_algorithm::calc_high_order_greeks(IN unsigned short option_type,
                                                             IN double         F,
                                                             IN double         K,
                                                             IN double         T,
                                                             IN double         r,
                                                             IN double         sig,
                                                             OUT double       *charm,
                                                             OUT double       *vanna,
                                                             OUT double       *vomma,
                                                             OUT double       *speed,
                                                             OUT double       *zomma) {
    double d1              = std::log(F / K) + 0.5 * sig * sig * T;
    d1                     = d1 / sig / std::sqrt(T);
    double d2              = d1 - sig * std::sqrt(T);
    double d1_ncdf         = math_helper::normal_cdf(d1);
    double d1_npdf         = math_helper::normal_pdf(d1);
    double discount_factor = std::exp(-r * T);
    if (option_type == PLUGIN_OPTION_CALL) {
        *charm = -discount_factor * (-d2 * d1_npdf / 2 / T - r * d1_ncdf) / TRADING_DAYS;
        // d(delta)/dt per day
    } else if (option_type == PLUGIN_OPTION_PUT) {
        *charm = -discount_factor * (-d2 * d1_npdf / 2 / T + r * d1_ncdf) / TRADING_DAYS;
    } else {
        return -1;
    }
    *vanna = -discount_factor * d2 * d1_npdf / sig / 100;  // d(vega)/dS per percent
    *vomma = F * discount_factor * d1_npdf * std::sqrt(T) * d1 * d2 / sig /
             10000;  // d(vega)/d(sigma) per percent
    *speed = -d1_npdf * discount_factor * (1 + d1 / (sig * std::sqrt(T))) /
             (F * F * sig * std::sqrt(T));  // d(gamma) / dS
    *zomma = (d1 * d2 - 1) * d1_npdf * discount_factor / (F * sig * sig * std::sqrt(T)) /
             100;  // d(gamma)/d(sigma) per percent
    return 0;
}

double fb_baw_hpc_theoretical_algorithm::payoff(IN double         spot,
                                                IN double         strike,
                                                IN unsigned short option_type) {
    if (option_type == PLUGIN_OPTION_CALL) {
        return std::max(spot - strike, 0.0);
    } else {
        return std::max(strike - spot, 0.0);
    }
}

double fb_baw_hpc_theoretical_algorithm::special_delta(IN double         spot,
                                                       IN double         strike,
                                                       IN unsigned short option_type) {
    if (spot > strike) {
        return (option_type == PLUGIN_OPTION_CALL) ? 1 : 0;
    } else if (spot < strike) {
        return (option_type == PLUGIN_OPTION_PUT) ? 0 : -1;
    } else {
        return 0;
    }
}

}  // namespace plugin
}  // namespace fb
}  // namespace cffex
