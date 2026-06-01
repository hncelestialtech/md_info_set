#ifndef FB_I_STRATEGY_MATH_HELP_H
#define FB_I_STRATEGY_MATH_HELP_H

#include <math.h>

namespace cffex {
namespace fb {

class i_math_helper {
public:
    static bool is_null(double v);
    static void set_null(double &v);

    static bool   equal(double v1, double v2);
    static bool   less(double v1, double v2);
    static bool   less_equal(double v1, double v2);
    static bool   greater(double v1, double v2);
    static bool   greater_equal(double v1, double v2);
    static double mm_fabs(double v1, double v2);
};

}  // namespace fb
}  // namespace cffex

#endif
