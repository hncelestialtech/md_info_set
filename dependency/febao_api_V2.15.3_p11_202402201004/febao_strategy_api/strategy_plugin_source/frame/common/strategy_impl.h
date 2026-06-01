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

#ifndef STRATEGY_FRAME_STRATEGY_IMPL_H
#define STRATEGY_FRAME_STRATEGY_IMPL_H

#include "strategy_api.h"
#include "strategy_executor.h"

namespace cffex {
namespace strategy {

class strategy_impl : public cffex::fb::i_strategy
{
public:
    strategy_impl();
    virtual ~strategy_impl() { }
    virtual void on_created(int strategy_instance_id);
    virtual void on_deleted();
    virtual bool on_started();
    virtual bool on_paused();

private:
    strategy_executor         *executor_;

};

}
}

#endif
