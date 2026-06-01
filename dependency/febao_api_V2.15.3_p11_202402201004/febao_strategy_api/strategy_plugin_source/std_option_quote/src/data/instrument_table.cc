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

#include "instrument_table.h"

namespace cffex {
namespace strategy {

instrument_table::~instrument_table()
{
    for(record_mapping::iterator itor = records_.begin(); itor != records_.end(); ++itor) {
        delete itor->second;
    }
}

record *instrument_table::first()
{
    if(records_.empty()) {
        return NULL;
    }
    itor_ = records_.begin();
    return itor_->second;
}

record *instrument_table::next()
{
    if(++itor_ == records_.end()) {
        return NULL;
    }
    return itor_->second;
}

record *instrument_table::get_record(const char *instrument_id, bool insert)
{
    if(records_.find(instrument_id) == records_.end() || records_.at(instrument_id) == NULL ) {
        if(!insert) {
            return NULL;
        }
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "instrument_table::%s, instrument_id[%s] not exist, create\n", __FUNCTION__, instrument_id);
        record *r = new record(instrument_id);
        records_.insert(std::pair<const char *, record *>(r->instrument_id, r));
    }
    return records_.at(instrument_id);
}

void instrument_table::update_band_price(const char *instrument, double down_price, double upper_price)
{
    if (!math_helper::valid_price(down_price) || !math_helper::valid_price(upper_price)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "instrument_table::%s, instrument[%s], down_band[%.2f] or upper_band[%.2f] is NULL\n",
            __FUNCTION__, instrument, down_price, upper_price);
        return;
    }
    record *r = get_record(instrument, false);
    //record *r = get_record(instrument, true);
    if(r == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "instrument_table::%s, invalid instrument[%s]\n",
            __FUNCTION__, instrument);
        return;
    }
    r->set_band_price(down_price, upper_price);
}

void instrument_table::clear_recancel_times(const char* instrument)
{
    record *r = get_record(instrument);
    r->recancel_times_ = 0;
}

bool instrument_table::check_recancel_times(const char* instrument)
{
    record *r = get_record(instrument);
    return r->check_recancel_times();
}

void instrument_table::record_recancel_times(const char* instrument)
{
    record *r = get_record(instrument);
    r->recancel_times_++;
}


}
}

