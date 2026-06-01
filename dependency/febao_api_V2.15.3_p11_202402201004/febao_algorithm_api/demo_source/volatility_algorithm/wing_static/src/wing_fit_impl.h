#ifndef CFFEX_MM_WING_FIT_IMPL_H
#define CFFEX_MM_WING_FIT_IMPL_H

#include "volatility_common.h"
#include <algorithm>

namespace cffex {
namespace fb {
namespace plugin{

#define DOUBLE_MIN 1e-13
#define MIN_WING_SINGLE_NUM 2

/** float defination */
#ifndef DBL_MAX
#define DBL_MAX __DBL_MAX__
#endif

#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

enum {
	WING_MIN_POINT_NUM = 4,
	WING_MAX_POINT_NUM = 64
};

struct wing_point {
	double x;
	double y;
};

class wing_fit_impl {
public:
	wing_fit_impl();
	virtual ~wing_fit_impl();

	int fit(const volatility_point *p_volatility_points,
			const unsigned int size,
		    const double atm_forward,
            const double left_trading_years,
            const double rate,
			char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
            char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

private:
    bool valid_price(double price);
    bool calculate_atm_volatility();
	bool calculate_cutoff();
	bool calculate_slope_curvature(wing_point *a_point, unsigned int size, double &slope, double &curvature);
	bool calculate_left_curvature(wing_point *a_points, unsigned int size, double &curvature);
	void construct_wing_para(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);


private:
    // basic Wing Parameters
    double atm_forward_;
    double atm_volatility_;
    double slope_;
    double call_curvature_;
    double call_cutoff_;
    double put_curvature_;
    double put_cutoff_;

    volatility_point a_vol_points_[WING_MAX_POINT_NUM];
    unsigned int size_;
    wing_point a_put_points_[WING_MAX_POINT_NUM];
    unsigned int put_size_;
    wing_point a_call_points_[WING_MAX_POINT_NUM];
    unsigned int call_size_;

};

}
}
}


#endif
