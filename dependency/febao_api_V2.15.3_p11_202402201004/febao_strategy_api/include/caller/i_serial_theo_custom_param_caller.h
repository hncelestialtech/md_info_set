#ifndef FB_SERIAL_THEO_CUSTOM_PARAM_CALLER_H
#define FB_SERIAL_THEO_CUSTOM_PARAM_CALLER_H

#include <stdint.h>

#include "i_caller.h"

namespace cffex {
namespace fb {

/**
 * @brief i_serial_theo_custom_param_caller 设置期权系列定价自定义参数
 *
 * @ingroup i_serial_theo_custom_param_caller
 */
class i_serial_theo_custom_param_caller :
    public i_caller_with_id<i_serial_theo_custom_param_caller,
                            STRATEGY_CALLER_SERIAL_THEO_CUSTOM_PARAM> {
public:
    /**
     * @brief
     * serial_theo_custom_param_entity接口类抽象了期权系列定价自定义参数设置结构，提供其操作界面；策略代码必须调用create_entity创建实例对象
     *        serial_theo_custom_param_entity对象的生命周期由策略进程管理：
     *        -
     * 策略代码调用modify_theo_custom_param完成报价后，无需删除serial_theo_custom_param_entity对象
     *        - 其生命周期是两次调用create_entity之间的时间
     *        - 调用create_custom_param_entity返回的是存有当前值的对象
     */
    class serial_theo_custom_param_entity { /* absolute value */
    public:
        virtual ~serial_theo_custom_param_entity() = default;
        /// 设置自定义参数(绝对值)
        virtual void set_custom_param(IN const char *param_name, IN double param_value) = 0;
        virtual void reset_entity()                                                     = 0;
    };

    /**
     * @brief
     * diff_serial_theo_custom_param_entity接口类抽象了期权系列定价自定义参数差值设置结构，提供其操作界面；策略代码必须调用create_entity创建实例对象
     *        diff_serial_theo_custom_param_entity对象的生命周期由策略进程管理：
     *        -
     * 策略代码调用modify_diff_theo_custom_param完成报价后，无需删除diff_serial_theo_custom_param_entity对象
     *        - 其生命周期是两次调用create_entity之间的时间
     *        - 调用create_diff_custom_param_entity返回的是存有当前值的对象
     */
    class diff_serial_theo_custom_param_entity { /* diff value */
    public:
        virtual ~diff_serial_theo_custom_param_entity() = default;
        /// 设置自定义参数(差值)
        virtual void set_diff_custom_param(IN const char *param_name, IN double diff_value) = 0;
        virtual void reset_entity()                                                         = 0;
    };

    i_serial_theo_custom_param_caller()          = default;
    virtual ~i_serial_theo_custom_param_caller() = default;

    /**
     * @brief 创建空的修改结构
     * @return 带有当前值的期权系列定价参数修改结构指针
     */
    virtual serial_theo_custom_param_entity *create_custom_param_entity(
        IN const char *option_serial_id) = 0;
    virtual serial_theo_custom_param_entity *create_custom_param_entity(
        IN const uint64_t option_serial_index) = 0;

    /**
     * @brief 创建空的差值修改结构
     * @return 带有当前值的期权系列定价自定义差值参数修改结构指针 默认值是0.000000
     */
    virtual diff_serial_theo_custom_param_entity *create_diff_custom_param_entity(
        IN const char *option_serial_id) = 0;
    virtual diff_serial_theo_custom_param_entity *create_diff_custom_param_entity(
        IN const uint64_t option_serial_index) = 0;

    /**
     * @brief 执行期权系列定价自定义参数绝对值修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_theo_custom_param(serial_theo_custom_param_entity *entity) = 0;

    /**
     * @brief 执行期权系列定价自定义参数差值修改
     * @param entity [IN] 修改结构指针
     * @return int32_t 成功返回0，失败返回-1
     */
    virtual int32_t modify_diff_theo_custom_param(diff_serial_theo_custom_param_entity *entity) = 0;
};

}  // namespace fb
}  // namespace cffex

#endif