#ifndef CFFEX_MM_CUBICSPLINE_STATIC_IMPL_H
#define CFFEX_MM_CUBICSPLINE_STATIC_IMPL_H

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
	//y = a + b*(x-strike) + c*(x-strike)^2 + d*(x-strike)^3
	double strike;	//the left endpoint for each interval, to classify the coefficients belong to which interval
	double a;		//the coefficient for zero order term
	double b;		//the coefficient for one order term
	double c;		//the coefficient for two order term
	double d;		//the coefficient for three order term
};


class cubicspline_static_impl {
public:
	cubicspline_static_impl();
    virtual ~cubicspline_static_impl();

    void get_param_list(char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

    void get_param_value(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

    int set_param_value(char const fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char const variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

    void get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]);

    bool get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward,IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]);

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
    /*
     *  generate matrix for gauss
     */
    void gen_coef_matrix(IN volatility_point *strike_vol, IN unsigned int size, OUT double *poly_matrix, OUT double *y_matrix);

    /*
     * use gauss method to calculate poly matrix
     */
    bool calculate_gauss(IN double *poly_matrix, IN double *y_matrix, IN unsigned int size, OUT double *result);

    /*
     * generate complete poly_param
     */
    void gen_poly_param(IN volatility_point *strike_vol, IN double *result, IN unsigned int size, OUT cubicspline_poly_param *poly_param);

private:
    volatility_point a_strike_vol_[CUBICSPLINE_MAX_POINT_NUM];
    double a_strike_[CUBICSPLINE_MAX_POINT_NUM];
    double a_volatility_[CUBICSPLINE_MAX_POINT_NUM];
    cubicspline_poly_param a_poly_param_[CUBICSPLINE_MAX_POINT_NUM];
	double a_poly_matrix_[CUBICSPLINE_MAX_POINT_NUM * CUBICSPLINE_MAX_POINT_NUM];
	double a_y_matrix_[CUBICSPLINE_MAX_POINT_NUM];
	double a_res_[CUBICSPLINE_MAX_POINT_NUM];
    unsigned int size_;
    double volatility_partial_;
};


}
}
}

#endif
