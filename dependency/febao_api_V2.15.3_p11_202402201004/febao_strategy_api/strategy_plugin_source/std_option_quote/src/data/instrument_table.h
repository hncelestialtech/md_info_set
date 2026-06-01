/**
 * CFFEX Confidential.
 *
 * @Copyright 2019 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2019-11-21
 */

#ifndef FEBAO_STRATEGY_INSTRUMENT_TABLE_H
#define FEBAO_STRATEGY_INSTRUMNET_TABLE_H

#include <map>
#include "strategy_enum_type.h"
#include "math_helper.h"
#include "string_helper.h"
#include "caller_handler.h"

namespace cffex {
namespace strategy {

#define RECANCEL_THRESHOLD 3

struct record
{
    char                    instrument_id[32];
    double                  down_band_;
    double                  upper_band_;
    unsigned int            recancel_times_;
    int64_t                 quote_id_;

    record(const char *instrument)
    {
        strcpy(instrument_id, instrument);
        down_band_  = DBL_MAX;
        upper_band_ = DBL_MAX;
        recancel_times_ = 0;
    }

    void set_band_price(double down_price, double upper_price)
    {
        down_band_ = down_price;
        upper_band_ = upper_price;
    }

    bool check_recancel_times()
    {
        return recancel_times_ < RECANCEL_THRESHOLD;
    }
};

typedef std::map<const char *, record *, string_less_functor>  record_mapping;

class instrument_table
{
 public:
    instrument_table(caller_handler *callers) : callers_(callers) {}
    ~instrument_table();

    record *first();

    record *next();

    record *get_record(const char *instrument_id, bool insert = true);

    void update_band_price(const char* instrument, double down_price, double upper_price);

    void clear_recancel_times(const char* instrument);
    bool check_recancel_times(const char* instrument);
    void record_recancel_times(const char* instrument);

protected:
    record_mapping              records_;
    record_mapping::iterator    itor_;
    caller_handler              *callers_;
};

}
}

#endif
