#include "common.h"
#include "ios/isys/subthread.h"
#include "gcc/string.h"
#include "ios/memory.h"
extern s32 rootPtn;
extern void isysSearchModuleFuncNameByAddr(char *, char *, void (*)(void));
extern isysGroup subThreadGroup;
extern isysObj* D_40045E98;
extern s32 D_40045E9C;


void _isysCreateSubMsgQueue(isysSubMsgQueue* queue, void* buffer, s32 messageSize, s32 capacity)
{
    queue->buffer = buffer;
    queue->messageSize = messageSize;
    queue->capacity = capacity;
    queue->read = 0;
    queue->write = 0;
}

s32 _isysRecvSubMsg(isysSubMsgQueue* queue, void* message, u32 messageSize)
{
    s32 read;
    s32 size;
    s32 next;

    size = queue->messageSize;
    if (size != messageSize)
        return 0;
    read = queue->read;
    if (read == queue->write)
        return 0;
    memcpy(message, (char*)queue->buffer + read * size, messageSize);
    next = queue->read + 1;
    queue->read = next >= queue->capacity ? 0 : next;
    return 1;
}

s32 _isysSendSubMsg(isysSubMsgQueue* queue, const void* message, u32 messageSize)
{
    s32 result = 0;
    s32 size;
    s32 capacity;
    s32 write;
    s32 next;
    s32 oldest;

    size = queue->messageSize;
    if (size == messageSize) {
        write = queue->write;
        capacity = queue->capacity;
        next = write + 1 < capacity ? write + 1 : 0;
        oldest = next + 1;
        if (next == queue->read)
            queue->read = oldest >= capacity ? 0 : oldest;
        memcpy((char*)queue->buffer + write * size, message, messageSize);
        queue->write = next;
        result = 1;
    }
    return result;
}

void InitSubThread(void)
{
    isysInitGroup(&subThreadGroup);
}

/* These original tables dispatch object types 6, 9, 11-14, 22, 29 and 33.
 * Their targets are exported C labels, preserving the existing rodata. */
extern void *D_40048AC0[];

