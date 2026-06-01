#include "stdio.h"
#include "cubicspline_mixed_impl.h"
#include <cstring>
#include <algorithm>
#include "stdlib.h"
#include <math.h>

#ifdef  _WINDOWS
#include <string.h>
#endif
#ifdef  _WINDOWS
	#define STRTOK strtok_s
#else
	#define STRTOK strtok_r
#endif

static bool cmp_strike_vol_(const volatility_point &left, const volatility_point &right) {
	return left.strike < right.strike;
}

namespace cffex {
namespace fb {
namespace plugin {

cubicspline_mixed_impl::cubicspline_mixed_impl() : size_(0)
{
	volatility_partial_ = CFFEX_DOUBLE_NULL;
}

cubicspline_mixed_impl::~cubicspline_mixed_impl() {}


void cubicspline_mixed_impl::get_param_list(char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	static const char cubic_spline_fixed_param[] = "atm_forward:double;atm_volatility:double;slope:double";
	static const char cubic_spline_pointwise_param[] = "strike:double;constant:double;first_order:double;second_order:double;third_order:double";
	strcpy(fixed_param_list, cubic_spline_fixed_param);
	strcpy(variable_param_list, cubic_spline_pointwise_param);
	return;
}

void cubicspline_mixed_impl::get_param_value(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	//fixed_model_param
	sprintf(fixed_model_param, "atm_forward:%.6f;atm_volatility:%.6f;slope:%.6f",
						atm_forward_, atm_volatility_, slope_);
	//variable_model_param
	if (size_ < 1) {
		strcpy(variable_model_param, "");
		return;
	}
	sprintf(variable_model_param, "strike:%.6f", a_poly_param_[0].strike);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_poly_param_[i].strike);
	}
	sprintf(variable_model_param + strlen(variable_model_param), ";constant:%.6f", a_poly_param_[0].constant);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_poly_param_[i].constant);
	}
	sprintf(variable_model_param + strlen(variable_model_param), ";first_order:%.6f", a_poly_param_[0].first_order);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_poly_param_[i].first_order);
	}
	sprintf(variable_model_param + strlen(variable_model_param), ";second_order:%.6f", a_poly_param_[0].second_order);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_poly_param_[i].second_order);
	}
	sprintf(variable_model_param + strlen(variable_model_param), ";third_order:%.6f", a_poly_param_[0].third_order);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_poly_param_[i].third_order);
	}
	return;
}

