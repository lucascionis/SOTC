#include "common.h"
#include "ios/memory.h"
#include "sdk/ee/eekernel.h"
#include "gcc/stdarg.h"
#include "gcc/stdio.h"
#include "gcc/string.h"
#include "loaderSysFileIO.h"

extern s32 D_4007CC78;
struct GarbageMemoryBlock
{
    u32 size:24, p0:8;
    u32 requested:24, p1:8;
    u32 next:24, p2:8;
    u32 previous:24, allocated:1, isPartition:1, rest:6;
    u32 callback:24, callbackFlags:8;
    u32 argument:24, argumentFlags:8;
};

/* Word addresses occupy 24 bits, with flags in the high byte. Free-list links
 * use bytes 28 and 32. The allocated payload starts at byte 36 before alignment;
 * its back pointer is stored immediately before the aligned address. */
typedef struct MemoryBlock
{
    u32 size:24;
    u32 ptn0:8;
    u32 requested:24;
    u32 ptn1:8;
    u32 next:24;
    u32 ptn2:8;
    u32 previous:24;
    u32 allocated:1;
    u32 isPartition:1;
    u32 alignment:3;
    u32 callbackLow:2;
    u32 reserved:1;
    u32 unk10:24;
    u32 callback0:8;
    u32 unk14:24;
    u32 callback1:8;
    u32 pc:24;
    u32 callback2:8;
    struct MemoryBlock* freeNext;
    struct MemoryBlock* freePrevious;
} MemoryBlock;
typedef struct MemoryBucket
{
    MemoryBlock* first;
    MemoryBlock* last;
} MemoryBucket;

typedef struct MemoryPartition
{
    u32 buckets:24;
    u32 pad0:8;
    u16 bucketCount;
    u16 pad6;
    u32 first:24;
    u32 pad8:8;
} MemoryPartition;

typedef struct FreeMemoryBlock {
    union { u32 unk0; struct { u32 size:24; u32 ptn0:8; }; };
    union { u32 unk4; struct { u32 requested:24; u32 ptn1:8; }; };
    union { u32 unk8; struct { u32 next:24; u32 ptn2:8; }; };
    union { u32 unkC; struct { u32 previous:24; u32 allocated:1; u32 isPartition:1; u32 alignment:3; u32 callbackLow:2; u32 reserved:1; }; };
    union { u32 unk10; struct { u32 value10:24; u32 callback0:8; }; };
    union { u32 unk14; struct { u32 value14:24; u32 callback1:8; }; };
    union { u32 unk18; struct { u32 pc:24; u32 callback2:8; }; };
    struct FreeMemoryBlock *unk1C;
    struct FreeMemoryBlock *freePrevious;
} FreeMemoryBlock;

typedef struct FreeMemoryBucket
{
    FreeMemoryBlock* first;
    FreeMemoryBlock* last;
} FreeMemoryBucket;

typedef char FreeMemoryBlockSizeCheck[sizeof(FreeMemoryBlock) == 36 ? 1 : -1];

typedef char MemoryBlockSizeCheck[sizeof(MemoryBlock) == 36 ? 1 : -1];
typedef char MemoryBucketSizeCheck[sizeof(MemoryBucket) == 8 ? 1 : -1];
typedef char MemoryPartitionSizeCheck[sizeof(MemoryPartition) == 12 ? 1 : -1];
extern s8 D_40045570[];
extern char D_400466D0[], D_40046710[], D_40046728[], D_40046738[], D_40046748[], D_400467C0[], D_400467C8[], D_400467D0[], D_40046800[], D_40046828[], D_40046878[], D_400468C0[], D_40046908[], D_40046948[];
void func_400133C8(MemoryBlock* block, void* buckets, s32 bucketCount, void* partition);
#define PTR_WORD(x) ((s32)(x) >> 2)
#define WORD_PTR(x) ((void*)((x) << 2))


extern s32 gbPtn;
extern s32 D_400455F4;
extern u32 iosDefaultPadData[];
extern char D_400465B8[], D_400465E8[], D_40046618[], D_40046628[], D_40046668[], D_40046690[];
void iosJumpRecoverPoint(const char* format, ...);
void func_40015F18(s32 partition, s32 detail);
void* func_40013A78(FreeMemoryBlock* block, FreeMemoryBlock* next, void* partition);
extern s32 D_40045560;
extern void (*D_40045568)(void);
extern s32 D_4004556C;
extern u8 rootPtn[];
extern void* mallocStartAdrs;
extern char D_40046708[];

s32 iosCreateSema(s32 initial, s32 maximum, s32 option);
void* func_400135C8(void* partition, u32 size, s32 alignment, void* pc);
void* func_40014770(void* ptr, void* start, s32 size);
void func_40014F88(void* partition, void* start, void* end, s32 garbageSize);

void func_400142E0(void* ptr);
void* func_40014AF8(s32 partition, void* ptr, s32 size, s32 alignment);
s32 func_40014D48(s32 partition, s32 size, void* pc);
void func_400155C0(s32 partition, s32 parent, s32* count);
void func_400152B8(void* partition, s32 depth, s32 detail, s32 fd);


void func_40015F58(void);
#define DMA_WORD(addr) (*(volatile u32*)(addr))
#define DMA_KEEP(x) __asm__ volatile("" : "+r"(x) :: "memory")
#define DMA_FENCE() __asm__ volatile("" ::: "memory")
void func_40013160(s32 destination, s32 source, u32 bytes)
{
    register u32 units __asm__("s5") = bytes >> 4;
    register s32 src __asm__("s4") = source;
    register s32 dst __asm__("s6") = destination;
    if (units >= 0x401U) {
        volatile u32* volatile regs[8];
        register u32 first __asm__("v0");
        register u32 second __asm__("v1");
        register u32 status __asm__("s7");
        register u32 enable __asm__("s0");
        register s32 mask __asm__("fp");
        register u32 quantum __asm__("s3");
        register u32 scratch __asm__("s2");
        register u32 dstTag __asm__("s1");
        first = 0x10000000;
        second = 0x10000000;
        status = 0x10000000;
        __asm__ volatile("" : "+r"(first), "+r"(second), "+r"(status) :: "memory");
        first |= 0xd410; DMA_KEEP(first);
        second |= 0xd420; DMA_KEEP(second);
        regs[0] = (volatile u32*)status; DMA_FENCE();
        regs[1] = (volatile u32*)status; DMA_FENCE();
        enable = 0x10000000; DMA_KEEP(enable);
        regs[0] = (volatile u32*)first; DMA_FENCE();
        first = 0x10000000; DMA_KEEP(first);
        regs[1] = (volatile u32*)second; DMA_FENCE();
        second = 0x10000000; DMA_KEEP(second);
        first |= 0xd480; DMA_KEEP(first);
        second |= 0xd400; DMA_KEEP(second);
        regs[2] = (volatile u32*)status; DMA_FENCE();
        enable |= 0xe020; DMA_KEEP(enable);
        regs[3] = (volatile u32*)status; DMA_FENCE();
        mask = -0x400; DMA_KEEP(mask);
        regs[2] = (volatile u32*)first; DMA_FENCE();
        first = 0x10000000; DMA_KEEP(first);
        regs[3] = (volatile u32*)second; DMA_FENCE();
        second = 0x10000000; DMA_KEEP(second);
        first |= 0xd010; DMA_KEEP(first);
        second |= 0xd020; DMA_KEEP(second);
        regs[4] = (volatile u32*)status; DMA_FENCE();
        quantum = 0x400; DMA_KEEP(quantum);
        regs[5] = (volatile u32*)status; DMA_FENCE();
        scratch = 0x70000000; DMA_KEEP(scratch);
        regs[4] = (volatile u32*)first; DMA_FENCE();
        first = 0x10000000; DMA_KEEP(first);
        regs[5] = (volatile u32*)second; DMA_FENCE();
        second = 0x10000000; DMA_KEEP(second);
        first |= 0xd080; DMA_KEEP(first);
        second |= 0xd000; DMA_KEEP(second);
        regs[6] = (volatile u32*)status; DMA_FENCE();
        dstTag = 0x100; DMA_KEEP(dstTag);
        regs[7] = (volatile u32*)status; DMA_FENCE();
        status |= 0xe010; DMA_KEEP(status);
        regs[6] = (volatile u32*)first; DMA_FENCE();
        regs[7] = (volatile u32*)second; DMA_FENCE();
        first = 0x200;
        do {
            DMA_WORD(enable) = mask;
            {
                __asm__("" :: "r"(first), "r"(mask));
                DMA_WORD(status) = first;
                DMA_FENCE();
            }
            units -= 0x400;
            second = (u32)regs[0]; DMA_KEEP(second);
            DMA_WORD(second) = src; DMA_FENCE();
            src += 0x4000;
            first = (u32)regs[1]; DMA_KEEP(first);
            DMA_WORD(first) = quantum; DMA_FENCE();
            second = (u32)regs[2]; DMA_KEEP(second);
            DMA_WORD(second) = scratch; DMA_FENCE();
            second = 0x101; DMA_KEEP(second);
            first = DMA_WORD(enable); DMA_KEEP(first);
            first |= 0x200; DMA_KEEP(first);
            DMA_WORD(enable) = first; DMA_FENCE();
            first = (u32)regs[3]; DMA_KEEP(first);
            DMA_WORD(first) = second;
            func_40015F58();
            __asm__("" :: "r"(mask));
            DMA_WORD(enable) = mask;
            DMA_WORD(status) = dstTag;
            first = (u32)regs[4]; DMA_KEEP(first);
            DMA_WORD(first) = dst; DMA_FENCE();
            dst += 0x4000;
            second = (u32)regs[5]; DMA_KEEP(second);
            DMA_WORD(second) = quantum; DMA_FENCE();
            first = (u32)regs[6]; DMA_KEEP(first);
            DMA_WORD(first) = scratch; DMA_FENCE();
            first = DMA_WORD(enable); DMA_KEEP(first);
            first |= 0x100; DMA_KEEP(first);
            DMA_WORD(enable) = first; DMA_FENCE();
            second = (u32)regs[7]; DMA_KEEP(second);
            DMA_WORD(second) = dstTag;
            func_40015F58();
            {
                register u32 again __asm__("v0") = units < 0x401U;
                __asm__("" :: "r"(again));
                if (again) break;
            }
            first = 0x200;
        } while (1);
    }
    DMA_WORD(0x1000e020) = -0x400;
    DMA_WORD(0x1000e010) = 0x200;
    DMA_WORD(0x1000d410) = src;
    DMA_WORD(0x1000d420) = units;
    DMA_WORD(0x1000d480) = 0x70000000;
    DMA_WORD(0x1000e020) |= 0x200;
    DMA_WORD(0x1000d400) = 0x101;
    func_40015F58();
    DMA_WORD(0x1000e020) = -0x400;
    DMA_WORD(0x1000e010) = 0x100;
    DMA_WORD(0x1000d010) = dst;
    DMA_WORD(0x1000d020) = units;
    DMA_WORD(0x1000d080) = 0x70000000;
    DMA_WORD(0x1000e020) |= 0x100;
    DMA_WORD(0x1000d000) = 0x100;
    func_40015F58();
}


