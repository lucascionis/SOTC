#include "common.h"
#include "ios/isys/obj.h"
extern isysLink D_4009ACC0[];
extern isysLink D_4009ACD8[];
extern u8 D_4009ACD2[];
extern u8 D_4009ACD4[];
extern u8 D_4009ACCE[];
extern u8 D_4009ACD0[];
extern isysLink* D_400F2B00;

#include "ios/memory.h"
#include "ios/kernel.h"
extern char D_40048998[];
extern char D_40048968[];
extern char D_40048940[];
void func_4002AF90(isysLink* prev, isysLink* link, s32 mode);
void func_4002B218(isysLink* link);
extern isysLink* D_400F2B04;
extern isysObj* D_400F2B08;
#include "sdk/ee/eekernel.h"

extern const char* D_40045B30[];
extern s32 D_40045C5C;
extern s32 D_40045C60;
extern s32 D_400F2B0C;




static __inline__ isysLink* linkFromIndex(u32 value)
{
    if (value) {
        register s32 index asm("v1");
        asm("" : "=r"(index) : "0"(value));
        {
            register isysLink* result asm("v0");
            result = &D_4009ACC0[index];
            asm("" : "+r"(result));
            return result;
        }
    }
    return NULL;
}

static __inline__ u16 linkToIndex(isysLink* link)
{
    if (link != NULL)
        return link - D_4009ACC0;
    return 0;
}

void isysDeleteObjManager(void)
{
    s32 thread = GetThreadId();
    register isysLink* scan asm("a0");
    isysLink* cursor;
    register isysLink* tail asm("v1");
    isysLink* next;
    register isysLink* retainedLinks asm("s1");
    register isysObj* object asm("s1");
    register isysObj* objectNext asm("s3");
    register isysObj* retainedObjects asm("s4");
    register isysObj* deletedObjects asm("s5");
    if (thread == D_40045C5C) {
        D_40045C60 += 1;
    } else {
        WaitSema(D_400F2B0C);
        D_40045C5C = thread;
        D_40045C60 = 1;
    }
    scan = D_400F2B04;
    retainedLinks = NULL;
    while (scan != NULL) {
        register u16 value asm("v0") = scan->pendingNext;
        if (value != 0) {
            register s32 index asm("v1");
            asm("" : "=r"(index) : "0"(value));
            next = &D_4009ACC0[index];
        } else next = NULL;
        if (*(u8*)((char*)scan->obj + 9) == 0) {
            func_4002B218(scan);
        } else {
            if (retainedLinks != NULL) {
                register s32 index asm("v0") = retainedLinks - D_4009ACC0;
                asm("" : "+r"(index));
                scan->pendingNext = index;
            }
            else scan->pendingNext = 0;
            retainedLinks = scan;
        }
        scan = next;
    }
    D_400F2B04 = retainedLinks;
    if (retainedLinks != NULL) {
        tail = retainedLinks;
        asm("" : "+r"(tail));
        do {
        register u16 value asm("v0") = tail->pendingNext;
        register isysLink* result asm("v0");
        if (value != 0) {
            register s32 index asm("v1");
            asm("" : "=r"(index) : "0"(value));
            result = &D_4009ACC0[index];
        } else result = NULL;
        asm("" : "+r"(result));
        tail = result;
        } while (tail != NULL);
    }
    retainedObjects = NULL;
    object = D_400F2B08;
    deletedObjects = NULL;
    while (object != NULL) {
        objectNext = object->unk10;
        if (!(object->flags & 0xFFFEFF00)) {
            register u16 value asm("v0") = object->unkC;
            if (value != 0) {
                register s32 index asm("v1");
                asm("" : "=r"(index) : "0"(value));
                cursor = &D_4009ACC0[index];
            } else cursor = NULL;
            while (cursor != NULL) {
                register u16 value asm("v0");
                isysLink* result;
                func_4002B218(cursor);
                value = cursor->objNext;
                result = NULL;
                if (value != 0) {
                    register s32 index asm("v1");
                    asm("" : "=r"(index) : "0"(value));
                    result = &D_4009ACC0[index];
                }
                cursor = result;
            }
            object->unk10 = deletedObjects;
            deletedObjects = object;
            asm("" : "+r"(object));
            object->unkC = 0;
        } else {
            object->unk10 = retainedObjects;
            retainedObjects = object;
        }
        object = objectNext;
    }
    object = deletedObjects;
    D_400F2B08 = retainedObjects;
    while (object != NULL) {
        s32 callback = object->unk14;
        isysObj* nextFree = object->unk10;
        if (callback != 0) ((void (*)(isysObj*))callback)(object);
        { s32 partition = object->partition;
          object = nextFree;
          iosFree((void*)partition); }
    }
    if (retainedObjects != NULL) {
        register isysObj* walk asm("v0") = retainedObjects;
        do { walk = walk->unk10; } while (walk != NULL);
    }
    asm volatile("" : : : "memory");
    { s32 count = D_40045C60 - 1;
      D_40045C60 = count;
      if (count == 0) {
          D_40045C5C = -1;
          SignalSema(D_400F2B0C);
      } }
}

