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
 * Date: 2019-12-08
 */

#ifndef PRICE_HELPER_H
#define PRICE_HELPER_H

#include "math_helper.h"
#include "stream_factory.h"
#include <limits.h>

namespace cffex {
namespace strategy {

const double INVALID_PRICE = -1.0;

class price_helper
{
public:

    static int get_bid_volume_by_price(double bid, md_msg_type *md_msg)
    {
        double bid2 = math_helper::active_price(md_msg->get_bid2_price()) ? md_msg->get_bid2_price() : 0.0;
        double bid3 = math_helper::active_price(md_msg->get_bid3_price()) ? md_msg->get_bid3_price() : 0.0;
        double bid4 = math_helper::active_price(md_msg->get_bid4_price()) ? md_msg->get_bid4_price() : 0.0;
        double bid5 = math_helper::active_price(md_msg->get_bid5_price()) ? md_msg->get_bid5_price() : 0.0;
        if( math_helper::less_equal(bid, md_msg->get_bid1_price()) && math_helper::greater(bid, bid2) ) {
            return md_msg->get_bid1_volume();
        }
        if( math_helper::less_equal(bid, bid2) && math_helper::greater(bid, bid3) ) {
            return md_msg->get_bid2_volume();
        }
        if( math_helper::less_equal(bid, bid3) && math_helper::greater(bid, bid4) ) {
            return md_msg->get_bid3_volume();
        }
        if( math_helper::less_equal(bid, bid4) && math_helper::greater(bid, bid5) ) {
            return md_msg->get_bid4_volume();
        }
        if( math_helper::less_equal(bid, bid5) ) {
            return md_msg->get_bid5_volume();
        }
        return 0;
    }

    static int get_ask_volume_by_price(double ask, md_msg_type *md_msg)
    {
        double ask2 = math_helper::active_price(md_msg->get_ask2_price()) ? md_msg->get_ask2_price() : md_msg->get_ask1_price() * 100;
        double ask3 = math_helper::active_price(md_msg->get_ask3_price()) ? md_msg->get_ask3_price() : md_msg->get_ask1_price() * 100;
        double ask4 = math_helper::active_price(md_msg->get_ask4_price()) ? md_msg->get_ask4_price() : md_msg->get_ask1_price() * 100;
        double ask5 = math_helper::active_price(md_msg->get_ask5_price()) ? md_msg->get_ask5_price() : md_msg->get_ask1_price() * 100;
        if( math_helper::greater_equal(ask, md_msg->get_ask1_price()) && math_helper::less(ask, ask2) ) {
            return md_msg->get_ask1_volume();
        }
        if( math_helper::greater_equal(ask, ask2) && math_helper::less(ask, ask3) ) {
            return md_msg->get_ask2_volume();
        }
        if( math_helper::greater_equal(ask, ask3) && math_helper::less(ask, ask4) ) {
            return md_msg->get_ask3_volume();
        }
        if( math_helper::greater_equal(ask, ask4) && math_helper::less(ask, ask5) ) {
            return md_msg->get_ask4_volume();
        }
        if( math_helper::greater_equal(ask, ask5) ) {
            return md_msg->get_ask5_volume();
        }
        return 0;
    }

    static int get_bid_level_by_price(double bid, md_msg_type *md_msg)
    {
        double bid2 = math_helper::active_price(md_msg->get_bid2_price()) ? md_msg->get_bid2_price() : 0.0;
        double bid3 = math_helper::active_price(md_msg->get_bid3_price()) ? md_msg->get_bid3_price() : 0.0;
        double bid4 = math_helper::active_price(md_msg->get_bid4_price()) ? md_msg->get_bid4_price() : 0.0;
        double bid5 = math_helper::active_price(md_msg->get_bid5_price()) ? md_msg->get_bid5_price() : 0.0;
        if( math_helper::less_equal(bid, md_msg->get_bid1_price()) && math_helper::greater(bid, bid2) ) {
            return 1;
        }
        if( math_helper::less_equal(bid, bid2) && math_helper::greater(bid, bid3) ) {
            return 2;
        }
        if( math_helper::less_equal(bid, bid3) && math_helper::greater(bid, bid4) ) {
            return 3;
        }
        if( math_helper::less_equal(bid, bid4) && math_helper::greater(bid, bid5) ) {
            return 4;
        }
        if( math_helper::less_equal(bid, bid5) ) {
            return 5;
        }
        return 0;
    }

