#ifndef FB_inquiry_quote_STREAM_H
#define FB_inquiry_quote_STREAM_H

#include <stdint.h>

#include "i_stream.h"


namespace cffex {
namespace fb {

class inquiry_quote_stream :
    public i_stream_with_id<inquiry_quote_stream,
                            STRATEGY_STREAM_INQUIRY_QUOTE> {
public:
    class i_stream_msg : public i_stream_base_msg {
    public:
        virtual ~i_stream_msg() {}

        virtual const char *get_inquiry_id() const       = 0;
        virtual uint64_t    get_instrument_index() const = 0;
        virtual const char *get_instrument_id() const    = 0;
        virtual int32_t     get_inquiry_time() const     = 0;

        virtual void        dump() const      = 0;
        virtual std::string to_string() const = 0;
    };

    // class i_stream_table {
    //  public:
    //     virtual ~i_stream_table() { }
    //     virtual const i_stream_msg *get_msg(IN int order_id)  const = 0;
    // };

    typedef std::function<void(IN const i_stream_msg *msg)> msg_callback;

public:
    inquiry_quote_stream(msg_callback callback);
    inquiry_quote_stream();
    ~inquiry_quote_stream();

    // i_stream_table *get_stream_table() { return table_; }
    // void set_stream_table(i_stream_table *table) { table_ = table; }
    // virtual bool is_need_snap() { return false; }

    /** called by strategy */
    virtual void on_data(const void *data);
    void         set_msg_callback(msg_callback callback);

private:
    msg_callback *callback_;
    // i_stream_table              *table_;
};

}  // namespace fb
}  // namespace cffex

#endif