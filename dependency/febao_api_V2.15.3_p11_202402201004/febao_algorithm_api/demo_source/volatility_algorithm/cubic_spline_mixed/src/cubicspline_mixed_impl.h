#ifndef CFFEX_MM_CUBICSPLINE_MIXED_IMPL_H
#define CFFEX_MM_CUBICSPLINE_MIXED_IMPL_H

#include "volatility_common.h"

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {
namespace plugin {

/** float defination */
#ifndef DBL_MAX
#define DBL_MAX __DBL_MAX__
#endif

#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

enum {
	CUBICSPLINE_MIN_POINT_NUM = 3,
	CUBICSPLINE_MAX_POINT_NUM = 128
};

//cubic spline model coefficients for each interval
struct cubicspline_poly_param
{
	//y = constant + first_order*(x-noralize_strike) + second_order*(x-noralize_strike)^2 + third_order*(x-noralize_strike)^3
	double strike;	//the left endpoint for each interval, to classify the coefficients belong to which interval
	double constant;		//the coefficient for zero order term
	double first_order;		//the coefficient for one order term
	double second_order;		//the coefficient for two order term
	double third_order;		//the coefficient for three order term
};


class cubicspline_mixed_impl {
public:
	cubicspline_mixed_impl();
    virtual ~cubicspline_mixed_impl();
    void get_param_list(char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
    void get_param_value(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
    int set_param_value(IN char const fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], IN char const variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);
    void get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]);
    bool get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward, IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]);

	bool is_dynamic_model();
    //If the return value is true, the get_volatility API will be called because of atm_forward or option_forward changes.
    //If the return value is false, the get_volatility API will not be called because of atm_forward or option_forward changes.

    int fit(const volatility_point *p_volatility_points,
			const unsigned int size,
		    const double atm_forward,
            const double left_trading_years,
            const double rate,
			char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
            char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

protected:
	bool gauss_derive();
    //generate matrix for gauss
    void gen_coef_matrix(IN volatility_point *strike_vol, IN unsigned int size, OUT double *poly_matrix, OUT double *y_matrix);
    //use gauss method to calculate poly matrix
    bool calculate_gauss(IN double *poly_matrix, IN double *y_matrix, IN unsigned int size, OUT double *result);
    //generate complete poly_param
    void gen_poly_param(IN volatility_point *strike_vol, IN double *result, IN unsigned int size, OUT cubicspline_poly_param *poly_param);
    bool set_fixed_single_param(char *key, float value);
    bool set_variable_single_param(char *key, char *value);
    //generate the fixed_params
    void get_atmvol_slope(IN cubicspline_poly_param *poly_param, OUT double *sigma_cur, OUT double *slope_cur);
    void normalize_strike_vol();
    //validate the parameters
    bool valid_price(double price);
    bool is_valid_atm_forward(double atm_forward) const;
    bool is_valid_atm_volatility(double atm_volatility) const;
    bool is_valid_slope(double slope) const;
private:
    // fixed_model_param
    double atm_forward_ = 0.0;
    double atm_volatility_ = 0.0;
    double slope_ = 0.0;

    volatility_point a_strike_vol_[CUBICSPLINE_MAX_POINT_NUM];
    volatility_point a_normal_strike_vol_[CUBICSPLINE_MAX_POINT_NUM];
    cubicspline_poly_param a_poly_param_[CUBICSPLINE_MAX_POINT_NUM];
	double a_poly_matrix_[CUBICSPLINE_MAX_POINT_NUM * CUBICSPLINE_MAX_POINT_NUM];
	double a_y_matrix_[CUBICSPLINE_MAX_POINT_NUM];
	double a_res_[CUBICSPLINE_MAX_POINT_NUM];
    unsigned int size_;

    char strike_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
    char constant_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
    char first_order_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
    char second_order_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
    char third_order_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
    double volatility_partial_;
};


}
}
}

#endif
