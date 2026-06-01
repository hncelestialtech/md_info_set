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
 * Date: 2018-08-17
 */

#ifndef FB_I_STRATEGY_H
#define FB_I_STRATEGY_H

#include <stdint.h>

#include "i_caller.h"
#include "i_stream.h"
#include "i_trialer.h"


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {

/** define the valid range of strategy identity */
enum { MIN_STRATEGY_INDENTITY = 0X10, MAX_STRATEGY_INDENTITY = 0XFF };

class i_strategy {
    typedef std::function<void(i_stream *stream)> register_stream_functor;
    typedef std::function<void(i_stream *stream)> unregister_stream_functor;

public:
    virtual ~i_strategy() {}

    /** called after set_caller; stream should be created in this function */
    virtual void on_created(int strategy_instance_id) {}
    virtual void on_deleted() {}
    virtual bool on_started() {
        return true;
    }
    virtual bool on_paused() {
        return true;
    }

    void set_caller(i_caller *cmd) {
        commands_[cmd->id()] = cmd;
    }
    void set_register_stream_functor(register_stream_functor func) {
        register_stream_functor_ = func;
    }
    void set_unregister_stream_functor(unregister_stream_functor func) {
        unregister_stream_functor_ = func;
    }
    i_caller *get_caller(uint16_t cmd_id) {
        return commands_[cmd_id];
    }

    void set_trialer(i_trialer *t) {
        trialers_[t->id()] = t;
    }
    i_trialer *get_trialer(uint16_t t_id) {
        return trialers_[t_id];
    }

public:
    /** must be called after on_created */
    void register_to_engine(i_stream *stream) {
        register_stream_functor_(stream);
    }
    void unregister_to_engine(i_stream *stream) {
        unregister_stream_functor_(stream);
    }

protected:
    register_stream_functor   register_stream_functor_;
    unregister_stream_functor unregister_stream_functor_;
    i_caller                 *commands_[STRATEGY_CALLER_ALL];
    i_trialer                *trialers_[STRATEGY_TRIALER_ALL];
};

}  // namespace fb
}  // namespace cffex

#endif
