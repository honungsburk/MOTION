#include <math.h>

extern "C" {

    double __exp_finite(double x) { return exp(x); }
    double __log_finite(double x) { return log(x); }
    double __pow_finite(double x, double y) { return pow(x, y); }

    double __exp2_finite(double x) { return exp2(x); }
    double __log2_finite(double x) { return log2(x); }
    double __log10_finite(double x) { return log10(x); }

    double _ZGVbN2v___log_finite(double x) { return log(x); }

    double __atan2_finite(double x, double y) { return atan2(x, y); }

    float __expf_finite(float x) { return expf(x); }
    float __logf_finite(float x) { return logf(x); }
    float __powf_finite(float x, float y) { return powf(x, y); }

    float __exp2f_finite(float x) { return exp2f(x); }
    float __log2f_finite(float x) { return log2f(x); }

}