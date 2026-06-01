#ifndef ID_HELPER_H
#define ID_HELPER_H

#include "math_helper.h"
#include "strategy_api.h"

namespace cffex {
namespace strategy {

class id_helper {
public:
    static bool is_quote(const int64_t &order_id)
    {
        return (order_id & 0X3) != 0X3;  // 0001  0010
    }

    static bool is_order(const int64_t &order_id)
    {
        return (order_id & 0X3) == 0X3;  // 0001  0010
    }

    static int64_t  conver_to_quote_id(const int64_t &order_id) {
        return (order_id & 0XFFFFFFFFFFFFFFFC);
    }
    static int64_t conver_to_bid_order_id(const int64_t &quote_id) {
        return (quote_id | 0X1);
    }
    static int64_t conver_to_ask_order_id(const int64_t &quote_id) {
        return (quote_id | 0X2);
    }
};

}
}

#endif