#ifndef ISYS_SUBTHREAD_H
#define ISYS_SUBTHREAD_H

#include "ios/isys/obj.h"

typedef struct isysSubMsgQueue {
    s32 messageSize;
    void* buffer;
    s32 capacity;
    s32 write;
    s32 read;
} isysSubMsgQueue;

typedef struct isysSubThreadEnv {
    void (*entry)(void);
    void (*initialEntry)(void);
    s32 reserved;
    char *stack;
    char *stackPointer;
    s32 stackSize;
    s32 ownsStack;
    s32 status;
} isysSubThreadEnv;

void isysInitSubThreadExecEnv(isysObj* obj, void (*entry)(void));
void isysDeleteSubThread(isysObj* obj);
void isysStartSubThread(isysObj* obj, u32 arg);
s32 isysInitSubThreadObj(isysObj* obj, void (*entry)(void), s32 unused, s32 stackSize);
isysObj* isysCreateSubThreadS(void (*entry)(void), s32 partition, s32 stackSize);
s32 isysWakeupSubThread(isysObj* obj);
void isysJumpSubThread(void (*entry)(void), u32 arg);
void isysDumpSubThreadStatus(isysObj* obj);
s32 isysGetSubThreadStackSize(isysObj* obj);
f32 isysGetSubThreadStackUse(isysObj* obj);

void _isysCreateSubMsgQueue(isysSubMsgQueue* queue, void* buffer, s32 messageSize, s32 capacity);
s32 _isysRecvSubMsg(isysSubMsgQueue* queue, void* message, u32 messageSize);
s32 _isysSendSubMsg(isysSubMsgQueue* queue, const void* message, u32 messageSize);
void isysSleepSubThread(void);
void isysSleepSubThreadM(s32 frames);
void InitSubThread(void);
s32 isysGetSubThreadWorkSize(s32 stackSize);
isysObj* isysGetThisSubThread(void);
void isysDebugSetSubThreadSleepProtection(s32 enabled);

#endif
