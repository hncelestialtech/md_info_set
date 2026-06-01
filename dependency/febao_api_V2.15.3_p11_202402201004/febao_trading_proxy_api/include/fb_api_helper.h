/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: fangyi
 * Date: 2019-02-25
 */
#ifndef FB_API_HELPER_H
#define FB_API_HELPER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

namespace cffex {
namespace fb {
class fb_local_id_manager {
public:
    static void    set_local_id(int64_t id);
    static int64_t get_last_local_id();
    static int64_t get_order_local_id();
    static int64_t get_quote_local_id();
    static int64_t get_comb_local_id();

    static void set_local_id_prefix(int64_t prefix);

    static void add_local_order(int64_t order_local_id, int64_t order_id);  // inside order
    static void add_local_quote(int64_t quote_local_id, int64_t quote_id);  // inside quote
    static void add_local_comb(int64_t comb_local_id, int64_t comb_id);     // inside comb
    static void add_local_instrument(int64_t local_id, const char *instrument_id);

    static int64_t     find_order_by_local(int64_t local_id);  // order_id by local_id or sys_id
    static int64_t     find_quote_by_local(int64_t local_id);  // quote_id by local_id or sys_id
    static int64_t     find_comb_by_local(int64_t local_id);   // comb_id by local_id
    static const char *find_instrument_by_local(int64_t local_id);  //thread not safe!
    static int64_t     find_local_by_order(int64_t order_id);
    static int64_t     find_local_by_quote(int64_t order_id);
    static int64_t     find_local_by_comb(int64_t comb_id);

    // exchange_id default NULL
    static int64_t find_order_by_sysid(const char *sys_id,
                                       const char *exchange_id);  // rsp_cancel, order_ref is null
    // exchange_id default NULL
    static int64_t find_quote_by_sysid(const char *sys_id,
                                       const char *exchange_id);  // rsp_cancel, quote_ref is null

    static int64_t preprocess_sysid(const char *sys_id);
    static int64_t generate_outside_id(const char *sys_id, char exchange_id);
    static int64_t generate_outside_id(const char *sys_id, const char *exchange_id);
    static int64_t generate_history_id(const char *sys_id, char exchange_id);
    static int64_t generate_history_id(const char *sys_id, const char *exchange_id);

    static void add_outside_order(int64_t order_id);
    static void add_outside_quote(int64_t quote_id);
    static void add_outside_comb(int64_t comb_id);

    static void set_trading_day(const char *trading_day);

    static void erase_order_id(int64_t order_id);
    static void erase_quote_id(int64_t quote_id);
    static void erase_comb_id(int64_t comb_id);

    static int64_t get_order_id(int64_t     order_ref,
                                const char *OrderSysID,
                                char        ExchangeID,
                                bool        recv_outside);
    static int64_t get_quote_id(int64_t     quote_ref,
                                const char *QuoteSysID,
                                char        ExchangeID,
                                bool        recv_outside);
};

class mm_id_manager {
public:
    static void    reset();
    static void    set_mark_id(int32_t id);
    static int32_t get_mark_id();
    static void    set_trade_id(int32_t id);
    static int32_t get_last_trade_id();

    static int64_t get_quote_id(const int16_t &node_id);
    static int64_t get_order_id(const int16_t &node_id);
    static int32_t get_trade_id(const int16_t &node_id);
    static int64_t get_comb_id(const int16_t &node_id);

    static bool is_quote(const int64_t &order_id);
    static bool is_order(const int64_t &order_id);
    static bool is_quote_bid(const int64_t &order_id);
    static bool is_quote_ask(const int64_t &order_id);
    static bool is_outside_trading(const int64_t &order_id);

    static int64_t conver_to_quote_id(const int64_t &order_id);

    static int64_t conver_to_bid_order_id(const int64_t &quote_id);

    static int64_t conver_to_ask_order_id(const int64_t &quote_id);

    static int64_t conver_to_other_side_order_id(const int64_t &order_id);
};

class mm_febao_id_helper {
public:
    static bool get_febao_id(const int8_t exchange_id, const char *counter_id, char *febao_id);
    static bool get_counter_id(const char *febao_id, char *counter_id);
};

class fb_converter_helper {
public:
    static int     init(const char *counter_name);
    static void    add_errorcode(int32_t counter_code, int32_t error_code);
    static void    add_counter_code_state(int32_t counter_code, const uint32_t &state);
    static int32_t get_errorcode(int32_t counter_code);
    static bool    is_counter_code_state(const int32_t &counter_code, char state);
};

class mm_tag_helper {
public:
    enum {
        // mm_trading_proxy
        TAG_TRADING_PROXY_INSERT_ORDER_SUB       = 0X70,
        TAG_TRADING_PROXY_INSERT_QUOTE_SUB       = 0X72,
        TAG_TRADING_PROXY_CANCEL_SUB             = 0X73,
        TAG_TRADING_PROXY_REQ_INSERT_ORDER_BEGIN = 0X79,
        TAG_TRADING_PROXY_REQ_INSERT_QUOTE_BEGIN = 0X7A,
        TAG_TRADING_PROXY_REQ_CANCEL_BEGIN       = 0X7B,
        TAG_TRADING_PROXY_RTN_ORDER_PUB          = 0X85,
        TAG_TRADING_PROXY_RTN_QUOTE_PUB          = 0X86,
        TAG_TRADING_PROXY_RTN_TRADE_PUB          = 0X87,

        // for roundtrip
        TAG_TRADING_PROXY_REQ_INSERT_ORDER_END = 0X74,
        TAG_TRADING_PROXY_REQ_INSERT_QUOTE_END = 0X75,
        TAG_TRADING_PROXY_REQ_CANCEL_ORDER_END = 0X76,
        TAG_TRADING_PROXY_REQ_CANCEL_QUOTE_END = 0X77,

        TAG_TRADING_PROXY_RTN_INSERT_ORDER_SUB = 0X89,
        TAG_TRADING_PROXY_RTN_INSERT_QUOTE_SUB = 0X8A,
        TAG_TRADING_PROXY_RTN_CANCEL_ORDER_SUB = 0X8B,
        TAG_TRADING_PROXY_RTN_CANCEL_QUOTE_SUB = 0X8C,
        TAG_TRADING_PROXY_RTN_TRADE_SUB        = 0X8D,
    };

    static void sig_start_tag(int n);
    static void sig_pause_tag(int n);

    static void CFFEX_TAG(uint16_t TAG_ID);
    static void CFFEX_TAG(uint16_t TAG_ID, uint64_t GUID);
    static void FEBAO_TAG_WITH_CONTEXT(const uint16_t &TAG_ID,
                                       const int64_t  &context,
                                       const int64_t  &node_id = 0);
};

class mm_guid_helper {
public:
    static void     set_module(uint8_t module_id);
    static uint64_t create_guid();
    static void     set_guid(uint64_t guid);
    static uint64_t get_guid();
};

class mm_helper {
public:
    static void get_febao_version();
};

class signal_helper {
public:
    typedef void (*cffex_signal_handler)(int);
    static void signal(int sig, cffex_signal_handler h);
};

}  // namespace fb
}  // namespace cffex

#endif