void func_4002AF90(isysLink* arg0, isysLink* arg1, s32 arg2)
{
    isysGroup* temp_a0;
    isysGroup* temp_a0_2;
    isysGroup* temp_a2;
    isysGroup* temp_v1_2;
    isysLink* var_a2;
    isysLink* var_a2_2;
    register u16 temp_v0 __asm__("v0");
    register u16 temp_v0_2 __asm__("v0");
    register u16 temp_v0_3 __asm__("v0");
    register u16 temp_v1 asm("v1");

    if (arg0 == NULL)
    {
        asm("" : "+r"(arg1));
        temp_a0 = arg1->group;
        if (arg1 != NULL) temp_a0->head = arg1 - D_4009ACC0;
        else temp_a0->head = 0;
        asm("" : "+r"(arg1));
        temp_a0_2 = arg1->group;
        if (arg1 != NULL) temp_a0_2->tail = arg1 - D_4009ACC0;
        else temp_a0_2->tail = 0;
        arg1->groupNext = 0;
        arg1->groupPrev = 0;
        return;
    }
    switch (arg2)
    {
    case 0:
        temp_v0 = arg0->groupPrev;
        {
        register s32 test asm("v1");
        asm("" : "=r"(test) : "0"(temp_v0));
        arg1->groupPrev = temp_v0;
        if (test != 0)
        {
            temp_v0_2 = arg0->groupPrev;
            if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_a2 = &D_4009ACC0[index];
        }
        else
        {
            var_a2 = NULL;
        }
            asm("" : "+r"(arg1));
            asm("" : "+r"(arg1));
        if (arg1 != NULL) var_a2->groupNext = arg1 - D_4009ACC0;
            else var_a2->groupNext = 0;
        }
        else
        {
            temp_a2 = arg0->group;
            asm("" : "+r"(arg1));
            asm("" : "+r"(arg1));
        if (arg1 != NULL) temp_a2->head = arg1 - D_4009ACC0;
            else temp_a2->head = 0;
        }
        asm("" : "+r"(arg0));
        if (arg0 != NULL) arg1->groupNext = arg0 - D_4009ACC0;
        else arg1->groupNext = 0;
        asm("" : "+r"(arg1));
        if (arg1 != NULL)
        {
            arg0->groupPrev = (arg1 - D_4009ACC0);
            return;
        }
        arg0->groupPrev = 0;
        return;
        }
    case 1:
        arg1->groupPrev = (arg0 - D_4009ACC0);
        temp_v1 = arg0->groupNext;
        {
        register s32 test asm("v0");
        asm("" : "=r"(test) : "0"(temp_v1));
        arg1->groupNext = temp_v1;
        if (test != 0)
        {
            temp_v0_3 = arg0->groupNext;
            if (temp_v0_3 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_3));
            var_a2_2 = &D_4009ACC0[index];
        }
        else
        {
            var_a2_2 = NULL;
        }
            asm("" : "+r"(arg1));
            asm("" : "+r"(arg1));
        if (arg1 != NULL) var_a2_2->groupPrev = arg1 - D_4009ACC0;
            else var_a2_2->groupPrev = 0;
        }
        else
        {
            temp_v1_2 = arg0->group;
            asm("" : "+r"(arg1));
            asm("" : "+r"(arg1));
        if (arg1 != NULL) temp_v1_2->tail = arg1 - D_4009ACC0;
            else temp_v1_2->tail = 0;
        }
        asm("" : "+r"(arg1));
        if (arg1 != NULL)
        {
            arg0->groupNext = (arg1 - D_4009ACC0);
            return;
        }
        arg0->groupNext = 0;
        return;
        }
    default:
        iosJumpRecoverPoint(D_40048940);
        return;
    }
}

