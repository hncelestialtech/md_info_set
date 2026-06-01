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


#ifndef FB_CUSTOM_FILTER_H
#define FB_CUSTOM_FILTER_H

#include <stdint.h>
#include "i_filter.h"

namespace cffex {
namespace fb {

class custom_equal_filter : public i_filter {
 public:
    custom_equal_filter(const char *custom_id);
    virtual ~custom_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool check(const i_filter::i_filter_msg *msg) const;

 private:
    char   custom_id_[32];
};

class custom_filter_factory : public filter_factory {
 public:
    static custom_filter_factory *get_instance();
    custom_filter_factory();
    ~custom_filter_factory();
    const i_filter *create_custom_equal_filter(const char *custom_id) ;
};

}
}

#endif
