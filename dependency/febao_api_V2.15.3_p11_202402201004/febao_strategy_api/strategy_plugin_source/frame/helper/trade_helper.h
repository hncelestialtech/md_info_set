#ifndef TRADE_HELPER_H
#define TRADE_HELPER_H

#include "strategy_api.h"

namespace cffex {
namespace strategy {

class trade_helper
{
public:
    static bool transfer_trade(cffex::fb::i_trade_caller *caller, int64_t trade_id, const char * dst_portfolio_name) {
        if (caller == NULL) {
            return false;
        }
        cffex::fb::i_trade_caller::transfer_trade_entity *en = caller->create_transfer_entity();
        en->set_dst_portfolio_name(dst_portfolio_name);
        en->set_trade_id(trade_id);
        return caller->transfer_trade(en) == -1 ? false : true;
    }

};

}
}

#endif
