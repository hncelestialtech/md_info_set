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
 * Date: 2018-03-20
 */


#include "fb_simple_baseprice_algorithm.h"
#include "fb_plugin_math_helper.h"
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <cstdio>
//#include <cmath>
#ifdef __cplusplus
extern "C"
{
#endif

void* create() {
    return new cffex::fb::plugin::fb_simple_baseprice_algorithm();
}
void destroy(void* p) {
    delete (cffex::fb::plugin::fb_simple_baseprice_algorithm *)p;
}
void identity(void* handle,register_func f) {
    f(handle,0x0001,"last_baseprice_algorithm");
    f(handle,0x0002,"mid_baseprice_algorithm");
    f(handle,0x0003,"bid1_baseprice_algorithm");
    f(handle,0x0004,"ask1_baseprice_algorithm");
    f(handle,0x0005,"vwap_algorithm");
    f(handle,0x0006,"expire_algorithm");
    f(handle,0x0007,"mid_first_baseprice_algorithm");
    f(handle,0x0008,"arbi_ask_baseprice_algorithm");
}

#ifdef __cplusplus
}
#endif



#ifndef DEBUG
#  define DEBUG
#endif


static void PLOG(const char *fmt,...)
{
#ifdef DEBUG
    va_list va;
    va_start( va, fmt );
    vfprintf(stderr,fmt,va);
    va_end( va );
#else
    NO_LOG
#endif
}


