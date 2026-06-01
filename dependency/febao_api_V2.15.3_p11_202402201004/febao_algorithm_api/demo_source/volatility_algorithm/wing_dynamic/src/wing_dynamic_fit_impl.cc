#include "stdio.h"
#include "wing_dynamic_fit_impl.h"
#include <cstring>
#include <algorithm>
#include "stdlib.h"
#include "math.h"

static bool cmp_strike_vol_(const volatility_point &left, const volatility_point &right) {
	return left.strike < right.strike;
}

namespace cffex {
namespace fb {
namespace plugin {

wing_dynamic_fit_impl::wing_dynamic_fit_impl()
{
    atm_forward_ = 0;
    atm_volatility_ = 0;
    slope_ = 0;
    call_curvature_ = 0;
    call_cutoff_ = 0;
    put_curvature_ = 0;
    put_cutoff_ = 0;

    size_ = 0;
	put_size_ = 0;
	call_size_ = 0;
}

wing_dynamic_fit_impl::~wing_dynamic_fit_impl() {}

int wing_dynamic_fit_impl::fit(const volatility_point *p_volatility_points,
		 const unsigned int size,
		 const double atm_forward,
		 const double left_trading_years,
         const double rate,
		 char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
		 char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH])
{
	// check if size if in proper range
	if (size < WING_MIN_POINT_NUM) {
		return DATA_NUM_IS_TOO_FEW;
	}

	if (size > WING_MAX_POINT_NUM) {
		return DATA_NUM_IS_TOO_MUCH;
	}

	// check nullptr
	if (p_volatility_points == NULL) {
		return INPUT_OUTPUT_PARAM_IS_NULL;
	}

	// check atm_forward value range
	if (!valid_price(atm_forward) || atm_forward <= 0) {
		return ATM_FORWARD_IS_INVALID;
	}

	size_ = size;
	atm_forward_ = atm_forward;

	// step 1: copy to my space
	memcpy(a_vol_points_, p_volatility_points, sizeof(volatility_point) * size);

	// step 2: sort input points base on strike
	std::sort(a_vol_points_, a_vol_points_ + size, cmp_strike_vol_);

	// step 3: transform x coordinate & calculate atm_volatility
	if ( !calculate_atm_volatility()) {
		return ATM_FORWARD_IS_NOT_IN_PROPER_RANGE;
	}

	// step 4: calculate cutoff
	if ( !calculate_cutoff() ) {
		return FIT_RANGE_IS_INVALID;
	}

	// step 5: calculate curvature & slope
	if (put_size_ >= call_size_) {
		calculate_slope_curvature(a_put_points_, put_size_, slope_, put_curvature_);
		calculate_left_curvature(a_call_points_, call_size_, call_curvature_);
	}
	else {
		calculate_slope_curvature(a_call_points_,call_size_, slope_, call_curvature_);
		calculate_left_curvature(a_put_points_, put_size_, put_curvature_);
	}

	// step 5: construct return value
	construct_wing_para(fixed_model_param, variable_model_param);
	return RETURN_SUCCESS;
}

bool wing_dynamic_fit_impl::valid_price(double price) {return (price > -DBL_MAX && price < DBL_MAX);}

bool wing_dynamic_fit_impl::calculate_atm_volatility()
{
	wing_point point;
	put_size_ = 0;
	call_size_ = 0;
	double last_put_volatility = 0.0;
	double last_put_x = 0.0;
	double first_call_volatility = 0.0;
	double first_call_x = 0.0;
	for (unsigned int i=0; i < size_; i++) {
		// adjust x coordinate
		point.x = log(a_vol_points_[i].strike / atm_forward_);
		point.y = a_vol_points_[i].volatility;

		if (point.x <= 0) {
			a_put_points_[put_size_] = point;
			put_size_ ++;
			last_put_x = point.x;
			last_put_volatility = point.y;
		}
		else {
			a_call_points_[call_size_] = point;
			call_size_ ++;
			if (call_size_ == 1) {
				first_call_x = point.x;
				first_call_volatility = point.y;
			}
		}
	}

	if (put_size_ < MIN_WING_SINGLE_NUM || call_size_ < MIN_WING_SINGLE_NUM) {
		// fit failed when atm_forward is not in point range
		return false;
	}

	if (first_call_x - last_put_x < DOUBLE_MIN) {
		// fit failed when two volatility point is too close
		return false;
	}
	atm_volatility_ = (first_call_x * last_put_volatility - last_put_x * first_call_volatility) / (first_call_x - last_put_x);

	// reverse put point array
	for (unsigned int i=0; i < put_size_ / 2; i++) {
		wing_point tmp = a_put_points_[i];
		a_put_points_[i] = a_put_points_[put_size_ -1 -i];
		a_put_points_[put_size_ -1 -i] = tmp;
	}
	return true;
}

bool wing_dynamic_fit_impl::calculate_cutoff() {
	put_size_ = (unsigned int)ceil(put_size_ );
	if (put_size_ < MIN_WING_SINGLE_NUM) {
		// put_size_ is too small
		return false;
	}
	put_cutoff_ = a_put_points_[put_size_ - 1].x;

	call_size_ = (unsigned int)ceil(call_size_ );
	if (call_size_ < MIN_WING_SINGLE_NUM) {
		// call_size_ is too small
		return false;
	}
	call_cutoff_ = a_call_points_[call_size_ - 1].x;

	return true;
}

bool wing_dynamic_fit_impl::calculate_slope_curvature(wing_point *a_point, unsigned int size, double &slope, double &curvature) {
	try
	{
		double totalX1 = 0.0;
		double totalX2 = 0.0;
		double totalX3 = 0.0;
		double totalX4 = 0.0;
		double totalvolX1 = 0.0;
		double totalvolX2 = 0.0;
		for (unsigned int i=0; i<size; i++)
		{
			double x_square = a_point[i].x * a_point[i].x;
			totalX1 += a_point[i].x;
			totalX2 += x_square;
			totalX3 += x_square * a_point[i].x;
			totalX4 += x_square * x_square;
			totalvolX1 += a_point[i].x * a_point[i].y;
			totalvolX2 += x_square * a_point[i].y;
		}
		double denomintor = totalX2 * totalX4 - totalX3 * totalX3;
		if (denomintor > - DOUBLE_MIN && denomintor < DOUBLE_MIN)
		{
			curvature = 0.001;
			slope = 0.0001;
			return true;
		}
		curvature = (totalvolX2*totalX2 -  totalvolX1*totalX3 + atm_volatility_ * (totalX1*totalX3 - totalX2 *totalX2)) / denomintor;
		slope = (totalvolX1*totalX4 -  totalvolX2*totalX3 + atm_volatility_  * (totalX2*totalX3 - totalX1 *totalX4)) / denomintor;

		return true;
	}
	catch(...)
	{
		slope = 0.0001;
		curvature = 0.001;
		return false;
	}
}

bool wing_dynamic_fit_impl::calculate_left_curvature(wing_point *a_point, unsigned int size, double &curvature) {
	try
	{
		double s1 = 0.0;
		double s2 = 0.0;
		for (unsigned int i=0; i<size; i++) {
			double x_square = a_point[i].x * a_point[i].x;
			s1 += x_square * (a_point[i].y - atm_volatility_ - slope_ * a_point[i].x);
			s2 += x_square * x_square;
		}
		if (s2 > -DOUBLE_MIN && s2 < DOUBLE_MIN)
		{
			curvature = 0.001;
			return true;
		}
		curvature =  s1 / s2 ;
		return true;
	}
	catch(...)
	{
		curvature = 0.001;
		return false;
	}
}

void wing_dynamic_fit_impl::construct_wing_para(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	sprintf(fixed_model_param, "atm_volatility:%.6f;slope:%.6f;call_curvature:%.6f;call_cutoff:%.6f;put_curvature:%.6f;put_cutoff:%.6f",
						atm_volatility_, slope_, call_curvature_, call_cutoff_, put_curvature_, put_cutoff_);
	variable_model_param[0] = '\0';
	return ;
}

}
}
}
