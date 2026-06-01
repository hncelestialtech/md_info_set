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

#ifndef FB_CUSTOM_PARAM_STREAM_H
#define FB_CUSTOM_PARAM_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class custom_param_stream :
    public i_stream_with_id<custom_param_stream, STRATEGY_STREAM_CUSTOM_PARAM> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}
        virtual const char  *get_custom_id() const           = 0;
        virtual const char  *get_param_key() const           = 0;
        virtual const char  *get_param_type() const          = 0;
        virtual const char  *get_param_value() const         = 0;
        virtual const int8_t get_last_operate_source() const = 0;
        virtual void         dump() const                    = 0;
        virtual std::string  to_string() const               = 0;
    };

    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual bool                get_param(IN const char *custom_id,
                                              IN const char *param_key,
                                              OUT int       *param_value) const = 0;
        virtual bool                get_param(IN const char *custom_id,
                                              IN const char *param_key,
                                              OUT double    *param_value) const = 0;
        virtual bool                get_param(IN const char *custom_id,
                                              IN const char *param_key,
                                              OUT char       param_value[512]) const = 0;
        virtual bool                get_param(IN const char *custom_id,
                                              IN const char *param_key,
                                              OUT bool      *param_value) const = 0;
        virtual const i_stream_msg *first(const i_filter *f = NULL) const  = 0;
        virtual const i_stream_msg *next(const i_filter *f = NULL) const   = 0;
    };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;
    typedef std::function<void(IN const i_stream_msg *msg,
                               IN const bool          is_last)>
        snap_callback;

public:
    custom_param_stream(msg_callback callback, snap_callback s_callback);
    custom_param_stream(msg_callback callback);
    custom_param_stream();
    ~custom_param_stream();

    i_stream_table *get_stream_table() {
        return table_;
    }

    /** will be called by strategy after registed*/
    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }
    virtual bool is_need_snap() {
        return snap_callback_ != NULL;
    }

    /** called by strategy */
    virtual void on_data(const void *data);
    virtual void on_snap(const void *data, const bool is_last);
    void         set_msg_callback(msg_callback callback);
    void         set_snap_callback(snap_callback s_callback);

private:
    msg_callback   *callback_;
    snap_callback  *snap_callback_;
    i_stream_table *table_;
};

}  // namespace fb
}  // namespace cffex

#endif
