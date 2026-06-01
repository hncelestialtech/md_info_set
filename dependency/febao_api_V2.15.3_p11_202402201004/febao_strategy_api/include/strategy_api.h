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
 * Date: 2018-08-17
 */


#ifndef FB_STRATEGY_API_H
#define FB_STRATEGY_API_H

#include "i_strategy.h"


#include "i_quote_caller.h"
#include "i_order_caller.h"
#include "i_timer_caller.h"
#include "i_trade_caller.h"
#include "i_instrument_param_caller.h"
#include "i_instrument_group_param_caller.h"
#include "i_log_caller.h"
#include "i_custom_param_caller.h"
#include "i_volatility_config_caller.h"
#include "i_volatility_param_caller.h"
#include "i_volatility_offset_caller.h"
#include "i_instance_operator_caller.h"
#include "i_instrument_pricing_param_caller.h"
#include "i_serial_pricing_param_caller.h"
#include "i_comb_stock_caller.h"
#include "i_comb_commodity_caller.h"
#include "i_instrument_theo_pricing_param_caller.h"
#include "i_serial_baseprice_config_caller.h"
#include "i_serial_theo_custom_param_caller.h"
#include "i_trialer_theoretical_price.h"
#include "i_trialer_iv.h"
#include "i_trialer_volatility_fit.h"
#include "i_trialer_volatility_calc.h"
#include "i_trade_caller.h"

// stream 30 total
#include "atm_forward_md_stream.h"
#include "comb_commodity_stream.h"
#include "comb_stock_stream.h"
#include "custom_param_stream.h"
#include "derived_md_stream.h"
#include "fund_component_stream.h"
#include "fund_instrument_info_stream.h"
#include "inquiry_quote_stream.h"

#include "instrument_param_stream.h"
#include "md_stream.h"
#include "strategy_instance_param_stream.h"
#include "volatility_param_stream.h"
#include "instrument_stream.h"
#include "instrument_trading_status_stream.h"
#include "order_stream.h"
#include "quote_stream.h"
#include "trade_stream.h"
#include "volatility_config_stream.h"
#include "volatility_offset_stream.h"
#include "volatility_fitting_param_config_stream.h"
#include "trading_day_stream.h"
#include "spread_template_stream.h"
#include "trading_time_template_stream.h"
#include "position_stream.h"
#include "security_position_stream.h"
#include "fund_position_stream.h"
#include "portfolio_stream.h"
#include "virtual_portfolio_config_stream.h"
#include "portfolio_position_stream.h"
#include "portfolio_position_risk_stream.h"
#include "portfolio_position_profit_stream.h"
#include "investor_account_fund_stream.h"
#include "instrument_pricing_param_config_stream.h"
#include "serial_pricing_param_config_stream.h"
#include "instrument_pricing_config_stream.h"
#include "portfolio_risk_stream.h"
#include "instrument_group_param_stream.h"
#include "instrument_group_config_stream.h"

#include "fund_instrument_info_stream.h"
#include "fund_component_stream.h"
#include "comb_stock_stream.h"
#include "comb_commodity_stream.h"
#include "instrument_theo_pricing_param_config_stream.h"
#include "serial_theo_custom_param_stream.h"



#include "instrument_filter.h"
#include "option_serial_filter.h"
#include "portfolio_id_filter.h"
#include "portfolio_name_filter.h"
#include "product_filter.h"
#include "custom_filter.h"
#include "strategy_instance_filter.h"
#include "instrument_group_filter.h"

#include "margin_fee_stream.h"


#include "enum_type.h"
#include "strategy_errorid.h"
#include "i_math_helper.h"
#include "strategy_version.h"

#endif
