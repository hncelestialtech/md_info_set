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
 * Date: 2019-11-18
 */

#ifndef FEBAO_STRATEGY_DATA_MANAGER_H
#define FEBAO_STRATEGY_DATA_MANAGER_H

#include "stream_factory.h"
#include "type_define.h"
#include "caller_handler.h"

namespace cffex {
namespace strategy {

class data_manager : public stream_factory {
 public:
    data_manager(cffex::fb::i_strategy *obj, caller_handler *callers);
    ~data_manager();
    /* called by executor, prepare and check params */
    bool init_params();
    bool is_in_trading_section();

    /* from strategy instance param */
    int get_portfolio();
    const char* get_portfolio_name();
    int get_volume();
    const char* get_product();
    const char* get_trading_section();
	double get_waiting_second();
    double get_duration_second();
    int get_netpos_limit();

    /* from custom_param */
    double get_quiry_spread(const char *instrument, double bid_price);

    /* from streams */
    double get_tick(const char *instrument);
    double get_market_bid(const char* instrument);
    double get_market_ask(const char* instrument);
    double get_market_mid(const char* instrument);
    int get_current_netpos(const char* instrument);

 private:

    caller_handler                      *callers_;

    char   portfolio_name_[256];
    int    portfolio_id_;
    int    volume_;
    char   product_[256];
    char   trading_section_[256];
    double waiting_second_;
    double duration_second_;
    int    netpos_limit_;

};

}
}

#endif