void func_4002B218(isysLink* arg0)
{
    register isysLink* link asm("a1");
    isysGroup* temp_a2;
    isysObj* temp_a3;
    register u32 temp_v0 __asm__("v0");
    register u32 temp_v0_2 __asm__("v0");
    register u32 temp_v0_3 __asm__("v0");
    register u32 temp_v0_4 __asm__("v0");
    register u16 var_v0 __asm__("v0");

    link = arg0;
    asm("" : "+r"(link));
    temp_v0 = link->groupNext;
    asm("" : "+r"(temp_v0));
    temp_a2 = link->group;
    temp_a3 = link->obj;
    if (temp_v0 != 0)
    {
        *(u16*)(D_4009ACD0 + temp_v0 * sizeof(isysLink)) = link->groupPrev;
    }
    else if (temp_a2 != NULL)
    {
        temp_a2->tail = link->groupPrev;
    }
    temp_v0_2 = link->groupPrev;
    asm("" : "+r"(temp_v0_2));
    if (temp_v0_2 != 0)
    {
        *(u16*)(D_4009ACCE + temp_v0_2 * sizeof(isysLink)) = link->groupNext;
    }
    else if (temp_a2 != NULL)
    {
        temp_a2->head = link->groupNext;
    }
    temp_v0_3 = link->objNext;
    asm("" : "+r"(temp_v0_3));
    if (temp_v0_3 != 0)
    {
        *(u16*)(D_4009ACD4 + temp_v0_3 * sizeof(isysLink)) = link->objPrev;
    }
    else
    {
        temp_a3->unkE = link->objPrev;
    }
    temp_v0_4 = link->objPrev;
    asm("" : "+r"(temp_v0_4));
    if (temp_v0_4 != 0)
    {
        *(u16*)(D_4009ACD2 + temp_v0_4 * sizeof(isysLink)) = link->objNext;
    }
    else
    {
        temp_a3->unkC = link->objNext;
    }
    link->obj = NULL;
    link->pending = 0;
    var_v0 = linkToIndex(D_400F2B00);
    link->groupNext = var_v0;
    D_400F2B00 = link;
}

s32 isysAddGroupWithLinkParam(isysObj* obj, isysGroup* group, s32 mode, s32 param)
{
    isysLink** freeHead;
    isysLink* temp_s0;
    isysLink* var_a0;
    isysLink* var_a0_2;
    isysLink* var_v0;
    s32 temp_v0;
    s32 temp_v0_4;
    register u16 temp_v0_2 __asm__("v0");
    register u16 temp_v0_3 __asm__("v0");
    u16 temp_v1;

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    freeHead = &D_400F2B00;
    asm("" : "+r"(freeHead));
    temp_s0 = *freeHead;
    if (temp_s0 != NULL)
    {
        temp_v0_2 = temp_s0->groupNext;
        if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_v0 = &D_4009ACC0[index];
        }
        else
        {
            var_v0 = NULL;
        }
        *freeHead = var_v0;
    }
    temp_s0->param = param;
    temp_s0->obj = obj;
    temp_s0->group = group;
    if ((group == NULL) || (obj == NULL))
    {
        iosJumpRecoverPoint(D_40048968, group, obj);
    }
    temp_v0_3 = temp_s0->group->tail;
    if (temp_v0_3 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_3));
            var_a0 = &D_4009ACC0[index];
        }
        else
        {
            var_a0 = NULL;
        }
    func_4002AF90(var_a0, temp_s0, 1);
    if ((u16) obj->unkC == 0)
    {
        asm("" : "+r"(temp_s0));
        temp_s0->objPrev = 0;
        if (temp_s0 != NULL) obj->unkC = temp_s0 - D_4009ACC0;
        else obj->unkC = 0;
    }
    else
    {
        temp_s0->objPrev = (u16) obj->unkE;
        temp_v1 = (u16) obj->unkE;
        if (temp_v1 != 0) {
            var_a0_2 = &D_4009ACC0[temp_v1];
        }
        else
        {
            var_a0_2 = NULL;
        }
        asm("" : "+r"(temp_s0));
        if (temp_s0 != NULL) var_a0_2->objNext = temp_s0 - D_4009ACC0;
        else var_a0_2->objNext = 0;
    }
    asm("" : "+r"(temp_s0));
    temp_s0->objNext = 0;
    if (temp_s0 != NULL) obj->unkE = temp_s0 - D_4009ACC0;
        else obj->unkE = 0;
    temp_v0_4 = D_40045C60 - 1;
    D_40045C60 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
    return temp_s0 != NULL;
}

