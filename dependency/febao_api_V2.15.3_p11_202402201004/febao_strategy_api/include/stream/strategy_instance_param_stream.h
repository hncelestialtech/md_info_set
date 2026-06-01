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
 * Date: 2018-09-15
 */

#ifndef FB_STRATEGY_INSTANCE_PARAM_STREAM_H
#define FB_STRATEGY_INSTANCE_PARAM_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class strategy_instance_param_stream :
    public i_stream_with_id<strategy_instance_param_stream,
                            STRATEGY_STREAM_STRATEGY_INSTANCE_PARAM> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual const char *get_param_name() const  = 0;
        virtual const char *get_param_value() const = 0;
        virtual void        dump() const            = 0;
        virtual std::string to_string() const       = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual bool get_param(IN const char *param_name,
                               OUT int       *param_value) const      = 0;
        virtual bool get_param(IN const char *param_name,
                               OUT double    *param_value) const   = 0;
        virtual bool get_param(IN const char *param_name,
                               OUT char       param_value[512]) const = 0;

        virtual const i_stream_msg *first(const i_filter *f = NULL) = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL)  = 0;
    };

public:
    strategy_instance_param_stream();
    ~strategy_instance_param_stream();

    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }
    i_stream_table *get_stream_table() {
        return table_;
    }

private:
    i_stream_table *table_;
};

}  // namespace fb
}  // namespace cffex

#endif
