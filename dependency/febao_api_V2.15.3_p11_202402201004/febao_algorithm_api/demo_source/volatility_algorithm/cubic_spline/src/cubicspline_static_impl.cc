#include "stdio.h"
#include "cubicspline_static_impl.h"
#include <cstring>
#include <algorithm>
#include "stdlib.h"

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

cubicspline_static_impl::cubicspline_static_impl() : size_(0)
{
    volatility_partial_ = CFFEX_DOUBLE_NULL;
}

cubicspline_static_impl::~cubicspline_static_impl() {}


void cubicspline_static_impl::get_param_list(char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	static const char cubic_spline_param[] = "strike:double;volatility:double";
	strcpy(variable_param_list, cubic_spline_param);
	strcpy(fixed_param_list, "");
	return;
}

void cubicspline_static_impl::get_param_value(char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	strcpy(fixed_model_param, "");
	if (size_ < 1) {
		strcpy(variable_model_param, "");
		return;
	}
	sprintf(variable_model_param, "strike:%.6f", a_strike_[0]);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_strike_[i]);
	}
	sprintf(variable_model_param + strlen(variable_model_param), ";volatility:%.6f", a_volatility_[0]);
	for (unsigned int i=1; i<size_; i++) {
		sprintf(variable_model_param + strlen(variable_model_param), "#%.6f", a_volatility_[i]);
	}
	return;
}

int cubicspline_static_impl::set_param_value(char const fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH], char const variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]) {
	// strike:3100-3200;volatility:0.123-0.129
	if (variable_model_param == NULL) {
		return INPUT_OUTPUT_PARAM_IS_NULL;
	}

	char param[MAX_VARIABLE_MODEL_PARAM_LENGTH];
	strcpy(param, variable_model_param);
	char strike_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
	char volatility_string[MAX_VARIABLE_MODEL_PARAM_LENGTH];
	const char word_delim[] = ":;";
	char *key;
	char *value;
	char *last;
	key = STRTOK(param, word_delim, &last);
	value = STRTOK(NULL, word_delim, &last);
	if (key == NULL || value == NULL) {
		return INPUT_PARAM_IS_INVALID;
	}
	if (strcmp(key, "strike") == 0) {
		strcpy(strike_string, value);
	}
	else if (strcmp(key, "volatility") == 0) {
		strcpy(volatility_string, value);
	}
	else {
		return INPUT_PARAM_IS_INVALID;
	}

	key = STRTOK(NULL, word_delim, &last);
	value = STRTOK(NULL, word_delim, &last);
	if (key == NULL || value == NULL) {
		return INPUT_PARAM_IS_INVALID;
	}
	if (strcmp(key, "strike") == 0) {
		strcpy(strike_string, value);
	}
	else if (strcmp(key, "volatility") == 0) {
		strcpy(volatility_string, value);
	}
	else {
		return INPUT_PARAM_IS_INVALID;
	}

	if (strike_string == NULL || volatility_string == NULL) {
		return INPUT_PARAM_IS_INVALID;
	}

	const char array_delim[] = "#";
	char *token;
	char *last_s = NULL;
	token = STRTOK(strike_string, array_delim, &last_s);
	unsigned int i = 0;
	while (token) {
		a_strike_vol_[i++].strike = atof(token);
		token = STRTOK(NULL, array_delim, &last_s);
	}
	// check if size is in proper range
	if (i < CUBICSPLINE_MIN_POINT_NUM) {
		return DATA_NUM_IS_TOO_FEW;
	}
	if (i > CUBICSPLINE_MAX_POINT_NUM) {
		return DATA_NUM_IS_TOO_MUCH;
	}

	char *last_v = NULL;
	token = STRTOK(volatility_string, array_delim, &last_v);
	unsigned j = 0;
	while (token) {
		a_strike_vol_[j++].volatility = atof(token);
		token = STRTOK(NULL, array_delim, &last_v);
	}
	// check if size is in proper range
	if (j < CUBICSPLINE_MIN_POINT_NUM) {
		return DATA_NUM_IS_TOO_FEW;
	}
	if (j > CUBICSPLINE_MAX_POINT_NUM) {
		return DATA_NUM_IS_TOO_MUCH;
	}

	if (i != j) {
		return STRIKE_VOLATILITY_MISMATCH;
	}
	size_ = i;

	if (gauss_derive()) {
		return RETURN_SUCCESS;
	}
	return UNKNOWN_ERROR;
}

bool cubicspline_static_impl::is_dynamic_model() {
	return false;
}