/* Insert by ascending block size into the selected free-list bucket. */
void func_400133C8(MemoryBlock* block, void* buckets, s32 count, void* partition)
{
    s32 size = block->size * 4;
    register s32 offset __asm__("t2");
    s32 index;
    MemoryBlock* entry;
    register MemoryBucket* initialBucket __asm__("t3");
    register MemoryBucket* bucket __asm__("a0");
    if (partition == rootPtn) {
        if (size <= 1024) {
            s32 result;
            if (size <= 128) { index = D_40045570[size]; goto classified; }
            result = 8;
            if (size > 192) { if (size > 256) result = size <= 512 ? 10 : 11; else result = 9; }
            index = result;
            goto classified;
        } else {
            register s32 classSize __asm__("a0") = size; s32 quotient; asm("" :: "r"(classSize));
            quotient = classSize >> 10;
            if (quotient & 0xFF00) goto high20; index=12; goto qloop; high20: quotient = classSize >> 18; index = 20;  qloop:;
            while (quotient >>= 1) index++;
            goto classified;
        }
    } else {
        s32 quotient = size / 1024;
        index = 1;
        while (quotient >>= 1) index++;
    }
classified:

    { u32 idx = index - 1; if (idx >= count) idx = count - 1; __asm__("" ::: "t2"); offset = idx * 8; }
    initialBucket = (MemoryBucket*)(offset + (s32)buckets);
    __asm__("" :: "r"(initialBucket));

    entry = initialBucket->first;
    __asm__ ("" : "+r"(offset));

    if (entry) {
        s32 freeWords=size>>2;

        bucket=initialBucket;


        while (1) {
            MemoryBlock* next;
            if ((u32)freeWords < entry->size) {
                u32 previous = entry->unk14;
                block->unk14 = previous;
                if (!previous) bucket->first = block;
                else ((MemoryBlock*)(block->unk14 * 4))->freeNext = block;
                block->freeNext = entry;
                entry->unk14 = (s32)block >> 2;
                return;
            }
            next = entry->freeNext;
            if (!next) {
                register MemoryBucket* appendBucket __asm__("v0")=(MemoryBucket*)(offset+(s32)buckets);
                register s32 previous __asm__("v1");
                register u32 highMask __asm__("a0");
                register u32 header __asm__("v0");


                previous=(s32)entry>>2;
                appendBucket->last=block;

                highMask=0xFF000000;
                *(MemoryBlock* volatile*)((u8*)entry+0x1C)=block;
                previous &= 0xFFFFFF;
                *(MemoryBlock* volatile*)((u8*)block+0x1C)=0;
                header=*(volatile u32*)((u8*)block+0x14);

                header &= highMask;
                header |= previous;
                *(u32*)((u8*)block+0x14)=header;
                return;
            }
            entry = next;
        }
    }

    {
        register MemoryBucket* emptyBucket __asm__("v0") = (MemoryBucket*)(offset+(s32)buckets);
        register u32 emptyMask __asm__("v1") =0xff000000;
        register u32 header __asm__("v0");

        *(MemoryBlock* volatile*)((u8*)emptyBucket)=block;
        *(MemoryBlock* volatile*)((u8*)emptyBucket+4)=block;
        *(MemoryBlock* volatile*)((u8*)block+0x1C)=0;
        header=*(volatile u32*)((u8*)block+0x14);
        header&=emptyMask;
        *(u32*)((u8*)block+0x14)=header;
    }

}


/* Root buckets use a lookup for small blocks, then logarithmic classes.
 * Other partitions use logarithmic classes starting at one. */
static inline s32 allocRequestedClass(void* partition, s32 size)
{
    if (partition == rootPtn)
    {
        s32 rootBias = size + 1023;
        if (size <= 1024)
        {
            register s32 result __asm__("a0");
            __asm__("" : : "r"(rootBias));
            if (size <= 128)
            {
                return D_40045570[size];
            }
            if (size <= 192)
            {
                result = 8;
            }
            else
            {
                if (size > 256)
                {
                    s32 smaller = size <= 512;
                    result = smaller ? 10 : 11;
                }
                else
                {
                    result = 9;
                }
            }
            __asm__("" : : "r"(result));
            return result;
        }
        else
        {
            register s32 adjusted __asm__("a0") = size < 0 ? rootBias : size;
            s32 q = size / 1024;
            register s32 result __asm__("a1") = 12;
            if (q & 0xFF00)
            {
                q >>= 8;
                result = 20;
            }
            while (q >>= 1)
            {
                result++;
            }
            return result;
        }
    }
    else
    {
        register s32 rounded __asm__("v0") = size;
        s32 q;
        register s32 result __asm__("a1") = 1;
        rounded = size < 0 ? size + 1023 : size;
        q = size / 1024;
        while (q >>= 1)
        {
            result++;
        }
        __asm__("" : : "r"(rounded));
        {
            register s32 out __asm__("v0") = result;
            __asm__("" : : "r"(out));
            return out;
        }
    }
}

/* Find an aligned payload, unlink its free block, and return any tail to a bucket. */
void* func_400135C8(void* partition, u32 bytes, s32 alignment, void* pc)
{
    MemoryBlock* ptn = (MemoryBlock*)partition;
    s32 size = bytes;
    register s32 align __asm__("t4");
    s32 buckets = ptn->size * 4;
    register u32 index __asm__("a0");
    register s32 classResult __asm__("v0");
    register s32 offset __asm__("v1");
    s32 words;
    register s32 count __asm__("a1");
    register MemoryBucket* bucket __asm__("t0");
    MemoryBlock* block;
    MemoryBlock* split;
    s32 result;
    u32 end;
    register s32 alignBits __asm__("v1");
    register s32 shifted __asm__("v0");
    register s32 limit __asm__("t6");
    register s32 remainingWords __asm__("a1");
    register s32 ptnLow __asm__("a3");
    register u32 sizeWord __asm__("v1");
    register s32 ptnHigh __asm__("a3");
    register u32 flagsWord __asm__("a2");
    register MemoryBlock* blockArg __asm__("a0");
    register void* bucketArg __asm__("a1");
    register void* ptnArg __asm__("a3");
    s32 ptnWords;
    if ((u32)size < 4) size = 4;
    align = alignment;
    if (align < 4) align = 4;
    size = (size + 3) & ~3;
    classResult = allocRequestedClass(ptn, size);
    count = *(u16*)((u8*)ptn + 4);
    __asm__(""::"r"(count));
    __asm__("" : "=r"(index) : "0"(classResult-1));
    offset = index * 8;
    words = size >> 2;
    __asm__(""::"r"(offset));
    bucket = (MemoryBucket*)(buckets + offset);
    if(index < count) {
    limit = count;
    __asm__(""::"r"(limit));
    do {
        block = bucket->first;
        if (block != 0) {
            register u32 freeMask __asm__("a3") = 0xffffff;
            s32 mask = align - 1;
            __asm__("" : : "r"(freeMask));
            do {
                u32 available = *(u32*)block & freeMask;
                if (available >= (u32)words) {
                    end = (u32)&block->freePrevious + available * 4;
                    result = (s32)(block + 1);
                    result = (result + mask) & ~mask;
                    split = (MemoryBlock*)(result + size);
                    if (end >= (u32)split) goto found;
                }
                block = block->freeNext;
            } while (block);
        }
        index++; bucket++;
    }while(index < limit);
    }
    return 0;
found:
    ((MemoryBlock**)result)[-1] = block;
    {
        MemoryBlock* previous = (MemoryBlock*)(block->unk14 * 4);
        MemoryBlock* next = block->freeNext;
        if (previous) previous->freeNext = next;
        else bucket->first = next;
        if (next) next->unk14 = block->unk14;
        else bucket->last = previous;
    }
    block->allocated = 1;
    block->isPartition = 0;
    block->requested = words;
    alignBits = 0;
    while ((shifted = align >> (alignBits + 3))) alignBits++;
    block->alignment = alignBits;
    if ((u32)split + 32 < end) {
        split->previous = (s32)block >> 2;
        split->next = block->next;
        block->next = (s32)split >> 2;
        if (split->next) {
            ((MemoryBlock*)(split->next * 4))->previous = (s32)split >> 2;
        }
        remainingWords = (s32)(end - (u32)split - 32) >> 2;
        ptnLow = block->ptn0;
        ptnWords = (block->ptn2 << 16) | (block->ptn1 << 8) | ptnLow;
        flagsWord = ((u32*)split)[3];
        split->ptn0 = ptnWords;
        ptnHigh = (u32)ptnWords >> 16;
        split->ptn2 = ptnHigh;
        flagsWord &= 0xfeffffff;
        ptnWords = (s32)((u32)ptnWords >> 8);
        split->ptn1 = ptnWords;
        sizeWord = ((u32*)split)[0];
        sizeWord = (sizeWord & 0xff000000) | (remainingWords & 0xffffff);
        ((volatile u32*)split)[0] = sizeWord;
        ((volatile u32*)split)[3] = flagsWord;
        blockArg=split;
        bucketArg=(void*)buckets;
        ptnArg=ptn;
        __asm__ volatile("" : "+r"(blockArg), "+r"(bucketArg), "+r"(ptnArg));

        func_400133C8(blockArg, bucketArg, *(u16*)((u8*)ptn + 4), ptnArg);
        block->size = ((s32)split - (s32)block - 32) >> 2;
    }
    if (ptn == (MemoryBlock*)rootPtn) D_40045560 -= block->size;
    block->pc = (s32)pc >> 2;
    block->callbackLow = 0;
    ((u8*)block)[0x13] = 0;
    block->callback1 = 0;
    block->callback2 = 0;
    return (void*)result;
}


