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


#ifndef FB_DYNAMIC_PARAM_FILTER_H
#define FB_DYNAMIC_PARAM_FILTER_H

#include <stdint.h>
#include "i_filter.h"

namespace cffex {
namespace fb {

#ifndef PARAM_NAME_LENGTH
#define PARAM_NAME_LENGTH 64
#endif

class dynamic_param_equal_filter : public i_filter {
 public:
    dynamic_param_equal_filter(const char *param_name);
    virtual ~dynamic_param_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool check(const i_filter::i_filter_msg *msg) const;

 private:
    char param_name_[PARAM_NAME_LENGTH];
};

class dynamic_param_filter_factory : public filter_factory {
 public:
    static dynamic_param_filter_factory *get_instance();
    dynamic_param_filter_factory();
    ~dynamic_param_filter_factory();
    const i_filter *create_dynamic_param_equal_filter(const char *param_name);
};

}
}

#endif