namespace cffex {
namespace fb {
namespace plugin {

static bool valid_price(double price) {
    return (price > 0 && price < DBL_MAX) ;
}

fb_simple_baseprice_algorithm::fb_simple_baseprice_algorithm() {

}
fb_simple_baseprice_algorithm::~fb_simple_baseprice_algorithm() {

}
int fb_simple_baseprice_algorithm::calculate_inner(unsigned short algo_id, IN const instrument_md_field_container *mds, double *baseprice)
{
    if (mds->size()==0) {
        PLOG("fb_simple_baseprice_algorithm::calculate, empty future mds\n");
        return -1;
    }

    const  instrument_md_field *instrument_md = mds->first();
    if(instrument_md == NULL) {
        PLOG("fb_simple_baseprice_algorithm::%s, instrument_md NULL \n", __FUNCTION__);
        return -1;
    }
    // PLOG("fb_simple_baseprice_algorithm::%s, update_md_ask1[%lf] md2[%lf]\n", __FUNCTION__, mds->get_update_md_field()->get_ask1_price(), mds->get_update_md_field()->get_bid1_price());

    // instrument_field *instrument = instrument_md->get_instrument_field();
    const md_field         *md         = instrument_md->get_md_field();
    double price = 0.0;

    switch(algo_id) {
        case 0x0001 : {
            price = md->get_last_price();
            break;
        }
        case 0x0002 :{
            double  bid1_price   = md->get_bid1_price();
            double  ask1_price   = md->get_ask1_price();
            if(valid_price(bid1_price) && valid_price(ask1_price))
            {
                price = (bid1_price + ask1_price)/2;
            }else {
                PLOG("fb_simple_baseprice_algorithm::calculate, invalid baseprice vwap\n");
                *baseprice = 0.0;
                return -1;
            }
            break;
        }
        case 0x0003 :{
            price = md->get_bid1_price();
            break;
        }
        case 0x0004 :{
            price = md->get_ask1_price();
            break;
        }
        case 0x0005: {
            double  bid1_price   = md->get_bid1_price();
            double  ask1_price   = md->get_ask1_price();
            int32_t bid1_volume = md->get_bid1_volume();
            int32_t ask1_volume = md->get_ask1_volume();
            if( (math_helper::valid_price(bid1_price)) && (math_helper::valid_price(ask1_price)) && (bid1_volume+ask1_volume>0)  ) {
                price = (bid1_price*ask1_volume + ask1_price*bid1_volume)/ (bid1_volume+ask1_volume);
            } else {
                PLOG("fb_simple_baseprice_algorithm::calculate, invalid baseprice vwap\n");
                *baseprice = 0.0;
                return -1;
            }
            break;
        }
        case 0x0006: {
            const double expire_date = instrument_md->get_left_trading_time();
            price = md->get_last_price() + expire_date;
            break;
        }
        case 0x0007 :{
            double  bid1_price   = md->get_bid1_price();
            double  ask1_price   = md->get_ask1_price();
            double  last_price   = md->get_last_price();
            if((math_helper::valid_price(bid1_price)) && (math_helper::valid_price(ask1_price))){
                price = (bid1_price + ask1_price)/2;
            }else if(!(math_helper::valid_price(bid1_price)) && (math_helper::valid_price(ask1_price))){
                price = ask1_price;
            }else if((math_helper::valid_price(bid1_price)) && !(math_helper::valid_price(ask1_price))){
                price = bid1_price;
            }else if((math_helper::valid_price(last_price))){
                price = last_price;
			}else {
				PLOG("fb_simple_baseprice_algorithm::calculate, invalid baseprice mid_first\n");
				*baseprice = 0.0;
				return -1;
			}

            break;
        }
        case 0x0008 :{
            if (calculate_arbi(algo_id, mds, &price) != 0) {
                PLOG("fb_simple_baseprice_algorithm::calculate, invalid baseprice arbi_ask\n");
				*baseprice = 0.0;
				return -1;
            }
            break;
        }
        default:
            break;
    }
    if(false == math_helper::valid_price(price))
    {
        PLOG("fb_simple_baseprice_algorithm::calculate, invalid baseprice\n");
        *baseprice = 0.0;
        return -1;
    }

    *baseprice = price;

    //PLOG("fb_simple_baseprice_algorithm::calculate, price[%f] baseprice[%f] \n",price, *baseprice);

    return 0;
}

int fb_simple_baseprice_algorithm::calculate(unsigned short algo_id, IN instrument_field* target_instrument, IN const instrument_md_field_container *mds, OUT double *baseprice)
{
    PLOG("fb_simple_baseprice_algorithm::%s, target_ins[%s] \n", __FUNCTION__, target_instrument->get_instrument_id());
    // PLOG("fb_simple_baseprice_algorithm::%s, theo_mds.size[%d] ins[%s] \n", __FUNCTION__, theo_mds->size(),theo_mds->first()->get_instrument_field()->get_instrument_id());
    return calculate_inner(algo_id, mds, baseprice);
}
int fb_simple_baseprice_algorithm::calculate(unsigned short algo_id, IN serial_field* target_serial, IN const instrument_md_field_container *mds, OUT double *baseprice)
{
    PLOG("fb_simple_baseprice_algorithm::%s, target_ins[%s] \n", __FUNCTION__, target_serial->get_option_serial_id());
    // PLOG("fb_simple_baseprice_algorithm::%s, theo_mds.size[%d] ins[%s] \n", __FUNCTION__, theo_mds->size(),theo_mds->first()->get_instrument_field()->get_instrument_id());
    return calculate_inner(algo_id, mds, baseprice);
}
// for test fund baseprice
int fb_simple_baseprice_algorithm::calculate(unsigned short algo_id, IN instrument_field *target_fund_instrument, IN const instrument_md_field_container *instrument_mds,
                                             IN const component_md_field_container *component_mds, OUT double *baseprice)
{
    if(algo_id == 0x0001) {
        return calculate_fund_by_underlying(instrument_mds, baseprice);
    }
    else {
        return calculate_fund_by_component(component_mds, target_fund_instrument, baseprice);
    }
}

int fb_simple_baseprice_algorithm::calculate_fund_by_underlying(IN const instrument_md_field_container *instrument_mds, OUT double *baseprice)
{
    if(instrument_mds->size() == 0) {
        PLOG("fb_simple_baseprice_algorithm::%s, empty instrument_mds\n", __FUNCTION__);
        return -1;
    }
    double num = 0.0;
    const md_field *md = NULL;
    const instrument_field *instrument = NULL;
    *baseprice = 0.0;
    for(const instrument_md_field *f_instrument_md = instrument_mds->first(); f_instrument_md != NULL; f_instrument_md = instrument_mds->next()) {
        md = f_instrument_md->get_md_field();
        instrument = f_instrument_md->get_instrument_field();
        if(md == NULL || instrument == NULL) {
            PLOG("fb_simple_baseprice_algorithm::%s, field NULL\n", __FUNCTION__);
            continue;
        }
        if(!math_helper::valid_price(md->get_last_price()) || md->get_last_price() <= 0) {
            continue;
        }
        PLOG("fb_simple_baseprice_algorithm::%s, num[%lf] instrument_id[%s] last_price[%lf] creation_redemption_unit[%lf] baseprice[%lf] \n", __FUNCTION__,
            num, instrument->get_instrument_id(), md->get_last_price(), instrument->get_creation_redemption_unit(), *baseprice);
        *baseprice += md->get_last_price();
        num += 1;
    }
    *baseprice = *baseprice / num;
    //PLOG("fb_simple_baseprice_algorithm::%s, finish num[%lf] baseprice[%lf]\n", __FUNCTION__, num, *baseprice);
    return 0;
}

int fb_simple_baseprice_algorithm::calculate_fund_by_component(IN const component_md_field_container *component_mds, IN const instrument_field *fund_instrument, OUT double *baseprice)
{
    if(component_mds->size() == 0 || fund_instrument == NULL) {
        PLOG("fb_simple_baseprice_algorithm::%s, empty component_mds\n", __FUNCTION__);
        return -1;
    }
    const md_field *md = NULL;
    const instrument_field *instrument = NULL;
    const fund_component_field *component = NULL;
    *baseprice = 0.0;
    for(const component_md_field *f_component_md = component_mds->first(); f_component_md != NULL; f_component_md = component_mds->next()) {
        md = f_component_md->get_md_field();
        instrument = f_component_md->get_instrument_field();
        component = f_component_md->get_fund_component_field();
        if(md == NULL || instrument == NULL || component == NULL) {
            PLOG("fb_simple_baseprice_algorithm::%s, field NULL\n", __FUNCTION__);
            continue;
        }
        if(!math_helper::valid_price(md->get_last_price()) || md->get_last_price() <= 0) {
            continue;
        }
        PLOG("fb_simple_baseprice_algorithm::%s, instrument_id[%s] last_price[%lf] component_share[%ld]\n", __FUNCTION__, instrument->get_instrument_id(), md->get_last_price(), component->get_component_share());
        *baseprice += md->get_last_price() * component->get_component_share();
    }
    *baseprice = (*baseprice + fund_instrument->get_estimate_cash_component()) / fund_instrument->get_creation_redemption_unit();
    //PLOG("fb_simple_baseprice_algorithm::%s, finish instrument_id[%s] baseprice[%lf] estimate_cash_component[%lf] creation_redemption_unit[%lf]\n", __FUNCTION__,
    //    fund_instrument->get_instrument_id(), *baseprice, fund_instrument->get_estimate_cash_component(), fund_instrument->get_creation_redemption_unit());
    return 0;
}

int fb_simple_baseprice_algorithm::calculate_arbi(unsigned short algo_id, IN const instrument_md_field_container *mds, OUT double *baseprice) {
    if (mds->size() != 2) {
        PLOG("fb_simple_baseprice_algorithm::%s, mds size not 2\n", __FUNCTION__);
        return -1;
    }
    char instrument_id[64] = {0};
    char leg1_instrument_id[64] = {0};
    char leg2_instrument_id[64] = {0};

    double arbi_ask = 0.0;
    double ask = 0.0;
    double bid = 0.0;
    *baseprice = 0.0;
    for(const instrument_md_field *f_instrument_md = mds->first(); f_instrument_md != NULL; f_instrument_md = mds->next()) {
        if(f_instrument_md->get_md_field() == NULL || f_instrument_md->get_instrument_field() == NULL) {
            PLOG("fb_simple_baseprice_algorithm::%s, field NULL\n", __FUNCTION__);
            return -1;
        }
        if (f_instrument_md->get_instrument_field()->get_instrument_type() == FB_INSTRUMENT_ARBITRAGE) {
            arbi_ask = f_instrument_md->get_md_field()->get_ask1_price();
            snprintf(leg1_instrument_id, sizeof(leg1_instrument_id) - 1, f_instrument_md->get_instrument_field()->get_leg1_instrument_id());
            snprintf(leg2_instrument_id, sizeof(leg2_instrument_id) - 1, f_instrument_md->get_instrument_field()->get_leg2_instrument_id());
            continue;
        }
        ask = f_instrument_md->get_md_field()->get_ask1_price();
        bid = f_instrument_md->get_md_field()->get_bid1_price();
        snprintf(instrument_id, sizeof(instrument_id) - 1, f_instrument_md->get_instrument_field()->get_instrument_id());
    }
    if (!math_helper::valid_price(arbi_ask)) {
        PLOG("fb_simple_baseprice_algorithm::%s, arbi_md NULL\n", __FUNCTION__);
        return -1;
    }
    if (strcmp(leg1_instrument_id, instrument_id) == 0) {
        if (!math_helper::valid_price(ask)) return -1;
        *baseprice = ask - arbi_ask;
        return 0;
    }
    if (strcmp(leg2_instrument_id, instrument_id) == 0) {
        if (!math_helper::valid_price(bid)) return -1;
        *baseprice = bid + arbi_ask;
        return 0;
    }
    PLOG("fb_simple_baseprice_algorithm::%s, end error\n", __FUNCTION__);
    return -1;
}

}
}
}

