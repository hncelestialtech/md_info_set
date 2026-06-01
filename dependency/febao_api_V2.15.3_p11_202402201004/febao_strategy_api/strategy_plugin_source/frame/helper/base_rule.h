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
 * Date: 2020-03-09
 */

#ifndef BASE_RULE_H
#define BASE_RULE_H

#include <sys/time.h>
#include <set>
#include <map>
#include <functional>
#include "data_manager.h"
#include "caller_handler.h"
#include "order_helper.h"
#include "quote_helper.h"
#include "struct_helper.h"
#include "id_helper.h"

namespace cffex {
namespace strategy {

#define STRATEGY_LOG(log_caller, level, fmt, ...)               \
    do {                                                        \
        if ((log_caller)->get_log_level() <= level) {           \
            (log_caller)->log(level, fmt, ## __VA_ARGS__);      \
        }                                                       \
    } while(0)

class base_rule
{
public:
    typedef std::map<int64_t, strategy_order_info> ORDER_MAP;
    typedef std::map<int64_t, strategy_quote_info> QUOTE_MAP;
    base_rule(caller_handler *callers, data_manager *data) : callers_(callers), data_(data), in_self_trade_(false) { }
    base_rule(caller_handler *callers, data_manager *data, int instance_id) : callers_(callers), data_(data), in_self_trade_(false), instance_id_(instance_id) { }
    virtual ~base_rule() { }

    void on_order(order_msg_type *msg);
    void on_quote(quote_msg_type *msg);
    void on_msg_order_cancel(order_cancel_msg_type *msg, int instance_id);
    bool on_msg_order_cancel(order_cancel_msg_type *msg, int instance_id, int cancel_time);
    void on_msg_quote_cancel(quote_cancel_msg_type *msg, int instance_id);
    bool on_msg_quote_cancel(quote_cancel_msg_type *msg, int instance_id, int cancel_time);
    bool exsit_order_in_book();
    ORDER_MAP* get_order_map(){return &order_map_;};
    QUOTE_MAP* get_quote_map(){return &quote_map_;};

    bool check_self_trade(double price, int8_t direction, const char* instrument = NULL);
    void check_self_trade_and_cancel(double price, int8_t direction, int64_t order_id, std::set<int64_t>* cancel_delay, const char* instrument = NULL);
    void cancel_instance_order(int instance_id = 0);    /* 默认实参是0，表示撤本策略实例的报单 */
    void cancel_instance_quote(int instance_id = 0);    /* 默认实参是0，表示撤本策略实例的报价 */
    void cancel_instrument_order(const char* instrument);
    void cancel_instrument_quote(const char* instrument);
public:
    static inline int64_t get_current_milli_seconds()
    {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return (int64_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
    }

    strategy_order_info *get_strategy_order_info(int64_t order_id) {
        ORDER_MAP::iterator itor = order_map_.find(order_id);
        return itor == order_map_.end() ? NULL : &(itor->second);
    }

    strategy_quote_info *get_strategy_quote_info(int64_t quote_id) {
        QUOTE_MAP::iterator itor = quote_map_.find(quote_id);
        return itor == quote_map_.end() ? NULL : &(itor->second);
    }
protected:
    caller_handler                  *callers_;
    data_manager                    *data_;

    bool                             in_self_trade_;

    ORDER_MAP                        order_map_;
    QUOTE_MAP                        quote_map_;
    std::map<int64_t, int>           cancel_map_;
    int                              instance_id_;

};

}
}

#endif