s32 isysAddGroupRelativeWithLinkParam(isysObj* arg0, isysGroup* arg1, isysLink* arg2, s32 arg3, s32 arg4, s32 arg5)
{
    isysLink** freeHead;
    isysLink* temp_s0;
    isysLink* var_a0;
    isysLink* var_v0;
    s32 temp_v0;
    s32 temp_v0_3;
    register u16 temp_v0_2 __asm__("v0");
    u16 temp_v1;

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    freeHead = &D_400F2B00;
    asm("" : "+r"(freeHead));
    temp_s0 = *freeHead;
    if (temp_s0 != NULL)
    {
        temp_v0_2 = temp_s0->groupNext;
        if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_v0 = &D_4009ACC0[index];
        }
        else
        {
            var_v0 = NULL;
        }
        *freeHead = var_v0;
    }
    temp_s0->param = arg5;
    temp_s0->obj = arg0;
    temp_s0->group = arg1;
    if ((arg1 == NULL) || (arg0 == NULL))
    {
        iosJumpRecoverPoint(D_40048968, arg1, arg0);
    }
    func_4002AF90(arg2, temp_s0, arg3);
    if ((u16) arg0->unkC == 0)
    {
        asm("" : "+r"(temp_s0));
        temp_s0->objPrev = 0;
        if (temp_s0 != NULL) arg0->unkC = temp_s0 - D_4009ACC0;
        else arg0->unkC = 0;
    }
    else
    {
        temp_s0->objPrev = (u16) arg0->unkE;
        temp_v1 = (u16) arg0->unkE;
        if (temp_v1 != 0) {
            var_a0 = &D_4009ACC0[temp_v1];
        }
        else
        {
            var_a0 = NULL;
        }
        asm("" : "+r"(temp_s0));
        if (temp_s0 != NULL) var_a0->objNext = temp_s0 - D_4009ACC0;
        else var_a0->objNext = 0;
    }
    asm("" : "+r"(temp_s0));
    temp_s0->objNext = 0;
    if (temp_s0 != NULL) arg0->unkE = temp_s0 - D_4009ACC0;
        else arg0->unkE = 0;
    temp_v0_3 = D_40045C60 - 1;
    D_40045C60 = temp_v0_3;
    if (temp_v0_3 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
    return temp_s0 != NULL;
}

void _isysDeleteGroup(isysObj* obj, isysGroup* group, s32 mode)
{
    register isysObj* object asm("s1");
    isysGroup* var_v0;
    isysLink* var_s0;
    register isysLink* var_v0_2 asm("v0");
    s32 temp_v0;
    s32 temp_v0_4;
    register u16 temp_v0_2 __asm__("v0");
    register u16 temp_v0_3 __asm__("v0");
    u16 var_v0_3;

    object = obj;
    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = (u16) object->unkC;
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_s0 = &D_4009ACC0[index];
        }
        else
        {
            var_s0 = NULL;
        }
    asm("" : : "r"(object));
    while (var_s0 != NULL)
    {
        if (var_s0->group == group) goto block_19;
        temp_v0_3 = var_s0->objNext;
        if (temp_v0_3) {
            register s32 index asm("v1");
            asm("" : "=r"(index) : "0"(temp_v0_3));
            var_v0_2 = &D_4009ACC0[index];
        } else var_v0_2 = NULL;
        asm("" : "+r"(var_v0_2));
        var_s0 = var_v0_2;
    }

