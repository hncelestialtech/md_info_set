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


#ifndef FB_PORTFOLIO_NAME_FILTER_H
#define FB_PORTFOLIO_NAME_FILTER_H

#include <stdint.h>
#include "i_filter.h"

namespace cffex {
namespace fb {

#ifndef PORTFOLIO_NAME_LENGTH
#define PORTFOLIO_NAME_LENGTH 64
#endif

class portfolio_name_equal_filter : public i_filter {
 public:
    portfolio_name_equal_filter(const char *portfolio_name);
    virtual ~portfolio_name_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool check(const i_filter::i_filter_msg *msg) const;

 private:
    char portfolio_name_[PORTFOLIO_NAME_LENGTH];
};

class portfolio_name_filter_factory : public filter_factory {
 public:
    static portfolio_name_filter_factory *get_instance();
    portfolio_name_filter_factory();
    ~portfolio_name_filter_factory();
    const i_filter *create_portfolio_name_equal_filter(const char *portfolio_name) ;
};

}
}

#endif
