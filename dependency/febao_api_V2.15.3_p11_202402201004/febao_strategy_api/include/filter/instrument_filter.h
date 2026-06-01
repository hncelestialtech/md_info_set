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

#ifndef FB_INSTRUMENT_FILTER_H
#define FB_INSTRUMENT_FILTER_H

#include <stdint.h>

#include "i_filter.h"

namespace cffex {
namespace fb {

#ifndef INSTRUMENT_ID_LENGTH
#define INSTRUMENT_ID_LENGTH 32
#endif

class instrument_equal_filter : public i_filter {
public:
    instrument_equal_filter(const char *instrument_id);
    virtual ~instrument_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool            check(const i_filter::i_filter_msg *msg) const;

private:
    char instrument_id_[INSTRUMENT_ID_LENGTH];
};

class instrument_index_equal_filter : public i_filter {
public:
    instrument_index_equal_filter(uint64_t instrument_index);
    virtual ~instrument_index_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool            check(const i_filter::i_filter_msg *msg) const;

private:
    uint64_t instrument_index_;
};

class instrument_filter_factory : public filter_factory {
public:
    static instrument_filter_factory *get_instance();
    instrument_filter_factory();
    ~instrument_filter_factory();
    const i_filter *create_instrument_equal_filter(const char *instrument_id);
    const i_filter *create_instrument_index_equal_filter(
        uint64_t instrument_index);
};

}  // namespace fb
}  // namespace cffex

#endif
