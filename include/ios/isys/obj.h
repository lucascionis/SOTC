#ifndef _ISYS_OBJ
#define _ISYS_OBJ

#include "common.h"


typedef struct isysObj {
    s32 partition;
    void* sourceFunc;
    s32 flags;
    s16 unkC;
    s16 unkE;
    struct isysObj* unk10;
    s32 unk14;
    void* immediateDeleteFunc;
} isysObj;

typedef struct isysGroup {
    u16 head;
    u16 tail;
} isysGroup;

typedef struct isysLink {
    isysObj* obj;
    isysGroup* group;
    s32 param;
    u8 pending;
    u8 unkD;
    u16 groupNext;
    u16 groupPrev;
    u16 objNext;
    u16 objPrev;
    u16 pendingNext;
} isysLink;

void isysInitGroup(isysGroup* group);
void _isysDeleteGroup(isysObj* obj, isysGroup* group, s32 mode);
isysLink* isysGroupForFirst(isysGroup* group, s32 mode);
isysLink* isysGroupForNext(isysLink* link, s32 mode);
void isysGroupForExit(isysObj* obj, s32 mode);
isysLink* isysGetGroupSrhNext(isysLink* link);
isysLink* isysGroupForNextForce(isysLink* link, s32 mode);
s32 isysAddGroupWithLinkParam(isysObj* obj, isysGroup* group, s32 mode, s32 param);
s32 isysAddGroupRelativeWithLinkParam(isysObj* obj, isysGroup* group, isysLink* relative, s32 mode, s32 unused, s32 param);
s32 isysAddGroupWithLinkParamNoWait(isysObj* obj, isysGroup* group, s32 mode, s32 param);
s32 isysGetLinkNum(void);
isysObj* _isysCreateObj(s32 heap, s32 type, s32 objSize, s32 extraSize, s32 arg4, s32 alignment);
void isysSetObjImmediateDeleteFunc(isysObj* obj, void* func);
void isysRequestDeleteObj(isysObj* obj);

void isysObjDeleteLock(isysObj* obj);
void isysObjDeleteUnLock(isysObj* obj);
s32 isysGetLinkMax(void);
const char* isysGetObjIdentifierByObj(isysObj* obj);
void isysObjWaitSema(void);
void isysObjSignalSema(void);










#endif /* _ISYS_OBJ */