block_17:
    if (mode == 0)
    {
        iosJumpRecoverPoint(D_40048998);
block_19:
        if (var_s0->pending != 1)
        {
            var_s0->pending = 1;
            var_s0->pendingNext = linkToIndex(D_400F2B04);
            D_400F2B04 = var_s0;
        }
    }
    temp_v0_4 = D_40045C60 - 1;
    D_40045C60 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}

void isysDeleteGroupWithLinkParam(isysObj* arg0, isysGroup* arg1, s32 arg2, s32 arg3)
{
    register isysObj* object asm("s1");
    isysGroup* var_v0;
    isysLink* var_s0;
    register isysLink* var_v0_2 asm("v0");
    s32 temp_v0;
    s32 temp_v0_4;
    register u16 temp_v0_2 __asm__("v0");
    register u16 temp_v0_3 __asm__("v0");
    u16 var_v0_3;

    object = arg0;
    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = *(u16*)((char*)(object) + 0xC);
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_s0 = &D_4009ACC0[index];
        }
        else
        {
            var_s0 = NULL;
        }
    asm("" : : "r"(object));
    while (var_s0 != NULL)
    {
        if (var_s0->group == arg1 && var_s0->param != 0) goto block_21;
        temp_v0_3 = var_s0->objNext;
        if (temp_v0_3) {
            register s32 index asm("v1");
            asm("" : "=r"(index) : "0"(temp_v0_3));
            var_v0_2 = &D_4009ACC0[index];
        } else var_v0_2 = NULL;
        asm("" : "+r"(var_v0_2));
        var_s0 = var_v0_2;
    }

block_19:
    if (arg3 == 0)
    {
        iosJumpRecoverPoint(D_40048998);
block_21:
        if (var_s0->pending != 1)
        {
            var_s0->pending = 1;
            var_s0->pendingNext = linkToIndex(D_400F2B04);
            D_400F2B04 = var_s0;
        }
    }
    temp_v0_4 = D_40045C60 - 1;
    D_40045C60 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}

void isysDeleteGroupAllObj(isysGroup* arg0)
{
    register s32* count asm("v1");
    isysLink* var_a0;
    isysLink* var_v0_2;
    s32 temp_v0;
    s32 temp_v0_4;
    register u16 temp_v0_2 __asm__("v0");
    register u32 temp_v0_3 __asm__("v0");
    register u16 var_v0 __asm__("v0");

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = *(u16*)((char*)(arg0) + 0);
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_a0 = &D_4009ACC0[index];
        }
        else
        {
            var_a0 = NULL;
        }
    if (var_a0 != NULL)
    {
        do
        {
            if (var_a0->pending != 1)
            {
                var_a0->pending = 1;
                var_a0->pendingNext = linkToIndex(D_400F2B04);
                D_400F2B04 = var_a0;
            }
            temp_v0_3 = var_a0->groupNext;
            var_a0->group = NULL;
            if (temp_v0_3 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_3));
            var_v0_2 = &D_4009ACC0[index];
        }
        else
        {
            var_v0_2 = NULL;
        }
            var_a0 = var_v0_2;
        } while (var_a0 != NULL);
    }
    arg0->head = 0;
    asm volatile("" : : : "memory");
    count = &D_40045C60;
    asm volatile("" : "+r"(count) : : "memory");
    arg0->tail = 0;
    asm volatile("" : : : "memory");
    temp_v0_4 = *count - 1;
    *count = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}

void isysDeleteAllGroup(isysObj* arg0, s32 (*arg1)(s32))
{
    isysLink* var_s0;
    isysLink* var_v0_2;
    s32 temp_v0;
    s32 temp_v0_4;
    register u16 temp_v0_2 __asm__("v0");
    u16 temp_v0_3;
    u16 var_v0;

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = *(u16*)((char*)(arg0) + 0xC);
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_s0 = &D_4009ACC0[index];
        }
        else
        {
            var_s0 = NULL;
        }
    if (var_s0 != NULL)
    {
        do
        {
            if (((arg1 == NULL) || (arg1(var_s0->param) != 0)) && (var_s0->pending != 1))
            {
                var_s0->pending = 1;
                var_s0->pendingNext = linkToIndex(D_400F2B04);
                D_400F2B04 = var_s0;
            }
            temp_v0_3 = var_s0->objNext;
            if (temp_v0_3 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_3));
            var_v0_2 = &D_4009ACC0[index];
        }
        else
        {
            var_v0_2 = NULL;
        }
            var_s0 = var_v0_2;
        } while (var_s0 != NULL);
    }
    temp_v0_4 = D_40045C60 - 1;
    D_40045C60 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}


