#include "common.h"

typedef struct {
    s32 unk0;
    s32 unk4;
    u8* data;
    s32 unkC;
    s32 width;
    s32 height;
    s32 unk18;
    s32 unk1C;
} IosFontInfo;

extern IosFontInfo D_400F5BF8[];

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", func_4002D888);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosConvertStringSJIStoEUC);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosConvertStringEUCtoSJIS);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosConvertStringEUCtoID);

extern s32 D_40045C90[];

s32 iosGetDataSizeOfFont(s32 font)
{
    return D_40045C90[font];
}

u8* iosGetFontDataWithID(IosFontInfo* info, s32 id, s32 font)
{
    IosFontInfo* selected = &D_400F5BF8[font];
    u8* data = selected->data + id * (selected->width * selected->height);
    if (info != NULL)
        *info = *selected;
    return data;
}

u8* iosGetFontDataWithJIS(IosFontInfo* info, s32 code, s32 font)
{
    IosFontInfo* selected = &D_400F5BF8[font];
    s32 id = (code >> 8) * 94 + (code & 0xFF) - 0xC3F;
    u8* data = selected->data + id * (selected->width * selected->height);
    if (info != NULL)
        *info = *selected;
    return data;
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosGetFontDataWithSJIS);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosGetFontDataWithEUC);

s32 iosGetFontIDWithJIS(s32 code)
{
    return (code >> 8) * 94 + (code & 0xFF) - 0xC3F;
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosGetFontIDWithSJIS);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/font", iosGetFontIDWithEUC);

u8* iosEntryLocaleFontNoLoad(u8* data, IosFontInfo* info)
{
    D_400F5BF8[0] = *info;
    D_400F5BF8[0].data = data;
    return data;
}

void iosGetFontInfo(IosFontInfo* info, s32 font)
{
    *info = D_400F5BF8[font];
}

s32 iosGet2ByteCharCode(const u8* str)
{
    return (str[1] << 8) | str[0];
}

asm(".align 3");
