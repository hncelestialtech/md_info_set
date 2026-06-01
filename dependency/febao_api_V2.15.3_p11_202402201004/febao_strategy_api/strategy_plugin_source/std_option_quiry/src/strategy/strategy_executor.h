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
 * Date: 2019-11-15
 */


#ifndef FEBAO_STRATEGY_EXECUTOR_H
#define FEBAO_STRATEGY_EXECUTOR_H

#include <string>
#include "strategy_api.h"
#include "rule_option_quiry.h"

namespace cffex {
namespace strategy {

class strategy_executor {

 public:
    strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy);
    ~strategy_executor();
    bool start();
    bool stop();

 private:
    void init_stream();
	bool show_instance_params();
	bool set_filters();

	cffex::fb::i_filter* create_product_filter();

	void on_msg_inquiry(inquiry_quote_msg_type *msg);
	void on_waiting_timer(std::string instrument, std::string quiry_id);
	void on_duration_timer(std::string instrument);
    void on_msg_trade(trade_msg_type *msg);


	void on_msg_quote(quote_msg_type *msg);
	void on_msg_quote_cancel(quote_cancel_msg_type *msg);
	void on_recancel_timer(std::string instrument, int64_t quote_id);

 private:
    const cffex::fb::i_filter    *product_filter_;
    const cffex::fb::i_filter    *instance_filter_;

    data_manager     *data_;
    caller_handler   *callers_;

    rule_option_quiry  *rule_;

    int              strategy_instance_id_;
    int              timer_id_;
};

}
}

#endif