    static int get_ask_level_by_price(double ask, md_msg_type *md_msg)
    {
        double ask2 = math_helper::active_price(md_msg->get_ask2_price()) ? md_msg->get_ask2_price() : md_msg->get_ask1_price() * 100;
        double ask3 = math_helper::active_price(md_msg->get_ask3_price()) ? md_msg->get_ask3_price() : md_msg->get_ask1_price() * 100;
        double ask4 = math_helper::active_price(md_msg->get_ask4_price()) ? md_msg->get_ask4_price() : md_msg->get_ask1_price() * 100;
        double ask5 = math_helper::active_price(md_msg->get_ask5_price()) ? md_msg->get_ask5_price() : md_msg->get_ask1_price() * 100;
        if( math_helper::greater_equal(ask, md_msg->get_ask1_price()) && math_helper::less(ask, ask2) ) {
            return 1;
        }
        if( math_helper::greater_equal(ask, ask2) && math_helper::less(ask, ask3) ) {
            return 2;
        }
        if( math_helper::greater_equal(ask, ask3) && math_helper::less(ask, ask4) ) {
            return 3;
        }
        if( math_helper::greater_equal(ask, ask4) && math_helper::less(ask, ask5) ) {
            return 4;
        }
        if( math_helper::greater_equal(ask, ask5) ) {
            return 5;
        }
        return 0;
    }

    static double get_price(int level, md_msg_type *md_msg, int8_t direction)
    {
        if(level == 1) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid1_price() : md_msg->get_ask1_price();
        }
        if(level == 2) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid2_price() : md_msg->get_ask2_price();
        }
        if(level == 3) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid3_price() : md_msg->get_ask3_price();
        }
        if(level == 4) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid4_price() : md_msg->get_ask4_price();
        }
        if(level == 5) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid5_price() : md_msg->get_ask5_price();
        }
        return INVALID_PRICE;
    }

    static int get_volume_by_price(double price, md_msg_type *md_msg, int8_t direction)
    {
        return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? get_bid_volume_by_price(price, md_msg) : get_ask_volume_by_price(price, md_msg);
    }

    static int get_volume_by_level(int level, md_msg_type *md_msg, int8_t direction)
    {
        if(level == 1) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid1_volume() : md_msg->get_ask1_volume();
        }
        if(level == 2) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid2_volume() : md_msg->get_ask2_volume();
        }
        if(level == 3) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid3_volume() : md_msg->get_ask3_volume();
        }
        if(level == 4) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid4_volume() : md_msg->get_ask4_volume();
        }
        if(level == 5) {
            return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? md_msg->get_bid5_volume() : md_msg->get_ask5_volume();
        }
        return 0;
    }

    static int get_level_by_price(double price, md_msg_type *md_msg, int8_t direction)
    {
        return direction == cffex::fb::STRATEGY_DIRECTION_BUY ? get_bid_level_by_price(price, md_msg) : get_ask_level_by_price(price, md_msg);
    }

    static int get_total_volume_by_level(int level, md_msg_type *md_msg, int8_t direction)
    {
        int total_volume = 0;
        for(int i = 1; i <= level; ++i) {
            total_volume += get_volume_by_level(i, md_msg, direction);
        }
        return total_volume;
    }

    static int get_total_volume_by_price(double price, md_msg_type *md_msg, int8_t direction, int own_volume = 0)
    {
        return get_total_volume_by_level(get_level_by_price(price, md_msg, direction), md_msg, direction) - own_volume;
    }

    static void calculate_by_mid(md_msg_type *md_msg, int spread, double tick, double *bid, double *ask, int volume_diff = INT_MAX)
    {
        double bid1 = md_msg->get_bid1_price();
        double ask1 = md_msg->get_ask1_price();
        double mid = (bid1 + ask1) * 0.5;
        int market_spread = math_helper::double_to_int((ask1 - bid1) / tick);
        if( !math_helper::active_price(bid1) || !math_helper::active_price(ask1) ) {
            *bid = INVALID_PRICE;
            *ask = INVALID_PRICE;
            return;
        }
        if(spread % 2 == market_spread % 2) {
            *bid = mid - spread * tick * 0.5;
            *ask = mid + spread * tick * 0.5;
        }
        else if(volume_diff != INT_MAX && md_msg->get_ask1_volume() > md_msg->get_bid1_volume() + volume_diff) {
            *bid = mid - spread * tick * 0.5 - tick * 0.5;
            *ask = mid + spread * tick * 0.5 - tick * 0.5;
        }
        else {
            *bid = mid - spread * tick * 0.5 + tick * 0.5;
            *ask = mid + spread * tick * 0.5 + tick * 0.5;
        }
    }

};

}
}

#endif
