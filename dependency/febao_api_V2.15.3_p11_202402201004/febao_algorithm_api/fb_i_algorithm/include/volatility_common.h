#ifndef CFFEX_FB_VOLATILITY_MODEL_COMMON_H
#define CFFEX_FB_VOLATILITY_MODEL_COMMON_H

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#define MAX_FIXED_MODEL_PARAM_LENGTH 512
#define MAX_VARIABLE_MODEL_PARAM_LENGTH 4096
#define MAX_VOLATILITY_CUSTOM_PARAM_NAME_LENGTH 1024
#define MAX_CUSTOM_PARAM_LIST_LENGTH 512
#define MAX_CUSTOM_PARAM_NAME_LENGTH 32
#define MAX_CUSTOM_PARAM_COUNT 10

struct volatility_point {
    double strike;
    double volatility;
    double forward_price;
};

enum volatility_return_errorcode {
    RETURN_SUCCESS                     = 0,    // 成功
    INPUT_PARAM_IS_INVALID             = -1,   // 输入参数无效
    DATA_NUM_IS_TOO_MUCH               = -2,   // 数据点太多
    DATA_NUM_IS_TOO_FEW                = -3,   // 数据点太少
    INPUT_OUTPUT_PARAM_IS_NULL         = -4,   // 输入输出参数为空
    ATM_FORWARD_IS_INVALID             = -5,   // atm_forward非法
    ATM_FORWARD_IS_NOT_IN_PROPER_RANGE = -6,   // atm_forward不在拟合范围
    FIT_RANGE_IS_INVALID               = -7,   // fit_range无效
    STRIKE_VOLATILITY_MISMATCH         = -8,   // strike与volatility不匹配
    UNKNOWN_ERROR                      = -20,  // 未知错误
};

#endif
