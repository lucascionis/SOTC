#ifndef IOS_HASH_H
#define IOS_HASH_H

#include "common.h"
#include "fl_xfftype.h"



typedef struct t_hashedSym {
    u32 hash;
    struct t_xffSymEnt* extSym;
    struct t_xffSymEnt* locSym;
    struct t_hashedSym* next;
} t_hashedSym;

typedef struct {
    u32 maxValue;
    t_hashedSym** table;
} t_hashTable;




void AddStrHashKey(volatile t_hashTable* hTable, volatile t_hashedSym* hSym);
void InitStrHash(t_hashTable* hTable, s32 heap, s32 count);
void DeleteStrHashKey(t_hashTable* hTable, t_hashedSym* sym);
s32 GetStrHashWorkBuffer(s32 shift, s32 index);
u32 MakeStrHashValue(const char* str);
void ReleaseStrHashKeyAll(t_hashTable* hTable);
t_hashedSym* SearchStrHashKey(t_hashTable* hTable, u32 hash);





#endif // IOS_HASH_H