void isysInitSubThreadExecEnv(isysObj *obj, void (*entry)(void))
{
    isysSubThreadEnv *env;
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    char *top;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if ((u32)type < 28)
        goto *D_40048AC0[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002E568\nfunc_4002E568:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002E570\nfunc_4002E570:");
    env = NULL;
gotEnv:
    top = env->stack + env->stackSize;
    env->initialEntry = entry;
    asm("" : : : "memory");
    env->entry = entry;
    asm("" : : : "memory");
    env->stackPointer = top;
}

/* These stack image stores may alias an execution environment. A union
 * preserves that alias when the callback word follows the argument quadword. */
typedef union isysSubThreadWord {
    u128 vector;
    u32 word;
} isysSubThreadWord;
typedef struct isysSubThreadVolatileEnv {
    void (*entry)(void);
    void (*initialEntry)(void);
    s32 reserved;
    char *stack;
    char *volatile stackPointer;
    s32 stackSize;
    s32 ownsStack;
    s32 status;
} isysSubThreadVolatileEnv;

extern void *D_40048B30[];

void isysStartSubThread(isysObj *obj, u32 arg)
{
    register isysObj *source asm("t0") = obj;
    register isysSubThreadVolatileEnv *env asm("a3");
    register char *top asm("a2");
    u64 argWord;
    u64 entryWord;
    register u32 type asm("v0");
    type = ((u8 *)&source->flags)[0] - 6;
    asm("" : "+r"(type), "+r"(source));
    obj = source;
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if ((u32)type < 28)
        goto *D_40048B30[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002E5C8\nfunc_4002E5C8:");
    env = (isysSubThreadVolatileEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002E5D0\nfunc_4002E5D0:");
    env = NULL;
gotEnv:
    top = env->stackPointer;
    argWord = (u64)arg;
    env->stackPointer = top - 0x2C0;
    ((isysSubThreadWord *)(top - 0x200))->vector = (u128)argWord;
    entryWord = ((isysSubThreadWord *)&env->initialEntry)->word;
    ((isysSubThreadWord *)(env->stackPointer + 0x270))->vector = (u128)entryWord;
    env->status = 0;
    isysWakeupSubThread(obj);
}


__asm__(".globl func_4002E5F8\nfunc_4002E5F8=isysStartSubThread+0x68");

extern void *D_40048BA0[];

void isysDeleteSubThread(isysObj *obj)
{
    isysSubThreadEnv *env;
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if ((u32)type < 28)
        goto *D_40048BA0[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002E650\nfunc_4002E650:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002E658\nfunc_4002E658:");
    env = NULL;
gotEnv:
    if (env->ownsStack != 0)
        iosFree(env->stack);
}

extern void *D_40048C10[];

void isysDumpSubThreadStatus(isysObj *obj)
{
    isysSubThreadEnv *env;
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if ((u32)type < 28)
        goto *D_40048C10[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002E6C0\nfunc_4002E6C0:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002E6C8\nfunc_4002E6C8:");
    env = NULL;
gotEnv:
    {
    char *moduleName;
    char *functionName;
    moduleName = iosMallocAlign((s32)&rootPtn, 256, 4);
    functionName = iosMallocAlign((s32)&rootPtn, 256, 4);
    isysSearchModuleFuncNameByAddr(moduleName, functionName, env->entry);
    iosFree(functionName);
    iosFree(moduleName);
    }
}

/* A context occupies 0x2C0 bytes: FPR words at 0x00, 128-bit GPR
 * slots from 0xA0, FP at 0x260, continuation at 0x270 and HI/LO/SA/
 * HI1/LO1 at 0x280..0x2A0. These instructions capture the actual CPU
 * registers and switch SP; C owns object dispatch and scheduler state. */
#define SUBTHREAD_SAVE_FPRS \
    ".irp reg,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31\n" \
    "mfc1 $v0,$f\\reg\n" \
    "sw $v0,(4*\\reg)($sp)\n" \
    ".endr\n"

#define SUBTHREAD_SAVE_GPRS \
    ".irp reg,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25\n" \
    "sq $\\reg,(0x80+16*\\reg)($sp)\n" \
    ".endr\n" \
    "sq $fp,0x260($sp)\n"

#define SUBTHREAD_SAVE_SPECIAL \
    "mfhi $v0\n" \
    "sd $v0, 0x280($sp)\n" \
    "mflo $v0\n" \
    "sd $v0, 0x288($sp)\n" \
    "mfsa $v0\n" \
    "sd $v0, 0x290($sp)\n" \
    "mfhi1 $v0\n" \
    "sd $v0, 0x298($sp)\n" \
    "mflo1 $v0\n" \
    "sd $v0, 0x2A0($sp)\n"

#define SUBTHREAD_RESTORE_FPRS_LOW \
    ".irp reg,0,1,2,3,4\n" \
    "lw $v0,(4*\\reg)($sp)\n" \
    "mtc1 $v0,$f\\reg\n" \
    ".endr\n"

#define SUBTHREAD_RESTORE_FPRS_MIDDLE \
    "mtc1 $v0, $f5\n" \
    "mtc1 $v0, $f6\n" \
    "mtc1 $v0, $f7\n" \
    "mtc1 $v0, $f8\n"

#define SUBTHREAD_RESTORE_FPRS_HIGH \
    ".irp reg,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31\n" \
    "lw $v0,(4*\\reg)($sp)\n" \
    "mtc1 $v0,$f\\reg\n" \
    ".endr\n"

#define SUBTHREAD_RESTORE_GPRS \
    "lq $zero,0x80($sp)\n" \
    ".irp reg,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25\n" \
    "lq $\\reg,(0x80+16*\\reg)($sp)\n" \
    ".endr\n" \
    "lq $fp,0x260($sp)\n"

#define SUBTHREAD_RESTORE_SPECIAL \
    "ld $v0, 0x280($sp)\n" \
    "mthi $v0\n" \
    "ld $v0, 0x288($sp)\n" \
    "mtlo $v0\n" \
    "ld $v0, 0x290($sp)\n" \
    "mtsa $v0\n" \
    "ld $v0, 0x298($sp)\n" \
    "mthi1 $v0\n" \
    "ld $v0, 0x2A0($sp)\n" \
    "mtlo1 $v0\n"

#define SUBTHREAD_SAVE_SP(symbol) \
    "lui $v0,%%hi(" #symbol ")\n" \
    "nop\n" \
    "addiu $v0,$v0,%%lo(" #symbol ")\n" \
    "nop\n" \
    "sw $sp,0($v0)\n"

#define SUBTHREAD_LOAD_SP(symbol) \
    "lui $v0,%%hi(" #symbol ")\n" \
    "nop\n" \
    "addiu $v0,$v0,%%lo(" #symbol ")\n" \
    "nop\n" \
    "lw $sp,0($v0)\n"

#define SUBTHREAD_SAVE_CPU \
    "addiu $sp,$sp,-0x2C0\n" \
    SUBTHREAD_SAVE_FPRS SUBTHREAD_SAVE_GPRS SUBTHREAD_SAVE_SPECIAL

/* Preserve both original restore paths: F5..F8 receive the F4 word. */
#define SUBTHREAD_WAKE_CPU \
    SUBTHREAD_SAVE_CPU \
    SUBTHREAD_SAVE_SP(D_400F5D38) SUBTHREAD_LOAD_SP(D_400F5D3C) \
    "lq $ra,0x270($sp)\n" \
    SUBTHREAD_RESTORE_FPRS_LOW \
    "mtc1 $v0,$f5\nmtc1 $v0,$f6\nmtc1 $v0,$f7\nmtc1 $v0,$f8\n" \
    SUBTHREAD_RESTORE_FPRS_HIGH SUBTHREAD_RESTORE_GPRS SUBTHREAD_RESTORE_SPECIAL \
    "addiu $sp,$sp,0x2C0\n"

#define SUBTHREAD_SLEEP_CPU \
    SUBTHREAD_SAVE_CPU SUBTHREAD_SAVE_SP(D_400F5D3C)

#define SUBTHREAD_RESTORE_CPU \
    SUBTHREAD_LOAD_SP(D_400F5D38) \
    SUBTHREAD_RESTORE_FPRS_LOW SUBTHREAD_RESTORE_FPRS_MIDDLE SUBTHREAD_RESTORE_FPRS_HIGH \
    SUBTHREAD_RESTORE_GPRS SUBTHREAD_RESTORE_SPECIAL \
    "addiu $sp,$sp,0x2C0\n"

#define SUBTHREAD_INVOKE_CPU \
    "lui        $v0, %%hi(D_400F5D48)\n" \
    ".globl func_4002EB00\nfunc_4002EB00:\n" \
    "nop\n" \
    "addiu      $v0, $v0, %%lo(D_400F5D48)\n" \
    "nop\n" \
    "lw         $v0, 0x0($v0)\n" \
    "jalr       $v0\n" \
    "nop\n" \
    "nop\n"

extern void *D_40048C80[];
extern char *D_400F5D38;
extern char *D_400F5D3C;
extern s32 D_400F5D40;
extern void (*D_400F5D48)(void);

s32 isysWakeupSubThread(isysObj *obj)
{
    register isysSubThreadEnv *env asm("a3");
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    register char **childStack asm("a1");
    register s32 *terminated asm("a2");
    register s32 result asm("v0");
    register void (*continuation)(void) asm("ra");
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    env = NULL;
    if (type < 28)
        goto *D_40048C80[type];
    goto gotEnv;
validType:
    asm(".globl func_4002E768\nfunc_4002E768:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002E770\nfunc_4002E770:");
    env = NULL;
gotEnv:
    asm volatile("pref 0,0(%0)" : : "r"(obj));
    result = 1;
    if (env->status != 0)
        goto done;
    terminated = &D_400F5D40;
    childStack = &D_400F5D3C;
    D_40045E98 = obj;
    D_400F5D38 = NULL;
    *(volatile s32 *)terminated = 0;
    *(char *volatile *)childStack = *(char *volatile *)&env->stackPointer;
    D_400F5D48 = *(void (*volatile *)(void))&env->initialEntry;
    asm("" : : "r"(env), "r"(terminated), "r"(childStack));
    asm volatile(".set noreorder\n" SUBTHREAD_WAKE_CPU
        ".globl func_4002EAFC\n.type func_4002EAFC,@function\nfunc_4002EAFC:\n.size func_4002EAFC,64\n"
        SUBTHREAD_INVOKE_CPU ".set reorder\n"
        : : : "memory", "ra");
    {
        register s32 *terminationFlag asm("v0");
        register s32 completed asm("v1");
        /* This handoff keeps the original split HI/LO address sequence. */
        asm volatile("lui %0,%%hi(D_400F5D40)" : "=r"(terminationFlag));
        completed = 0;
        asm("" : "+r"(completed));
        asm volatile("addiu %0,%0,%%lo(D_400F5D40)"
            : "+r"(terminationFlag) : "r"(completed));
        completed++;
        *terminationFlag = completed;
    }
    asm volatile(".set noreorder\n"
        "nop\nj SubThreadTerminateRestorePoint\nnop\n"
        ".globl isysSleepSubThread\n.type isysSleepSubThread,@function\nisysSleepSubThread:\n.size isysSleepSubThread,420\n"
        SUBTHREAD_SLEEP_CPU
        ".globl SubThreadTerminateRestorePoint\n.type SubThreadTerminateRestorePoint,@function\nSubThreadTerminateRestorePoint:\n.size SubThreadTerminateRestorePoint,444\n"
        SUBTHREAD_RESTORE_CPU ".set reorder\n"
        : "=r"(continuation) : : "memory", "ra");
    *(void (*volatile *)(void))&env->initialEntry = continuation;
    asm("" : : : "memory");
    {
        register char *savedStack asm("v0") = *childStack;
        asm("" : "+r"(savedStack));
        *(char *volatile *)&env->stackPointer = savedStack;
        *(void (*volatile *)(void))(savedStack + 0x270) = continuation;
    }
    result = *(volatile s32 *)terminated;
    *(volatile s32 *)&env->status = result;
done:
    return result;
}
__asm__(".size isysWakeupSubThread,972");

isysObj* isysGetThisSubThread(void)
{
    return D_40045E98;
}

extern void *D_40048CF0[];

__asm__(".p2align 3");
s32 isysInitSubThreadObj(isysObj *obj, void (*entry)(void), s32 unused, s32 stackSize)
{
    isysSubThreadEnv *env;
    register void (*source)(void) asm("a1") = entry;
    register u32 valid asm("v0");
    register u32 rawType asm("v1") = ((u8 *)&obj->flags)[0] - 6;
    u32 type;
    asm("" : "+r"(rawType));
    type = rawType;
    valid = (u32)type < 28;
    asm("" : "+r"(valid), "+r"(source) : "g"(&&validType), "g"(&&invalidType));
    entry = source;
    if (valid == 0)
        goto invalidType;
    goto *D_40048CF0[type];
validType:
    asm(".globl func_4002EF00\nfunc_4002EF00:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002EF08\nfunc_4002EF08:");
    env = NULL;
gotEnv:
    env->stack = iosMallocAlign(obj->partition, stackSize, 16);
    if (env->stack == NULL)
        return -1;
    *(char *volatile *)&env->stackPointer = env->stack + stackSize;
    *(volatile s32 *)&env->ownsStack = 1;
    *(void (*volatile *)(void))&env->initialEntry = entry;
    *(volatile s32 *)&env->stackSize = stackSize;
    *(void (*volatile *)(void))&env->entry = entry;
    isysAddGroupWithLinkParam(obj, &subThreadGroup, 0, 0);
    return 0;
}

extern void *D_40048D60[];
isysObj *isysCreateSubThreadS(void (*entry)(void), s32 partition, s32 stackSize)
{
    isysObj *obj = _isysCreateObj(partition, 3, 0x40, stackSize + 0x10, (s32)isysDeleteSubThread, 4);
    isysSubThreadEnv *env;
    register u32 type asm("v0");
    s32 result;
    if (obj == NULL)
        return NULL;
    type = ((u8 *)&obj->flags)[0] - 6;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if (type < 28)
        goto *D_40048D60[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002EFF8\nfunc_4002EFF8:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002F000\nfunc_4002F000:");
    env = NULL;
gotEnv:
    env->stack = iosMallocAlign(obj->partition, stackSize, 16);
    result = -1;
    if (env->stack != NULL) {
        *(volatile s32 *)&env->ownsStack = 1;
        *(char *volatile *)&env->stackPointer = env->stack + stackSize;
        *(void (*volatile *)(void))&env->initialEntry = entry;
        *(volatile s32 *)&env->stackSize = stackSize;
        *(void (*volatile *)(void))&env->entry = entry;
        isysAddGroupWithLinkParam(obj, &subThreadGroup, 0, 0);
        result = 0;
    }
    return result == 0 ? obj : NULL;
}


void isysSleepSubThreadM(s32 frames)
{
    s32 count;
    if (frames > 0) {
        count = frames;
        do {
            isysSleepSubThread();
            count--;
        } while (count != 0);
    }
}

extern void *D_40048DD0[];
extern char *D_400F5D3C;
extern u32 D_400F5D44;
extern void (*D_400F5D48)(void);
void isysJumpSubThread(void (*entry)(void), u32 arg)
{
    isysObj *obj = isysGetThisSubThread();
    register isysSubThreadEnv *env asm("a3");
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    env = NULL;
    if (type < 28)
        goto *D_40048DD0[type];
    goto gotEnv;
validType:
    asm(".globl func_4002F108\nfunc_4002F108:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002F110\nfunc_4002F110:");
    env = NULL;
gotEnv:
    {
    s32 size = *(volatile s32 *)&env->stackSize;
    char *top = env->stack;
    env->initialEntry = entry;
    env->entry = entry;
    env->stackPointer = top + size;
    D_400F5D44 = arg;
    D_400F5D3C = *(char *volatile *)&env->stackPointer;
    D_400F5D48 = *(void (*volatile *)(void))&env->initialEntry;
    }
    /* Execution continues on the newly selected child stack. */
    asm volatile(
        ".set noreorder\n"
        "lui $v0,%%hi(D_400F5D3C)\n"
        "nop\n"
        "addiu $v0,$v0,%%lo(D_400F5D3C)\n"
        "nop\n"
        "lw $sp,0($v0)\n"
        "lui $v0,%%hi(D_400F5D44)\n"
        "nop\n"
        "addiu $v0,$v0,%%lo(D_400F5D44)\n"
        "nop\n"
        "lw $a0,0($v0)\n"
        "j func_4002EAFC\n"
        "nop\n"
        ".set reorder\n"
        ".globl func_4002F17C\n"
        ".type func_4002F17C,@function\n"
        "func_4002F17C:\n"
        ".size func_4002F17C,20\n"
        : : : "memory");
}
__asm__(".size isysJumpSubThread,196");

s32 isysGetSubThreadWorkSize(s32 stackSize)
{
    return stackSize + 0x33;
}

extern void *D_40048E40[];

f32 isysGetSubThreadStackUse(isysObj *obj)
{
    register isysSubThreadEnv *env asm("v0");
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if ((u32)type < 28)
        goto *D_40048E40[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002F1C8\nfunc_4002F1C8:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002F1D0\nfunc_4002F1D0:");
    env = NULL;
gotEnv:
    asm("" : "+r"(env));
    {
        register s32 size asm("a2") = env->stackSize;
        register s32 unused asm("a1");
        register s32 freeBytes asm("v0");
        f32 fraction;
        f32 numerator;
        register f32 one asm("$f2");
        asm("" : : "r"(size));
        unused = 0;
        if (size > 0) {
            register u8 *stack asm("a0") = (u8 *)env->stack;
            register s32 sentinel asm("t0") = 0xFE;
            register s32 limit asm("a3") = size;
            asm("" : "+r"(limit) : "r"(stack), "r"(unused), "r"(sentinel));
            do {
                if (stack[unused] != sentinel) {
                    freeBytes = unused;
                    goto scannedStack;
                }
                unused++;
            } while (unused < limit);
        }
        freeBytes = size;
scannedStack:
        asm("" : : "r"(freeBytes));
        numerator = (f32)freeBytes;
        asm("" : "+f"(numerator) : : "$f2");
        one = 1.0f;
        asm("" : : "f"(one));
        fraction = numerator / (f32)size;
        return one - fraction;
    }
}

extern void *D_40048EB0[];

__asm__(".p2align 3");

s32 isysGetSubThreadStackSize(isysObj *obj)
{
    isysSubThreadEnv *env;
    register u32 type asm("v0") = ((u8 *)&obj->flags)[0] - 6;
    asm("" : "+r"(type));
    asm("" : : "g"(&&validType), "g"(&&invalidType));
    if ((u32)type < 28)
        goto *D_40048EB0[type];
    env = NULL;
    goto gotEnv;
validType:
    asm(".globl func_4002F278\nfunc_4002F278:");
    env = (isysSubThreadEnv *)((char *)obj + 0x20);
    goto gotEnv;
invalidType:
    asm(".globl func_4002F280\nfunc_4002F280:");
    env = NULL;
gotEnv:
    return env->stackSize;
}

void isysDumpSubThreadPC(void)
{}

void isysDebugSetSubThreadSleepProtection(s32 enabled)
{
    D_40045E9C = enabled;
}

/* Preserve the original eight-byte boundary after the final setter. */
__asm__(".align 3");
