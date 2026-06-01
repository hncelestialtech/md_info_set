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


#ifndef FB_INSTRUMENT_GROUP_FILTER_H
#define FB_INSTRUMENT_GROUP_FILTER_H

#include <stdint.h>
#include "i_filter.h"

namespace cffex {
namespace fb {

#ifndef INSTRUMENT_GROUP_ID_LENGTH
#define INSTRUMENT_GROUP_ID_LENGTH 64
#endif

class instrument_group_equal_filter : public i_filter {
 public:
    instrument_group_equal_filter(const char *instrument_group_id);
    virtual ~instrument_group_equal_filter();
    virtual const i_filter *clone() const;
    virtual bool check(const i_filter::i_filter_msg *msg) const;

 private:
    char instrument_group_id_[INSTRUMENT_GROUP_ID_LENGTH];
};

class instrument_group_filter_factory : public filter_factory {
 public:
    static instrument_group_filter_factory *get_instance();
    instrument_group_filter_factory();
    ~instrument_group_filter_factory();
    const i_filter *create_instrument_group_equal_filter(const char *instrument_group_id);
};

}
}

#endif
