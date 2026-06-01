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
 * Date: 2018-10-15
 */

#ifndef FB_I_STRATEGY_FILTER_H
#define FB_I_STRATEGY_FILTER_H

#include <stddef.h>

#include <cstdint>

namespace cffex {
namespace fb {

class i_filter {
public:
    class i_filter_msg {
    public:
        virtual const uint64_t get_instrument_index() const {
            return 0;
        }
        virtual const uint64_t get_option_serial_index() const {
            return 0;
        }
        virtual const uint64_t get_product_index() const {
            return 0;
        }
        virtual const char *get_instrument_id() const {
            return NULL;
        }
        virtual const char *get_option_serial_id() const {
            return NULL;
        }
        virtual const char *get_product_id() const {
            return NULL;
        }
        virtual const int get_strategy_instance_id() const {
            return -1;
        }
        virtual const int get_portfolio_id() const {
            return -1;
        }
        virtual const char *get_portfolio_name() const {
            return NULL;
        }
        virtual const char *get_custom_id() const {
            return NULL;
        }
        virtual const char *get_instrument_group_id() const {
            return NULL;
        }
        virtual const char *get_param_key() const {
            return NULL;
        }
        virtual const int get_trading_account_id() const {
            return -1;
        }
    };

public:
    virtual ~i_filter() {}
    virtual bool            check(const i_filter::i_filter_msg *msg) const = 0;
    virtual const i_filter *clone() const                                  = 0;
};

const i_filter &operator&(const i_filter &f1, const i_filter &f2);
const i_filter &operator|(const i_filter &f1, const i_filter &f2);
const i_filter &operator!(const i_filter &f1);

class filter_factory {
public:
    filter_factory();
    ~filter_factory();

    virtual void destory(const i_filter *f);
};

}  // namespace fb
}  // namespace cffex

#endif
