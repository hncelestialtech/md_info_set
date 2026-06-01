/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2019-12-17
 */

#ifndef STD_OPTION_QUOTE_DATA_MANAGER_H
#define STD_OPTION_QUOTE_DATA_MANAGER_H

#include "stream_factory.h"
#include "strategy_enum_type.h"
#include "instrument_table.h"

namespace cffex {
namespace strategy {

class data_manager : public stream_factory
{
public:
    data_manager(cffex::fb::i_strategy *obj, caller_handler *callers);
    virtual ~data_manager();

    /* called by executor, prepare and check params */
    bool init_params();
    bool is_in_trading_section();

    /* from instrument_param */
    bool get_quote_switch(const char *instrument);
    double get_bid_offset(const char *instrument);
    double get_ask_offset(const char *instrument);

    /* from custom_param */
    int get_netpos_limit(const char *instrument);
    double get_update_threshold(const char *instrument);
    double get_quote_spread(const char *instrument, double price);

    /* from streams */
    double get_base_price(const char *instrument_id);

    double get_tick(const char *instrument);

    double get_market_ask(const char* instrument);
    double get_market_bid(const char* instrument);

    int get_current_netpos(const char* instrument);
    void get_self(const char *instrument, double *bid, double *ask);

private:
    bool init_instance_params();
    double get_theo_price(const char* instrument);

    double get_market_mid(const char* instrument);

    static inline void split(const std::string &line, char delim, std::vector<std::string > *ret)
    {
        std::string::size_type begin = 0, end = -1;
        while (std::string::npos != (end = line.find(delim, begin))) {
            ret->push_back(line.substr(begin, end - begin));
            begin = end + 1;
        }
        ret->push_back(line.substr(begin));
    }

    bool is_final(int8_t quote_status);

    caller_handler                      *callers_;

public:
    /* instance param */
    char                                portfolio[128];
    int                                 volume;
    char                                serial[128];
    char                                trading_section[128];
    int                                 base_mode;
    double                              trade_pending_second;
    int                                 replace_quote;
    instrument_table                   *instrument_table_;
};


}
}

#endif