/* Relocate a garbage allocation and coalesce its free tail. */
void* func_40013A78(FreeMemoryBlock* block, FreeMemoryBlock* oldBlock, void* inputPartition)
{
    register void* partition __asm__("s7")=inputPartition;
    void* buckets;
    register void* end __asm__("s6");
    s32 callback;
    register s32 requestedBytes __asm__("s1");
    register s32 wholeBytes __asm__("s0");
    register s32 tailBytes __asm__("s4");
    s32 mask;
    register void* dst __asm__("s5");
    register void* src __asm__("s3");
    register FreeMemoryBlock* split __asm__("s3");


    u32 partitionWord;
    {
        register u32 lowMask __asm__("v0") = 0xFFFFFF;
        register u32 word __asm__("v1");
        register u32 bucketWord __asm__("a0");
        register s32 size __asm__("a2");
        register void* root __asm__("v0");
        register s32 sizeCopy __asm__("v1");
        register s32 index __asm__("a0");
        /* This opaque register value is used only by scheduling constraints.
         * The real bucket address replaces it before any memory access. */
        __asm__("" : "=r"(buckets) : "r"(lowMask));
        __asm__("" :: "r"(buckets));
        __asm__("" :: "r"(lowMask));
        word=block->unk0;
        __asm__("" :: "r"(word));
        bucketWord=((FreeMemoryBlock*)partition)->unk0;
        __asm__("" :: "r"(bucketWord));
        word &= lowMask;
        __asm__("" :: "r"(word));
        size=word<<2;
        __asm__("" :: "r"(size));
        bucketWord &= lowMask;
        root=rootPtn;
        __asm__("" :: "r"(root));
        buckets=(void*)(bucketWord<<2);
        __asm__("" :: "r"(root), "r"(buckets));
        sizeCopy=size;

        if(partition==root){
            if(size<=1024){
                register s32 biased __asm__("v0")=size+1023;
                __asm__ volatile("" : "+r"(biased));
                { register s32 tiny __asm__("v0")=size<=128; __asm__("" :: "r"(tiny)); if(tiny){index=D_40045570[size];goto first_index;} }
                index=8;
                if(size>192){if(size>256){register s32 smaller __asm__("v1")=size<=512;__asm__(""::"r"(smaller));index=smaller?10:11;}else index=9;}
                goto first_index;
            }else{
                register s32 biased __asm__("v0")=size+1023;
                register s32 adjusted __asm__("a0");
                register s32 quotient __asm__("a2");
                register s32 count __asm__("v1");
                __asm__ volatile("" : "+r"(biased));
                adjusted=size;
                __asm__("" : "+r"(adjusted));
                if(size<0)adjusted=biased;
                quotient=adjusted>>10;
                __asm__("" ::: "v1");
                count=12;
                if(quotient&0xFF00){quotient=adjusted>>18;count=20;}
                while(quotient>>=1)count++;
                index=count;
                goto first_index;
            }
        }else{
            register s32 quotient __asm__("a2")=(u32)sizeCopy>>11;
            register s32 count __asm__("v1")=1;
            if(quotient)do{quotient>>=1;count++;}while(quotient);
            index=count;
        }
first_index:
        {
            register u32 previousWord __asm__("v0")=block->unk14;
            register u32 mask __asm__("v1");
            register FreeMemoryBucket* bucket __asm__("v1");
            register FreeMemoryBlock* next __asm__("a2");
            __asm__("" :: "r"(previousWord));
            mask=0xFFFFFF; __asm__("" :: "r"(mask));
            index--;
            __asm__("" :: "r"(index));
            previousWord &= mask;
            __asm__("" :: "r"(previousWord));
            index <<=3;
            __asm__("" :: "r"(index));
            previousWord <<=2;
            __asm__("" :: "r"(previousWord));
            bucket=(FreeMemoryBucket*)(buckets+index);
            __asm__("" :: "r"(bucket));
            dst=(void*)block+36;
            __asm__("" :: "r"(dst));
            next=block->unk1C;
            if(previousWord)((FreeMemoryBlock*)previousWord)->unk1C=next;
            else bucket->first=next;
            if(next)next->value14=block->value14;
            else bucket->last=(FreeMemoryBlock*)previousWord;
        }
    }

    {
        register u32 sourceWord __asm__("v0");
        register u32 maskWord __asm__("v1");
        register u32 auxiliary __asm__("a0");
        register u32 flags __asm__("a1");
        register u32 requestedWord __asm__("a2");
        register u32 shiftWord __asm__("a3");
        register u32 low14 __asm__("t0");
        register u32 lowPc __asm__("t1");
        register u32 flagMask __asm__("t2");
        register u32 highMask __asm__("t3");
        register u32 lowMask __asm__("t5");
        sourceWord = oldBlock->unk0;
        __asm__("" : : "r"(sourceWord));
        lowMask = 0x00FF0000;
        __asm__("" : : "r"(lowMask));
        lowMask |= 0xFFFF;
        __asm__("" : : "r"(lowMask));
        maskWord = (u32)oldBlock + 0x20;
        __asm__("" : : "r"(maskWord));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        auxiliary = block->unk0;
        __asm__("" : : "r"(auxiliary));
        sourceWord <<= 2;
        __asm__("" : : "r"(sourceWord));

        highMask = 0xFF000000;
        __asm__("" : : "r"(highMask));
        end = (void*)(maskWord + sourceWord);
        __asm__("" : : "r"(end));
        maskWord = block->unk8;
        __asm__("" : : "r"(maskWord));
        sourceWord = (u32)end - (u32)block;
        __asm__("" : : "r"(sourceWord));
        auxiliary &= highMask;
        __asm__("" : : "r"(auxiliary));
        sourceWord -= 0x20;
        __asm__("" : : "r"(sourceWord));
        maskWord &= highMask;
        __asm__("" : : "r"(maskWord));
        sourceWord = (s32)sourceWord >> 2;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        auxiliary |= sourceWord;
        __asm__("" : : "r"(auxiliary));
        block->unk0 = auxiliary;
        sourceWord = oldBlock->unk8;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        maskWord |= sourceWord;
        __asm__("" : : "r"(maskWord));
        block->unk8 = maskWord;
        if (sourceWord != 0) {
            sourceWord = oldBlock->unk8; __asm__("" : : "r"(sourceWord));
            auxiliary = oldBlock->unkC; __asm__("" : : "r"(auxiliary));
            sourceWord &= lowMask; __asm__("" : : "r"(sourceWord));
            sourceWord <<= 2; __asm__("" : : "r"(sourceWord));
            auxiliary &= lowMask; __asm__("" : : "r"(auxiliary));
            maskWord = ((FreeMemoryBlock*)sourceWord)->unkC; __asm__("" : : "r"(maskWord));
            maskWord &= highMask; __asm__("" : : "r"(maskWord));
            maskWord |= auxiliary; __asm__("" : : "r"(maskWord));
            ((FreeMemoryBlock*)sourceWord)->unkC = maskWord;
        }
        maskWord = ((u8*)oldBlock)[15];
        __asm__("" : : "r"(maskWord));
        sourceWord = 0xFEFF0000;
        __asm__("" : : "r"(sourceWord));
        flags = block->unkC;
        __asm__("" : : "r"(flags));
        sourceWord |= 0xFFFF;
        __asm__("" : : "r"(sourceWord));
        maskWord &= 1;
        __asm__("" : : "r"(maskWord));
        auxiliary = 0xE3FF0000;
        __asm__("" : : "r"(auxiliary));
        flags &= sourceWord;
        __asm__("" : : "r"(flags));
        maskWord <<= 24;
        __asm__("" : : "r"(maskWord));
        flags |= maskWord;
        __asm__("" : : "r"(flags));
        maskWord = 0xFDFF0000;
        __asm__("" : : "r"(maskWord));
        block->unkC = flags;
        shiftWord = 0x1C000000;
        __asm__("" : : "r"(shiftWord));
        auxiliary |= 0xFFFF;
        __asm__("" : : "r"(auxiliary));
        requestedWord = block->unk4;
        __asm__("" : : "r"(requestedWord));
        sourceWord = oldBlock->unkC;
        __asm__("" : : "r"(sourceWord));
        flags &= auxiliary;
        __asm__("" : : "r"(flags));
        requestedWord &= highMask;
        __asm__("" : : "r"(requestedWord));
        maskWord |= 0xFFFF;
        __asm__("" : : "r"(maskWord));
        sourceWord &= shiftWord;
        __asm__("" : : "r"(sourceWord));
        shiftWord = 1; __asm__("" : : "r"(shiftWord));
        flags |= sourceWord;
        __asm__("" : : "r"(flags));
        flagMask = 0x02000000;
        __asm__("" : : "r"(flagMask));
        block->unkC = flags;
        flags &= maskWord;
        __asm__("" : : "r"(flags));
        auxiliary = block->unk10;
        __asm__("" : : "r"(auxiliary));
        maskWord = 0x9FFF0000;
        __asm__("" : : "r"(maskWord));
        sourceWord = oldBlock->unk4;
        __asm__("" : : "r"(sourceWord));
        maskWord |= 0xFFFF;
        __asm__("" : : "r"(maskWord));
        auxiliary &= highMask;
        __asm__("" : : "r"(auxiliary));
        low14 = block->unk14;
        __asm__("" : : "r"(low14));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        lowPc = block->unk18;
        __asm__("" : : "r"(lowPc));
        requestedWord |= sourceWord;
        __asm__("" : : "r"(requestedWord));
        low14 &= highMask;
        __asm__("" : : "r"(low14));
        block->unk4 = requestedWord;
        lowPc &= highMask;
        __asm__("" : : "r"(lowPc));
        src = (void*)oldBlock + 0x24; __asm__("" : : "r"(src));
        highMask = (u32)-16; __asm__("" : : "r"(highMask));
        sourceWord = oldBlock->unkC;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= flagMask;
        __asm__("" : : "r"(sourceWord));
        flags |= sourceWord;
        __asm__("" : : "r"(flags));
        block->unkC = flags;
        flags &= maskWord;
        __asm__("" : : "r"(flags));
        sourceWord = oldBlock->unk10;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        auxiliary |= sourceWord;
        __asm__("" : : "r"(auxiliary));
        block->unk10 = auxiliary;
        sourceWord = oldBlock->unk14;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        low14 |= sourceWord;
        __asm__("" : : "r"(low14));
        block->unk14 = low14;
        sourceWord = oldBlock->unk18;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= lowMask;
        __asm__("" : : "r"(sourceWord));
        lowPc |= sourceWord;
        __asm__("" : : "r"(lowPc));
        block->unk18 = lowPc;
        maskWord = oldBlock->unkC;
        __asm__("" : : "r"(maskWord));
        sourceWord = oldBlock->callback2;
        __asm__("" : : "r"(sourceWord));
        requestedWord = oldBlock->callback0;
        __asm__("" : : "r"(requestedWord));
        maskWord >>= 29;
        __asm__("" : : "r"(maskWord));
        auxiliary = oldBlock->callback1;
        __asm__("" : : "r"(auxiliary));
        maskWord &= 3;
        __asm__("" : : "r"(maskWord));
        sourceWord <<= 18;
        __asm__("" : : "r"(sourceWord));
        requestedWord <<= 2;
        __asm__("" : : "r"(requestedWord));
        auxiliary <<= 10;
        __asm__("" : : "r"(auxiliary));
        sourceWord |= maskWord;
        __asm__("" : : "r"(sourceWord));
        requestedWord |= auxiliary;
        __asm__("" : : "r"(requestedWord));
        maskWord = sourceWord << 29;
        __asm__("" : : "r"(maskWord));
        sourceWord |= requestedWord;
        __asm__("" : : "r"(sourceWord));
        flags |= maskWord;
        __asm__("" : : "r"(flags));
        auxiliary = (s32)sourceWord >> 18;
        __asm__("" : : "r"(auxiliary));
        maskWord = (s32)sourceWord >> 2;
        __asm__("" : : "r"(maskWord));
        sourceWord = (s32)sourceWord >> 10;
        __asm__("" : : "r"(sourceWord));
        block->unkC = flags;
        block->callback0 = maskWord; __asm__("" : : : "memory");
        block->callback1 = sourceWord; __asm__("" : : : "memory");
        block->callback2 = auxiliary;
        sourceWord = oldBlock->unkC;
        __asm__("" : : "r"(sourceWord));
        maskWord = oldBlock->unk4;
        __asm__("" : : "r"(maskWord));
        sourceWord >>= 26;
        __asm__("" : : "r"(sourceWord));
        sourceWord &= 7;
        __asm__("" : : "r"(sourceWord));
        maskWord &= lowMask;
        __asm__("" : : "r"(maskWord));
        sourceWord += 2;
        __asm__("" : : "r"(sourceWord));
        requestedBytes = maskWord << 2;
        __asm__("" : : "r"(requestedBytes));
        shiftWord <<= sourceWord;
        __asm__("" : : "r"(shiftWord));
        wholeBytes = requestedBytes & highMask;
        __asm__("" : : "r"(wholeBytes));
        __asm__("" : "+r"(requestedBytes));
        shiftWord--;
        __asm__("" : : "r"(shiftWord));
        tailBytes = requestedBytes & 15;
        __asm__("" : : "r"(tailBytes));
        sourceWord = (u32)dst + shiftWord;
        __asm__("" : : "r"(sourceWord));
        maskWord = ~shiftWord;
        __asm__("" : : "r"(maskWord));
        dst = (void*)(sourceWord & maskWord);
        __asm__("" : : "r"(dst));
        shiftWord = (u32)src + shiftWord;
        __asm__("" : : "r"(shiftWord));
        sourceWord = (u32)dst & 15; __asm__("" : : "r"(sourceWord));
        src = (void*)(shiftWord & maskWord);
    if (sourceWord != 0 || ((s32)src & 0xF)) { __asm__("" : : "r"(src)); memmove(dst, src, requestedBytes); }
    else {
        __asm__("" : : "r"(src));
        FlushCache(0);
        if (wholeBytes != 0) func_40013160((s32)dst, (s32)src, wholeBytes);
        if (tailBytes != 0) memmove(dst + wholeBytes, src + wholeBytes, tailBytes);
    }
    }
    split = dst + requestedBytes;
    ((FreeMemoryBlock**)dst)[-1] = block;
    if ((u32)((void*)split + 0x20) < (u32)end) {
        register u32 word __asm__("v0");
        register u32 value __asm__("v1");
        register u32 limit __asm__("a0");
        register u32 low __asm__("a1");
        register u32 high __asm__("a2");
        register FreeMemoryBlock* next __asm__("a3");
        register u32 splitLow __asm__("t0");
        register s32 splitWord __asm__("t1");
        register u32 bucketCount __asm__("t2");
        register u32 maskLow __asm__("s0");
        register u32 maskHigh __asm__("s1");
        value = split->unkC;
        __asm__("" : : "r"(value));
        low = 0x00FF0000;
        __asm__("" : : "r"(low));
        low |= 0xFFFF;
        __asm__("" : : "r"(low));
        high = 0xFF000000;
        __asm__("" : : "r"(high));
        word = (s32)block >> 2;
        __asm__("" : : "r"(word));
        value &= high;
        __asm__("" : : "r"(value));
        word &= low;
        __asm__("" : : "r"(word));
        limit = block->unk8;
        __asm__("" : : "r"(limit));
        value |= word;
        __asm__("" : : "r"(value));
        bucketCount = *(u16*)(partition + 4);
        __asm__("" : : "r"(bucketCount));
        split->unkC = value;
        splitWord = (s32)split >> 2;
        __asm__("" : : "r"(splitWord));
        value = split->unk8;
        __asm__("" : : "r"(value));
        limit &= low;
        __asm__("" : : "r"(limit));
        word = block->unk8;
        __asm__("" : : "r"(word));
        splitLow = splitWord & low;
        __asm__("" : : "r"(splitLow));
        value &= high;
        __asm__("" : : "r"(value));
        next = (FreeMemoryBlock*)(limit << 2);
        __asm__("" : : "r"(next));
        word &= low;
        __asm__("" : : "r"(word));
        limit = (u32)end;
        __asm__("" : : "r"(limit));
        value |= word;
        __asm__("" : : "r"(value));
        split->unk8 = value;
        word = block->unk8;
        __asm__("" : : "r"(word));
        word &= high;
        __asm__("" : : "r"(word));
        word |= splitLow;
        __asm__("" : : "r"(word));
        block->unk8 = word;
        if (next != NULL) {
            value = next->unkC; __asm__("" : : "r"(value));
            word = value >> 24; __asm__("" : : "r"(word));
            word &= 1; __asm__("" : : "r"(word));
            if (word != 0) {
                word = value & high; __asm__("" : : "r"(word));
                word |= splitLow; __asm__("" : : "r"(word));
                next->unkC = word;
                        } else {
                {
            register s32 index __asm__("a0");
            register s32 rawSize __asm__("v0") = next->unk0;
            register s32 originalSize __asm__("v0");
            register s32 smallResult __asm__("a1");
            rawSize &= low;
            index = rawSize << 2;
            __asm__("" : "+r"(index));
            originalSize = index;
            if (partition == rootPtn) {
                if (index <= 1024) {
                    if (index <= 128) {
                        /* A fresh assembler fragment preserves the symbolic LB
                         * macro's HI16 opcode; the directive emits no bytes. */
                        __asm__ volatile(".org .");
                        index = D_40045570[index];
                        goto nclassDone;
                    }
                    smallResult = 8;
                    if (index <= 192) goto nsmallReady;
                    if (index > 256) {
                        register s32 smaller __asm__("v1") = index <= 512;
                        __asm__("" : : "r"(smaller));
                        smallResult = smaller ? 10 : 11;
                    } else smallResult = 9;
nsmallReady:
                    index = smallResult;
                    goto nclassDone;
                } else {
                    register s32 extra __asm__("v0") = index + 1023;
                    register s32 numerator __asm__("a1");
                    register s32 count __asm__("v1");

                    __asm__("" : "+r"(extra), "+r"(index));

                    numerator = index;
                    __asm__("" : "+r"(numerator));
                    if (index < 0) numerator = extra;
                    index = numerator >> 10;
                    __asm__("" : : : "v1");
                    count = 12;
                    if (index & 0xFF00) { index = numerator >> 18; count = 20; }
                    while (index >>= 1) count++;
                    index = count;
                    goto nclassDone;
                }
            } else {
                register s32 count __asm__("v1") = 1;
                index = (u32)originalSize >> 11;
                if (index) { do { index >>= 1; count++; } while (index); }
                index = count;
            }
nclassDone: ;
                {
                    register u32 encodedPrevious __asm__("v0") = next->unk14;
                    register u32 mask __asm__("v1");
                    register FreeMemoryBucket* bucket __asm__("v1");
                    register FreeMemoryBlock* freePrevious __asm__("v0");
                    register FreeMemoryBlock* freeNext __asm__("a2");
                    __asm__("" : : "r"(encodedPrevious), "r"(index));
                    mask = 0x00FF0000; __asm__("" : : "r"(mask));
                    mask |= 0xFFFF; __asm__("" : : "r"(mask));
                    index <<= 3; __asm__("" : : "r"(index));
                    encodedPrevious &= mask; __asm__("" : : "r"(encodedPrevious));
                    index = (u32)buckets + index; __asm__("" : : "r"(index));
                    freePrevious = (FreeMemoryBlock*)(encodedPrevious << 2); __asm__("" : : "r"(freePrevious));
                    bucket = (FreeMemoryBucket*)(index - 8); __asm__("" : : "r"(bucket));
                    freeNext = next->unk1C;
                    if (freePrevious != NULL) freePrevious->unk1C = freeNext;
                    else bucket->first = freeNext;
                    if (freeNext != NULL) freeNext->value14 = next->value14;
                    else bucket->last = freePrevious;
                }
                }
                value = next->unk8;
                __asm__("" : : "r"(value));
                low = 0x00FF0000;
                __asm__("" : : "r"(low));
                word = split->unk8;
                __asm__("" : : "r"(word));
                low |= 0xFFFF;
                __asm__("" : : "r"(low));
                high = 0xFF000000;
                __asm__("" : : "r"(high));
                value &= low;
                __asm__("" : : "r"(value));
                word &= high;
                __asm__("" : : "r"(word));
                word |= value;
                __asm__("" : : "r"(word));
                split->unk8 = word;
                if (value != 0) {
                word = next->unk8;
                __asm__("" : : "r"(word));
                limit = splitWord & low;
                __asm__("" : : "r"(limit));
                word &= low;
                __asm__("" : : "r"(word));
                word <<= 2;
                __asm__("" : : "r"(word));
                value = ((FreeMemoryBlock*)word)->unkC;
                __asm__("" : : "r"(value));
                value &= high;
                __asm__("" : : "r"(value));
                value |= limit;
                __asm__("" : : "r"(value));
                ((FreeMemoryBlock*)word)->unkC = value;
                }
                word = next->unk0;
                __asm__("" : : "r"(word));
                value = (u32)next + 0x20;
                __asm__("" : : "r"(value));
                word &= low;
                __asm__("" : : "r"(word));
                word <<= 2;
                __asm__("" : : "r"(word));
                limit = value + word;
                __asm__("" : : "r"(limit));
                        }
        }
        word = block->ptn2;
        __asm__("" : : "r"(word));
        limit -= (u32)split;
        __asm__("" : : "r"(limit));
        value = block->ptn1;
        __asm__("" : : "r"(value));
        limit -= 32;
        __asm__("" : : "r"(limit));
        low = block->ptn0;
        __asm__("" : : "r"(low));
        word <<= 16;
        __asm__("" : : "r"(word));
        value <<= 8;
        __asm__("" : : "r"(value));
        maskLow = 0x00FF0000;
        __asm__("" : : "r"(maskLow));
        word |= value;
        __asm__("" : : "r"(word));
        value = 0xFEFF0000;
        __asm__("" : : "r"(value));
        word |= low;
        __asm__("" : : "r"(word));
        low = (u32)buckets;
        __asm__("" : : "r"(low));
        split->ptn0 = word;
        maskLow |= 0xFFFF;
        __asm__("" : : "r"(maskLow));
        splitLow = split->unkC;
        __asm__("" : : "r"(splitLow));
        maskHigh = 0xFF000000;
        __asm__("" : : "r"(maskHigh));
        next = (FreeMemoryBlock*)split->unk0;
        __asm__("" : : "r"(next));
        limit = (s32)limit >> 2;
        __asm__("" : : "r"(limit));
        value |= 0xFFFF;
        __asm__("" : : "r"(value));
        limit &= maskLow;
        __asm__("" : : "r"(limit));
        next = (FreeMemoryBlock*)((u32)next & maskHigh);
        __asm__("" : : "r"(next));
        high = word >> 16;
        __asm__("" : : "r"(high));
        splitLow &= value;
        __asm__("" : : "r"(splitLow));
        next = (FreeMemoryBlock*)((u32)next | limit);
        __asm__("" : : "r"(next));
        word >>= 8;
        __asm__("" : : "r"(word));
        split->ptn2 = high;
        split->ptn1 = word; __asm__("" : : : "memory");
        limit = (u32)split;
        __asm__("" : : "r"(limit));
        split->unk0 = (u32)next;
        next = (FreeMemoryBlock*)partition;
        __asm__("" : : "r"(next));
        split->unkC = splitLow;
        func_400133C8((MemoryBlock*)limit, (void*)low, bucketCount, next);
        word = (u32)split - (u32)block; __asm__("" : : "r"(word));
        value = block->unk0; __asm__("" : : "r"(value));
        word -= 32; __asm__("" : : "r"(word));
        word = (s32)word >> 2; __asm__("" : : "r"(word));
        word &= maskLow; __asm__("" : : "r"(word));
        value &= maskHigh; __asm__("" : : "r"(value));
        value |= word; __asm__("" : : "r"(value));
        block->unk0 = value;
    }
    return dst;
}


