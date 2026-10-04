#ifndef _IOS_MATH_H
#define _IOS_MATH_H

#include "common.h"

f32 iosPowf(f32 base, f32 exponent);
f32 iosFastPowf(f32 base, f32 exponent);

/* The original names refer to three and four control points, respectively. */
f32 iosCubicBezier(f32 t, const f32* points);
f32 iosFourthBezier(f32 t, const f32* points);

#endif /* _IOS_MATH_H */
