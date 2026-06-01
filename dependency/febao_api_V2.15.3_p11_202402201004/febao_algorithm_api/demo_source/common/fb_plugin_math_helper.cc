#include "fb_plugin_math_helper.h"

namespace cffex {
namespace fb {
namespace plugin{

double math_helper::round(double v, double precision, int mode /*= ROUNDING_MODEL_ROUND*/) {
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

double math_helper::normal_pdf(double x) {
    static double tmp = 1 / std::sqrt(2 * M_PI);
    return std::exp(0 - (x * x) / 2) * tmp;
}
/** valid in 15 bits, use HART algorithm  */
double math_helper::normal_cdf(double x) {
	double y = std::fabs(x);
	double CND, Exponential, SumA, SumB;
	if (y > 37) {
		CND = 0;
	}
	else {
		Exponential = std::exp(-y * y / 2);
		if (y < 7.07106781186547) {
			SumA = 0.0352624965998911 * y + 0.700383064443688;
			SumA = SumA * y + 6.37396220353165;
			SumA = SumA * y + 33.912866078383;
			SumA = SumA * y + 112.079291497871;
			SumA = SumA * y + 221.213596169931;
			SumA = SumA * y + 220.206867912376;
			SumB = 0.0883883476483184 * y + 1.75566716318264;
			SumB = SumB * y + 16.064177579207;
			SumB = SumB * y + 86.7807322029461;
			SumB = SumB * y + 296.564248779674;
			SumB = SumB * y + 637.333633378831;
			SumB = SumB * y + 793.826512519948;
			SumB = SumB * y + 440.413735824752;
			CND = Exponential * SumA / SumB;
		}
		else {
			SumA = y + 0.65;
			SumA = y + 4 / SumA;
			SumA = y + 3 / SumA;
			SumA = y + 2 / SumA;
			SumA = y + 1 / SumA;
			CND = Exponential / (SumA * 2.506628274631);
		}
	}

	if (x > 0) {
		return 1 - CND;
	}
	else {
		return CND;
	}
}

}
}
}