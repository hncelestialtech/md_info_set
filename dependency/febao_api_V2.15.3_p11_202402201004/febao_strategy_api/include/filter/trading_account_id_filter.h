/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: zhr
 * Date: 2018-10-16
 */


#ifndef TRADING_ACCOUNT_ID_FILTER_H
#define TRADING_ACCOUNT_ID_FILTER_H

#include <stdint.h>
#include "i_filter.h"

namespace cffex {
namespace fb {

class trading_account_id_equal_filter : public i_filter {
 public:
    trading_account_id_equal_filter(const int trading_account_id);
    virtual ~trading_account_id_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool check(const i_filter::i_filter_msg *msg) const;
    // virtual const int   get_trading_account_id()   const { return trading_account_id_; }

 private:
    int   trading_account_id_;
};

/**
 * @brief trading_account_id_filter_factory 自定义参数过滤器创建工厂 \n
 *        策略代码使用本类提供的API完成自定义参数过滤器创建，调用接口完成功能：\n
 * @ingroup trading_account_id_filter_factory
 */
class trading_account_id_filter_factory : public filter_factory {
 public:
    static trading_account_id_filter_factory *get_instance();
    trading_account_id_filter_factory();
    ~trading_account_id_filter_factory();
    /// 创建自定义参数ID过滤器实例
    const i_filter *create_trading_account_id_equal_filter(const int trading_account_id) ;
};

}
}

#endif
