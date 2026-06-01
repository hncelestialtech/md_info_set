#ifndef FB_TRADING_DAY_STREAM_H
#define FB_TRADING_DAY_STREAM_H

#include <stdint.h>

#include "i_stream.h"

namespace cffex {
namespace fb {

class trading_day_stream :
    public i_stream_with_id<trading_day_stream, STRATEGY_STREAM_TRADING_DAY> {
public:
    class i_stream_table {
    public:
        virtual ~i_stream_table() {}
        virtual const char *get_trading_day() const = 0;
    };

public:
    trading_day_stream();

    ~trading_day_stream();

    i_stream_table *get_stream_table() {
        return table_;
    }
    void set_stream_table(i_stream_table *table) {
        table_ = table;
    }

    virtual void on_data(const void *data) {
        return;
    }

private:
    i_stream_table *table_;
};

}  // namespace fb
}  // namespace cffex

#endif