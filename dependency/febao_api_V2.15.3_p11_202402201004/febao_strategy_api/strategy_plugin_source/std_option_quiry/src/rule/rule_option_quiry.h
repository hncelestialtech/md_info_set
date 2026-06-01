/**
 * CFFEX Confidential.
 *
 * @Copyright 2019 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty
 * Date: 2019-07-26
 */

#ifndef FEBAO_STRATEGY_RULE_QUOTE_QUIRY_H
#define FEBAO_STRATEGY_RULE_QUOTE_QUIRY_H

#include <map>
#include "strategy_api.h"
#include "data_manager.h"
#include "base_rule.h"

#define RECANCEL_THRESHOLD 3

namespace cffex {
namespace strategy {

struct instrument_record {
    char                    instrument_id[32];
    unsigned int            recancel_times_;
    char                    quiry_id_[32];

    instrument_record(const char *instrument_id) {
        strcpy(this->instrument_id, instrument_id);
        recancel_times_ = 0;
        quiry_id_[0] = 0;
    }

    const char *get_instrument_id() const {
        return instrument_id;
    }
    void clear_recancel_times() {
        recancel_times_ = 0;
    }
    bool check_recancel_times() {
        if (recancel_times_ >= RECANCEL_THRESHOLD) {
            recancel_times_ = 0;
            return false;
        }
        return true;
    }
    void record_recancel_times() {
        recancel_times_ ++;
    }
    bool update_quiry_id(const char* quiry_id) {
        if (0 == strcmp(quiry_id, quiry_id_)) {
            return false;
        }
        strcpy(quiry_id_, quiry_id);
        return true;
    }

};
class rule_option_quiry {
public:
    rule_option_quiry(caller_handler *caller, data_manager *data): data_(data), caller_(caller) { }
    //rule_option_quiry() {}
    ~rule_option_quiry() {}

    //void set(data_manager *data) { data_ = data; }
    //void set(caller_handler *caller) { caller_ = caller; }

    data_manager *get_data_manager() { return data_; }
    caller_handler *get_caller() { return caller_; }
    //instrument_table  *get_instrument_table() { return data_->get_instrument_table(); }

    void reset();
    /* insert quote to exchange */
    void insert_quote(const char *instrument_id, const char *quiry_id);

    /* cancel single instrument quote to exchange */
    void cancel_quote(const char *instrument_id);
    /* cancel single instrument quote to exchange */
    void cancel_quote(const char *instrument_id, int64_t quote_id);
    /* cancel instance quote to exchange */
    void cancel_quote();

    void clear_recancel_times(const char* instrument);
    bool check_recancel_times(const char* instrument);
    void record_recancel_times(const char* instrument);

    bool update_quiry_id(const char* instrument, const char* quiry_id);
    void delete_finish_quotes(int64_t quote_id) {   unfinish_quotes_.erase(quote_id); }

 private:
    bool check_trading_section();

    bool create_quote(const char *instrument_id);

    bool check_market(const char *instrument_id);

    bool check_risk(const char *instrument_id);

    void send_quote(const char *instrument_id, const char *req_quote_id );

 private:
    data_manager     *data_;
    caller_handler   *caller_;

    double quote_bid_;
    double quote_ask_;
    int    quote_volume_;
    double market_bid_;
    double market_ask_;
    int netpos_limit_;
    int portfolio_;

private:
    instrument_record *get_record(const char *instrument);

    struct string_less_functor {
        bool operator ()(const char *s1, const char *s2) const {
            return 0 < strcmp(s1, s2);
        }
    };
    typedef std::map<const char *, instrument_record *, string_less_functor>  record_mapping;
    record_mapping  records_;
    std::set<int64_t>   unfinish_quotes_;
};

}
}

#endif