void cubicspline_static_impl::get_volatility_custom_param_name(OUT char volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]) {
	static const char cubic_spline_volatility_custom_param[] = "volatility_partial";
	strcpy(volatility_custom_param_name, cubic_spline_volatility_custom_param);
	return;
}

bool cubicspline_static_impl::get_volatility(IN const double strike_price, IN const double atm_forward, IN const double option_forward,IN const double left_trading_years, IN const double rate, OUT double* volatility, OUT double volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]) {
	if (volatility == NULL || size_ == 0){
		return false;
	}
	volatility_custom_param_value[0] = volatility_partial_;
	if (strike_price < a_poly_param_[0].strike) {
		*volatility = a_poly_param_[0].a + a_poly_param_[0].b * (strike_price - a_poly_param_[0].strike);
		return true;
	}

	if (strike_price >= a_poly_param_[size_-1].strike) {
		double h = a_poly_param_[size_-1].strike - a_poly_param_[size_-2].strike;
		double aux1 = a_poly_param_[size_-2].a + a_poly_param_[size_-2].b * h + a_poly_param_[size_-2].c * h*h + a_poly_param_[size_-2].d * h*h*h;
		double aux2 = a_poly_param_[size_-2].b + 2 * a_poly_param_[size_-2].c * h + 3 * a_poly_param_[size_-2].d * h*h;
		*volatility = aux1 + aux2 * (strike_price - a_poly_param_[size_-1].strike);
		return true;
	}

	for (unsigned int i = 0; i < size_-1; i++)
	{
		if (strike_price >= a_poly_param_[i].strike && strike_price < a_poly_param_[i+1].strike)
		{
			double h = strike_price - a_poly_param_[i].strike;
			*volatility = a_poly_param_[i].a + a_poly_param_[i].b * h + a_poly_param_[i].c * h*h + a_poly_param_[i].d * h*h*h;
			return true;
		}
	}
	return false;
}

int cubicspline_static_impl::fit(const volatility_point *p_volatility_points,
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
	size_ = size;
	memcpy(a_strike_vol_, p_volatility_points, sizeof(volatility_point) * size);
	if (gauss_derive()) {
		get_param_value(fixed_model_param,variable_model_param);
		return RETURN_SUCCESS;
	}
	return UNKNOWN_ERROR;
}

/****** protected ********/
bool cubicspline_static_impl::gauss_derive() {
	// the following steps is using gauss method to calculate poly parameter
	// step 1 sort input points
	std::sort(a_strike_vol_, a_strike_vol_+size_, cmp_strike_vol_);
	for (unsigned int i=0; i<size_; i++) {
		// get ready for get_param_value return
		a_strike_[i] = a_strike_vol_[i].strike;
		a_volatility_[i] = a_strike_vol_[i].volatility;
	}

	// step 2
	gen_coef_matrix(a_strike_vol_, size_, a_poly_matrix_, a_y_matrix_);

	// step 3
	if ( ! calculate_gauss(a_poly_matrix_, a_y_matrix_, size_, a_res_) ) {
		size_ = 0;
		return false;
	}

	// step 4
	gen_poly_param(a_strike_vol_, a_res_, size_, a_poly_param_);

	return true;
}

void cubicspline_static_impl::gen_coef_matrix(IN volatility_point *strike_vol, IN unsigned int size, OUT double *poly_matrix, OUT double *y_matrix) {
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


bool cubicspline_static_impl::calculate_gauss(IN double *poly_matrix, IN double* y_matrix, IN unsigned int size, OUT double *result) {
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


void cubicspline_static_impl::gen_poly_param(IN volatility_point *strike_vol, IN double *result, IN unsigned int size, OUT cubicspline_poly_param *poly_param) {
	// for index = 0 ~ size-2
	for(unsigned int i = 0; i < size-1; i++)
	{
		double hi = strike_vol[i+1].strike - strike_vol[i].strike;
		double a1 = (strike_vol[i+1].volatility - strike_vol[i].volatility ) / hi;
		double a2 = (hi * (result[i+1] + 2 * result[i])) / 6;

		poly_param[i].strike = strike_vol[i].strike;
		poly_param[i].a = strike_vol[i].volatility;
		poly_param[i].b = a1 - a2;
		poly_param[i].c = result[i] * 0.5;
		poly_param[i].d = (result[i+1] - result[i]) / (6 * hi);
	}
	// for index = size-1
		poly_param[size-1].strike = strike_vol[size-1].strike;
		poly_param[size-1].a = strike_vol[size-1].volatility;
		poly_param[size-1].b = 0;
		poly_param[size-1].c = result[size-1] * 0.5;
		poly_param[size-1].d = 0;
}



}
}
}
