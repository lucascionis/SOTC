#include "common.h"

extern void (*D_400F6D88)(void);
extern void (*D_400F6D8C)(void);
extern s32 soundTickProcCnt;
extern s32 sg2NicoReverbMode;
extern s32 D_40045EA8;
extern void *Sg2SlotTbl(s32);
extern void Sg2SlotSetOutputMode(s32);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", func_4002F2A8);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepTickProc);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepReq);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepPlayerSrh);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepPlayerSrh2);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepStopReq);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepSetVol);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepSetPan);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepSetDefaultParam);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepPauseAll);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepPause);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepCont);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepStopTypeAll);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepInitTickParam);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndSepInit);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndVabCreate);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndVabDelete);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndVabHDOpen);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndVabHDClose);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndBDOpen);

void sndBDClose(void) {
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", sndVabInit);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", initSound);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/snd", tickProcSound);

void addAdpcmTickProcFunc(void (*callback)(void))
{
    D_400F6D88 = callback;
}

void setPcmTickProcFunc(void (*callback)(void))
{
    D_400F6D8C = callback;
}

s32 soundGetTickProcCnt(void)
{
    return soundTickProcCnt;
}

void soundSetRevMode(s32 mode, s32 mask)
{
    if (mode == 0)
        sg2NicoReverbMode |= mask;
    else
        sg2NicoReverbMode &= ~mask;
}

s32 soundGetRevMode(void)
{
    return sg2NicoReverbMode;
}

void *getSlotInfo(s32 slot)
{
    return Sg2SlotTbl(slot);
}

void setSoundMode(s32 mode)
{
    D_40045EA8 = mode != 0 ? 1 : 2;
    Sg2SlotSetOutputMode(D_40045EA8);
}

asm(".align 3");
