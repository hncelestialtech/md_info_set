#include "base_rule.h"

namespace cffex {
namespace strategy {

bool base_rule::insert_order(double price, int volume, int8_t direction, const char *portfolio_name, int64_t &order_id, int8_t offset_flag) {
    bool precheck = math_helper::active_price(price);
    precheck &= (volume > 0);

    if (precheck) {
        cffex::fb::i_order_caller::order_entity *en = callers_->get_order_caller()->create_order_entity();
        en->set_portfolio_name(portfolio_name);
        en->set_instrument_id(data_->underlying_id);
        en->set_direction(direction);
        en->set_volume(volume);
        en->set_price(price);
        en->set_offset_flag(offset_flag);
        en->set_volume_condition(cffex::fb::STRATEGY_VOLUME_ANY);
        en->set_price_category(cffex::fb::STRATEGY_PRICE_CATEGORY_LIMIT);
        en->set_time_condition(cffex::fb::STRATEGY_TIME_CONDITION_IOC);
        en->set_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
        en->set_custom_flag("deltah");

        int ret = callers_->get_order_caller()->insert_order(en, order_id);
        if(ret < 0) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, failed, ret[%d], instrument_id[%s] price[%.2f], volume[%d], direction[%d], offset_flag[%d], portfolio[%s]\n",
                __FUNCTION__, ret, data_->underlying_id, price, volume, direction, offset_flag, portfolio_name);
            return false;
        }
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "base_rule::%s, order_id[%ld], instrument_id[%s] price[%.2f], volume[%d] direction[%d], offset_flag[%d], portfolio[%s]\n",
            __FUNCTION__, order_id, data_->underlying_id, price, volume, direction, offset_flag, portfolio_name);
        return true;
    }
    return false;
}

bool base_rule::check_trading_phase(trading_time_msg_type *msg) {
    if(msg == NULL) {
        msg = data_->get_stream<cffex::fb::trading_time_template_stream>()->get_stream_table()->get_msg(data_->trading_section);
        if(msg == NULL) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, failed to get trading_time_template_stream msg, [%s]\n", __FUNCTION__, data_->trading_section);
            return false;
        }
    }
    return msg->is_trading_time();
}

double base_rule::get_market_bid(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = data_->get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return 0.0;
    }
    return msg->get_bid1_price();
}

double base_rule::get_market_ask(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = data_->get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return 0.0;
    }
    return msg->get_ask1_price();
}

int base_rule::get_market_bid_volume(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = data_->get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "config_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    return msg->get_bid1_volume();
}

int base_rule::get_market_ask_volume(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = data_->get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "config_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    return msg->get_ask1_volume();
}

}
}