void* iosMallocInGarbagePtn(s32 size, s32 alignment)
{
    register void* callerPc __asm__("ra");
    void* pc = callerPc;
    void* result;
    u32* header;

    WaitSema(D_4007CC78);
    alignment = alignment < 16 ? 16 : alignment;
    result = func_400135C8((void*)gbPtn, size, alignment, pc);
    if (result != NULL)
    {
        header = ((u32**)result)[-1];
        header[4] |= 0xFFFFFF;
    }
    SignalSema(D_4007CC78);
    if (result == NULL)
    {
        if (!(D_400455F4++ & 0x1F))
        {
            LoaderSysPrintf(D_40046668);
        }
        if (iosDefaultPadData[2] & 0x400)
        {
            iosDebugPartitionDump(gbPtn, 1);
        }
        if (D_400455F4 >= 0x101)
        {
            iosJumpRecoverPoint(D_40046690, size, pc);
        }
    }
    else
    {
        D_400455F4 = 0;
    }
    return result;
}

/* Merge adjacent free blocks before reinserting the combined block. */
void func_400142E0(void* ptr)
{
    void* root;
    register u32 wordMask __asm__("t1");
    register MemoryBlock* block __asm__("t0");
    register void* partition __asm__("t3");
    register MemoryBlock* next __asm__("t2");
    register MemoryBlock* previous __asm__("a3");
    register void* buckets __asm__("t4");
    register u32 mergeMask __asm__("a1");
    register u32 ptnWord __asm__("v0");
    register u32 ptnMiddle __asm__("v1");
    register u32 ptnLow __asm__("a2");
    register u32 nextWord __asm__("a1");
    register u32 previousWord __asm__("a0");
    wordMask = 0xFFFFFF;
    __asm__("" : : "r"(wordMask));
    root = rootPtn;
    block = ((MemoryBlock**)ptr)[-1];
    __asm__("" : : "r"(block), "r"(root));
    ptnWord = ((u8*)block)[11];
    ptnMiddle = ((u8*)block)[7];
    ptnWord <<= 16;
    __asm__("" : : "r"(ptnWord));
    ptnLow = ((u8*)block)[3];
    ptnMiddle <<= 8;
    __asm__("" : : "r"(ptnMiddle));
    nextWord = ((u32*)block)[2];
    __asm__("" : : "r"(nextWord));
    ptnWord |= ptnMiddle;
    __asm__("" : : "r"(ptnWord));
    previousWord = ((u32*)block)[3];
    __asm__("" : : "r"(previousWord));
    ptnWord |= ptnLow;
    __asm__("" : : "r"(ptnWord));
    nextWord &= wordMask;
    __asm__("" : : "r"(nextWord));
    partition = (void*)(ptnWord << 2);
    __asm__("" : : "r"(partition));
    previousWord &= wordMask;
    __asm__("" : : "r"(previousWord));
    ptnWord = ((u32*)partition)[0];
    __asm__("" : : "r"(ptnWord));
    next = (MemoryBlock*)(nextWord << 2);
    previous = (MemoryBlock*)(previousWord << 2);
    ptnWord &= wordMask;
    buckets = (void*)(ptnWord << 2);
    __asm__("" : "+r"(partition), "+r"(next), "+r"(previous) : "r"(wordMask));
    if (partition == root) {
        register s32* counter __asm__("a0") = &D_40045560;
        register u32 sizeWord __asm__("v1") = ((u32*)block)[0];
        register s32 count __asm__("v0");
        __asm__("" : : "r"(sizeWord));
        count = *counter;
        sizeWord &= wordMask;
        *counter = count + sizeWord;
    }
    if (previous != NULL && !previous->allocated) {
        {
            register u32 highMask __asm__("a0") = 0xFF000000;
            register u32 source __asm__("v1") = ((u32*)block)[2];
            register u32 dest __asm__("v0") = ((u32*)previous)[2];
            __asm__("" : : "r"(source), "r"(dest));
            source &= wordMask;
            dest &= highMask;
            ((u32*)previous)[2] = dest | source;
            if (next != NULL) {
                source = ((u32*)block)[3];
                dest = ((u32*)next)[3];
                __asm__("" : : "r"(source), "r"(dest));
                source &= wordMask;
                __asm__("" : : "r"(source));
                dest &= highMask;
                ((u32*)next)[3] = dest | source;
            }
        }
        __asm__("" : : "r"(wordMask));
        {
            register s32 index __asm__("a0");
            register s32 rawSize __asm__("v0") = ((u32*)previous)[0];
            register s32 originalSize __asm__("v0");
            register s32 smallResult __asm__("a1");
            rawSize &= wordMask;
            index = rawSize << 2;
            __asm__("" : : "r"(index));
            originalSize = index;
            if (partition == root) {
                if (index <= 1024) {
                    if (index <= 128) { index = D_40045570[index]; goto pclassDone; }
                    smallResult = 8;
                    if (index <= 192) goto psmallReady;
                    if (index > 256) {
                        register s32 smaller __asm__("v1") = index <= 512;
                        __asm__("" : : "r"(smaller));
                        smallResult = smaller ? 10 : 11;
                    } else smallResult = 9;
psmallReady:
                    index = smallResult;
                    goto pclassDone;
                } else {
                    register s32 extra __asm__("v0") = index + 1023;
                    register s32 numerator __asm__("a1");
                    register s32 count __asm__("v1");
                    __asm__("" : "+r"(extra), "+r"(index));
                    numerator = index;
                    __asm__("" : "+r"(numerator));
                    if (index < 0) numerator = extra;
                    {
                    index = numerator >> 10;
                    __asm__("" : : : "v1");
                    count = 12;
                    if (index & 0xFF00) { index = numerator >> 18; count = 20; }
                    while (index >>= 1) count++;
                    index = count;
                    goto pclassDone;
                    }
                }
            } else {
                register s32 count __asm__("v1") = 1;
                index = (u32)originalSize >> 11;
                if (index) { do { index >>= 1; count++; } while (index); }
                index = count;
            }
pclassDone: ;
            {
            register u32 encodedPrevious __asm__("v0") = ((u32*)previous)[5];
            register u32 mask __asm__("v1");
            register MemoryBucket* bucket __asm__("v1");
            register MemoryBlock* freePrevious __asm__("v0");
            register MemoryBlock* freeNext __asm__("a2");
            __asm__("" : : "r"(encodedPrevious), "r"(index));
            mask = 0xFFFFFF;
            __asm__("" : : "r"(mask));
            index--;
            __asm__("" : : "r"(index));
            encodedPrevious &= mask;
            __asm__("" : : "r"(encodedPrevious));
            index <<= 3;
            __asm__("" : : "r"(index));
            freePrevious = (MemoryBlock*)(encodedPrevious << 2);
            __asm__("" : : "r"(freePrevious));
            bucket = (MemoryBucket*)((s32)index + (s32)buckets);
            __asm__("" : : "r"(bucket), "r"(freePrevious));
            freeNext = previous->freeNext;
            if (freePrevious != NULL) freePrevious->freeNext = freeNext;
            else bucket->first = freeNext;
            if (freeNext != NULL) freeNext->unk14 = previous->unk14;
            else bucket->last = freePrevious;
            }
        }

        {
            register s32 sizeWord __asm__("v0") = ((u32*)block)[0];
            register u32 mask __asm__("a1");
            register u32 previousPacked __asm__("v1");
            register u32 highMask __asm__("a0");
            __asm__("" : : "r"(sizeWord));
            mask = 0xFFFFFF;
            __asm__("" : : "r"(mask));
            previousPacked = ((u32*)previous)[0];
            __asm__("" : : "r"(previousPacked));
            sizeWord &= mask;
            __asm__("" : : "r"(sizeWord));
            highMask = 0xFF000000;
            __asm__("" : : "r"(highMask));
            sizeWord <<= 2;
            __asm__("" : : "r"(sizeWord), "r"(highMask));
            previousPacked &= highMask;
            __asm__("" : : "r"(previousPacked));
            sizeWord = (s32)block + sizeWord;
            __asm__("" : : "r"(sizeWord));
            block = previous;
            __asm__("" : "+r"(block), "+r"(sizeWord));
            sizeWord -= (s32)previous;
            sizeWord >>= 2;
            sizeWord &= mask;
            previousPacked |= sizeWord;
            __asm__("" : : "r"(previousPacked));
            ((u32*)previous)[0] = previousPacked;
        }
    }
    if (next != NULL && !next->allocated) {
        {
            register u32 source __asm__("v1") = ((u32*)next)[2];
            register u32 highMask __asm__("a2");
            register u32 dest __asm__("v0");
            register u32 destSize __asm__("a0");
            __asm__("" : : "r"(source));
            mergeMask = 0xFF0000;
            __asm__("" : : "r"(mergeMask));
            dest = ((u32*)block)[2];
            __asm__("" : : "r"(dest));
            mergeMask |= 0xFFFF;
            __asm__("" : : "r"(mergeMask));
            highMask = 0xFF000000;
            __asm__("" : : "r"(highMask));
            source &= mergeMask;
            __asm__("" : : "r"(source));
            dest &= highMask;
            __asm__("" : : "r"(dest));
            destSize = ((u32*)block)[0];
            __asm__("" : : "r"(destSize));
            dest |= source;
            ((u32*)block)[2] = dest;
            __asm__("" : : : "memory");
            destSize &= highMask;
            __asm__("" : : "r"(destSize));
            dest = ((u32*)next)[0];
            __asm__("" : : "r"(dest));
            dest &= mergeMask;
            dest <<= 2;
            dest = (s32)next + dest;
            dest -= (s32)block;
            dest = (s32)dest >> 2;
            dest &= mergeMask;
            destSize |= dest;
            __asm__("" : : "r"(destSize));
            ((u32*)block)[0] = destSize;
            dest = ((u32*)next)[2] & mergeMask;
            destSize = dest << 2;
            if (dest) {
                source = ((u32*)next)[3];
                dest = ((u32*)destSize)[3];
                __asm__("" : : "r"(source), "r"(dest));
                source &= mergeMask;
                __asm__("" : : "r"(source));
                dest &= highMask;
                dest |= source;
                ((u32*)destSize)[3] = dest;
            }
        }
        {
            register s32 index __asm__("a0");
            register s32 rawSize __asm__("v0") = ((u32*)next)[0];
            register void* newRoot __asm__("v1");
            register s32 originalSize __asm__("v0");
            register s32 smallResult __asm__("a1");
            __asm__("" : : "r"(rawSize));
            newRoot = rootPtn;
            __asm__("" : : "r"(newRoot));
            rawSize &= mergeMask;
            index = rawSize << 2;
            __asm__("" : : "r"(index));
            originalSize = index;
            if (partition == newRoot) {
                if (index <= 1024) {
                    if (index <= 128) { index = D_40045570[index]; goto nclassDone; }
                    smallResult = 8;
                    if (index <= 192) goto nsmallReady;
                    if (index > 256) {
                        register s32 smaller __asm__("v1") = index <= 512;
                        __asm__("" : : "r"(smaller));
                        smallResult = smaller ? 10 : 11;
                    } else smallResult = 9;
nsmallReady:
                    index = smallResult;
                    goto nclassDone;
                } else {
                    register s32 extra __asm__("v0") = index + 1023;
                    register s32 numerator __asm__("a1");
                    register s32 count __asm__("v1");
                    __asm__("" : "+r"(extra), "+r"(index));
                    numerator = index;
                    __asm__("" : "+r"(numerator));
                    if (index < 0) numerator = extra;
                    {
                    index = numerator >> 10;
                    __asm__("" : : : "v1");
                    count = 12;
                    if (index & 0xFF00) { index = numerator >> 18; count = 20; }
                    while (index >>= 1) count++;
                    index = count;
                    goto nclassDone;
                    }
                }
            } else {
                register s32 count __asm__("v1") = 1;
                index = (u32)originalSize >> 11;
                if (index) { do { index >>= 1; count++; } while (index); }
                index = count;
            }
nclassDone: ;
            {
            register u32 encodedPrevious __asm__("v0") = ((u32*)next)[5];
            register u32 mask __asm__("v1");
            register MemoryBucket* bucket __asm__("v1");
            register MemoryBlock* freePrevious __asm__("v0");
            register MemoryBlock* freeNext __asm__("a2");
            __asm__("" : : "r"(encodedPrevious), "r"(index));
            mask = 0xFFFFFF;
            __asm__("" : : "r"(mask));
            index--;
            __asm__("" : : "r"(index));
            encodedPrevious &= mask;
            __asm__("" : : "r"(encodedPrevious));
            index <<= 3;
            __asm__("" : : "r"(index));
            freePrevious = (MemoryBlock*)(encodedPrevious << 2);
            __asm__("" : : "r"(freePrevious));
            bucket = (MemoryBucket*)((s32)index + (s32)buckets);
            __asm__("" : : "r"(bucket), "r"(freePrevious));
            freeNext = next->freeNext;
            if (freePrevious != NULL) freePrevious->freeNext = freeNext;
            else bucket->first = freeNext;
            if (freeNext != NULL) freeNext->unk14 = next->unk14;
            else bucket->last = freePrevious;
            }
        }

    }
    {
        register u32 attrs __asm__("v1") = ((u32*)block)[3];
        register u32 clearMask __asm__("v0") = 0xFEFFFFFF;
        register MemoryBlock* argBlock __asm__("a0");
        register void* argBuckets __asm__("a1");
        register void* argPartition __asm__("a3");
        register s32 argCount __asm__("a2");
        attrs &= clearMask;
        __asm__("" : "+r"(attrs) : : "a0", "a1", "a2", "a3");
        argBlock = block;
        __asm__("" : "+r"(argBlock), "+m"(((u32*)block)[3]) : "r"(attrs) : "a1", "a2", "a3");
        ((volatile u32*)block)[3] = attrs;
        argBuckets = buckets;
        argPartition = partition;
        __asm__("" : "+r"(argPartition) : "r"(argBuckets), "r"(attrs), "m"(((u32*)block)[3]) : "a2");
        argCount = *(u16*)(partition + 4);
        func_400133C8(argBlock, argBuckets, argCount, argPartition);
    }
}


