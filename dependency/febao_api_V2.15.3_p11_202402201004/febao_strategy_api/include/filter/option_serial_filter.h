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

#ifndef FB_OPTION_SERIAL_FILTER_H
#define FB_OPTION_SERIAL_FILTER_H

#include <stdint.h>

#include "i_filter.h"

namespace cffex {
namespace fb {

#ifndef OPTION_SERIAL_ID_LENGTH
#define OPTION_SERIAL_ID_LENGTH 32
#endif

class option_serial_equal_filter : public i_filter {
public:
    option_serial_equal_filter(const char *option_serial_id);
    virtual ~option_serial_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool            check(const i_filter::i_filter_msg *msg) const;

private:
    char option_serial_id_[OPTION_SERIAL_ID_LENGTH];
};

class option_serial_index_equal_filter : public i_filter {
public:
    option_serial_index_equal_filter(uint64_t option_serial_index);
    virtual ~option_serial_index_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool            check(const i_filter::i_filter_msg *msg) const;

private:
    uint64_t option_serial_index_;
};

class option_serial_filter_factory : public filter_factory {
public:
    static option_serial_filter_factory *get_instance();
    option_serial_filter_factory();
    ~option_serial_filter_factory();
    const i_filter *create_option_serial_equal_filter(
        const char *option_serial_id);
    const i_filter *create_option_serial_index_equal_filter(
        uint64_t option_serial_index);
};

}  // namespace fb
}  // namespace cffex

#endif