isysLink* isysGroupForFirst(isysGroup* group, s32 mode)
{
    isysLink* var_s0;
    isysLink* var_v0;
    register u32 nextIndex asm("v0");
    isysObj* temp_v1;
    register u32 flags asm("v0");
    s32 temp_v0;
    s32 temp_v0_3;
    register u32 temp_v0_2 __asm__("v0");

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = group->head;
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_s0 = &D_4009ACC0[index];
        }
        else
        {
            var_s0 = NULL;
        }
    while (var_s0 != NULL)
    {
        if (!var_s0->pending) {
            temp_v1 = var_s0->obj;
            flags = *(u16*)((char*)temp_v1 + 0xA);
            asm("" : "+r"(flags));
            if (!(flags & 1)) {
                *(u8*)((char*)temp_v1 + 9) += 1;
                break;
            }
        }
        var_v0 = linkFromIndex(var_s0->groupNext);
        var_s0 = var_v0;
    }
    temp_v0_3 = D_40045C60 - 1;
    D_40045C60 = temp_v0_3;
    if (temp_v0_3 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
    return var_s0;
}

isysLink* isysGroupForFirstForce(u16* arg0)
{
    isysLink* var_s0;
    isysObj* temp_v1;
    s32 temp_v0;
    s32 temp_v0_3;
    register u16 temp_v0_2 __asm__("v0");

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = *arg0;
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_s0 = &D_4009ACC0[index];
        }
        else
        {
            var_s0 = NULL;
        }
    if (var_s0 != NULL)
    {
        temp_v1 = var_s0->obj;
        *(u8*)((char*)(temp_v1) + 9) = (u8) (*(u8*)((char*)(temp_v1) + 9) + 1);
    }
    temp_v0_3 = D_40045C60 - 1;
    D_40045C60 = temp_v0_3;
    if (temp_v0_3 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
    return var_s0;
}

s32 isysGroupGetObjNumForce(u16* arg0)
{
    isysLink* var_a0;
    register isysLink* var_v0 asm("v0");
    s32 temp_v0;
    s32 temp_v0_4;
    s32 var_s0;
    register u16 temp_v0_2 __asm__("v0");
    register u16 temp_v0_3 __asm__("v0");

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    temp_v0_2 = *arg0;
    if (temp_v0_2 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_2));
            var_a0 = &D_4009ACC0[index];
        }
        else
        {
            var_a0 = NULL;
        }
    var_s0 = 0;
    asm("" : : : "v1");
    if (var_a0 != NULL)
    {
        do
        {
            temp_v0_3 = var_a0->groupNext;
            if (temp_v0_3 != 0)
        {
            register s32 index __asm__("v1");
            __asm__("" : "=r"(index) : "0"(temp_v0_3));
            var_v0 = &D_4009ACC0[index];
        }
        else
        {
            var_v0 = NULL;
        }
            asm("" : "+r"(var_v0));
            var_a0 = var_v0;
            var_s0 += 1;
        } while (var_a0 != NULL);
    }
    temp_v0_4 = D_40045C60 - 1;
    D_40045C60 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
    return var_s0;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/isys/obj", _isysCreateObj);
