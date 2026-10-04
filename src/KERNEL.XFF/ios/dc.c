#include "common.h"

extern UNK_PTR D_40045A00;
extern UNK_PTR D_40045A04;
extern char D_40092688[];
extern void iosDlChainTailCurrent(UNK_PTR);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", func_40024218);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosCreateDC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosReleaseDC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosSetDCParam);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosSetDCClearParam);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosSetDCViewPort);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosSetDCViewPortCenter);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosSetDC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosPushDC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosPopDC);

UNK_PTR iosGetRootDC(void)
{
    return D_40045A04;
}

void iosSetRootDC(void)
{
    iosDlChainTailCurrent(D_40045A04);
}

UNK_PTR iosGetCurrentDC(void)
{
    return D_40045A00;
}

void iosSetCurrentDC(void)
{
    iosDlChainTailCurrent(D_40045A00);
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosResetDC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosInitDC);

void *iosGetDCGroup(void)
{
    return D_40092688;
}

void func_40024A30(void)
{}

void func_40024A38(void)
{}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosSetDCWithClear);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", _iosPushDC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/dc", iosPushDCWithClear);
