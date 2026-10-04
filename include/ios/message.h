#ifndef _MESSAGES_H
#define _MESSAGES_H

#include "ios/thread.h"


typedef struct{
    s32* unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    iosThread* unk14;
    iosThread* unk18;
} t_iosMessageQueue;



s32 iosSendMsg(t_iosMessageQueue* queue, s32 message, s32 wait);
s32 iosRecvMsg(t_iosMessageQueue* arg0, s32* arg1, s32 arg2);
void iosInitMessageSystem(void);
void iosCreateMsgQueue(t_iosMessageQueue* arg0, void* arg1, s32 arg2);





#endif // _MESSAGES_H
