#include "common.h"
#include "ios/message.h"
#include "ios/kernel.h"



extern s32 iosSystemSema;



// prototypes
s32 func_40016BB8(iosThread* arg0);
void func_40016BD0(iosThread* arg0);


// .rodata
//extern char D_40046C40;



s32 iosSendMsg(t_iosMessageQueue* queue, s32 message, s32 wait)
{
    union MessageSlot { s32 value; void* pointer; };
    s32 next;
    iosThread* receiver;
    s32 result;
    iosThread* wake;
    iosThread* head;
    iosThread* tail;
    iosThread* self;
    iosThread* link;
    iosThread* node;

    wake = NULL;
    self = iosGetThread();
loop:
    WaitSema(iosSystemSema);
    if (queue->unk8 < queue->unk4)
    {
        result = 0;
        ((union MessageSlot*)queue->unk0)[queue->unkC * 2].value = message;
        next = queue->unkC + 1;
        receiver = queue->unk14;
        queue->unk8++;
        queue->unkC = next >= queue->unk4 ? 0 : next;
        if (receiver != NULL)
        {
            if (wake != NULL)
                asm volatile("break 0" : : "r"(receiver) : "memory");
            wake = receiver;
            queue->unk14 = NULL;
        }
    }
    else
        result = -1;
    if (result == -1 && wait == 1)
    {
        head = queue->unk18;
        self->unk28 = NULL;
        if (head != NULL)
        {
            while (head->unk28 != NULL)
                head = head->unk28;
            head->unk28 = self;
        }
        else
            queue->unk18 = self;
        SignalSema(iosSystemSema);
        iosSleepThread();
        if (queue->unk0 == NULL)
            return result;
        goto loop;
    }
    if (wake != NULL)
        iosCallBackThreadMgr(func_40016BB8, (s32)wake);
    SignalSema(iosSystemSema);
    return result;
}


//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/message", iosRecvMsg);
s32 iosRecvMsg(t_iosMessageQueue* arg0, s32* arg1, s32 arg2)
{
    s32 temp_a0;
    s32 temp_a1;
    s32 var_s2;
    s32 var_v1;
    iosThread* temp_v1;
    iosThread* var_s4;
    void* var_s6;

    var_s4 = NULL;
    var_s6 = NULL;
    if (arg0->unk14 != 0)
//        iosJumpRecoverPoint(&D_40046C40);
        iosJumpRecoverPoint("iosmessage: iosRecvMsg 複数の場所でメッセージ待ちになってる\n");
    
loop_5:
    WaitSema(iosSystemSema);
    arg0->unk14 = iosGetThread();
    if (arg0->unk0 != 0)
    {
        if (arg0->unk8 != 0)
        {
            var_v1 = arg0->unkC - arg0->unk8;
            var_v1 = var_v1 < 0 ? arg0->unk4 + var_v1 : var_v1;
            
            if (arg1 != NULL)
                *arg1 = arg0->unk0[var_v1 * 2];
            
            temp_v1 = arg0->unk18;
            arg0->unk8 = (s32) (arg0->unk8 - 1);
            var_s2 = 0;
            if (temp_v1 != NULL)
            {
                var_s4 = temp_v1;
                arg0->unk18 = var_s4->unk28;
            }
        }
        else
        {
            var_s2 = -1;
            if (arg2 == 1)
            {
                SignalSema(iosSystemSema);
                iosSleepThread();
                goto loop_5;
            }
        }
    }
    else
    {
        var_s6 = arg0->unk18;
        var_s2 = 0;
        arg0->unk18 = NULL;
    }
    arg0->unk14 = 0;
    if (var_s4 != NULL)
        iosCallBackThreadMgr(func_40016BB8, (s32)arg0->unk18);
    
    if (var_s6 != NULL)
        iosCallBackThreadMgr(func_40016BD0, (s32)var_s6);
    
    SignalSema(iosSystemSema);
    return var_s2;
}

void iosInitMessageSystem(void)
{}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/message", iosCreateMsgQueue);
void iosCreateMsgQueue(t_iosMessageQueue* arg0, void* arg1, s32 arg2)
{
    memset(arg0, 0, 0x1C);
    if ((arg1 != 0) && (arg2 > 0))
        memset(arg1, 0, arg2 * 8);
    
    arg0->unk0 = arg1;
    arg0->unk4 = arg2;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/message", func_40016BB8);
s32 func_40016BB8(iosThread* arg0)
{
    return iosWakeupThread(arg0);
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/message", func_40016BD0);
void func_40016BD0(iosThread* arg0)
{
    if (arg0 != NULL)
    {
        do
        {
            iosWakeupThread(arg0);
            arg0 = arg0->unk28;
        } while (arg0 != NULL);
    }
}