void* func_40014770(void* ptr, void* start, s32 size)
{
    register void* oldPtr __asm__("t6") = ptr;
    register s32 alignment __asm__("t0") = size;
    register s32 result __asm__("s0") = size;
    register FreeMemoryBlock* block __asm__("t2") = ((FreeMemoryBlock**)oldPtr)[-1];
    register void* data __asm__("t3") = start;
    s32 requestedBytes;
    s32 rawSize;
    s32 oldSize;
    register FreeMemoryBlock* split __asm__("t1");
    void* base;
    u32 next;
    u32 splitWord;
    u32 partition;
    s32 callbackLow;
    s32 callback;
    s32 leftSize;
    s32 remain;
    if (alignment < 4) alignment = 4;
    {
        register u32 requestWord __asm__("v0") = block->unk4;
        requestedBytes = (requestWord & 0xFFFFFF) << 2;
    }
    rawSize = block->unk0;
    oldSize = rawSize & 0xFFFFFF;
    if (result == 0) { __asm__("" : "+r"(data)); split = data; data += 0x24; }
    else { split = data - 0x24; }
    base = (void*)block + 0x20;
    { register void* limit __asm__("v1") = (void*)block + 0x24; if ((u32)split < (u32)limit) return NULL; }
    { register void* limit __asm__("v1") = base + ((rawSize & 0xFFFFFF) << 2); if ((u32)data >= (u32)limit) return NULL; }
    next = block->next;
    split->previous = (s32)block >> 2;
    splitWord = (s32)split >> 2;
    block->next = splitWord;
    split->next = next;
    if (next != 0) ((FreeMemoryBlock*)(next << 2))->previous = splitWord;
    leftSize = (s32)((void*)split - (void*)block - 0x20) >> 2;
    partition = (block->ptn2 << 16) | (block->ptn1 << 8) | block->ptn0;
    split->ptn0 = partition;
    split->ptn1 = partition >> 8;
    split->ptn2 = partition >> 16;
    split->size = (s32)((base + (block->size << 2)) - (void*)split - 0x20) >> 2;
    block->size = leftSize;
    if ((((block->ptn2 << 16) | (block->ptn1 << 8) | block->ptn0) << 2) == (u32)rootPtn) {
        remain = D_40045560 + oldSize;
        *(volatile s32*)&D_40045560 = remain;
        *(volatile s32*)&D_40045560 = remain - (split->size + block->size);
    }
    callbackLow = (block->callback2 << 18) | block->callbackLow;
    callback = callbackLow | ((block->callback0 << 2) | (block->callback1 << 10));
    split->unkC = (split->unkC & 0x9FFFFFFF) | (callbackLow << 29);
    split->callback0 = callback >> 2;
    split->callback1 = callback >> 10;
    split->callback2 = callback >> 18;
    ((FreeMemoryBlock**)data)[-1] = split;
    if (result != 0) {
        register s32 exponent __asm__("v1");
        register s32 shifted __asm__("v0");
        register u32 word10 __asm__("a1");
        register u32 word14 __asm__("a2");
        register u32 word4 __asm__("a0");
        register u32 lowMask __asm__("a3");
        register u32 highMask __asm__("t0");
        void* requestedEnd;
        result = (s32)data;
        exponent = 0;
        while ((shifted = alignment >> (exponent + 3))) exponent++;
        word4 = (split->unkC & 0xE3FFFFFF) | ((exponent & 7) << 26);
        /* Empty constraints preserve EE-GCC register lifetimes and scheduling. */
        {
        register u32 sourceWord __asm__("v1");
        register u32 requestedWord __asm__("v0");
        requestedEnd = oldPtr + requestedBytes;
        word10 = split->unk10;
        __asm__ volatile("" : : "r"(word10));
        lowMask = 0xFFFFFF;
        split->unkC = word4;
        __asm__ volatile("" : : "r"(lowMask));
        highMask = 0xFF000000;
        __asm__ volatile("" : : "r"(highMask));
        word14 = split->unk14;
        __asm__ volatile("" : : "r"(word14));
        sourceWord = block->unk10;
        __asm__ volatile("" : : "r"(sourceWord));
        word10 &= highMask;
        __asm__ volatile("" : : "r"(word10));
        word4 = split->unk4;
        __asm__ volatile("" : : "r"(word4));
        requestedWord = (u32)requestedEnd - (u32)data;
        __asm__ volatile("" : : "r"(requestedWord));
        sourceWord &= lowMask;
        __asm__ volatile("" : : "r"(sourceWord));
        requestedWord = (s32)requestedWord >> 2;
        __asm__ volatile("" : : "r"(requestedWord));
        word10 |= sourceWord;
        __asm__ volatile("" : : "r"(word10));
        requestedWord &= lowMask;
        __asm__ volatile("" : : "r"(requestedWord));
        split->unk10 = word10;
        word10 = 0xFEFF0000;
        __asm__ volatile("" : : "r"(word10));
        word4 &= highMask;
        __asm__ volatile("" : : "r"(word4));
        word14 &= highMask;
        __asm__ volatile("" : : "r"(word14));
        sourceWord = block->unk14;
        __asm__ volatile("" : : "r"(sourceWord));
        word4 |= requestedWord;
        __asm__ volatile("" : : "r"(word4));
        split->unk4 = word4;
        word4 = (u32)oldPtr;
        __asm__ volatile("" : : "r"(word4));
        sourceWord &= lowMask;
        __asm__ volatile("" : : "r"(sourceWord));
        lowMask = 0x01000000;
        __asm__ volatile("" : : "r"(lowMask));
        word14 |= sourceWord;
        __asm__ volatile("" : : "r"(word14));
        word10 |= 0xFFFF;
        __asm__ volatile("" : : "r"(word10));
        split->unk14 = word14;
        requestedWord = block->unkC;
        __asm__ volatile("" : : "r"(requestedWord));
        requestedWord &= word10;
        __asm__ volatile("" : : "r"(requestedWord));
        block->unkC = requestedWord;
        sourceWord = split->unkC;
        __asm__ volatile("" : : "r"(sourceWord));
        sourceWord |= lowMask;
        __asm__ volatile("" : : "r"(sourceWord));
        split->unkC = sourceWord;
        }
        func_400142E0((void*)word4);
    } else {
        result = (s32)oldPtr;
        block->allocated = 1;
        split->allocated = 0;
        block->requested = (s32)((void*)split - (void*)result) >> 2;
        func_400142E0(data);
    }
    return (void*)result;
}


