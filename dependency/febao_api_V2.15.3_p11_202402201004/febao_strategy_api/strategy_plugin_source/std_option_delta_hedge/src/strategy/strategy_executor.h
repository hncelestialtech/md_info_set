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
#include "delta_hedge_rule.h"

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
    bool set_filter();

    void on_snap_portfolio_risk(portfolio_risk_msg_type *msg, bool is_last);
    void on_msg_portfolio_risk(portfolio_risk_msg_type *msg);

	void on_timer();

 private:
    int                    instance_id_;
    data_manager           *data_;
    caller_handler         *callers_;
    delta_hedge_rule       *delta_hedge_rule_;

    const cffex::fb::i_filter    *portfolio_filter_;
    const cffex::fb::i_filter    *instance_filter_;

    int timer_id_;
};

}
}

#endif
