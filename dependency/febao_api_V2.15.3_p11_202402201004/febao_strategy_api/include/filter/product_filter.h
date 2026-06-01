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

#ifndef FB_PRODUCT_FILTER_H
#define FB_PRODUCT_FILTER_H

#include <stdint.h>

#include "i_filter.h"

namespace cffex {
namespace fb {

#ifndef PRODUCT_ID_LENGTH
#define PRODUCT_ID_LENGTH 32
#endif

class product_equal_filter : public i_filter {
public:
    product_equal_filter(const char *product_id);
    virtual ~product_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool            check(const i_filter::i_filter_msg *msg) const;

private:
    char product_id_[PRODUCT_ID_LENGTH];
};

class product_index_equal_filter : public i_filter {
public:
    product_index_equal_filter(uint64_t product_index);
    virtual ~product_index_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool            check(const i_filter::i_filter_msg *msg) const;

private:
    uint64_t product_index_;
};

class product_filter_factory : public filter_factory {
public:
    static product_filter_factory *get_instance();
    product_filter_factory();
    ~product_filter_factory();
    const i_filter *create_product_equal_filter(const char *product_id);
    const i_filter *create_product_index_equal_filter(uint64_t product_index);
};

}  // namespace fb
}  // namespace cffex

#endif
