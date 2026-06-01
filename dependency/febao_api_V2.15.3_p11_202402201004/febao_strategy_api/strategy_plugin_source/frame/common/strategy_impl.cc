/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: gp
 * Date: 2019-10-18
 */

#include "strategy_impl.h"
#include <functional>

namespace cffex {
namespace strategy {

strategy_impl::strategy_impl() : executor_(NULL) { }

void strategy_impl::on_created(int strategy_instance_id)
{
    // SLOG(PLOG_INFO, "strategy_impl::%s, strategy_instance_id[%llu]trading_account_id[%d]\n", __FUNCTION__, strategy_instance_id_, trading_account_id_);
    executor_ = new strategy_executor(strategy_instance_id, this);
}

void strategy_impl::on_deleted()
{
    // SLOG(PLOG_INFO, "strategy_impl::%s, strategy_instance_id[%llu]\n", __FUNCTION__, strategy_instance_id_);
    if(executor_) {
        delete executor_;
        executor_ = NULL;
    }
}

bool strategy_impl::on_started()
{
    // SLOG(PLOG_INFO, "strategy_impl::%s, strategy_instance_id[%llu]\n", __FUNCTION__, strategy_instance_id_);
    return executor_->start();
}

bool strategy_impl::on_paused()
{
    // SLOG(PLOG_INFO, "strategy_impl::%s, strategy_instance_id[%llu]\n", __FUNCTION__, strategy_instance_id_);
    return executor_->stop();
}


}
}