void* func_40014AF8(s32 partition, void* ptr, s32 size, s32 alignment)
{
    register void* callerPc __asm__("ra");
    void* pc = callerPc;
    u32 rounded;
    register void* result __asm__("s2");
    alignment = alignment < 4 ? 4 : alignment;
    rounded = (size + 3) & ~3;
    if (ptr != NULL) {
        MemoryBlock* block = ((MemoryBlock**)ptr)[-1];
        register u32 wordMask __asm__("t2");
        register s32 mask __asm__("v1");
        register s32 body __asm__("t0");
        register s32 ptn __asm__("v0");
        register s32 middle __asm__("a0");
        register s32 low __asm__("a2");
        register u32 capacity __asm__("a1");
        register s32 alignMask __asm__("a3");
        register u32 wordLoaded __asm__("t3");
        u32 oldWord;
        u32 previousSize;
        __asm__("" : : "r"(block));
        wordMask = 0xFFFFFF;
        __asm__("" : : "r"(wordMask));
        mask = alignment - 1;
        ptn = ((u8*)block)[11];
        body = (s32)(block + 1);
        middle = ((u8*)block)[7];
        alignMask = ~mask;
        capacity = ((u32*)block)[0];
        __asm__("" : : "r"(capacity));
        ptn <<= 16;
        __asm__("" : : "r"(ptn));
        middle <<= 8;
        __asm__("" : : "r"(middle));
        low = ((u8*)block)[3];
        __asm__("" : : "r"(low));
        ptn |= middle;
        __asm__("" : : "r"(ptn));
        capacity &= wordMask;
        __asm__("" : : "r"(capacity));
        wordLoaded = ((u32*)block)[1];
        __asm__("" : : "r"(wordLoaded));
        oldWord = wordLoaded;
        mask = body + mask;
        __asm__("" : : "r"(mask));
        mask &= alignMask;
        __asm__("" : : "r"(mask));
        ptn |= low;
        __asm__("" : : "r"(ptn));
        capacity <<= 2;
        __asm__("" : : "r"(capacity));
        mask -= body;
        __asm__("" : : "r"(mask));
        capacity -= 4;
        __asm__("" : : "r"(capacity));
        middle = oldWord & wordMask;
        __asm__("" : : "r"(middle));
        ptn <<= 2;
        __asm__("" : : "r"(ptn));
        capacity -= mask;
        previousSize = middle << 2;
        if (partition == ptn) {
            u32 attributes = ((u32*)block)[3];
            register s32 tooSmall __asm__("v0");
            if ((1 << (((attributes >> 26) & 7) + 2)) < alignment) goto allocate;
            tooSmall = capacity < rounded;
            if (tooSmall) goto allocate;
            {
                register s32 power __asm__("a1");
                register u32 merged __asm__("v1");
                register s32 alignmentCheck __asm__("a0");
                u32 highMask = 0xFF000000;
                u32 requested = ((s32)rounded >> 2) & wordMask;
                __asm__("" : : "r"(requested), "r"(highMask));
                merged = (oldWord & highMask) | requested;
                __asm__("" : : "r"(merged));
                alignmentCheck = alignment >> 3;
                __asm__("" : : "r"(alignmentCheck));
                ((u32*)block)[1] = merged;
                __asm__("" : : "r"(alignmentCheck) : "memory");
                power = 0;
                if (alignmentCheck != 0) {
                    do { power++; } while (alignment >> (power + 3));
                }
                {
                    register u32 maskedPower __asm__("a0") = power & 7;
                    ((u32*)block)[3] = (attributes & 0xE3FFFFFF) | (maskedPower << 26);
                    block->pc = (s32)pc >> 2;
                }
                return ptr;
            }
        }
allocate:
        result = func_400135C8((void*)partition, rounded, alignment, pc);
        memcpy(result, ptr, rounded < previousSize ? rounded : previousSize);
        {
            MemoryBlock* dest = ((MemoryBlock**)result)[-1];
            s32 lower = (((u8*)block)[27] << 18) | ((((u32*)block)[3] >> 29) & 3);
            s32 upper = (((u8*)block)[19] << 2) | (((u8*)block)[23] << 10);
            s32 callback = lower | upper;
            dest->callbackLow = lower;
            dest->callback0 = callback >> 2;
            dest->callback1 = callback >> 10;
            dest->callback2 = callback >> 18;
        }
        func_400142E0(ptr);
    } else result = func_400135C8((void*)partition, rounded, alignment, pc);
    return result;
}


