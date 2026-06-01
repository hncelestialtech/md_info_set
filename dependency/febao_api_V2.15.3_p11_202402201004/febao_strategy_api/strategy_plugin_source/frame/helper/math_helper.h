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

#ifndef STRATEGY_FRAME_MATH_HELPER_H
#define STRATEGY_FRAME_MATH_HELPER_H

#include <math.h>
#include <float.h>

namespace cffex {
namespace strategy {

class math_helper
{
public:
    enum {
        ROUNDING_MODEL_CEIL                 =    '1'  ,    /* 舍入， 向上取整 */
        ROUNDING_MODEL_FLOOR                =    '2'  ,    /* 舍出， 向下取整 */
        ROUNDING_MODEL_ROUND                =    '3'       /* 四舍五入 */
    };

    static double round(double v, double precision, int mode = ROUNDING_MODEL_ROUND)
    {
        double ret = v;
        switch (mode)
        {
          case math_helper::ROUNDING_MODEL_CEIL:  ret  =  ::ceil( v / precision) * precision;   break;
          case math_helper::ROUNDING_MODEL_FLOOR: ret  =  ::floor(v / precision) * precision;   break;
          case math_helper::ROUNDING_MODEL_ROUND: ret  =  ::round(v / precision) * precision;   break;
          default:                                ret  =  ::round(v / precision) * precision;   break;
        }
        return ret;
    }

    static bool  is_null(double v)
    {
        static double MAX_DOUBLE_1 = 99999999999999.0;
        static double MAX_DOUBLE_2 = 0 - 99999999999999.0;
        return (v <= MAX_DOUBLE_2 || v >= MAX_DOUBLE_1);
    }

    static bool valid_price(double price)
    {
        return (price > -DBL_MAX && price < DBL_MAX) ;
    }

    static bool active_price(double price)
    {
        return (price > 0.00001 && price < DBL_MAX ) ;
    }

    static bool equal(double v1, double v2)
    {
        static const double DOUBLE_PRECISION = 0.0000001;
        static const double DOUBLE_PRECISION_2 = -0.0000001;
        double tt = v1 - v2;
        return tt < DOUBLE_PRECISION && tt > DOUBLE_PRECISION_2;
    }
    static bool less(double v1, double v2)
    {
        if (equal(v1, v2)) {
            return false;
        }
        return v1 < v2;
    }
    static bool less_equal(double v1, double v2)
    {
        if (equal(v1, v2)) {
            return true;
        }
        return v1 < v2;
    }
    static bool greater(double v1, double v2)
    {
        if (equal(v1, v2)) {
            return false;
        }
        return v1 > v2;
    }
    static bool greater_equal(double v1, double v2)
    {
        if (equal(v1, v2)) {
            return true;
        }
        return v1 > v2;
    }

    static double max(double v1, double v2)
    {
        return greater(v1, v2) ? v1 : v2;
    }

    static double min(double v1, double v2)
    {
        return less(v1, v2) ? v1 : v2;
    }

    static int max(int v1, int v2)
    {
        return v1 > v2 ? v1 : v2;
    }

    static int min(int v1, int v2)
    {
        return v1 < v2 ? v1 : v2;
    }

    static int double_to_int(double v)
    {
        return (int)(::round(v));
    }

    static bool is_multiple(double v1, double v2)
    {
        double min_double = 0.00001;
        return fabs(v1 / v2 - ::round(v1 / v2)) < min_double ? true : false;
    }

};

}
}

#endif