int cubicspline_mixed_impl::set_param_value(char const fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char const variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	//atm_forward:3000;atm_volatility:0.24;slope:0.35
	// strike:3100-3200;constant:0.123-0.129;first_order:0.25-0.28;second_order:0.35-0.36;third_order:0.37-0.36
	static const int cubic_fixed_param = 3;
	static const int cubic_variable_param = 5;
	if (fixed_model_param == NULL || variable_model_param == NULL) {
		return INPUT_OUTPUT_PARAM_IS_NULL;
	}

	// resolve the fixed_model_param
	char fixed_param[MAX_FIXED_MODEL_PARAM_LENGTH];
	strncpy(fixed_param, fixed_model_param, sizeof(char)*MAX_FIXED_MODEL_PARAM_LENGTH);
	const char word_delim[] = ":;";
	char *key_fixed;
	char *value_fixed;
	char *last_fixed = NULL;
	int num = 1;
	key_fixed = STRTOK(fixed_param, word_delim, &last_fixed);
	value_fixed = STRTOK(NULL, word_delim, &last_fixed);
	while (key_fixed!=NULL && value_fixed!=NULL) {
		if (!set_fixed_single_param(key_fixed, atof(value_fixed))) {
			return INPUT_PARAM_IS_INVALID;
		}
		if (num++ == cubic_fixed_param) {
			goto resolve_variable_string;
		}
		key_fixed = STRTOK(NULL, word_delim, &last_fixed);
		if (key_fixed == NULL) {
			return INPUT_PARAM_IS_INVALID;
		}
		value_fixed = STRTOK(NULL, word_delim, &last_fixed);
		if (value_fixed == NULL) {
			return INPUT_PARAM_IS_INVALID;
		}
	}
	// resolve the variable_model_param
	resolve_variable_string:
	char pointwise_param[MAX_VARIABLE_MODEL_PARAM_LENGTH];
	strcpy(pointwise_param, variable_model_param);
	char *key_variable;
	char *value_variable;
	char *last_variable = NULL;
	num = 1;
	key_variable = STRTOK(pointwise_param, word_delim, &last_variable);
	value_variable = STRTOK(NULL, word_delim, &last_variable);
	while (key_variable!=NULL && value_variable!=NULL) {
		if (!set_variable_single_param(key_variable, value_variable)) {
			return INPUT_PARAM_IS_INVALID;
		}
		if (num++ == cubic_variable_param) {
			goto resolve_variable_params;
		}
		key_variable = STRTOK(NULL, word_delim, &last_variable);
		if (key_variable == NULL) {
			return INPUT_PARAM_IS_INVALID;
		}
		value_variable = STRTOK(NULL, word_delim, &last_variable);
		if (value_variable == NULL) {
			return INPUT_PARAM_IS_INVALID;
		}
	}
	resolve_variable_params:
	if (strike_string == NULL || constant_string == NULL || first_order_string == NULL || second_order_string == NULL || third_order_string == NULL) {
		return INPUT_PARAM_IS_INVALID;
	}
	const char array_delim[] = "#";
	char *token;
	char *last_string = NULL;
	token = STRTOK(strike_string, array_delim, &last_string);
	unsigned int i = 0;
	while (token) {
		a_poly_param_[i++].strike = atof(token);
		token = STRTOK(NULL, array_delim, &last_string);
	}
	// check if size is in proper range
	if (i < CUBICSPLINE_MIN_POINT_NUM) {
		return DATA_NUM_IS_TOO_FEW;
	}
	if (i > CUBICSPLINE_MAX_POINT_NUM) {
		return DATA_NUM_IS_TOO_MUCH;
	}
	last_string = NULL;
	token = STRTOK(constant_string, array_delim, &last_string);
	unsigned int j = 0;
	while (token) {
		a_poly_param_[j++].constant = atof(token);
		token = STRTOK(NULL, array_delim, &last_string);
	}
	if (i != j) {
		return STRIKE_VOLATILITY_MISMATCH;
	}

	last_string = NULL;
	token = STRTOK(first_order_string, array_delim, &last_string);
	j = 0;
	while (token) {
		a_poly_param_[j++].first_order = atof(token);
		token = STRTOK(NULL, array_delim, &last_string);
	}
	if (i != j) {
		return STRIKE_VOLATILITY_MISMATCH;
	}

	last_string = NULL;
	token = STRTOK(second_order_string, array_delim, &last_string);
	j = 0;
	while (token) {
		a_poly_param_[j++].second_order = atof(token);
		token = STRTOK(NULL, array_delim, &last_string);
	}
	if (i != j) {
		return STRIKE_VOLATILITY_MISMATCH;
	}

	last_string = NULL;
	token = STRTOK(third_order_string, array_delim, &last_string);
	j = 0;
	while (token) {
		a_poly_param_[j++].third_order = atof(token);
		token = STRTOK(NULL, array_delim, &last_string);
	}
	if (i != j) {
		return STRIKE_VOLATILITY_MISMATCH;
	}
	else{
		size_ = i;
		return RETURN_SUCCESS;
	}
	return UNKNOWN_ERROR;
}

bool cubicspline_mixed_impl::is_dynamic_model() {
	return false;
}

void cubicspline_mixed_impl::get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]) {
	static const char cubic_spline_volatility_custom_param[] = "volatility_partial";
	strcpy(volatility_custom_param_name, cubic_spline_volatility_custom_param);
	return;
}

