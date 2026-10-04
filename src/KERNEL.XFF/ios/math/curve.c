#include "common.h"
#include "ios/math.h"

f32 iosCubicBezier(f32 t, const f32* points)
{
    f32 u = 1.0f - t;
    f32 tSquared = t * t;

    return (u * u) * points[0] + ((u + u) * t) * points[1] + tSquared * points[2];
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/math/curve", iosCubicInterVectorXYZ);

f32 iosFourthBezier(f32 t, const f32* points)
{
    f32 u = 1.0f - t;
    f32 tSquared = t * t;
    f32 tCubed = tSquared * t;
    f32 uSquared = u * u;

    return (uSquared * u) * points[0] + ((uSquared * 3.0f) * t) * points[1]
        + ((u * 3.0f) * tSquared) * points[2] + tCubed * points[3];
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/math/curve", iosCubicBezierVectorXYZ);
