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
 * Date: 2019-07-11
 */

#ifndef STRATEGY_FRAME_STREAM_FACTORY_H
#define STRATEGY_FRAME_STREAM_FACTORY_H

#include "strategy_api.h"
#include <vector>
#include <string.h>

namespace cffex {
namespace strategy {

typedef const cffex::fb::instrument_stream::i_stream_msg                           instrument_msg_type;
typedef const cffex::fb::instrument_param_stream::i_stream_msg                     instrument_param_msg_type;
typedef const cffex::fb::custom_param_stream::i_stream_msg                         custom_param_msg_type;
typedef const cffex::fb::strategy_instance_param_stream::i_stream_msg              strategy_instance_param_msg_type;
typedef const cffex::fb::md_stream::i_stream_msg                                   md_msg_type;
typedef const cffex::fb::derived_md_stream::i_stream_msg                           derived_md_msg_type;
typedef const cffex::fb::position_stream::i_stream_msg                             position_msg_type;
typedef const cffex::fb::order_stream::i_stream_msg                                order_msg_type;
typedef const cffex::fb::order_stream::i_stream_notify_cancel_msg                  order_cancel_msg_type;
typedef const cffex::fb::quote_stream::i_stream_msg                                quote_msg_type;
typedef const cffex::fb::quote_stream::i_stream_notify_cancel_msg                  quote_cancel_msg_type;
typedef const cffex::fb::trade_stream::i_stream_msg                                trade_msg_type;
typedef const cffex::fb::portfolio_position_risk_stream::i_stream_msg              portfolio_position_risk_msg_type;
typedef const cffex::fb::portfolio_risk_stream::i_stream_msg                       portfolio_risk_msg_type;
typedef const cffex::fb::portfolio_position_profit_stream::i_stream_msg            portfolio_position_profit_msg_type;
typedef const cffex::fb::portfolio_position_stream::i_stream_msg                   portfolio_position_msg_type;
typedef const cffex::fb::portfolio_stream::i_stream_msg                            portfolio_msg_type;
typedef const cffex::fb::spread_template_stream::i_stream_msg                      spread_template_msg_type;
typedef const cffex::fb::inquiry_quote_stream::i_stream_msg                        inquiry_quote_msg_type;
typedef const cffex::fb::trading_time_template_stream::i_stream_msg                trading_time_msg_type;
typedef const cffex::fb::instrument_trading_status_stream::i_stream_msg            instrument_trading_status_msg_type;
typedef const cffex::fb::serial_pricing_param_config_stream::i_stream_msg          serial_pricing_param_config_msg_type;
typedef const cffex::fb::volatility_config_stream::i_stream_msg                    volatility_config_msg_type;
typedef const cffex::fb::volatility_offset_stream::i_stream_msg                    volatility_offset_msg_type;
typedef const cffex::fb::volatility_param_stream::i_stream_msg                     volatility_param_msg_type;
//typedef const cffex::fb::trading_day_stream::i_stream_msg                          trading_day_msg_type;
typedef const cffex::fb::investor_account_fund_stream::i_stream_msg                investor_account_fund_msg_type;
typedef const cffex::fb::instrument_pricing_config_stream::i_stream_msg            instrument_pricing_config_msg_type;
typedef const cffex::fb::instrument_pricing_param_config_stream::i_stream_msg      instrument_pricing_param_config_msg_type;
typedef const cffex::fb::instrument_group_param_stream::i_stream_msg               instrument_group_param_msg_type;
typedef const cffex::fb::instrument_group_config_stream::i_stream_msg              instrument_group_config_msg_type;
typedef const cffex::fb::fund_instrument_info_stream::i_stream_msg                 fund_instrument_info_msg_type;
typedef const cffex::fb::fund_component_stream::i_stream_msg                       fund_component_msg_type;

class stream_factory
{
public:
    stream_factory(cffex::fb::i_strategy *obj) : obj_(obj)
    {
        memset(streams_, 0 , sizeof(streams_));
    }
    virtual ~stream_factory()
    {
        for (int i = 0; i < cffex::fb::STRATEGY_STREAM_ALL; ++i) {
            if (streams_[i] != NULL) {
                delete streams_[i];
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////

    template <typename stream_type>
    void register_stream() {
        obj_->register_to_engine(get_stream<stream_type>());
    }

    template <typename stream_type>
    void unregister_stream() {
        obj_->unregister_to_engine(get_stream<stream_type>());
        delete streams_[stream_type::ID];
        streams_[stream_type::ID] = NULL;
    }

    template <typename stream_type>
    stream_type *get_stream()
    {
        if (streams_[stream_type::ID] == NULL) {
            // SLOG(PLOG_INFO, "stream_factory::%s, create steam[%02X]\n", __FUNCTION__, stream_type::ID);
            return create_stream<stream_type>();
        }
        return ((stream_type *)streams_[stream_type::ID]);
    }

    template <typename stream_type>
    bool get_stream(stream_type **s)
    {
        if (streams_[stream_type::ID] == NULL) {
            // SLOG(PLOG_INFO, "stream_factory::%s, create steam[%02X]\n", __FUNCTION__, stream_type::ID);
            *s = create_stream<stream_type>();
            return true;
        }
        *s = ((stream_type *)streams_[stream_type::ID]);
        return true;
    }

    int get_portfolio_id(const char* portfolio_name) {
        const cffex::fb::portfolio_stream::i_stream_msg* msg = get_stream<cffex::fb::portfolio_stream>()->get_stream_table()->get_msg(portfolio_name);
        if(msg == NULL)
            return 0;
        else
            return msg->get_portfolio_id();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename out_type>
    bool get_instance_param(const char *k, out_type param)
    {
        if (!get_stream<cffex::fb::strategy_instance_param_stream>()->get_stream_table()->get_param(k, param)) {
            // SLOG(PLOG_WARNING, "stream_factory::%s, no strategy instance param[%s]\n", __FUNCTION__, k);
            return false;
        }
        return true;
    }

    template<typename out_type>
    bool get_instrument_param(const char *instrument_id, const char *param_name, out_type param)
    {
        if (!get_stream<cffex::fb::instrument_param_stream>()->get_stream_table()->get_param(instrument_id, param_name, param)) {
            // SLOG(PLOG_WARNING, "stream_factory::%s, instrument_id[%s] no instrument param[%s]\n", __FUNCTION__, instrument_id, param_name);
            return false;
        }
        return true;
    }

    template<typename out_type>
    bool get_custom_param(const char *custom_id, const char *param_name, out_type param)
    {
        if (!get_stream<cffex::fb::custom_param_stream>()->get_stream_table()->get_param(custom_id, param_name, param)) {
            // SLOG(PLOG_WARNING, "stream_factory::%s, no param_name[%s]-custom_id[%s]-trading_account_id[%d]\n", __FUNCTION__, param_name,custom_id);
            return false;
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename stream_type>
    const typename stream_type::i_stream_msg *get_msg_by_instrument(const char *instrument_id)
    {
        if (streams_[stream_type::ID] == NULL) {
            // SLOG(PLOG_ERROR, "stream_factory::%s, no steam[%02X]\n", __FUNCTION__, stream_type::ID);
            return NULL;
        }
        return ((stream_type *)streams_[stream_type::ID])->get_stream_table()->get_msg(instrument_id);
    }

    template <typename stream_type>
    const typename stream_type::i_stream_msg *get_msg_by_instrument(uint64_t instrument_index)
    {
        if (streams_[stream_type::ID] == NULL) {
            // SLOG(PLOG_ERROR, "stream_factory::%s, no steam[%02X]\n", __FUNCTION__, stream_type::ID);
            return NULL;
        }
        return ((stream_type *)streams_[stream_type::ID])->get_stream_table()->get_msg(instrument_index);
    }

    template <typename stream_type>
    const typename stream_type::i_stream_msg *get_msg_by_id(int64_t id)
    {
        if (streams_[stream_type::ID] == NULL) {
            // SLOG(PLOG_ERROR, "stream_factory::%s, no steam[%02X]\n", __FUNCTION__, stream_type::ID);
            return NULL;
        }
        return ((stream_type *)streams_[stream_type::ID])->get_stream_table()->get_msg(id);
    }

private:
    template <typename stream_type>
    stream_type *create_stream()
    {
        stream_type *stream = new stream_type();
        streams_[stream_type::ID] = stream;
        return stream;
    }

private:
    cffex::fb::i_stream *streams_[cffex::fb::STRATEGY_STREAM_ALL];
    cffex::fb::i_strategy *obj_;
};

}
}

#endif
