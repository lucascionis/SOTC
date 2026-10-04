#include "common.h"
#include "gcc/string.h"
#include "ios/hash.h"
#include "ios/kernel.h"
#include "ios/memory.h"


// .rodata
//extern const char D_40048FC8;
//extern const char D_40048F98;




//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/hash", GetStrHashWorkBuffer);
s32 GetStrHashWorkBuffer(s32 shift, s32 index)
{
    return (4 << shift) + (index * 0x10) + 0x66;
}

void InitStrHash(t_hashTable* hTable, s32 heap, s32 count)
{
    union hashSlot { s32 value; t_hashedSym* sym; };
    s32 shift = -2;
    s32 i;

    while (count != 0)
    {
        count >>= 1;
        shift++;
    }
    if (shift < 0)
        shift = 0;
    hTable->maxValue = 1 << shift;
    hTable->table = iosMallocAlign(heap, hTable->maxValue * 4, 0x10);
    for (i = 0; i < (s32)hTable->maxValue; i++)
        ((union hashSlot*)hTable->table)[i].value = 0;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/hash", AddStrHashKey);
void AddStrHashKey(volatile t_hashTable* hTable, volatile t_hashedSym* hSym)
{
    u32 maxVal;
    u32 hash;
    u32 off;

    maxVal = hTable->maxValue - 1;
    hash = hSym->hash;
    off = hash & maxVal;
    
    hSym->next = hTable->table[off];
    hTable->table[hash & (hTable->maxValue - 1)] = (t_hashedSym*)hSym;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/hash", MakeStrHashValue);
u32 MakeStrHashValue(const char* str)
{
    int i;
    u32 j;
    u32 hash;

    hash = 0;
    
//    if (str == NULL || strcmp(&D_40048FC8, str) == 0)
    if (str == NULL || strcmp("NULL", str) == 0)
        return 0;
    
    for (i=0; i<2; i++)
    {
        
        for (j=i; j<strlen(str); j+=2)
            hash = (hash * 0x4D) + (str[j] - 0x2D);
    }
    
    return hash;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/hash", SearchStrHashKey);
t_hashedSym* SearchStrHashKey(t_hashTable* hTable, u32 hash)
{
    
    t_hashedSym* cSym = hTable->table[hash & (hTable->maxValue - 1)];
    
    while (cSym != NULL)
    {
        if (hash == cSym->hash)
            return cSym;
        
        cSym = cSym->next;
    }
    
    return NULL;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/hash", ReleaseStrHashKeyAll);
void ReleaseStrHashKeyAll(t_hashTable* hTable)
{
    iosFree(hTable->table);
    hTable->table = NULL;
    hTable->maxValue = 0;
}

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/hash", DeleteStrHashKey);
void DeleteStrHashKey(t_hashTable* hTable, t_hashedSym* sym)
{
    t_hashedSym* nSym;
    t_hashedSym* pSym;
    t_hashedSym* cSym;
    
    cSym = hTable->table[sym->hash & (hTable->maxValue - 1)];
    pSym = NULL;
    
    while (cSym != NULL)
    {
        nSym = cSym->next;
        if (sym == cSym)
        {
            if (pSym == NULL)
                hTable->table[cSym->hash & (hTable->maxValue - 1)] = nSym;
            
            else
                pSym->next = nSym;
            
            return;
        }
        
        pSym = cSym;
        cSym = nSym;
    }
    
//    iosJumpRecoverPoint(&D_40048F98);
    iosJumpRecoverPoint("DeleteStrHashKey:ハッシュキーがみつかりません\n");
}