bool cubicspline_mixed_impl::get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward, IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]) {
	if (volatility == NULL || size_ == 0){
		return false;
	}
	volatility_custom_param_value[0] = volatility_partial_;
	double normalize_strike = log(strike_price / atm_forward_);
	if (strike_price < a_poly_param_[0].strike) {
		*volatility = atm_volatility_ + slope_ * normalize_strike + a_poly_param_[0].constant + a_poly_param_[0].first_order * (normalize_strike - log(a_poly_param_[0].strike /atm_forward_));
		return true;
	}

	if (strike_price >= a_poly_param_[size_-1].strike) {
		double h = log(a_poly_param_[size_-1].strike / atm_forward_) - log(a_poly_param_[size_-2].strike / atm_forward_);
		double aux1 = a_poly_param_[size_-2].constant + a_poly_param_[size_-2].first_order * h + a_poly_param_[size_-2].second_order * h*h + a_poly_param_[size_-2].third_order * h*h*h;
		double aux2 = a_poly_param_[size_-2].first_order + 2 * a_poly_param_[size_-2].second_order * h + 3 * a_poly_param_[size_-2].third_order * h*h;
		*volatility = atm_volatility_ + slope_ * normalize_strike + aux1 + aux2 * (normalize_strike - log(a_poly_param_[size_-1].strike / atm_forward_));
		return true;
	}

	for (unsigned int i = 0; i < size_-1; i++)
	{
		if (strike_price >= a_poly_param_[i].strike && strike_price < a_poly_param_[i+1].strike)
		{
			double h = normalize_strike - log(a_poly_param_[i].strike / atm_forward_);
			*volatility = atm_volatility_ + slope_ * normalize_strike + a_poly_param_[i].constant + a_poly_param_[i].first_order * h + a_poly_param_[i].second_order * h*h + a_poly_param_[i].third_order * h*h*h;
			return true;
		}
	}
	return false;
}