/* Allocate the partition descriptor, bucket array, and initial free block. */
s32 func_40014D48(s32 partition, s32 size, void* pc)
{
    register void* allocationPc __asm__("a3");
    register s32 alignMask __asm__("v0") = -4;
    register s32 rounded __asm__("t1") = (size + 3) & alignMask;
    register s32 available __asm__("s3") = rounded + 4;
    register s32 smallInput __asm__("a1");
    register void* root __asm__("v1") = rootPtn;
    register s32 result __asm__("a2");
    s32 count;
    s32 bucketsSize;
    register s32 request __asm__("a1");
    register s32 allocationAlignment __asm__("a2");
    MemoryBlock* ptn;
    __asm__("" : "+r"(available), "+r"(pc) : "r"(root) : "a3");
    allocationPc=pc; __asm__("" : "+r"(allocationPc));
    smallInput = available;
    /* Preserve the original root-address test used to choose bucket classes. */
    if (NULL == root) {
        if (available <= 1024) {
            if (available <= 128)
            {
                /* EE as needs a new fragment here to preserve the LB macro's
                 * HI16 opcode. This directive emits no bytes or padding. */
                __asm__ volatile(".org .");
                count = D_40045570[available];
                goto bucketsReady;
            }
            result = 8;
            if (available <= 192) goto countReady;
            if (available > 256) { register s32 smaller __asm__("v1") = available <= 512; __asm__("" : : "r"(smaller)); result = smaller ? 10 : 11; } else result = 9;
            goto countReady;
        } else {
            register s32 extra __asm__("v1") = rounded + 1027;
            register s32 adjusted __asm__("t0") = available;
            register s32 quotient __asm__("a1");
            __asm__("" : "+r"(adjusted));
            if (available < 0) adjusted = extra;
            quotient = adjusted >> 10;
            result = 12;
            if (quotient & 0xFF00) { quotient = adjusted >> 18; result = 20; }
            while (quotient >>= 1) result++;
        }
    } else {
        register s32 adjusted __asm__("v0") = smallInput + 1023;
        register s32 quotient __asm__("a1");
        __asm__("" : "+r"(adjusted));
        if (smallInput >= 0) adjusted = smallInput;
        quotient = adjusted >> 11;
        result = 1;
        if (quotient != 0) {
            do { quotient >>= 1; result++; } while (quotient);
        }
    }
countReady:
    count = result;
bucketsReady:
    bucketsSize = count * 8;
    request = bucketsSize + 0x30;
    allocationAlignment = 4;
    __asm__("" : : "r"(request), "r"(allocationAlignment));
    ptn = func_400135C8((void*)partition, request + rounded, allocationAlignment, allocationPc);
    if (ptn == NULL) return 0;
    {
        MemoryBlock* block = ((MemoryBlock**)ptn)[-1];
        void* buckets = (u8*)ptn + 12;
        MemoryBlock* first = (MemoryBlock*)((u8*)buckets + bucketsSize);
        block->isPartition = 1;
        memset(buckets, 0, bucketsSize);
        {
            register u32 word __asm__("v1");
            register u32 lowMask __asm__("t1");
            register u32 highMask __asm__("t2");
            register s32 pointer __asm__("v0");
            register s32 partitionWord __asm__("a3");
            register u32 upper __asm__("a2");
            register u32 attrMask __asm__("a1");
            register u32 sizeWord __asm__("a0");
            register u32 nextWord __asm__("a2");
            register s32 firstWord __asm__("t0");
            register void* bucketArg __asm__("a1");
            register MemoryBlock* blockArg __asm__("a0");
            register void* partitionArg __asm__("a3");
            register s32 countArg __asm__("a2");
            *(u16*)((u8*)ptn + 4) = count;
            word = ((u32*)ptn)[0];
            __asm__("" : : "r"(word));
            lowMask = 0xFFFFFF;
            highMask = 0xFF000000;
            __asm__("" : : "r"(lowMask), "r"(highMask));
            pointer = PTR_WORD(buckets);
            __asm__("" : : "r"(pointer));
            word &= highMask;
            __asm__("" : : "r"(word));
            pointer &= lowMask;
            __asm__("" : : "r"(pointer));
            partitionWord = PTR_WORD(ptn);
            __asm__("" : : "r"(partitionWord));
            word |= pointer;
            __asm__("" : : "r"(word));
            upper = (u32)partitionWord >> 16;
            __asm__("" : : "r"(upper));
            ((u32*)ptn)[0] = word;
            __asm__("" : : "r"(word) : "memory");
            attrMask = 0xFEFF0000;
            __asm__("" : : "r"(attrMask));
            ((u8*)first)[3] = partitionWord;
            partitionWord = (u32)partitionWord >> 8;
            __asm__("" : : "r"(partitionWord));
            ((u8*)first)[11] = upper;
            attrMask |= 0xFFFF;
            __asm__("" : : "r"(attrMask));
            pointer = ((u32*)first)[0];
            __asm__("" : : "r"(pointer));
            sizeWord = available >> 2;
            __asm__("" : : "r"(sizeWord));
            word = ((u32*)first)[3];
            __asm__("" : : "r"(word));
            sizeWord &= lowMask;
            __asm__("" : : "r"(sizeWord));
            nextWord = ((u32*)first)[2];
            __asm__("" : : "r"(nextWord));
            pointer &= highMask;
            __asm__("" : : "r"(pointer));
            word &= attrMask;
            __asm__("" : : "r"(word));
            bucketArg = buckets;
            __asm__("" : : "r"(bucketArg));
            pointer |= sizeWord;
            __asm__("" : : "r"(pointer));
            blockArg = first;
            __asm__("" : "+r"(blockArg));
            nextWord &= highMask;
            __asm__("" : : "r"(nextWord));
            word &= highMask;
            __asm__("" : : "r"(word));
            ((u8*)first)[7] = partitionWord;
            partitionArg = ptn;
            __asm__("" : : "r"(partitionArg));
            ((u32*)first)[2] = nextWord;
            countArg = count;
            __asm__("" : : "r"(countArg));
            ((u32*)first)[0] = pointer;
            __asm__("" : : "r"(pointer) : "memory");
            firstWord = PTR_WORD(first);
            ((u32*)first)[3] = word;
            firstWord &= lowMask;
            __asm__("" : : "r"(firstWord));
            pointer = ((u32*)ptn)[2];
            __asm__("" : : "r"(pointer));
            pointer &= highMask;
            __asm__("" : : "r"(pointer));
            pointer |= firstWord;
            ((u32*)ptn)[2] = pointer;
            func_400133C8(blockArg, bucketArg, countArg, partitionArg);
        }
        __asm__("" : : "r"(available));
    }
    return (s32)ptn;
}


static inline s32 memoryInitBucketCount(void* partition, s32 argSize, void* root)
{
    register s32 size __asm__("s0") = argSize;
    register s32 result __asm__("a2");
    if (partition == root) {
        if (size <= 1024) {
            if (size <= 128) return D_40045570[size];
            result = 8;
            if (size <= 192) goto finish;
            if (size > 256) { register s32 smaller __asm__("v1") = size <= 512; __asm__("" : : "r"(smaller)); result = smaller ? 10 : 11; } else result = 9;
 goto finish;
        } else {
            register s32 adjusted __asm__("s0") = size;
            register s32 quotient __asm__("v1");
            if (adjusted < 0) adjusted += 1023;
            quotient = adjusted >> 10;
            result = 12;
            if (quotient & 0xFF00) { quotient = adjusted >> 18; result = 20; }
            while (quotient >>= 1) result++;
        }
    } else {
        register s32 quotient __asm__("v1");
        size = size < 0 ? size + 1023 : size;
        quotient = size >> 11;
        result = 1;
        if (quotient != 0) {
            do { quotient >>= 1; result++; } while (quotient);
        }
    }
finish:
    return result;
}

void func_40014F88(void* partition, void* start, void* end, s32 garbageSize)
{
    MemoryBlock* ptn = partition;
    register s32 alignMask __asm__("v1") = -4;
    void* root = rootPtn;
    s32 low = ((s32)start + 3) & alignMask;
    u32 high = (u32)end & alignMask;
    register s32 available __asm__("s0") = high - low;
    s32 count;
    s32 bucketsSize;
    MemoryBlock* first;
    s32 firstEnd;
    s32 remain;
    register void* callerPc __asm__("ra");
    void* pc;
    s32 garbage;
    u32 newBuckets;
    count = memoryInitBucketCount(partition, available, root);
    bucketsSize = count * 8;
    first = (MemoryBlock*)(low + bucketsSize);
    firstEnd = (s32)first + 32;
    if (high < (u32)firstEnd) iosJumpRecoverPoint(D_400466D0);
    remain = (s32)(high - firstEnd) >> 2;
    D_40045560 = remain;
    memset((void*)low, 0, bucketsSize);
    newBuckets = (((u32*)ptn)[0] & 0xFF000000) | ((low >> 2) & 0xFFFFFF);
    *(volatile u16*)((u8*)ptn + 4) = count;
    ((volatile u32*)ptn)[0] = newBuckets;
    first->ptn0 = PTR_WORD(ptn);
    first->ptn2 = (u32)PTR_WORD(ptn) >> 16;
    first->ptn1 = (u32)PTR_WORD(ptn) >> 8;
    first->size = remain;
    first->allocated = 0;
    first->previous = 0;
    first->next = 0;
    ptn->next = PTR_WORD(first);
    func_400133C8(first, (void*)low, count, ptn);
    pc = callerPc;
    WaitSema(D_4007CC78);
    garbage = func_40014D48((s32)ptn, garbageSize, pc);
    SignalSema(D_4007CC78);
    gbPtn = garbage;
}



