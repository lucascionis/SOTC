#include "common.h"

typedef struct {
    s32 count;
    u8* slots;
} ShockSlotList;

typedef struct {
    u16 first;
    u16 second;
} ShockControler;

typedef union {
    u16 halfwords[8];
    void* pointers[4];
} ShockVoiceData;


INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", func_40018D88);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", func_40019018);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", func_400192E0);

void Init_ShockVoiceSet(ShockVoiceData* set, ShockVoiceData* data)
{
    set->pointers[0] = data;
    set->pointers[3] = (char*)data + data->halfwords[5] * 4;
    set->pointers[1] = (char*)data + data->halfwords[1] * 4;
    set->pointers[2] = (char*)data + data->halfwords[3] * 4;
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", Init_Shock);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", Shock_SetShockVoiceSet);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", Init_Player);

void Init_Controler(ShockControler* controler)
{
    controler->second = 0;
    controler->first = 0;
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", Shock_Request);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", Shock_Decode);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/shock", Shock_SetMotor);

void* func_40019958(void)
{
    return NULL;
}

void* func_40019960(ShockSlotList* list)
{
    s32 i;
    u8* slot = list->slots;
    for (i = 0; i < list->count; i++, slot += 0x40)
        if (*slot == 0)
            return slot;
    return NULL;
}

void func_400199A8(u8* slot)
{
    *slot = 0;
}
