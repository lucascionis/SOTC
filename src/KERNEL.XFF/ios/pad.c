#include "common.h"

extern void (*D_40045718)(void);
extern void *padSysGet(s32, s32);

typedef struct {
    char data[120];
} IosPadData;
extern IosPadData iosDefaultPadData[];

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/pad", iosPadTickProc);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/pad", iosPadGetXZInputL);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/pad", iosPadGetXZInputLwithThreshold);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/pad", iosPadGetXZInputLwithThresholdIndepAxis);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/pad", iosPadGetXZInputR);

void iosPadEntryTickFunc(void (*callback)(void))
{
    D_40045718 = callback;
}

void iosPadRead(s32 port, IosPadData *data)
{
    *data = iosDefaultPadData[port];
}

s32 iosPadChkConnect(s32 port)
{
    return *(s32 *)((char *)padSysGet(port, 0) + 0x118) == 1;
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/pad", iosPadInit);