s32 func_40015210(s32 fd, const char* format, ...)
{
    char buffer[0x100];
    va_list args;

    va_start(args, format);
    vsprintf(buffer, format, args);
    if (fd < 0)
    {
        LoaderSysPrintf(D_40046708, buffer);
    }
    else
    {
        LoaderSysFWrite(fd, buffer, strlen(buffer));
    }
    return strlen(buffer);
}

void func_400152B8(void* partition, s32 depth, s32 detail, s32 fd)
{
    register s32 file __asm__("s5") = fd;
    register MemoryPartition* ptn __asm__("s6") = (MemoryPartition*)partition;
    char spaces[depth + 1];
    s32 bucketIndex;
    MemoryBlock** buckets;
    MemoryBlock* block;
    asm("" : "+r"(file), "+r"(ptn));
    if (depth != 0) memset(spaces, ' ', depth);
    spaces[depth] = 0;
    func_40015210(file, D_40046710, spaces, ptn);
    func_40015210(file, D_40046728, ptn->bucketCount);
    buckets = WORD_PTR(ptn->buckets);
    for (bucketIndex = 0; bucketIndex < ptn->bucketCount; bucketIndex++) {
        MemoryBlock* freeBlock = buckets[0];
        s32 freeCount = 0;
        while (freeBlock != NULL) {
            func_40015210(file, D_40046738, freeCount++, freeBlock->size << 2);
            freeBlock = freeBlock->freeNext;
        }
        func_40015210(file, D_40046748, bucketIndex, freeCount);
        buckets += 2;
    }
    block = WORD_PTR(ptn->first);
    if (block != NULL) {
        asm("" : "+r"(block));
        if (block != NULL) {
        asm volatile("" ::: "memory");
        while (1) {
            MemoryBlock* current = block;
            if (block->next && (u32)block + (block->size << 2) + 32 != (block->next << 2)) {
                func_40015210(file, D_400467C0);
            }
            if (block->allocated) {
                register s32 lower __asm__("v0");
                register s32 upper __asm__("a0");
                register s32 lowBits __asm__("a1");
                register s32 middle __asm__("v1");
                register s32 encoded __asm__("v0");
                char* callback;
                lowBits = ((u32*)block)[3] >> 29;
                lower = ((u8*)block)[27];
                upper = ((u8*)block)[19];
                middle = ((u8*)block)[23];
                lowBits &= 3;
                asm("" : "+r"(lowBits), "+r"(lower), "+r"(upper), "+r"(middle));
                lower <<= 18;
                upper <<= 2;
                asm volatile("" : "+r"(lower), "+r"(upper), "+r"(middle));
                lower |= lowBits;
                asm volatile("" : "+r"(lower), "+r"(middle));
                middle <<= 10;
                upper |= middle;
                asm("" : "+r"(lower), "+r"(upper));
                encoded = lower | upper; asm("" :: "r"(encoded));
                callback = (char*)encoded;
                if (encoded == 0) callback = D_400467C8;
                asm volatile("" :: "r"(callback) : "v0");

                func_40015210(file, D_400467D0, spaces, block, (u32)block + (block->size << 2) + 32, block->previous << 2, block->size << 2, block->pc << 2, callback);
                if (block->isPartition) {
                    s32 child = (s32)(block + 1);
                    if (child == gbPtn || detail == 0) func_400152B8((void*)child, depth + 1, detail, file);
                }
            } else {
                func_40015210(file, D_40046800, spaces, block, (u32)block + (block->size << 2) + 32, block->previous << 2, block->size << 2);
            }
            block = WORD_PTR(block->next);
            if (block == NULL || current >= block) break;
        }
    }
}
}

void func_400155C0(s32 partition, s32 parent, s32* count)
{
    MemoryPartition* ptn = (MemoryPartition*)partition;
    MemoryBlock* block = WORD_PTR(ptn->first);
    MemoryBlock* previous = block;
    while (block != NULL) {
        MemoryBlock* next;
        if (block < previous || (u32)block > 0x02000000) {
            if (parent != 0) {
                func_400152B8((void*)parent, 0, 0, -1);
                LoaderSysPrintf(D_40046828);
            }
            func_400152B8((void*)partition, 0, 1, -1);
            LoaderSysPrintf(D_40046878);
            LoaderSysPrintf(D_400468C0);
        }
        (*count)++;
        previous = block;
        if ((((u32*)block)[3] & 0x03000000) == 0x03000000) {
            func_400155C0((s32)(block + 1), partition, count);
        }
        next = WORD_PTR(block->next);
        if (next != NULL) {
            if (next < block || (u32)next > 0x02000000) {
                LoaderSysPrintf(D_40046908, block, next);
                __asm__ volatile("break 0");
                func_400152B8(rootPtn, 0, 0, -1);
                __asm__ volatile("break 0");
            }
            if (next != NULL && !block->allocated && !next->allocated) {
                LoaderSysPrintf(D_40046948, block, next);
                func_400152B8((void*)partition, 0, 1, -1);
            }
        }
        block = next;
    }
}




void iosWaitMalloc(void)
{
    WaitSema(D_4007CC78);
}

void iosSignalMalloc(void)
{
    SignalSema(D_4007CC78);
}

void iosMallocGarbageTickProc(s32 (*keepGoing)(s32), s32 argument)
{
    struct GarbageMemoryBlock* block;
    struct GarbageMemoryBlock* next;
    s32 callbackWord;
    s32 result;
    s32 callback;
    register s32 callbackArgument __asm__("s0");

    WaitSema(D_4007CC78);
    block = (struct GarbageMemoryBlock*)(((struct GarbageMemoryBlock*)gbPtn)->next << 2);
    while (block != NULL && keepGoing(argument))
    {
        next = (struct GarbageMemoryBlock*)(block->next << 2);
        if (!block->allocated && next != NULL && next->allocated)
        {
            callbackWord = next->callback;
            callback = callbackWord << 2;
            if (callbackWord != 0xFFFFFF)
            {
                callbackArgument = next->argument;
                result = (s32)func_40013A78((FreeMemoryBlock*)block, (FreeMemoryBlock*)next, (void*)gbPtn);
                callbackArgument <<= 2;
                /* Keep the callback argument in its original saved register. */
                asm("" : "+r"(callbackArgument));
                if (callback != 0)
                {
                    ((void (*)(s32, s32))callback)(callbackArgument, result);
                }
                next = (struct GarbageMemoryBlock*)(block->next << 2);
            }
        }
        block = next;
    }
    SignalSema(D_4007CC78);
}

void* iosMallocAlign(s32 partition, s32 size, s32 alignment)
{
    register void* callerPc __asm__("ra");
    void* pc = callerPc;
    void* result;

    WaitSema(D_4007CC78);
    result = func_400135C8((void*)partition, size, alignment, pc);
    if (result == NULL)
    {
        LoaderSysPrintf(D_400465B8, partition, size, pc);
        func_40015F18(partition, 1);
        LoaderSysPrintf(D_400465E8, rootPtn, gbPtn);
    }
    SignalSema(D_4007CC78);
    if (result == NULL)
    {
        LoaderSysPrintf(D_40046618, D_40045568);
        if (D_40045568 != NULL)
        {
            D_40045568();
        }
        iosJumpRecoverPoint(D_40046628, partition, size, pc);
    }
    return result;
}

void* iosMallocAlignNoCheck(s32 partition, s32 size, s32 alignment)
{
    register void* callerPc __asm__("ra");
    void* pc = callerPc;
    void* result;

    WaitSema(D_4007CC78);
    result = func_400135C8((void*)partition, size, alignment, pc);
    SignalSema(D_4007CC78);
    return result;
}

void iosMallocSetExecGarbageCallback(void* ptr, s32 callback, s32 argument)
{
    u32* header;

    WaitSema(D_4007CC78);
    header = ((u32**)ptr)[-1];
    header[4] = (header[4] & 0xFF000000) | ((callback >> 2) & 0xFFFFFF);
    header[5] = (header[5] & 0xFF000000) | ((argument >> 2) & 0xFFFFFF);
    SignalSema(D_4007CC78);
}

void* iosReallocAlign(s32 partition, void* ptr, s32 size, s32 alignment)
{
    void* result;

    WaitSema(D_4007CC78);
    result = func_40014AF8(partition, ptr, size, alignment);
    SignalSema(D_4007CC78);
    return result;
}

void iosFree(void* ptr)
{
    WaitSema(D_4007CC78);
    func_400142E0(ptr);
    SignalSema(D_4007CC78);
}

void* iosFreeParts(void* ptr, void* start, s32 size)
{
    void* result;

    WaitSema(D_4007CC78);
    result = func_40014770(ptr, start, size);
    SignalSema(D_4007CC78);
    return result;
}

s32 iosCreatePartition(s32 partition, s32 size)
{
    register void* callerPc __asm__("ra");
    void* pc = callerPc;
    s32 result;

    WaitSema(D_4007CC78);
    result = func_40014D48(partition, size, pc);
    SignalSema(D_4007CC78);
    return result;
}

s32 iosCreatePartitionWithPc(s32 partition, s32 size, void* pc)
{
    s32 result;

    WaitSema(D_4007CC78);
    result = func_40014D48(partition, size, pc);
    SignalSema(D_4007CC78);
    return result;
}

void iosInitMallocSystem(void* start, void* end, s32 garbageSize)
{
    D_4007CC78 = iosCreateSema(1, 1, 0);
    mallocStartAdrs = start;
    func_40014F88(rootPtn, start, end, garbageSize);
}

void* iosGetMallocPtn(void* ptr)
{
    u8* header = ((u8**)ptr)[-1];

    /* The partition address is packed into the high byte of three words. */
    return (void*)(((header[11] << 16) | (header[7] << 8) | header[3]) << 2);
}

s32 iosGetMallocRootPtnRemainSize(void)
{
    return D_40045560 << 2;
}

s32 iosFollowAllLink(s32 partition)
{
    s32 count = 0;

    WaitSema(D_4007CC78);
    func_400155C0(partition, 0, &count);
    SignalSema(D_4007CC78);
    return count;
}

void iosDebugPartitionDump(s32 partition, s32 detail)
{
    WaitSema(D_4007CC78);
    func_400152B8((void*)partition, 0, detail, -1);
    SignalSema(D_4007CC78);
}

void iosDebugPartitionFileDump(s32 partition, s32 detail, s32 fd)
{
    WaitSema(D_4007CC78);
    func_400152B8((void*)partition, 0, detail, fd);
    SignalSema(D_4007CC78);
}

void func_40015F18(s32 partition, s32 detail)
{
    func_400152B8((void*)partition, 0, detail, -1);
}

void addMemoryDebugCallback(void (*callback)(void))
{
    D_40045568 = callback;
}

s32* getMemorySafetyLockFlag(void)
{
    return &D_4004556C;
}

void func_40015F58(void)
{
    /* PS2 DMA completion barrier: coprocessor condition and exact pipeline delay. */
    asm volatile(
        ".set push\n"
        ".set noreorder\n"
        "sync\n"
        "1: nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "bc0f 1b\n"
        "nop\n"
        "nop\n"
        ".set pop\n"
        ::: "memory");
}

asm(".align 3");