isysObj* _isysCreateObj(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s32 part;
    isysObj* obj;
    register void* ra asm("ra");
    void* tmp;

    tmp = ra;
    part = iosCreatePartitionWithPc(arg0, arg2 + arg3 + sizeof(isysObj) + 8, tmp);
    if (part != 0)
    {
        obj = iosMallocAlign(part, arg2, arg5);
        if (obj != NULL)
        {
            obj->partition = part;
            obj->sourceFunc = tmp;
            *(u8*)&obj->flags = arg1;
            *(u8*)(((s32)&obj->flags)+1) = 0;
            obj->flags = *(u16*)&obj->flags;
            obj->unk14 = arg4;
            obj->unkC = obj->unkE = 0;
            obj->immediateDeleteFunc = NULL;
        }
        return obj;
    }
    return NULL;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/isys/obj", isysSetObjImmediateDeleteFunc);
void isysSetObjImmediateDeleteFunc(isysObj* obj, void* func)
{
    obj->immediateDeleteFunc = func;
}

void isysRequestDeleteObj(isysObj* arg0)
{
    register void* ra asm("ra");
    register u32 flags asm("a0");
    register u32 mask asm("v0");
    register isysObj** pending asm("v1");
    register s32* count asm("a1");
    void (*temp_v0)(void);
    s32 temp_v0_2;
    s32 temp_v0_3;

    temp_v0 = arg0->immediateDeleteFunc;
    if (temp_v0 != NULL)
    {
        temp_v0();
        arg0->immediateDeleteFunc = NULL;
    }
    temp_v0_2 = GetThreadId();
    if (temp_v0_2 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0_2;
        D_40045C60 = 1;
    }
    flags = arg0->flags;
    asm("" : "+r"(flags));
    mask = 0x10000;
    asm volatile("" : "+r"(mask) : : "memory");
    arg0->sourceFunc = ra;
    asm volatile("" : "+m"(arg0->sourceFunc));
    flags |= mask;
    pending = &D_400F2B08;
    asm("" : "+r"(flags), "+r"(pending));
    arg0->flags = flags;
    asm volatile("" : : : "memory");
    count = &D_40045C60;
    asm("" : "+r"(count) : : "memory");
    arg0->unk10 = *pending;
    *pending = arg0;
    asm volatile("" : : : "memory");
    temp_v0_3 = *count - 1;
    *count = temp_v0_3;
    if (temp_v0_3 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}

void isysInitGroup(isysGroup* group)
{
    group->tail = 0;
    group->head = 0;
}

s32 isysAddGroupWithLinkParamNoWait(isysObj* obj, isysGroup* group, s32 mode, s32 param)
{
    return isysAddGroupWithLinkParam(obj, group, mode, param);
}

void isysObjDeleteLock(isysObj* obj)
{
    u32 flags = obj->flags;

    obj->flags = (flags & 0x1FFFF) | (((flags >> 17) + 1) << 17);
}

void isysObjDeleteUnLock(isysObj* obj)
{
    u32 flags = obj->flags;

    obj->flags = (flags & 0x1FFFF) | (((flags >> 17) - 1) << 17);
}

isysLink* isysGroupForNext(isysLink* input, s32 mode)
{
 register isysLink* cursor asm("a1")=input;
 register isysLink* next asm("a0");
 isysLink* pool=D_4009ACC0;
 register isysObj* previous asm("a3");
 isysObj* obj;
 register u32 value asm("v0");
 register u32 flags asm("v0");
 previous=cursor->obj;

 asm("" : "+r"(previous) : : "memory");
 value=cursor->groupNext;
asm(".p2align 3");
loop:

 if(value) {
  register s32 index asm("v1");
  asm("" : "=r"(index) : "0"(value));
  {register s32 offset asm("v0")=index*24;asm("" : "+r"(offset));next=(isysLink*)((u32)offset+(u32)pool);}
 }else next=NULL;

 cursor=next;
 if(next) {
  if(!next->pending){
   obj=next->obj;
   flags=*(u16*)((char*)obj+10);

   if(!(flags&1)){

    *(u8*)((char*)previous+9)+=0xff;
    obj=next->obj;
    *(u8*)((char*)obj+9)+=1;
    goto done;
   }
  }
  value=cursor->groupNext;

  goto loop;
 }
done:

 return next;
}

isysLink* isysGroupForNextForce(isysLink* link, s32 mode)
{
    register isysObj* prev asm("a2");
    register u16 value asm("v0");
    isysLink* next;
    register isysObj* obj asm("v1");
    register u32 refs asm("v0");
    prev = link->obj;
    asm("" : "+r"(prev) : : "memory");
    value = link->groupNext;
    next = NULL;
    if (value) {
        register s32 index asm("v1");
        asm("" : "=r"(index) : "0"(value));
        next = &D_4009ACC0[index];
        asm(".p2align 3");
    }
    asm("" : : : "a0");
    if (next) {
        refs = *(u8*)((char*)prev + 9);
        asm("" : "+r"(refs));
        *(u8*)((char*)prev + 9) = refs + 0xFF;
        obj = next->obj;
        asm("" : "+r"(obj));
        refs = *(u8*)((char*)obj + 9);
        asm("" : "+r"(refs));
        *(u8*)((char*)obj + 9) = refs + 1;
    }
    return next;
}


void isysGroupForExit(isysObj* obj, s32 mode)
{
    s32 temp_v0;
    s32 temp_v1;

    temp_v0 = GetThreadId();
    if (temp_v0 == D_40045C5C)
    {
        D_40045C60 += 1;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = temp_v0;
        D_40045C60 = 1;
    }
    *(u8*)((char*)(obj) + 9) = (u8) (*(u8*)((char*)(obj) + 9) + 0xFF);
    temp_v1 = D_40045C60 - 1;
    D_40045C60 = temp_v1;
    if (temp_v1 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}

extern void initMemset(void*, s32, u32) asm("memset");
void isysInitObjSystem(void)
{
    register char* base asm("s0");
    register char* nextBase asm("a3");
    register u32 inverse asm("a2");
    s32 delta;
    s32 offset;
    s32 count;
    D_400F2B08 = NULL;
    D_400F2B04 = NULL;
    base = (char*)D_4009ACC0;
    initMemset(base, 0, 15000 * sizeof(isysLink));
    asm("" : : : "a2", "a3");
    asm("" : "+r"(base));
    inverse = 0xAAAAAAAB;
    nextBase = (base + 24);
    asm("" : "+r"(inverse), "+r"(nextBase));
    delta = nextBase - base;
    offset = 0;
    count = 14998;
    do {
        isysLink* link = (isysLink*)(offset + (s32)base);
        if ((isysLink*)(offset + (s32)nextBase))
            link->groupNext = ((offset + delta) >> 3) * inverse;
        else link->groupNext = 0;
        count--;
        offset += sizeof(isysLink);
    } while (count >= 0);
    D_400F2B00 = D_4009ACD8;
    D_400F2B0C = iosCreateSema(1, 1, 0);
}


s32 isysGetLinkNum(void)
{
    isysLink* link = D_400F2B00;
    s32 count = 0;
    while (link != NULL) {
        register u16 value asm("v0") = link->groupNext;
        register isysLink* next asm("v0");
        count++;
        if (value) {
            register s32 index asm("v1");
            asm("" : "=r"(index) : "0"(value));
            next = &D_4009ACC0[index];
        } else next = NULL;
        asm("" : "+r"(next));
        link = next;
    }
    return 15000 - count;
}


s32 isysGetLinkMax(void)
{
    return 15000;
}

/* Flush the old assembler macro expansion at the original function boundary. */
__asm__(".p2align 3");
const char* isysGetObjIdentifierByObj(isysObj* obj)
{
    s32 index;
    const char* identifier;
    asm(".p2align 3");
    index = *(u8*)&obj->flags;

    if (index < 37)
    {
        identifier = D_40045B30[index];
    }
    else
    {
        identifier = D_40045B30[0];
    }
    return identifier;
}

isysLink* isysGetGroupSrhNext(isysLink* link)
{
    u16 value = link->groupNext;
    register s32 index __asm__("v1");
    if (value != 0)
    {
        __asm__("" : "=r"(index) : "0"(value));
        {
            register isysLink* result asm("v0");
            result = &D_4009ACC0[index];
            asm("" : "+r"(result));
            return result;
        }
    }
    return NULL;
}

void isysObjWaitSema(void)
{
    s32 threadId = GetThreadId();

    if (threadId == D_40045C5C)
    {
        D_40045C60++;
    }
    else
    {
        WaitSema(D_400F2B0C);
        D_40045C5C = threadId;
        D_40045C60 = 1;
    }
}

void isysObjSignalSema(void)
{
    if (--D_40045C60 == 0)
    {
        D_40045C5C = -1;
        SignalSema(D_400F2B0C);
    }
}

/* Preserve the original padding after the last function. */
__asm__(".align 3");
