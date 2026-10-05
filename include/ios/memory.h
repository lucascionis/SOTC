#ifndef _MEMORY_H
#define _MEMORY_H

#include "common.h"

void iosWaitMalloc(void);
void iosSignalMalloc(void);
void* iosMallocInGarbagePtn(s32 size, s32 alignment);
void iosMallocGarbageTickProc(s32 (*keepGoing)(s32), s32 argument);
void* iosMallocAlign(s32 partition, s32 size, s32 alignment);
void* iosMallocAlignNoCheck(s32 partition, s32 size, s32 alignment);
void iosMallocSetExecGarbageCallback(void* ptr, s32 callback, s32 argument);
void* iosReallocAlign(s32 partition, void* ptr, s32 size, s32 alignment);
void iosFree(void* ptr);
void* iosFreeParts(void* ptr, void* start, s32 size);
s32 iosCreatePartition(s32 partition, s32 size);
s32 iosCreatePartitionWithPc(s32 partition, s32 size, void* pc);
void iosInitMallocSystem(void* start, void* end, s32 garbageSize);
void* iosGetMallocPtn(void* ptr);
s32 iosGetMallocRootPtnRemainSize(void);
s32 iosFollowAllLink(s32 partition);
void iosDebugPartitionDump(s32 partition, s32 detail);
void iosDebugPartitionFileDump(s32 partition, s32 detail, s32 fd);
void addMemoryDebugCallback(void (*callback)(void));
s32* getMemorySafetyLockFlag(void);

#endif // _MEMORY_H
