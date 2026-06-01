#ifndef fb_plugin_math_helper_H
#define fb_plugin_math_helper_H

//#include <cffex/platform.h>
#include <cmath>

namespace cffex {
namespace fb {
namespace plugin {

#ifndef DBL_MAX
#define DBL_MAX __DBL_MAX__
#endif

#ifndef CFFEX_DOUBLE_NULL
#define CFFEX_DOUBLE_NULL DBL_MAX
#endif

class math_helper {
    enum {
        ROUNDING_MODEL_CEIL                 =    '1'  ,    /* 舍入， 向上取整 */
        ROUNDING_MODEL_FLOOR                =    '2'  ,    /* 舍出， 向下取整 */
        ROUNDING_MODEL_ROUND                =    '3'       /* 四舍五入 */
    };
public:
    static double round(double v, double precision, int mode = ROUNDING_MODEL_ROUND);

    static double normal_pdf(double x);
    static double normal_cdf(double x);

    static bool valid_price(double price) {
        return (price > -DBL_MAX && price < DBL_MAX) ;
    }

    static bool active_price(double price, bool allow_zero = true)
    {
        if(allow_zero) {
            return (price > - 0.00001 && price < DBL_MAX );
        }
        return (price > 0.00001 && price < DBL_MAX );
    }

    static bool equal(double v1, double v2) {
        static const double DOUBLE_PRECISION = 0.000000001;
        static const double DOUBLE_PRECISION_2 = -0.000000001;
        double tt = v1 - v2;
        return tt < DOUBLE_PRECISION && tt > DOUBLE_PRECISION_2;
    }
    static bool less(double v1, double v2) {
        if (equal(v1, v2)) {
            return false;
        }
        return v1 < v2;
    }
    static bool less_equal(double v1, double v2) {
        if (equal(v1, v2)) {
            return true;
        }
        return v1 < v2;
    }
    static bool greater(double v1, double v2) {
        if (equal(v1, v2)) {
            return false;
        }
        return v1 > v2;
    }
    static bool greater_equal(double v1, double v2) {
        if (equal(v1, v2)) {
            return true;
        }
        return v1 > v2;
    }
    static double mm_fabs(double v1, double v2) {
        return fabs(v1-v2);
    }
};

}
}
}

#endif