int cubicspline_mixed_impl::fit(const volatility_point *p_volatility_points,
		 						 const unsigned int size,
		 						 const double atm_forward,
								 const double left_trading_years,
            					 const double rate,
		 						 char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
		 						 char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH])
{
	if (size < CUBICSPLINE_MIN_POINT_NUM) {
		return DATA_NUM_IS_TOO_FEW;
	}
	if (size > CUBICSPLINE_MAX_POINT_NUM) {
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

	// step 1: copy volatility points to my space and sort it.
	size_ = size;
	atm_forward_ = atm_forward;
	memcpy(a_strike_vol_, p_volatility_points, sizeof(volatility_point) * size);
	std::sort(a_strike_vol_, a_strike_vol_+size_, cmp_strike_vol_);
	//step 2: process the raw_data
	normalize_strike_vol();
	//step 3: fit the normalized_strike_vol, try to fit (normalize_strike_i, volatility_i)
	//calculate atm_volatility and slope; update the normalized_strike_vol
	if (gauss_derive()) {
		get_atmvol_slope(a_poly_param_, &atm_volatility_, &slope_);
		for(unsigned int i = 0; i < size_ ; i++){
			a_normal_strike_vol_[i].volatility = a_strike_vol_[i].volatility - atm_volatility_ - slope_ * a_normal_strike_vol_[i].strike;
		}
	}
	//step 4 :fit (normalize_strike_i, volatility_i - atm_volatility_i - slope_i * normalize_strike_i)
	if (gauss_derive()) {
		get_param_value(fixed_model_param, variable_model_param);
		return RETURN_SUCCESS;
	}
	return UNKNOWN_ERROR;
}

/****** protected ********/
void cubicspline_mixed_impl::normalize_strike_vol(){
	for(unsigned int i = 0; i < size_ ; i++){
		a_normal_strike_vol_[i].strike = log(a_strike_vol_[i].strike / atm_forward_);
		a_normal_strike_vol_[i].volatility = a_strike_vol_[i].volatility;
	}
}

bool cubicspline_mixed_impl::gauss_derive() {
	//initialize the matrix
	//a_poly_matrix_[CUBICSPLINE_MAX_POINT_NUM * CUBICSPLINE_MAX_POINT_NUM] = {0};
	//a_y_matrix_[CUBICSPLINE_MAX_POINT_NUM] = {0};
	//a_res_[CUBICSPLINE_MAX_POINT_NUM] = {0};
	// the following steps is using gauss method to calculate polynomial parameter
	// step 1 generate coefficients matrix
	gen_coef_matrix(a_normal_strike_vol_, size_, a_poly_matrix_, a_y_matrix_);

	// step 2 calculate second_orders
	if ( ! calculate_gauss(a_poly_matrix_, a_y_matrix_, size_, a_res_) ) {
		size_ = 0;
		return false;
	}

	// step 3 generate polynomial parameters
	gen_poly_param(a_normal_strike_vol_, a_res_, size_, a_poly_param_);

	return true;
}

void cubicspline_mixed_impl::gen_coef_matrix(IN volatility_point *strike_vol, IN unsigned int size, OUT double *poly_matrix, OUT double *y_matrix) {
	// first row
	poly_matrix[0] = 1.0;
	for (unsigned int j = 1; j < size; j++) { poly_matrix[j] = 0.0; }
	y_matrix[0] = 0;

	// mid rows
	double h_0 = 0.0;
	double h = 0.0;
	double h_1 = 0.0;
	int index = 0;

	for (unsigned int i = 1; i < size-1; i++) {
		for (unsigned int j = 0; j < size; j++) {
			index = i * size + j;
			if (j == i-1) {
				h_0 = strike_vol[i].strike - strike_vol[i-1].strike;
				poly_matrix[index] = h_0;
			}
			else if (j == i) {
				h = 2 * (strike_vol[i+1].strike - strike_vol[i-1].strike);
				poly_matrix[index] = h;
			}
			else if (j == i+1) {
				h_1 = strike_vol[i+1].strike - strike_vol[i].strike;
				poly_matrix[index] = h_1;
			}
			else {
				poly_matrix[index] = 0.0;
			}
		}
		y_matrix[i] = 6 * (
						((strike_vol[i+1].volatility - strike_vol[i].volatility) / h_1) -
						((strike_vol[i].volatility - strike_vol[i-1].volatility) / h_0)
						  );
	}

	//last row
	for (unsigned int j = size*size-size; j < size*size-1; j++) { poly_matrix[j] = 0.0; }
	poly_matrix[size*size-1] = 1.0;
	y_matrix[size-1] = 0;
}


bool cubicspline_mixed_impl::calculate_gauss(IN double *poly_matrix, IN double* y_matrix, IN unsigned int size, OUT double *result) {
	//whether can use gauss. If there is 0 on diagonal, the gauss elimination method cannot be used.
	for (unsigned int i = 0; i < size; i++)
	{
		if(poly_matrix[i*size + i] == 0)
		{
			size_ = 0;
			return false;
		}
	}

	//the elimination process, size-1 in total
	double c[CUBICSPLINE_MAX_POINT_NUM];
	for (unsigned int k = 0; k < size - 1; k++)
	{
		//find the coefficient of the k-th elementary row operation
		for (unsigned int i = k + 1; i < size; i++)
		{
			c[i] = poly_matrix[i*size + k] / poly_matrix[k*size + k];
		}

		//the k-th elimination
		for (unsigned int i = k + 1; i < size; i++)
		{
			for (unsigned int j = 0; j < size; j++)
			{
				poly_matrix[i*size + j] = poly_matrix[i*size + j] - c[i] * poly_matrix[k*size + j];
			}
			y_matrix[i] = y_matrix[i] - c[i] * y_matrix[k];
		}
	}

	//the array to store the solutions
	//figure out the last unknown first
	result[size - 1] = y_matrix[size - 1] / poly_matrix[(size - 1) * (size + 1)];
	//figure out all the unknown
	for (int i = (int)(size - 2); i >= 0; i--)
	{
		double sum = 0;
		for (int j = i + 1; j < (int)size; j++)
		{
			sum +=  poly_matrix[i*size + j] * result[j];
		}
		result[i] = (y_matrix[i] - sum) / poly_matrix[i*size + i];
	}

	return true;
}


void cubicspline_mixed_impl::gen_poly_param(IN volatility_point *strike_vol, IN double *result, IN unsigned int size, OUT cubicspline_poly_param *poly_param) {
	// for index = 0 ~ size-2
	for(unsigned int i = 0; i < size-1; i++)
	{
		double hi = strike_vol[i+1].strike - strike_vol[i].strike;
		double a1 = (strike_vol[i+1].volatility - strike_vol[i].volatility ) / hi;
		double a2 = (hi * (result[i+1] + 2 * result[i])) / 6;

		poly_param[i].strike = a_strike_vol_[i].strike;
		poly_param[i].constant = strike_vol[i].volatility;
		poly_param[i].first_order = a1 - a2;
		poly_param[i].second_order = result[i] * 0.5;
		poly_param[i].third_order = (result[i+1] - result[i]) / (6 * hi);
	}
	// for index = size-1
		poly_param[size-1].strike = a_strike_vol_[size-1].strike;
		poly_param[size-1].constant = strike_vol[size-1].volatility;
		poly_param[size-1].first_order = 0;
		poly_param[size-1].second_order = result[size-1] * 0.5;
		poly_param[size-1].third_order = 0;
}

bool cubicspline_mixed_impl::valid_price(double price) {
	return (price > -DBL_MAX && price < DBL_MAX);
}

void cubicspline_mixed_impl::get_atmvol_slope(IN cubicspline_poly_param *poly_param, OUT double *sigma_cur, OUT double *slope_cur) {
	if (atm_forward_ < poly_param[0].strike) {
		*sigma_cur = poly_param[0].constant + poly_param[0].first_order * (0 - log(poly_param[0].strike /atm_forward_));
		*slope_cur = 0;
		return;
	}
	if (atm_forward_ >= poly_param[size_-1].strike) {
		double h = log(poly_param[size_-1].strike / atm_forward_) - log(poly_param[size_-2].strike / atm_forward_);
		double aux1 = poly_param[size_-2].constant + poly_param[size_-2].first_order * h + poly_param[size_-2].second_order * h*h + poly_param[size_-2].third_order * h*h*h;
		double aux2 = poly_param[size_-2].first_order + 2 * poly_param[size_-2].second_order * h + 3 * poly_param[size_-2].third_order * h*h;
		*sigma_cur = aux1 + aux2 * (0 - log(poly_param[size_-1].strike / atm_forward_));
		*slope_cur = 0;
		return;
	}

	for (unsigned int i = 0; i < size_-1; i++)
	{
		if (atm_forward_ >= poly_param[i].strike && atm_forward_ < poly_param[i+1].strike)
		{
			double h = 0 - log(poly_param[i].strike / atm_forward_);
			*sigma_cur = poly_param[i].constant + poly_param[i].first_order * h + poly_param[i].second_order * h*h + poly_param[i].third_order * h*h*h;
			*slope_cur = poly_param[i].first_order + 2 * poly_param[i].second_order * h + 3 * poly_param[i].third_order * h*h;
			return;
		}
	}
	return;
}

bool cubicspline_mixed_impl::set_fixed_single_param(char *key, float value) {
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
	else {
		return false;
	}
	return true;
}

bool cubicspline_mixed_impl::set_variable_single_param(char *key, char *value) {
	if(key == NULL || value == NULL){
		return false;
	}
	else if (strcmp(key, "strike") == 0) {
		strcpy(strike_string, value);
	}
	else if (strcmp(key, "constant") == 0) {
		strcpy(constant_string, value);
	}
	else if (strcmp(key, "first_order") == 0) {
		strcpy(first_order_string, value);
	}
	else if (strcmp(key, "second_order") == 0) {
		strcpy(second_order_string, value);
	}
	else if (strcmp(key, "third_order") == 0) {
		strcpy(third_order_string, value);
	}
	else {
		return false;
	}
	return true;
}

bool cubicspline_mixed_impl::is_valid_atm_forward(double atm_forward) const { return atm_forward > 0; }
bool cubicspline_mixed_impl::is_valid_atm_volatility(double atm_volatility) const { return atm_volatility >= 0; }
bool cubicspline_mixed_impl::is_valid_slope(double slope) const { return true; }


}
}
}
