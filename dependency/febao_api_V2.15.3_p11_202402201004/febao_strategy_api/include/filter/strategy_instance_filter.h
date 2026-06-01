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


#ifndef FB_STRATEGY_INSTANCE_FILTER_H
#define FB_STRATEGY_INSTANCE_FILTER_H

#include <stdint.h>
#include "i_filter.h"

namespace cffex {
namespace fb {

class strategy_instance_equal_filter : public i_filter {
 public:
    strategy_instance_equal_filter(const int strategy_instance_id);
    virtual ~strategy_instance_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool check(const i_filter::i_filter_msg *msg) const;

 private:
    int strategy_instance_id_;
};

class strategy_instance_filter_factory : public filter_factory {
 public:
    static strategy_instance_filter_factory *get_instance();
    strategy_instance_filter_factory();
    ~strategy_instance_filter_factory();
    const i_filter *create_strategy_instance_equal_filter(const int strategy_instance_id);
};

}
}

#endif
