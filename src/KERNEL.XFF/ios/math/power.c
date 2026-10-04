#include "common.h"
#include "ios/math.h"
#include "gcc/math.h"

f32 iosPowf(f32 base, f32 exponent)
{
    return powf(base, exponent);
}

f32 iosFastPowf(f32 base, f32 exponent)
{
    return powf(base, exponent);
}
