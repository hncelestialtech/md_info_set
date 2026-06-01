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
 * Date: 2019-07-13
 */

#ifndef STRATEGY_FRAME_TRIALER_HANDLER_H
#define STRATEGY_FRAME_TRIALER_HANDLER_H

#include "strategy_api.h"

namespace cffex {
namespace strategy {

class trialer_handler
{
 public:
    trialer_handler(cffex::fb::i_strategy *obj) : obj_(obj) { }
    ~trialer_handler() { }

    cffex::fb::i_trialer_iv   *get_iv_trialer()
    {
        return get_trialer<cffex::fb::i_trialer_iv>();
    }
    cffex::fb::i_trialer_theoretical_price   *get_theoretical_price_trialer()
    {
        return get_trialer<cffex::fb::i_trialer_theoretical_price>();
    }
    cffex::fb::i_trialer_volatility_calc   *get_volatility_calc_trialer()
    {
        return get_trialer<cffex::fb::i_trialer_volatility_calc>();
    }
    cffex::fb::i_trialer_volatility_fit   *get_volatility_fit_trialer()
    {
        return get_trialer<cffex::fb::i_trialer_volatility_fit>();
    }


 private:
    template <typename trialer_type>
    trialer_type *get_trialer()
    {
        return (trialer_type *)obj_->get_trialer(trialer_type::ID);
    }

 private:
    cffex::fb::i_strategy *obj_;
};

}
}

#endif
