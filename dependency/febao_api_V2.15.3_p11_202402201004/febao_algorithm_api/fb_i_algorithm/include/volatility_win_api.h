#ifndef CFFEX_FB_VOLATILITY_API_H
#define CFFEX_FB_VOLATILITY_API_H

#include "volatility_common.h"

#ifdef _WINDOWS
#ifdef API_EXPORTS
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* get model parameter list
 ** output @param_list:  return parameter list in JSON format
 ** output @fixed_number: indicate whether paramter count is fixed, for example
 *(wing is fixed, cubicspline is not fixed)
 ** */
API void get_param_list(
    OUT char fixed_param_list[MAX_FIXED_MODEL_PARAM_LENGTH],
    OUT char variable_param_list[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

/* get model parameter value */
/* return in JSON format*/
API void get_param_value(
    OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
    OUT char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

/* set model parameter, in JSON format */
/* return errorcode explained in volatility_common.h*/
API int set_param_value(
    IN const char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
    IN const char variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

/* get custom parameter list */
/* return in JSON format */
API void get_volatility_custom_param_name(
    OUT char
        volatility_custom_param_name[MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH]);

/* return fit volatility for a given strike price
 * */
API bool get_volatility(
    IN const double strike_price,
    IN const double atm_forward,
    IN const double option_forward,
    IN const double left_trading_years,
    IN const double rate,
    OUT double     *volatility,
    OUT double      volatility_custom_param_value[MAX_CUSTOM_PARAM_COUNT]);

API bool is_dynamic_model();

/* fit interface, transform imply volatility points to model parameter
** input  @p_volatility_points: the pointer of the array of strike-volatility
*pair
** input  @size:				size of the array of
*strike-volatility pair, shouldn't be less than 3
** input  @atm_forward:			use to split the points to put & call
*curve
** input  @fit_range:           use to decide the fit range of IV centered on
*atm_price, this value should be (0, 1]
** output @fixed_model_param:			return fitting result(fixed
*model param) to invoker
** output @variable_model_param:           return fitting result(pointwise
*param) to invoker
** return errorcode explained in volatility_common.h
*/

// in FeBao 2.0, fit is used in middel-end, no need for front-end
// API int fit(IN const volatility_point *p_volatility_points,
//               	  IN const unsigned int size,
//               	  IN const double atm_forward,
//               	  IN const double fit_range,
//                   OUT char fixed_model_param[MAX_FIXED_MODEL_PARAM_LENGTH],
//                    OUT char
//                    variable_model_param[MAX_VARIABLE_MODEL_PARAM_LENGTH]);

#ifdef __cplusplus
}
#endif

#endif
