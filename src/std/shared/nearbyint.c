#include <fenv.h>
#include <math.h>

float nearbyintf(float x)
{
    switch (fegetround()) {
        case FE_TONEAREST:  break;
        case FE_TOWARDZERO: return truncf(x);
        case FE_DOWNWARD:   return floorf(x);
        case FE_UPWARD:     return ceilf(x);
    }
    float i, f = modff(x, &i);
    if (!isgreaterequal(f, .5f)) return i;
    if (f == .5f) {
        float half = ldexp(i, -1);
        if (truncf(half) == half) return i;
    }
    return signbit(i) ? i - 1 : i + 1;
}

double nearbyint(double) __attribute__((alias("nearbyintf")));
