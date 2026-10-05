#include "common.h"
#include "gcc/string.h"
extern char* D_40045824;
#include "ios/isys/module.h"
#include "ios/memory.h"
#include "ios/kernel.h"
#include "ios/hash.h"
#include "ios/cd.h"
#include "ios/iop.h"
#include "loaderSysFileIO.h"
s32 func_4001AB68(isysExecModuleObj*,char*,struct t_xffEntPntHdr*);
extern char D_40047138[];
extern char rootPtn[];
extern s32 D_40045820;
extern s32 D_40081A20;
extern s32 D_40081A28[];
extern const char D_40046EC0[];
void func_4001CFE0(u32 address);

extern isysGroup executableModuleGroup;
extern t_hashTable D_40081A18;
extern memory_info D_400819B0;
extern char D_40047068[], D_40047088[], D_40047090[], D_400470A8[], D_400470B8[], D_400470D8[];
extern char D_40047400[], D_400474A8[];
extern char D_40047060[];
s32 iosSPrintf(char*buffer,const char*format,...);
s32 printf(const char*format,...);
void isysLaunchModule(isysExecModuleObj* module);
void isysCreateExceptionThread(void);
isysExecModuleObj* func_4001AE88(char* name, struct t_xffEntPntHdr* xff, s32 heap);
s32 func_4001D120(s32 address);
s32 func_4001CDD8(char* name, char* symbol, isysObj* module, u32 address);
s32 iosGetTCount(void);
extern isysGroup needResolveModuleGroup;
extern isysGroup symbolDefModuleGroup;
void func_40019A00(isysModuleObj* module, s32 relocate);
void func_4001A2F8(isysModuleObj* module);
extern void* DldSysUndefSymbol;
extern u32 D_40045828;
struct t_xffEntPntHdr*func_40019D68(struct t_xffEntPntHdr*,isysModuleObj*,s32,void*(*)(s32,s32),void*(*)(s32),void*,s32(*)(char*,...),s32);
void func_40019B58(isysModuleObj*,s32);
void func_4001A170(isysModuleRelocations*,s32,struct t_xffEntPntHdr*);
s32 func_4001C440(void);
s32 func_40019FF0(isysModuleObj* module);
isysExecModuleObj* func_4001AD58(char* name, s32 size, s32 partition);
void iosResetCpuRapCounter(s32 counter);
s32 iosGetCpuRapCountPar1Int(void);

#include <stdarg.h>

//INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/isys/module", func_400199B0);
s32 func_400199B0(s32 arg0, ...)
{
    return 0;
}

static inline void clear_import_relocation_flags(struct t_xffEntPntHdr*xff)
{
 s32 count=xff->impSymIxsNrE;s32 remaining;register struct t_xffImpSymIxs*imports __asm__("a0");register struct t_xffSymEnt*symbols __asm__("a1");
 if(count!=0){symbols=xff->symTab;asm("" : "+r"(symbols));remaining=count-1;asm("" : "+r"(count), "+r"(remaining));
 imports=xff->impSymIxs;
 if(count!=0){do{u32 index=imports->stIx;imports++;remaining--;symbols[index].unk0D=0;}while(remaining!=-1);}}
}

void func_40019A00(isysModuleObj* module, s32 relocate)
{
    register struct t_xffEntPntHdr* xff __asm__("s2");
    struct t_xffSymEnt* symbols;
    isysModuleRelocation* pending;
    register s32 remaining __asm__("s1");
    if (!(module->flags & 0x40000000))
    {
        xff = module->xff;
        if (relocate != 0)
        {
            RelocateCode(xff);
            clear_import_relocation_flags(xff);
        }
        else {
            register s32 count __asm__("a1")=xff->impSymIxsNrE;
            asm("" : "+r"(count));
            if(count!=0) {
            asm("" : "+r"(xff));
            remaining=module->relocations.count-1;asm("" : "+r"(remaining) : : "memory");
            pending=module->relocations.entries;
            if(remaining!=-1) {
                symbols=xff->symTab;
                do {
                    struct t_xffRelocEnt* table=pending->table;
                    register struct t_xffRelocAddrEnt* addr __asm__("v1")=table->addr;
                    s32 index=pending->index;
                    register s32 offset __asm__("a0");
                    register u32 flag __asm__("v1");
                    offset=index*8;addr=(struct t_xffRelocAddrEnt*)((char*)addr+offset);
                    flag=symbols[addr->tyIx >> 8].unk0D;

                    pending++;
                    if(flag!=0) {
                    asm("" : : "r"(pending));
                        ResolveRelocation(xff,table,index);
                        symbols=xff->symTab;
                    }
                }while(--remaining!=-1);
                count=xff->impSymIxsNrE;
            }else symbols=xff->symTab;
            {
                struct t_xffImpSymIxs*imports=xff->impSymIxs;
                s32 remaining=count-1;
                if(count!=0){do{u32 index=imports->stIx;imports++;remaining--;symbols[index].unk0D=0;}while(remaining!=-1);}
            }
            }
        }
    }
}




void func_40019B58(isysModuleObj* arg0, s32 arg1)
{
    s32 sp0;register struct t_xffSymEnt*symBase __asm__("v1");register s32 symOffset __asm__("a1");register s32 total __asm__("v0");register s32 imports __asm__("v1");
    isysModuleImport* temp_v0_6;
    register isysModuleImport* var_s1_2 __asm__("s1");
    s32 temp_s0;
    s32 temp_s2;
    s32 temp_v0_2;
    s32 var_a1;
    register s32 var_a2 __asm__("a2");
    register s32 var_s2 __asm__("s2");
    register s32 var_s2_2 __asm__("s2");
    s8* temp_s6;
    s8* temp_v0_5;
    struct t_xffEntPntHdr* temp_s4;
    register struct t_xffImpSymIxs* var_s3 __asm__("s3");
    struct t_xffSymEnt* temp_s0_2;
    struct t_xffSymEnt* temp_s4_2;
    register struct t_xffSymEnt* var_s0 __asm__("s0");
    t_hashedSym* temp_v0_3;
    t_hashedSym* var_s1;
    u32 temp_v0_7;
    u8 temp_v0;
    u8 temp_v0_4;

    sp0 = arg1;
    temp_s4 = arg0->xff;
    temp_v0 = *(u8*)&temp_s4->specSectNrE;
    temp_s6 = temp_s4->symTabStr;
    var_a1 = temp_v0 + 1;
    if (temp_v0 == 0)
    {
        asm("");var_a1 = 2;
    }
    imports=temp_s4->impSymIxsNrE;asm("" : "+r"(imports));temp_v0_2 = (temp_s4->symTabNrE - imports) - var_a1;
    if (temp_v0_2 != 0)
    {
        temp_v0_3 = iosMallocAlign(sp0, temp_v0_2 * 0x10, 4);
        arg0->definedSymbols = temp_v0_3;
        var_s1 = temp_v0_3;
        temp_v0_4 = *(u8*)&temp_s4->specSectNrE;
        var_a2 = temp_v0_4 + 1;
        if (temp_v0_4 == 0)
        {
            asm("");var_a2 = 2;
        }
        asm("" : "+r"(var_a2));total=temp_s4->symTabNrE;symOffset=var_a2*16;symBase=temp_s4->symTab;asm("" : "+r"(total), "+r"(symOffset), "+r"(symBase));var_s2=total-var_a2;asm("" : "+r"(var_s2));var_s2--;asm("" : "+r"(var_s2));var_s0=(struct t_xffSymEnt*)((char*)symBase+symOffset);
        if (var_s2 != -1)
        {asm("" : "+r"(var_s0));
            do
            {
                if (var_s0->sect != 0)
                {
                    temp_v0_5 = &temp_s6[var_s0->nameOffs];
                    if (*temp_v0_5 != 0)
                    {
                        var_s1->locSym = var_s0;
                        var_s1->hash = MakeStrHashValue(temp_v0_5);
                        AddStrHashKey(&D_40081A18, var_s1);
                    }
                    else
                    {
                        var_s1->locSym = NULL;
                    }
                    var_s1->extSym = NULL;
                    var_s1++;
                }
                var_s2 -= 1;
                var_s0++;
            } while (var_s2 != -1);
        }
        {register s32 flags __asm__("a0")=arg0->flags;register s32 count __asm__("v1")=(char*)var_s1-(char*)temp_v0_3;register u32 mask __asm__("v0");asm("" : "+r"(flags), "+r"(count));count>>=4;mask=0x1fffffff;asm("" : "+r"(mask));count&=mask;asm("" : "+r"(flags) : "r"(count));arg0->flags=(flags&0xe0000000)|count;}
    }
    else
    {
        arg0->definedSymbols = NULL;
    }
    temp_s2 = temp_s4->impSymIxsNrE;
    if (temp_s2 != 0)
    {
        var_s2_2 = temp_s2 - 1;
        temp_v0_6 = iosMallocAlign(sp0, temp_s2 * 0x10, 4);
        arg0->importedSymbols = temp_v0_6;
        var_s1_2 = temp_v0_6;asm("" : "+r"(var_s1_2) : : "memory");
        var_s3 = temp_s4->impSymIxs;asm("" : "+r"(var_s3), "+r"(var_s1_2), "+r"(var_s2_2));
        temp_s4_2 = temp_s4->symTab;
        if (var_s2_2 != -1)
        {
            do
            {
                temp_s0 = var_s3->stIx;
                var_s3++;
                var_s2_2 -= 1;
                temp_s0_2 = &temp_s4_2[temp_s0];
                temp_v0_7 = MakeStrHashValue(&temp_s6[temp_s0_2->nameOffs]);
                var_s1_2->next = NULL;
                var_s1_2->hash = temp_v0_7;
                var_s1_2->previous = 0;
                var_s1_2->symbol = temp_s0_2;
                var_s1_2++;
                temp_s0_2->unk0D = 1;
            } while (var_s2_2 != -1);
        }
    }
    else
    {
        arg0->importedSymbols = NULL;
    }
    arg0->flags |= 0x80000000;
}


struct t_xffEntPntHdr* func_40019D68(struct t_xffEntPntHdr* arg0, isysModuleObj* arg1, s32 arg2, void* (*arg3)(s32, s32), void* (*arg4)(s32), void* arg5, s32 (*arg6)(s8*, ...), s32 arg7)
{
    s32 sp0;register isysModuleObj* callModule __asm__("a0");
    s32 temp_s2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a1;
    s32 var_s5;
    s32 var_v0;
    s32 var_v1;
    s32 var_v1_2;
    struct t_xffEntPntHdr* temp_s4;
    struct t_xffSectEnt* var_a2;
    struct t_xffSectEnt* var_v1_3;
    struct t_xffSymEnt* temp_v0_4;
    u32 temp_v0;
    u8 temp_v0_2;

    var_v1 = 0;
    sp0 = arg2;
    if (arg7 != 0)
    {
        arg0->relocTab=(struct t_xffRelocEnt*)((u8*)arg0+arg0->relocTab_Rel);
        var_v1 = arg0->impSymIxsNrE == 0;
    }
    if (var_v1 != 0)
    {
        var_s5 = 0;
        RelocateElfInfoHeader(arg0);
        var_a2 = arg0->sectTab + 1;
        var_v1_2 = arg0->sectNrE - 2;
        var_a1 = 0;
        if (var_v1_2 != -1)
        {
            do
            {
                temp_v0 = var_a2->type;
                var_a2->flags = 1;
                if (temp_v0 != 8)
                {
                    if (temp_v0 < 9U)
                    {
                        if (temp_v0 != 1)
                        {

                        }
                        else
                        {
                            goto block_10;
                        }
                    }
                    else if (temp_v0 == 0x7FFFF420)
                    {
                        goto block_10;
                    }
                }
                else
                {
block_10:
                    var_a1 = var_a1 + var_a2->size + 0x33;
                }
                var_v1_2 -= 1;
                var_a2++;
            } while (var_v1_2 != -1);
        }
        temp_v0_2 = *(u8*)&arg0->specSectNrE;
        var_a0 = temp_v0_2 + 1;
        if (temp_v0_2 == 0)
        {
            asm("");var_a0 = 2;
        }
        temp_v1 = arg0->symTabNrE;
        temp_s2 = (temp_v1 - arg0->impSymIxsNrE) - var_a0;
        if (temp_s2 != 0)
        {
            var_s5 = temp_v1 * 0x10;
            var_a1 += var_s5;
        }
        temp_v0_3 = iosCreatePartition(arg7,var_a1+0x3B);
        D_40081A20 = temp_v0_3;
        temp_s4 = iosMallocAlign(temp_v0_3, 8, 4);
        if (temp_s2 != 0)
        {
            temp_v0_4 = iosMallocAlign(D_40081A20, var_s5, 4);
            memcpy(temp_v0_4, arg0->symTab, (u32) var_s5);
            arg0->symTab = temp_v0_4;
        }
        DecodeSection(arg0, (void* (*)(s32, u32)) func_4001C220, func_4001C248, (ldrDbgPrintf_func*)arg6);
        RelocateSelfSymbol(arg0, arg5);
        RelocateCode(arg0);
        if (temp_s2 != 0)
        {
            register u32 mask __asm__("v0");
            register s32 flags __asm__("v1")=arg1->flags;
            register s32 callFlags __asm__("a1");
            callModule=arg1;asm volatile("" : : "r"(callModule));
            mask=0xdfff0000;arg1->xff=arg0;
            asm("" : "+r"(mask) : "r"(flags) : "memory");
            mask|=0xffff;
            flags&=mask;asm("" : "+r"(flags));
            callFlags=sp0;asm volatile("" : : "r"(callFlags));
            arg1->flags=flags;
            asm("" : : : "memory");
            arg1->entryPoint=arg0->entryPnt;
            func_40019B58(callModule, callFlags);
            isysAddGroupWithLinkParam(&arg1->obj, &symbolDefModuleGroup, 0, 0);
        }
        var_v0 = arg0->sectNrE - 2;
        var_v1_3 = arg0->sectTab + 1;
        if (var_v0 != -1)
        {
            do
            {
                var_v0 -= 1;
                var_v1_3->moved = 0;
                var_v1_3++;
            } while (var_v0 != -1);
        }
        temp_s4->ident = (u32)arg0->entryPnt;
        temp_s4->u04 = D_40081A20;
        func_4001C468(arg0);
        return temp_s4;
    }
    LoaderSysRelocateOnlineElfInfo(arg0, arg3, arg4, arg5, arg6);
    return NULL;
}


s32 func_40019FF0(isysModuleObj* arg0)
{
    struct t_xffSymEnt* sp0;
    s128 sp10;
    isysModuleImport* temp_v1_2;
    isysModuleImport* var_s0;
    u32 var_a2;register u32 tag asm("s7");register u32 one asm("s6");
    s32 temp_s2;register s32 stop __asm__("v0");register struct t_xffSymEnt*firstSymbols __asm__("a1");
    register s32 var_s2 __asm__("s2");
    s32 var_s4;
    struct t_xffEntPntHdr* temp_v1;
    struct t_xffImpSymIxs* var_s3;
    struct t_xffSymEnt* temp_s1;
    struct t_xffSymEnt* temp_v1_3;
    struct t_xffSymEnt* temp_v1_4;
    t_hashedSym* temp_v0;
    u32 temp_a0;
    register void* var_s5 __asm__("s5");


    temp_v1 = arg0->xff;
    temp_s2 = temp_v1->impSymIxsNrE;
    var_s4 = 0;
    if (temp_s2 != 0)
    {
        var_s2 = temp_s2 - 1;asm("" : "+r"(var_s2));
        firstSymbols=temp_v1->symTab;asm("" : "+r"(firstSymbols));stop=-1;asm("" : "+r"(stop), "+r"(firstSymbols) : "r"(var_s2));sp0=firstSymbols;
        var_s0 = arg0->importedSymbols;
        var_s3 = temp_v1->impSymIxs;
        if(var_s2 != stop)
        {
            var_a2=0x7fff0000;asm("" : : "m"(DldSysUndefSymbol), "r"(var_a2));tag=0x80000000;one=1;asm("" : : "r"(tag), "r"(one));var_a2|=0xffff;
            do
            {
                temp_s1 = &sp0[var_s3->stIx];
                if (temp_s1->addr == DldSysUndefSymbol)
                {

                    temp_v0 = SearchStrHashKey(&D_40081A18, var_s0->hash);
                    if (temp_v0 != NULL)
                    {
                        var_s5 = temp_v0->locSym->addr;asm("" : "+r"(var_s5));
                    }
                    asm("" : "+r"(temp_v0));if (temp_v0 != NULL)
                    {
                        temp_a0 = var_s0->previous;
                        if (temp_a0 != 0)
                        {
                            temp_v1_2 = var_s0->next;
                            if (temp_a0 & tag)
                            {
                                *(isysModuleImport**)((temp_a0 & (u32)var_a2)+4) = temp_v1_2;
                            }
                            else
                            {
                                ((isysModuleImport*)temp_a0)->next = temp_v1_2;
                            }
                            if (temp_v1_2 != NULL)
                            {
                                temp_v1_2->previous = temp_a0;
                            }
                            temp_v1_3 = var_s0->symbol;
                            var_s0->next = NULL;
                            var_s0->previous = 0;
                            temp_v1_3->unk0D = one;
                            temp_v1_3->addr = DldSysUndefSymbol;
                        }
                        temp_v1_4 = temp_v0->extSym;
                        var_s0->previous = (s32) temp_v0 | tag;
                        var_s0->next = (isysModuleImport* ) temp_v1_4;asm("" : : : "memory");
                        temp_v0->extSym = (struct t_xffSymEnt* ) var_s0;
                        if (temp_v1_4 != NULL)
                        {
                            temp_v1_4->size = (u32) var_s0;
                        }
                        temp_s1->unk0D = one;
                        temp_s1->addr = var_s5;
                    }
                    if(temp_s1->unk0D!=0)var_s4=var_s4+1;
                }
                var_s2 -= 1;
                var_s3++;
                var_s0++;
            } while (var_s2 != -1);
        }
    }
    return var_s4;
}

void func_4001A170(isysModuleRelocations* arg0, s32 arg1, struct t_xffEntPntHdr* arg2)
{
    isysModuleRelocation* temp_v0_2;
    isysModuleRelocation* var_a3_2;
    register s32 var_a0 __asm__("a0");
    register s32 var_a1 __asm__("a1");
    register s32 var_a4 __asm__("t0");
    register s32 var_a4_2 __asm__("t0");
    register s32 var_a5 __asm__("t1");
    struct t_xffRelocAddrEnt* var_a1_2;
    struct t_xffRelocAddrEnt* var_a2;
    register struct t_xffRelocEnt* var_a2_2 __asm__("a2");
    register struct t_xffRelocEnt* var_a3 __asm__("a3");
    struct t_xffSymEnt* temp_s2;
    u32 temp_v0;
    u32 temp_v0_3;
    u32 var_v0;

    arg0->count = 0;
    arg0->entries = NULL;
    if (arg2->impSymIxsNrE != 0)
    {
        temp_s2 = arg2->symTab;
        var_a4 = arg2->relocTabNrE - 1;asm("" : "+r"(var_a4));
        var_a3 = arg2->relocTab;
        if (var_a4 != -1)
        {
            do
            {
                temp_v0 = var_a3->type;
                if ((temp_v0 == 4) || (temp_v0 == 9))
                {
                    var_a1=var_a3->nrEnt;asm("" : "+r"(var_a1));var_a1--;asm("" : "+r"(var_a1));
                    var_a2 = var_a3->addr;
                    if (var_a1 != -1)
                    {
                        do {
                        var_v0=var_a2->tyIx;var_a2++;
                        if(temp_s2[var_v0>>8].sect==0)arg0->count++;
                        var_a1--;
                        }while(var_a1!=-1);
                    }
                }
                var_a4 -= 1;
                var_a3++;
            } while (var_a4 != -1);
        }
        temp_v0_2 = iosMallocAlign(arg1, arg0->count * 8, 4);
        arg0->entries = temp_v0_2;
        var_a3_2 = temp_v0_2;asm("" : "+r"(var_a3_2) : : "memory");
        var_a4_2 = arg2->relocTabNrE - 1;asm("" : "+r"(var_a4_2) : : "memory");
        var_a2_2 = arg2->relocTab;
        if (var_a4_2 != -1)
        {
            do
            {
                temp_v0_3 = var_a2_2->type;
                if ((temp_v0_3 == 4) || (temp_v0_3 == 9))
                {
                    var_a5 = 0;asm("" : "+r"(var_a5));
                    var_a0=var_a2_2->nrEnt;asm("" : "+r"(var_a0));var_a0--;asm("" : "+r"(var_a0));
                    var_a1_2 = var_a2_2->addr;
                    if (var_a0 != -1)
                    {
                        do
                        {
                            u32 word=var_a1_2->tyIx;var_a1_2++;
                            if (temp_s2[word >> 8].sect == 0)
                            {
                                var_a3_2->index = var_a5;
                                var_a3_2->table = var_a2_2;
                                var_a3_2++;
                            }
                            var_a0 -= 1;
                            var_a5 += 1;
                        } while (var_a0 != -1);
                    }
                }
                var_a4_2 -= 1;
                var_a2_2++;
            } while (var_a4_2 != -1);
        }
    }
}


void func_4001A2F8(isysModuleObj* arg0)
{
    isysModuleImport* temp_a0_3;
    isysModuleImport* temp_a0_4;
    isysModuleImport* var_a1_2;
    register s32 temp_a1 __asm__("a1");register u32 mask __asm__("v0");
    register s32 var_a2 __asm__("a2");
    register s32 var_s1 __asm__("s1");
    isysModuleImport* temp_a0_2;
    struct t_xffSymEnt* temp_v1_2;
    isysModuleImport* var_a1;
    t_hashedSym* temp_a0;
    t_hashedSym* var_s0;
    u32 temp_v1;
    struct t_xffSymEnt* var_v0;

    temp_a1 = arg0->flags;asm("" : "+r"(temp_a1));
    if (temp_a1 < 0)
    {
        temp_a0 = arg0->definedSymbols;
        var_s0 = temp_a0;
        if (temp_a0 != NULL)
        {
            mask=0x1fffffff;asm("" : "+r"(mask));var_s1=temp_a1&mask;asm("" : "+r"(var_s1));var_s1--;asm("" : "+r"(var_s1));
            if (var_s1 != -1)
            {
                do
                {
                    if (var_s0->locSym != NULL)
                    {
                        var_a1 = (isysModuleImport*)var_s0->extSym;
                        var_s0->extSym = NULL;
                        if (var_a1 != NULL)
                        {
                            do {
                              var_v0=var_a1->symbol;
                              temp_a0_2=var_a1->next;
                              var_v0->unk0D=1;
                              var_v0->addr=DldSysUndefSymbol;asm("" : : : "memory");
                              var_a1->previous=0;asm("" : : : "memory");
                              var_a1->next=NULL;
                              var_a1=temp_a0_2;
                            }while(var_a1!=NULL);
                        }
                        DeleteStrHashKey(&D_40081A18, var_s0);
                    }
                    var_s1 -= 1;
                    var_s0++;
                } while (var_s1 != -1);
            }
            iosFree(arg0->definedSymbols);
            arg0->definedSymbols = NULL;
        }
        temp_a0_3 = arg0->importedSymbols;
        var_a1_2 = temp_a0_3;
        if (temp_a0_3 != NULL)
        {
            var_a2=arg0->xff->impSymIxsNrE-1;asm("" : "+r"(var_a2));
            if (var_a2 != -1)
            {
                do
                {
                    temp_v1 = var_a1_2->previous;
                    temp_a0_4 = var_a1_2->next;
                    if (temp_v1 != 0)
                    {
                        if (temp_v1 & 0x80000000)
                        {
                            *(isysModuleImport**)((temp_v1 & 0x7fffffff)+4) = temp_a0_4;
                        }
                        else
                        {
                            ((isysModuleImport*)temp_v1)->next = temp_a0_4;
                        }
                    }
                    if (temp_a0_4 != NULL)
                    {
                        temp_a0_4->previous = temp_v1;
                    }
                    temp_v1_2 = var_a1_2->symbol;
                    var_a2 -= 1;
                    var_a1_2->next = NULL;
                    var_a1_2->previous = 0;
                    var_a1_2++;
                    temp_v1_2->unk0D = 1;
                    temp_v1_2->addr = DldSysUndefSymbol;
                } while (var_a2 != -1);
            }
            iosFree(arg0->importedSymbols);
            arg0->importedSymbols = NULL;
        }
        arg0->flags &= 0x7FFFFFFF;
    }
}


void isysMoveModuleObj(isysModuleObj* arg0, struct t_xffEntPntHdr* arg1)
{
    register isysModuleImport* var_a1_3 __asm__("a1");
    s32 temp_v0_2;
    register s32 temp_v1 __asm__("v1");
    register s32 var_a1 __asm__("a1");
    register s32 var_a1_2 __asm__("a1");
    register s32 var_v1 __asm__("v1");
    register struct t_xffImpSymIxs* var_a0_2 __asm__("a0");
    register struct t_xffSymEnt* var_a0 __asm__("a0");
    register t_hashedSym* var_a2 __asm__("a2");
    register u8 temp_v0 __asm__("v0");
    register struct t_xffSymEnt*symbols asm("a3");
    register s32 total asm("v0");
    register s32 stop asm("v0");
    register s32 byteOffset asm("a0");

    func_40019D68(arg1, arg0, arg0->obj.partition, func_4001C3B0, (void*(*)(s32))func_4001C440, DldSysUndefSymbol, (s32(*)(char*,...))func_400199B0, 0);
    arg0->entryPoint = arg1->entryPnt;
    temp_v1 = arg0->flags & 0xDFFFFFFF;
    arg0->xff = arg1;
    arg0->flags = temp_v1;
    if (temp_v1 < 0)
    {
        var_a2 = arg0->definedSymbols;
        if (var_a2 != NULL)
        {
            temp_v0 = *(u8*)&arg1->specSectNrE;
            var_a1 = temp_v0 + 1;
            if (temp_v0 == 0)
            {
                asm("");var_a1=2;
            }
            total=arg1->symTabNrE;byteOffset=var_a1*16;symbols=arg1->symTab;

            var_a1_2=total-var_a1;
            var_a1_2--;
            var_a0=(struct t_xffSymEnt*)((u32)symbols+(u32)byteOffset);
            if (var_a1_2 != -1)
            {
                do
                {
                    if (var_a0->sect != 0)
                    {
                        if (var_a2->locSym != NULL)
                        {
                            var_a2->locSym = var_a0;
                        }
                        var_a2++;
                    }
                    var_a1_2 -= 1;
                    var_a0++;
                } while (var_a1_2 != -1);
            }
        }
        symbols=arg1->symTab;asm("" : "+r"(symbols) : : "memory");
        var_v1=arg1->impSymIxsNrE;asm("" : "+r"(var_v1));
        stop=-1; asm("" : "+r"(stop) : : "memory");
        var_a1_3=arg0->importedSymbols;asm("" : "+r"(var_a1_3), "+r"(var_v1) : "r"(stop));

        var_v1--;
        var_a0_2 = arg1->impSymIxs;
        if (var_v1 != stop)
        {
            do
            {
                temp_v0_2 = var_a0_2->stIx;
                var_a0_2++;
                var_v1 -= 1;
                {register struct t_xffSymEnt* result asm("v0")=symbols+temp_v0_2;asm("" : "+r"(result));var_a1_3->symbol=result;}
                var_a1_3++;
            } while (var_v1 != -1);
        }
    }
}

isysModuleObj* isysCreateModuleObjExisted(struct t_xffEntPntHdr* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6)
{
    isysObj* temp_v0;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v0_3;
    s32 var_a3;
    s32 var_s7;
    s32 var_v0;
    s32 var_v1;
    s32 var_v1_2;
    register struct t_xffEntPntHdr*temp_s3 __asm__("s3");
    struct t_xffEntPntHdr* temp_v0_4;
    u32 var_a2;
    u8 temp_v0_2;

    var_a3 = arg1;
    if (arg5 != 0)
    {
        temp_v0_2 = *(u8*)&arg0->specSectNrE;
        var_v1 = temp_v0_2 + 1;
        if (temp_v0_2 == 0)
        {
            var_v1=2;asm("");
        }
        temp_v0_3=arg0->symTabNrE;temp_a3=arg0->impSymIxsNrE;temp_v0_3-=temp_a3;temp_v0_3-=var_v1;
        if (temp_v0_3 != 0)
        {
            var_v1_2 = (temp_v0_3 * 0x10) + 0x33;
        }
        else
        {
            var_v1_2 = 0;
        }
        if (temp_a3 != 0)
        {
            var_v0 = var_v1_2 + (temp_a3 * 0x10) + 0x33;
        }
        else
        {
            var_v0 = var_v1_2;
        }
        var_a3 = var_v0 + arg1;
    }
    var_s7 = 0;
    temp_v0 = _isysCreateObj(arg2, 1, 0x38, var_a3, (s32) func_4001C4F8, 4);
    ((isysModuleObj*)temp_v0)->flags = (s32) (((isysModuleObj*)temp_v0)->flags & 0x7FFFFFFF);
    temp_v0_4 = func_40019D68(arg0, (isysModuleObj* ) temp_v0, temp_v0->partition, func_4001C3B0, (void*(*)(s32))func_4001C440, DldSysUndefSymbol, (s32(*)(char*,...))func_400199B0, arg6);
    temp_a0 = ((isysModuleObj*)temp_v0)->flags;
    if (temp_v0_4 != NULL)
    {
        var_s7 = temp_a0 >> 0x1F;asm("");
    }
    temp_s3 = (temp_v0_4 != NULL) ? temp_v0_4 : arg0;
    if (temp_v0_4 != NULL)
    {
        var_a2 = temp_v0_4->ident;
    }
    else
    {
        var_a2 = (u32) arg0->entryPnt;
    }
    temp_a1 = temp_v0_4 != NULL;
    ((isysModuleObj*)temp_v0)->entryPoint=(void*)var_a2;
    ((isysModuleObj*)temp_v0)->xff = temp_s3;
    ((isysModuleObj*)temp_v0)->flags = (s32) (((temp_a0 & 0x7FFFFFFF & 0xBFFFFFFF) | (temp_a1 << 0x1E)) & 0xDFFFFFFF);
    if (temp_a1 == 0)
    {
        if ((temp_s3->symTabNrE != 0) && (arg3 != 0))
        {
            isysAddGroupWithLinkParam(temp_v0, &symbolDefModuleGroup, 0, 0);
        }
        if ((temp_s3->impSymIxsNrE != 0) && (arg4 != 0))
        {
            isysAddGroupWithLinkParam(temp_v0, &needResolveModuleGroup, 0, 0);
        }
        if (arg5 != 0)
        {
            func_40019B58((isysModuleObj* ) temp_v0, temp_v0->partition);
        }
        func_4001A170(&((isysModuleObj*)temp_v0)->relocations, (s32) rootPtn, temp_s3);
    }
    if (temp_v0_4 != NULL)
    {
        ((isysModuleObj*)temp_v0)->flags = (s32) ((((isysModuleObj*)temp_v0)->flags & 0x7FFFFFFF) | (var_s7 << 0x1F));
    }
    return (isysModuleObj* ) temp_v0;
}

s32 isysResolveAllModuleObjHasUndefSymbol(void)
{
    isysLink* var_s1;
    isysObj* var_s0;
    s32 temp_s2;

    var_s0 = NULL;
    iosGetTCount();
    for (var_s1 = isysGroupForFirst(&needResolveModuleGroup, 0); var_s1 != NULL; var_s1 = isysGroupForNext(var_s1, 0))
    {
        var_s0 = var_s1->obj;
        iosResetCpuRapCounter(2);
        temp_s2 = func_40019FF0((isysModuleObj* ) var_s0);
        iosGetCpuRapCountPar1Int();
        if (((isysModuleObj*)var_s0)->flags & 0x20000000)
        {
            if (temp_s2 != 0)
            {
                func_40019A00((isysModuleObj* ) var_s0, 0);
            }
        }
        else
        {
            func_40019A00((isysModuleObj* ) var_s0, 1);
            ((isysModuleObj*)var_s0)->flags = (s32) (((isysModuleObj*)var_s0)->flags | 0x20000000);
        }
        iosGetCpuRapCountPar1Int();
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
    return 1;
}

isysModuleObj* func_4001A918(s8* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    isysObj* temp_v0;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a3;
    s32 var_s7;
    struct t_xffEntPntHdr* temp_s3;
    struct t_xffEntPntHdr* temp_v0_2;
    struct t_xffEntPntHdr* temp_v0_5;
    u32 var_a2;
    u8 temp_v0_3;

    temp_v0_2 = iosMallocAlign((s32) rootPtn, arg1, 0x10);
    func_4001C350(temp_v0_2, arg0, arg1);
    temp_v0_3 = *(u8*)&temp_v0_2->specSectNrE;
    var_a0 = temp_v0_3 + 1;
    if (temp_v0_3 == 0)
    {
        var_a0=2;asm("");
    }
    temp_v0_4=temp_v0_2->symTabNrE;temp_v1=temp_v0_2->impSymIxsNrE;temp_v0_4-=temp_v1;temp_v0_4-=var_a0;
    if (temp_v0_4 != 0)
    {
        var_a3 = (temp_v0_4 * 0x10) + 0x33;
    }
    else
    {
        var_a3 = 0;
    }
    if (temp_v1 != 0)
    {
        var_a3 = var_a3 + (temp_v1 * 0x10) + 0x33;
    }
    temp_v0 = _isysCreateObj(arg2, 1, 0x38, var_a3, (s32) func_4001C4F8, 4);
    ((isysModuleObj*)temp_v0)->flags = (s32) (((isysModuleObj*)temp_v0)->flags & 0x7FFFFFFF);
    var_s7 = 0;
    temp_v0_5 = func_40019D68(temp_v0_2, (isysModuleObj* ) temp_v0, temp_v0->partition, func_4001C3B0, (void*(*)(s32))func_4001C440, DldSysUndefSymbol, (s32(*)(char*,...))func_400199B0, 0);
    temp_a0 = ((isysModuleObj*)temp_v0)->flags;
    if (temp_v0_5 != NULL)
    {
        var_s7 = temp_a0 >> 0x1F;asm("");
    }
    temp_s3 = (temp_v0_5 != NULL) ? temp_v0_5 : temp_v0_2;
    if (temp_v0_5 != NULL)
    {
        var_a2 = temp_v0_5->ident;
    }
    else
    {
        var_a2 = (u32) temp_v0_2->entryPnt;
    }
    temp_a1 = temp_v0_5 != NULL;
    ((isysModuleObj*)temp_v0)->entryPoint=(void*)var_a2;
    ((isysModuleObj*)temp_v0)->xff = temp_s3;
    ((isysModuleObj*)temp_v0)->flags = (s32) (((temp_a0 & 0x7FFFFFFF & 0xBFFFFFFF) | (temp_a1 << 0x1E)) & 0xDFFFFFFF);
    if (temp_a1 == 0)
    {
        if ((temp_s3->symTabNrE != 0) && (arg3 != 0))
        {
            isysAddGroupWithLinkParam(temp_v0, &symbolDefModuleGroup, 0, 0);
        }
        if ((temp_s3->impSymIxsNrE != 0) && (arg4 != 0))
        {
            isysAddGroupWithLinkParam(temp_v0, &needResolveModuleGroup, 0, 0);
        }
        if (arg5 != 0)
        {
            func_40019B58((isysModuleObj* ) temp_v0, temp_v0->partition);
        }
        func_4001A170(&((isysModuleObj*)temp_v0)->relocations, (s32) rootPtn, temp_s3);
    }
    if (temp_v0_5 != NULL)
    {
        ((isysModuleObj*)temp_v0)->flags = (s32) ((((isysModuleObj*)temp_v0)->flags & 0x7FFFFFFF) | (var_s7 << 0x1F));
    }
    return (isysModuleObj* ) temp_v0;
}

s32 func_4001AB68(isysExecModuleObj* arg0, s8* arg1, struct t_xffEntPntHdr* arg2)
{
    s32 temp_a0;
    s32 temp_a1;
    s32 var_s5;
    s8* temp_v0_2;
    register struct t_xffEntPntHdr*temp_s2 __asm__("s2");
    struct t_xffEntPntHdr* temp_v0;
    u32 var_a2;

    var_s5 = 0;
    arg0->module.flags &= 0x7FFFFFFF;
    temp_v0 = func_40019D68(arg2, &arg0->module, arg0->module.obj.partition, func_4001C3B0, func_4001C3F8, DldSysUndefSymbol, (s32(*)(char*,...))func_400199B0, 0);
    temp_a0 = arg0->module.flags;
    if (temp_v0 != NULL)
    {
        var_s5 = temp_a0 >> 0x1F;asm("");
    }
    temp_s2 = (temp_v0 != NULL) ? temp_v0 : arg2;
    if (temp_v0 != NULL)
    {
        var_a2 = temp_v0->ident;
    }
    else
    {
        var_a2 = (u32) arg2->entryPnt;
    }
    temp_a1 = temp_v0 != NULL;
    arg0->module.entryPoint = (void* ) var_a2;
    arg0->module.xff = temp_s2;
    arg0->module.flags = ((temp_a0 & 0x7FFFFFFF & 0xBFFFFFFF) | (temp_a1 << 0x1E)) & 0xDFFFFFFF;
    if (temp_a1 == 0)
    {
        if (temp_s2->symTabNrE != 0)
        {
            isysAddGroupWithLinkParam(&arg0->module.obj, &symbolDefModuleGroup, 0, 0);
        }
        if (temp_s2->impSymIxsNrE != 0)
        {
            isysAddGroupWithLinkParam(&arg0->module.obj, &needResolveModuleGroup, 0, 0);
        }
        func_40019B58(&arg0->module, arg0->module.obj.partition);
        func_4001A170(&arg0->module.relocations, (s32) rootPtn, temp_s2);
    }
    if (temp_v0 != NULL)
    {
        arg0->module.flags = (arg0->module.flags & 0x7FFFFFFF) | (var_s5 << 0x1F);
    }
    if (arg1 != NULL)
    {
        temp_v0_2 = iosMallocAlign(arg0->module.obj.partition, strlen(arg1) + 1, 1);
        arg0->name = temp_v0_2;
        strcpy(temp_v0_2, arg1);
    }
    else
    {
        arg0->name = D_40045824;
    }
    isysAddGroupWithLinkParam(&arg0->module.obj, &executableModuleGroup, 0, 0);
    arg0->loadedBytes = 0;
    return 1;
}

isysExecModuleObj* func_4001AD58(s8* arg0, s32 arg1, s32 arg2)
{
    isysExecModuleObj* temp_v0;
    s32 temp_s5;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a1;register s32 importBytes __asm__("a2");
    s32 var_a3;
    s32 var_s0;
    struct t_xffEntPntHdr* temp_v0_2;
    u8 temp_v0_3;

    temp_s5 = D_40045820;
    temp_v0_2 = iosMallocAlign((s32) rootPtn, arg1, 0x40);
    func_4001C350(temp_v0_2, arg0, arg1);
    temp_v0_3 = *(u8*)&temp_v0_2->specSectNrE;
    var_a3 = 0;
    var_a1 = 2;
    if (temp_v0_3 != 0)
    {
        var_a1 = temp_v0_3 + 1;asm("" : "+r"(var_a1));
    }
    temp_v1=temp_v0_2->impSymIxsNrE;importBytes=temp_v1*16;asm("" : "+r"(importBytes));
    temp_v0_4 = (temp_v0_2->symTabNrE - temp_v1) - var_a1;
    if (temp_v0_4 != 0)
    {
        register s32 bytes __asm__("a1")=temp_v0_4*16;var_a3=bytes+0x33;
    }
    if (temp_v1 != 0)
    {
        var_s0 = var_a3 + importBytes + 0x33;
    }
    else
    {
        var_s0 = var_a3;
    }
    asm("" : "+r"(var_s0) : "r"(importBytes));
    { s32 length=strlen(arg0);asm("" : "+r"(length));
    temp_v0 = (isysExecModuleObj*)_isysCreateObj(arg2, 2, 0x40, length + var_s0 + 1, (s32) func_4001C5E8, 4);
    }
    func_4001AB68(temp_v0, arg0, temp_v0_2);
    temp_v1_2 = D_40045820 + arg1;
    D_40045820 = temp_v1_2;
    temp_v0->loadedBytes = temp_v1_2 - temp_s5;
    return temp_v0;
}

isysExecModuleObj* func_4001AE88(s8* arg0, struct t_xffEntPntHdr* arg1, s32 arg2)
{
    isysObj* temp_v0;
    s32 temp_a0;s32 flags;
    s32 temp_v0_3;
    s32 var_s0;
    s32 var_v1;
    s32 var_v1_2;
    s8* temp_v0_4;
    u8 temp_v0_2;

    temp_v0_2 = *(u8*)&arg1->specSectNrE;
    var_v1 = temp_v0_2 + 1;
    if (temp_v0_2 == 0)
    {
        var_v1=2;asm("");
    }
    temp_v0_3=arg1->symTabNrE;temp_a0=arg1->impSymIxsNrE;temp_v0_3-=temp_a0;temp_v0_3-=var_v1;
    if (temp_v0_3 != 0)
    {
        var_v1_2 = (temp_v0_3 * 0x10) + 0x33;
    }
    else
    {
        var_v1_2 = 0;
    }
    if (temp_a0 != 0)
    {
        var_s0 = var_v1_2 + (temp_a0 * 0x10) + 0x33;
    }
    else
    {
        var_s0 = var_v1_2;
    }
    temp_v0 = _isysCreateObj(arg2, 2, 0x40, strlen(arg0) + var_s0 + 1, 0, 4);
    ((isysModuleObj*)temp_v0)->entryPoint = (void* ) arg1->entryPnt;
    ((isysModuleObj*)temp_v0)->xff = arg1;
    flags=((isysModuleObj*)temp_v0)->flags;flags&=0x7fffffff;flags&=0xbfffffff;flags&=0xdfffffff;((isysModuleObj*)temp_v0)->flags=flags;
    if (arg1->symTabNrE != 0)
    {
        isysAddGroupWithLinkParam(temp_v0, &symbolDefModuleGroup, 0, 0);
    }
    if (arg1->impSymIxsNrE != 0)
    {
        isysAddGroupWithLinkParam(temp_v0, &needResolveModuleGroup, 0, 0);
    }
    func_40019B58((isysModuleObj* ) temp_v0, temp_v0->partition);
    func_4001A170(&((isysModuleObj*)temp_v0)->relocations, (s32) rootPtn, arg1);
    if (arg0 != NULL)
    {
        temp_v0_4 = iosMallocAlign(temp_v0->partition, strlen(arg0) + 1, 1);
        ((isysExecModuleObj*)temp_v0)->name = temp_v0_4;
        strcpy(temp_v0_4, arg0);
    }
    else
    {
        ((isysExecModuleObj*)temp_v0)->name = (s8* ) D_40045824;
    }
    isysAddGroupWithLinkParam(temp_v0, &executableModuleGroup, 0, 0);
    ((isysExecModuleObj*)temp_v0)->loadedBytes = 0;
    return (isysExecModuleObj* ) temp_v0;
}

static inline s32 dispose_module_relocations(isysModuleObj* arg0)
{
    s32 temp_s0;
    s32 var_a0_2;
    register s32 var_a1 __asm__("a1");
    s32 var_v0;register u32 identifier __asm__("v1");
    s32 var_v0_2;
    register s32 var_v1 __asm__("v1");
    register s32 var_v1_2 __asm__("v1");
    struct t_xffEntPntHdr* temp_s1;
    u32 temp_v0;
    register u32*var_a0 __asm__("a0");
    void* temp_v0_2;

    temp_s1 = arg0->xff;
    var_v0 = 0;
    identifier=temp_s1->ident;if(identifier==D_40045828)
    {
        temp_s0 = (s32) temp_s1->relocTabNrE >> 1;
        var_a1 = 0;
        if (temp_s0 > 0)
        {
            var_v1 = temp_s0;
            var_a0 = &temp_s1->relocTab->nrEnt;
            do
            {
                temp_v0 = *var_a0;
                var_a0+=7;
                var_v1 -= 1;
                var_a1 += temp_v0;
            } while (var_v1 != 0);
        }
        func_4001C8F0(temp_s1, temp_s1->relocTab->addr+var_a1,0);
        var_v0_2 = temp_s0 * 8;
        if (temp_s0 > 0)
        {
            var_v1_2=temp_s0;asm("" : "+r"(var_v0_2) : "r"(var_v1_2));
            var_a0_2=(var_v0_2-temp_s0)*4;
            do
            {
                var_v1_2 -= 1;
                temp_v0_2=(void*)(var_a0_2+(u32)*(struct t_xffRelocEnt*volatile*)&temp_s1->relocTab);
                var_a0_2 += 0x1C;
                ((struct t_xffRelocEnt*)temp_v0_2)->nrEnt=0;
            } while (var_v1_2 != 0);
            asm("" : "+r"(temp_s0));var_v0_2=temp_s0*8;
        }
{ register u32 table __asm__("a0")=(u32)temp_s1->relocTab;
register s32 offset __asm__("v0")=var_v0_2;asm("" : "+r"(offset) : "r"(table));offset-=temp_s0;asm volatile("" : : "r"(offset));{register s32 size __asm__("a1")=temp_s1->stack_Rel;register s32 end __asm__("v1");
asm("" : "+r"(offset) : "r"(size));offset*=4;end=*(s32*)(offset+table+12)-(s32)temp_s1;
var_v0=size-end;}}
    }
    return var_v0;
}

void isysLaunchModule(isysExecModuleObj* arg0)
{
    isysLink* var_s1;
    isysObj* var_s0;
    char filename[1024];s32 temp_s0_2;
    s32 temp_s2;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_v0;
    s32 var_v1;
    s32 var_v1_2;
    struct t_xffEntPntHdr* temp_s0;
    struct t_xffEntPntHdr* temp_s1;
    struct t_xffRelocEnt* temp_a2;
    u32 temp_v0;
    u32* var_a0;
    void* temp_v0_2;

    var_s0 = NULL;
    var_s1 = isysGroupForFirst(&needResolveModuleGroup, 0);
    if (var_s1 != NULL)
    {
        do
        {
            var_s0 = var_s1->obj;
            iosResetCpuRapCounter(2);
            temp_s2 = func_40019FF0((isysModuleObj* ) var_s0);
            iosGetCpuRapCountPar1Int();
            if (((isysModuleObj*)var_s0)->flags & 0x20000000)
            {
                if (temp_s2 != 0)
                {
                    func_40019A00((isysModuleObj* ) var_s0, 0);
                }
            }
            else
            {
                func_40019A00((isysModuleObj* ) var_s0, 1);
                ((isysModuleObj*)var_s0)->flags = (s32) (((isysModuleObj*)var_s0)->flags | 0x20000000);
            }
            iosGetCpuRapCountPar1Int();
            var_s1 = isysGroupForNext(var_s1, 0);
        } while (var_s1 != NULL);
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
    temp_s0 = arg0->module.xff;
    iosSPrintf(filename, D_40047060, arg0->name);
    OutputLinkerScriptFile(temp_s0, filename, printf);
    var_a1=dispose_module_relocations(&arg0->module);
    {register s32 loaded __asm__("v0")=arg0->loadedBytes;register s32*counter __asm__("a0");register u64 lateS0 asm("s0");register s32 balance __asm__("v1");asm("" : "+r"(loaded) : : "a0");counter=&D_40045820;asm volatile("" : "=r"(lateS0) : "r"(counter));asm("" : "+r"(loaded) : "r"(counter));loaded-=var_a1;arg0->loadedBytes=loaded;balance=*counter;*counter=balance-var_a1;}
}


extern char D_40047028[],D_40047030[],D_40047058[],D_400470E0[],D_400470F8[],D_40047110[];
extern u32 iosDefaultPadData;
void LoaderSysHookPoint(void);
static inline isysExecModuleObj* simulationInitialModule(char* name,s32 heap,sceCdlFILE* file)
{
 register s32 size asm("a1");
 isysExecModuleObj* module;
 if(strncmp(name,D_40047058,7)==0) {
  char* cdName=strstr(name,D_40047028)+1;
  if(!sceCdSearchFile(file,cdName))iosJumpRecoverPoint(D_40047030,cdName);
  module=NULL;
  size=(s32)file->size;
  if(size>=0)module=func_4001AD58(name,size,heap);
 }else {
  size=iosLoaderGetFileSizeName(name);
  module=NULL;
  if(size>=0)module=func_4001AD58(name,size,heap);
 }

 return module;
}
static inline isysExecModuleObj* simulationLoadModule(char* name,s32 heap,sceCdlFILE* file)
{
 register s32 size asm("a1");
 register char*callName asm("a0");
 s32 count;
 register isysExecModuleObj* module asm("s2")=NULL;
 if(strncmp(name,D_40047058,7)==0) {
  char* cdName=strstr(name,D_40047028)+1;
  if(!sceCdSearchFile(file,cdName))iosJumpRecoverPoint(D_40047030,cdName);
  callName=name;size=(s32)file->size;
  if(size>=0)goto load;
 }else {
  count=iosLoaderGetFileSizeName(name);
  if(count>=0){callName=name;asm("" : "+r"(callName));size=count;
load: module=func_4001AD58(callName,size,heap);}
 }
 return module;
}
s32 IosCdvdManagerSimulation(void)
{
 sceCdlFILE file;
 register isysExecModuleObj* module asm("s1");
 register isysExecModuleObj* defaultModule asm("s2");
 register u32(*entry)(isysObj*) asm("s0");
 register u32 entryResult asm("v0");
 register char* secondName asm("s1");
 if((iosDefaultPadData & 0x900)!=0x900) {
  module=simulationInitialModule(D_400470E0,(s32)rootPtn,&file);
  asm("" : "+r"(module));
  if(!module)return 0;
  isysLaunchModule(module);
  entry=(u32(*)(isysObj*))module->module.xff->entryPnt;
  FlushCache(0);FlushCache(2);entryResult=entry((isysObj*)module);
  /* Retain the callback result register through the next filename address. */
  secondName=D_400470F8;
  asm("" : "+r"(entryResult) : "r"(secondName));
  module=simulationLoadModule(secondName,(s32)rootPtn,&file);
  asm("" : "+r"(module));
  if(!module)return 0;
  LoaderSysHookPoint();
  isysLaunchModule(module);
  entry=(u32(*)(isysObj*))module->module.xff->entryPnt;
  FlushCache(0);FlushCache(2);entry((isysObj*)module);
 }else {
  defaultModule=simulationLoadModule(D_40047110,(s32)rootPtn,&file);
  asm("" : "+r"(defaultModule));
  if(!defaultModule)return 0;
  isysLaunchModule(defaultModule);
  entry=(u32(*)(isysObj*))defaultModule->module.xff->entryPnt;
  FlushCache(0);FlushCache(2);entry((isysObj*)defaultModule);
 }
 return 1;
}
__asm__(".globl func_4001B2E0\nfunc_4001B2E0=IosCdvdManagerSimulation+0x98\n.globl func_4001B3D8\nfunc_4001B3D8=IosCdvdManagerSimulation+0x190\n.globl func_4001B498\nfunc_4001B498=IosCdvdManagerSimulation+0x250");


extern char D_40046EF8[], D_40046F58[], D_40046FC0[];
void* isysGetXffEntryPointPreRelocation(struct t_xffEntPntHdr* xff)
{
    struct t_xffSectEnt* section;
    s32 i;
    register s32 nextCount asm("v0");
    xff->sectTab = (void*)((char*)xff + xff->sectTab_Rel);
    section = xff->sectTab + 1;
    i=1;
    if(i<xff->sectNrE)do
    {
        section->filePt = (char*)xff + section->offs_Rel;
        if (section->size != 0)
        {
            switch(section->type)
            {
                case 1:
                case 0x7FFFF420:
                    if (section->flags != 0)
                        iosJumpRecoverPoint(D_40046EF8);
                    else {register s32 align asm("a1");register u32 mask asm("v0");
                        align=section->align;
                        mask=(u32)section->filePt & (align - 1);
                        if(mask)iosJumpRecoverPoint(D_40046F58);
                        else return (char*)section->filePt + xff->entryPnt_Rel;
                    }
                    break;
                case 8:
                    iosJumpRecoverPoint(D_40046FC0, section->align);
                    break;
            }
        }
        nextCount=xff->sectNrE;
        asm("" : "+r"(nextCount));
        i++;
        section++;
    }while(i<nextCount);
    return NULL;
}


void InitDld(void)
{
    iosCdInit();
    InitStrHash(&D_40081A18, (s32)rootPtn, 0x10000);
    isysInitGroup(&executableModuleGroup);
    isysInitGroup(&needResolveModuleGroup);
    isysInitGroup(&symbolDefModuleGroup);
    LoaderSysGetMemoryInfo(&D_400819B0);
    func_4001AE88(D_400819B0.module_info.path, (struct t_xffEntPntHdr*)D_400819B0.module_info.unk44, (s32) &rootPtn);
    isysCreateExceptionThread();
}

void IosLoadIrxSimulation(void)
{
    iosLoadIopModule(D_40047068, D_40047088, 0, 0);
    iosLoadIopModule(D_40047090, D_400470A8, 0, 0);
    iosLoadIopModule(D_400470B8, D_400470D8, 0, 0);
}

char* IosGetMergeDataFileName(void)
{
    return D_40047138;
}

s32 isysSearchSymbol(void**out,u32 hash){
 t_hashedSym* found ;
s32 result;
if(!hash){*out=NULL;return 1;}
found=SearchStrHashKey(&D_40081A18,hash);

if(found){ struct t_xffSymEnt*symbol =found->locSym;*out=symbol->addr;}
return found!=NULL;
}

s32 isysSearchModuleFuncNameByAddr(char*moduleName,char*functionName,u32 address){
isysLink*link;isysObj*obj=NULL;register s32 result __asm__("s6")=0;char*found;struct t_xffEntPntHdr*xff;
link=isysGroupForFirst(&executableModuleGroup,0);
if(link){do{obj=link->obj;xff=((isysModuleObj*)obj)->xff;{
s32 remaining=xff->symTabNrE-2;char*strings=xff->symTabStr;asm("" : : "r"(strings));{struct t_xffSymEnt*symbol=xff->symTab;
if(remaining!=-1){do{if(symbol->sect!=0xfff1 && (*(u8*)((u8*)symbol+12)&15)==2 && (u32)symbol->addr==address){found=strings+symbol->nameOffs;goto gotSymbol;}remaining--;symbol++;}while(remaining!=-1);}found=NULL;gotSymbol:;}}

if(found){if(moduleName)strcpy(moduleName,((isysExecModuleObj*)obj)->name);if(functionName){strcpy(functionName,found);result=1;}else result=1;break;}
link=isysGroupForNext(link,0);
}while(link);}
asm("" : "+r"(result));if(obj)isysGroupForExit(obj,0);return result;}


void isysDumpExecutableModules(void)
{
    isysLink* var_a0;
    isysLink* var_v0;
    isysObj* var_s0;

    var_s0 = NULL;
    var_v0 = isysGroupForFirst(&executableModuleGroup, 0);
    var_a0 = var_v0;
    if (var_v0 != NULL)
    {
        do
        {
            var_s0 = var_v0->obj;
            var_v0 = isysGroupForNext(var_a0, 0);
            var_a0 = var_v0;
        } while (var_v0 != NULL);
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
}

isysObj* isysSearchModuleSymbolByGroup(void** out, char* name, isysGroup* group, isysObj* exclude)
{
    isysObj* volatile result;
    isysLink* link;
    isysObj* obj=NULL;
    struct t_xffEntPntHdr* xff;
    struct t_xffSymEnt* symbol;
    char* strings;
    s32 remaining;
    register s32 count asm("v0");
    register s32 sentinel asm("v1");
    s32 found;
    u32 hash;
    register u32 masked asm("v0");
    register void* address asm("v0");
    t_hashedSym* hashed;

    link=isysGroupForFirst(group,0);
    result=NULL;
    if(link!=NULL)do {
        obj=link->obj;
        if(obj==exclude)goto next;
        hash=MakeStrHashValue(name);
        masked=((isysModuleObj*)obj)->flags;
        asm("" : "+r"(masked) : : "v1");
        masked &=0x80000000;
        asm("" : "+r"(masked) : : "memory");
        xff=((isysModuleObj*)obj)->xff;
        if(masked) {
            hashed=SearchStrHashKey(&D_40081A18,hash);
            found=1;
            if(hashed) {
                address=hashed->locSym->addr;*out=address;
                goto checked;
            }
            goto failed;
        }
        goto symbols;
symbolFound:
        found=1;
        *out=address;
        goto checked;
symbols:
        count=xff->symTabNrE;
        sentinel=-1;
        strings=xff->symTabStr;asm("" : : "r"(strings),"r"(count),"r"(sentinel));
        remaining=count-2;
        symbol=xff->symTab;
        if(remaining!=sentinel)do {
            if(symbol->sect && !strcmp(strings+symbol->nameOffs,name)) {
                address=symbol->addr;
                goto symbolFound;
            }
            /* The original loop tests this same symbol on each pass. */
            remaining--;
        }while(remaining!=-1);
failed:
        found=0;
checked:
        if(found) {result=obj;break;}
next:
        link=isysGroupForNext(link,0);
    }while(link);
    if(obj)isysGroupForExit(obj,0);
    return (isysObj*)result;
}


void isysRemoveModuleObjGroup(isysObj* arg0)
{
    if (((isysModuleObj*)arg0)->xff->symTabNrE != 0)
    {
        _isysDeleteGroup(arg0, &symbolDefModuleGroup, 0);
    }
    if (((isysModuleObj*)arg0)->xff->impSymIxsNrE != 0)
    {
        _isysDeleteGroup(arg0, &needResolveModuleGroup, 0);
    }
}

s32 isysResolveOneModuleObj(isysModuleObj* arg0)
{
    s32 temp_s1;

    iosResetCpuRapCounter(2);
    temp_s1 = func_40019FF0(arg0);
    iosGetCpuRapCountPar1Int();
    if (arg0->flags & 0x20000000)
    {
        if (temp_s1 != 0)
        {
            func_40019A00(arg0, 0);
        }
    }
    else
    {
        func_40019A00(arg0, 1);
        arg0->flags |= 0x20000000;
    }
    iosGetCpuRapCountPar1Int();
    return temp_s1;
}

s32 isysResolveAllProgramModule(void)
{
    isysLink* var_s1;
    isysObj* var_s0;
    s32 temp_s2;

    var_s0 = NULL;
    iosGetTCount();
    for (var_s1 = isysGroupForFirst(&executableModuleGroup, 0); var_s1 != NULL; var_s1 = isysGroupForNext(var_s1, 0))
    {
        var_s0 = var_s1->obj;
        iosResetCpuRapCounter(2);
        temp_s2 = func_40019FF0((isysModuleObj* ) var_s0);
        iosGetCpuRapCountPar1Int();
        if (((isysModuleObj*)var_s0)->flags & 0x20000000)
        {
            if (temp_s2 != 0)
            {
                func_40019A00((isysModuleObj* ) var_s0, 0);
            }
        }
        else
        {
            func_40019A00((isysModuleObj* ) var_s0, 1);
            ((isysModuleObj*)var_s0)->flags = (s32) (((isysModuleObj*)var_s0)->flags | 0x20000000);
        }
        iosGetCpuRapCountPar1Int();
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
    return 1;
}

void isysLaunchExecModule(void* arg0)
{
    void (*temp_s1)(void*);

    isysLaunchModule(arg0);
    temp_s1 = (void (*)(void*))((isysModuleObj*)arg0)->xff->entryPnt;
    FlushCache(0);
    FlushCache(2);
    temp_s1(arg0);
}

isysModuleObj* isysInitModuleObjExisted(isysModuleObj* arg0, struct t_xffEntPntHdr* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s32 temp_a0;
    s32 temp_a1;
    s32 var_s7;
    struct t_xffEntPntHdr* temp_s2;
    struct t_xffEntPntHdr* temp_v0;
    u32 var_a2;

    var_s7 = 0;
    arg0->flags &= 0x7FFFFFFF;
    temp_v0 = func_40019D68(arg1, arg0, arg0->obj.partition, func_4001C3B0, (void*(*)(s32))func_4001C440, DldSysUndefSymbol, (s32(*)(char*,...))func_400199B0, arg5);
    temp_a0 = arg0->flags;
    if (temp_v0 != NULL)
    {
        var_s7 = temp_a0 >> 0x1F;asm("");
    }
    temp_s2 = (temp_v0 != NULL) ? temp_v0 : arg1;
    if (temp_v0 != NULL)
    {
        var_a2 = temp_v0->ident;
    }
    else
    {
        var_a2 = (u32) arg1->entryPnt;
    }
    temp_a1 = temp_v0 != NULL;
    arg0->entryPoint = (void* ) var_a2;
    arg0->xff = temp_s2;
    arg0->flags = ((temp_a0 & 0x7FFFFFFF & 0xBFFFFFFF) | (temp_a1 << 0x1E)) & 0xDFFFFFFF;
    if (temp_a1 == 0)
    {
        if ((temp_s2->symTabNrE != 0) && (arg2 != 0))
        {
            isysAddGroupWithLinkParam(&arg0->obj, &symbolDefModuleGroup, 0, 0);
        }
        if ((temp_s2->impSymIxsNrE != 0) && (arg3 != 0))
        {
            isysAddGroupWithLinkParam(&arg0->obj, &needResolveModuleGroup, 0, 0);
        }
        if (arg4 != 0)
        {
            func_40019B58(arg0, arg0->obj.partition);
        }
        func_4001A170(&arg0->relocations, (s32) rootPtn, temp_s2);
    }
    if (temp_v0 != NULL)
    {
        arg0->flags = (arg0->flags & 0x7FFFFFFF) | (var_s7 << 0x1F);
    }
    return arg0;
}

s32 isysFlushModuleObj(isysModuleObj* arg0)
{
    isysModuleRelocation*temp_a0;
    isysModuleRelocations* list;
    s32 var_s1;
    struct t_xffEntPntHdr* temp_s2;
    struct t_xffSectEnt* var_s0;

    if(arg0->flags&0x40000000){iosFree((void*)arg0->xff->u04);func_4001A2F8(arg0);}
    else{
        list=&arg0->relocations;
        temp_a0=list->entries;
        if (temp_a0 != NULL)
        {
            iosFree(temp_a0);
            list->count=0;
        }
        func_4001A2F8(arg0);
        temp_s2 = arg0->xff;
        if (temp_s2 != NULL)
        {
            var_s1 = temp_s2->sectNrE - 2;
            var_s0 = temp_s2->sectTab + 1;
            if (var_s1 != -1)
            {
                do
                {
                    if (var_s0->moved != 0)
                    {
                        var_s0->moved = 0;
                        iosFree(var_s0->memPt);
                    }
                    var_s1 -= 1;
                    var_s0++;
                } while (var_s1 != -1);
            }
            iosFree(temp_s2);
        }
    }
return 0;
}

char* GetSymbolByIndex(isysModuleObj* module, s32 index)
{
    struct t_xffEntPntHdr* xff = module->xff;
    return xff->symTabStr + xff->symTab[index + xff->sectNrE].nameOffs;
}

void isysRelocateModuleObj(isysModuleObj* module){
register s32 mode __asm__("a1")=0;
if(module->flags&0x20000000){func_40019A00(module,mode);return;}func_40019A00(module,1);module->flags|=0x20000000;}

s32 isysGetXffWorkSize(struct t_xffEntPntHdr*xff){
u8 special=*(u8*)&xff->specSectNrE;
register s32 specialCount __asm__("a1")=special+1;
register s32 imports __asm__("v1");
register s32 importBytes __asm__("a2");
s32 exports;s32 work;
if(!special){asm("");specialCount=2;}

imports=xff->impSymIxsNrE;

importBytes=imports*16;

exports=xff->symTabNrE-imports-specialCount;
if(exports)work=exports*16+0x33;else work=0;
if(imports){work=work+importBytes;asm("" : "+r"(work));work+=0x33;}
return work;}

s32 isysGetModuleObjMallocSize(isysModuleObj*module){u32**header;register s32 flags __asm__("v0");void*xff;
flags=module->flags;
if(flags&0x40000000){register s32 original __asm__("v1");
xff=module->xff;original=*(s32*)((u8*)xff+4);
header=(u32**)(original-4);
}else header=(u32**)((s32)module->xff-4);
return(**header&0xFFFFFF)*4;}

s32 isysDisposeModuleObjRelocationElement(isysModuleObj* arg0)
{
    s32 temp_s0;
    s32 var_a0_2;
    register s32 var_a1 __asm__("a1");
    register s32 var_v0 __asm__("v0");
    s32 var_v0_2;
    register s32 var_v1 __asm__("v1");
    register s32 var_v1_2 __asm__("v1");
    struct t_xffEntPntHdr* temp_s1;
    u32 temp_v0;
    register u32*var_a0 __asm__("a0");
    void* temp_v0_2;

    temp_s1 = arg0->xff;
    var_v0 = 0;
    if (temp_s1->ident == D_40045828)
    {
        temp_s0 = (s32) temp_s1->relocTabNrE >> 1;
        var_a1 = 0;
        if (temp_s0 > 0)
        {
            var_v1 = temp_s0;
            var_a0 = &temp_s1->relocTab->nrEnt;
            do
            {
                temp_v0 = *var_a0;
                var_a0+=7;
                var_v1 -= 1;
                var_a1 += temp_v0;
            } while (var_v1 != 0);
        }
        func_4001C8F0(temp_s1, temp_s1->relocTab->addr+var_a1,0);
        var_v0_2 = temp_s0 * 8;
        if (temp_s0 > 0)
        {
            var_v1_2=temp_s0;asm("" : "+r"(var_v0_2) : "r"(var_v1_2));
            var_a0_2=(var_v0_2-temp_s0)*4;
            do
            {
                var_v1_2 -= 1;
                temp_v0_2=(void*)(var_a0_2+(u32)*(struct t_xffRelocEnt*volatile*)&temp_s1->relocTab);
                var_a0_2 += 0x1C;
                ((struct t_xffRelocEnt*)temp_v0_2)->nrEnt=0;
            } while (var_v1_2 != 0);
            asm("" : "+r"(temp_s0));var_v0_2=temp_s0*8;
        }
{ register u32 table __asm__("a0")=(u32)temp_s1->relocTab;
register s32 offset __asm__("v0")=var_v0_2;asm("" : "+r"(offset) : "r"(table));offset-=temp_s0;asm volatile("" : : "r"(offset));{register s32 size __asm__("a1")=temp_s1->stack_Rel;register s32 end __asm__("v1");
asm("" : "+r"(offset) : "r"(size));offset*=4;end=*(s32*)(offset+table+12)-(s32)temp_s1;
var_v0=size-end;}}
    }
    return var_v0;
}

s32 GetExecModuleSize(void)
{
    return D_40045820;
}

u32 getModuleSize(isysModuleObj* module)
{
    return module->xff->stack_Rel;
}

void* func_4001C220(s32 size, s32 alignment)
{
    return iosMallocAlign(D_40081A20, size, alignment);
}

void* func_4001C248(s32 size)
{
    return iosMallocAlign(D_40081A20, size, 0x10);
}

s32 isysGetUndefModuleObjNum(void)
{
    isysLink* var_v0;
    isysObj* var_s0;
    s32 var_s1;

    var_s0 = NULL;
    var_s1 = 0;
    var_v0 = isysGroupForFirst(&needResolveModuleGroup, 0);
    if (var_v0 != NULL)
    {
        do
        {
            var_s0 = var_v0->obj;
            var_v0 = isysGroupForNext(var_v0, 0);
            var_s1 += 1;
        } while (var_v0 != NULL);
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
    return var_s1;
}

s32 isysGetSymdefModuleObjNum(void)
{
    isysLink* var_v0;
    isysObj* var_s0;
    s32 var_s1;

    var_s0 = NULL;
    var_s1 = 0;
    var_v0 = isysGroupForFirst(&symbolDefModuleGroup, 0);
    if (var_v0 != NULL)
    {
        do
        {
            var_s0 = var_v0->obj;
            var_v0 = isysGroupForNext(var_v0, 0);
            var_s1 += 1;
        } while (var_v0 != NULL);
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
    return var_s1;
}

void func_4001C350(void* buffer, s8* filename, s32 size)
{
    s32 temp_v0;

    temp_v0 = iosLoaderFOpen(filename, 1, 0);
    iosLoaderFRead(temp_v0, buffer, size);
    iosLoaderFClose(temp_v0);
}

void* func_4001C3B0(s32 size,s32 alignment){void*result=iosMallocAlign((s32)rootPtn,size,alignment);D_40045820+=size;return result;}

void* func_4001C3F8(s32 size){void*result=iosMallocAlign((s32)rootPtn,size,0x40);D_40045820+=size;return result;}

s32 func_4001C440(void)
{
    iosJumpRecoverPoint(D_40046EC0);
    return 0;
}

struct t_xffEntPntHdr* func_4001C468(struct t_xffEntPntHdr* arg0)
{
    s32 var_s1;
    struct t_xffSectEnt* var_s0;

    if (arg0 != NULL)
    {
        var_s1 = arg0->sectNrE - 2;
        var_s0 = arg0->sectTab + 1;
        if (var_s1 != -1)
        {
            do
            {
                if (var_s0->moved != 0)
                {
                    var_s0->moved = 0;
                    iosFree(var_s0->memPt);
                }
                var_s1 -= 1;
                var_s0++;
            } while (var_s1 != -1);
        }
        iosFree(arg0);
    }
    return arg0;
}

void func_4001C4F8(isysModuleObj* arg0)
{
    isysModuleRelocation*temp_a0;
    isysModuleRelocations* list;
    s32 var_s1;
    struct t_xffEntPntHdr* temp_s2;
    struct t_xffSectEnt* var_s0;

    if(arg0->flags&0x40000000){iosFree((void*)arg0->xff->u04);func_4001A2F8(arg0);return;}

        list=&arg0->relocations;
        temp_a0=list->entries;
        if (temp_a0 != NULL)
        {
            iosFree(temp_a0);
            list->count=0;
        }
        func_4001A2F8(arg0);
        temp_s2 = arg0->xff;
        if (temp_s2 != NULL)
        {
            var_s1 = temp_s2->sectNrE - 2;
            var_s0 = temp_s2->sectTab + 1;
            if (var_s1 != -1)
            {
                do
                {
                    if (var_s0->moved != 0)
                    {
                        var_s0->moved = 0;
                        iosFree(var_s0->memPt);
                    }
                    var_s1 -= 1;
                    var_s0++;
                } while (var_s1 != -1);
            }
            iosFree(temp_s2);
        }


}

void func_4001C5E8(isysExecModuleObj* arg0)
{
    isysModuleRelocation*temp_a0;
    isysModuleRelocations* list;
    s32 var_s1;
    struct t_xffEntPntHdr* temp_s3;
    struct t_xffSectEnt* var_s0;

    if(arg0->module.flags&0x40000000){iosFree((void*)arg0->module.xff->u04);func_4001A2F8(&arg0->module);}
    else{
        list=&arg0->module.relocations;
        temp_a0=list->entries;
        if (temp_a0 != NULL)
        {
            iosFree(temp_a0);
            list->count=0;
        }
        func_4001A2F8(&arg0->module);
        temp_s3 = arg0->module.xff;
        if (temp_s3 != NULL)
        {
            var_s1 = temp_s3->sectNrE - 2;
            var_s0 = temp_s3->sectTab + 1;
            if (var_s1 != -1)
            {
                do
                {
                    if (var_s0->moved != 0)
                    {
                        var_s0->moved = 0;
                        iosFree(var_s0->memPt);
                    }
                    var_s1 -= 1;
                    var_s0++;
                } while (var_s1 != -1);
            }
            iosFree(temp_s3);
        }
    }
D_40045820-=arg0->loadedBytes;
}

s32 isysResolveAllModuleObjOfGroup(isysGroup* arg0)
{
    register isysLink* var_s1 __asm__("s1");
    register isysObj* var_s0 __asm__("s0");
    register s32 temp_s2 __asm__("s2");
    s32 var_s4;

    var_s0 = NULL;
    var_s4 = 0;
    var_s1 = isysGroupForFirst(arg0, 0);
    if (var_s1 != NULL)
    {
        do
        {
            var_s0 = var_s1->obj;
            iosResetCpuRapCounter(2);
            temp_s2 = func_40019FF0((isysModuleObj* ) var_s0);
            iosGetCpuRapCountPar1Int();
            if (((isysModuleObj*)var_s0)->flags & 0x20000000)
            {
                if (temp_s2 != 0)
                {
                    func_40019A00((isysModuleObj* ) var_s0, 0);
                }
            }
            else
            {
                func_40019A00((isysModuleObj* ) var_s0, 1);
                ((isysModuleObj*)var_s0)->flags = (s32) (((isysModuleObj*)var_s0)->flags | 0x20000000);
            }
            var_s4 += 1;
            iosGetCpuRapCountPar1Int();
            var_s1 = isysGroupForNext(var_s1, 0);
        } while (var_s1 != NULL);
    }
    if (var_s0 != NULL)
    {
        isysGroupForExit(var_s0, 0);
    }
    return var_s4;
}

extern char D_40047058[], D_40047028[], D_40047030[];

isysExecModuleObj* readEModule(char* arg0, s32 arg1)
{
    sceCdlFILE file;
    register isysExecModuleObj*var_s2 __asm__("s2");
    char* temp_s0;
    s32 temp_v0;register char* callName __asm__("a0");
    register s32 var_a1 __asm__("a1");

    var_s2 = NULL;
    if (strncmp(arg0, D_40047058, 7U) == 0)
    {
        temp_s0 = strstr(arg0, D_40047028) + 1;
        if (sceCdSearchFile(&file, temp_s0) == 0)
        {
            iosJumpRecoverPoint(D_40047030, temp_s0);
        }
        callName=arg0;var_a1 = (s32)file.size;
        if (var_a1 >= 0)
        {
            goto block_8;
        }
    }
    else
    {
        temp_v0 = iosLoaderGetFileSizeName(arg0);
        if (temp_v0 >= 0)
        {
            callName=arg0;asm("" : "+r"(callName));var_a1 = temp_v0;
block_8:
            var_s2 = func_4001AD58(callName, var_a1, arg1);
        }
    }
    return var_s2;
}


void isysWriteLinkerScriptFile(void* arg0)
{
    char filename[1024];
    struct t_xffEntPntHdr* temp_s0;

    temp_s0 = ((isysModuleObj*)arg0)->xff;
    iosSPrintf(filename, D_40047060, *(char**)((u8*)arg0+0x38));
    OutputLinkerScriptFile(temp_s0, filename, printf);
}

void func_4001C8F0(void* ptr, void* start, s32 size)
{
    iosFreeParts(ptr, start, 0);
}

isysObj* loadtest(s8* arg0, s32 arg1)
{
    isysObj* var_v1;
    s32 temp_v0;

    temp_v0 = iosLoaderGetFileSizeName(arg0);
    var_v1 = NULL;
    if (temp_v0 >= 0)
    {
        var_v1 = (isysObj*)func_4001AD58(arg0, temp_v0, arg1);
    }
    return var_v1;
}

/* Distinct compiler symbols prevent caching this table address across
 * Loader calls; assembler aliases retain the original relocation target. */
extern s32 loaderCacheOpen[], loaderCacheSize[], loaderCacheClose[];
__asm__("loaderCacheOpen=D_40081A28\nloaderCacheSize=D_40081A28\nloaderCacheClose=D_40081A28");
s32 iosLoaderGetFileSizeName(char* arg0)
{
    struct sce_stat stats;register s32*previous __asm__("v1");register s32 wanted __asm__("a3");
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    register s32 statValue asm("v1");
    register s32 var_a2 asm("a2");
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_s1;
    s32* var_a0;
    s32* var_a0_2;
    s32* var_a0_3;
    register s32* var_a1 asm("a1");
    s32* var_a1_2;
    register s32* var_a1_3 asm("a1");
    s32* var_s1_2;
    register s32* var_v0 asm("v0");

    temp_v0 = LoaderSysFOpen(arg0, 1, 0);
    wanted=-1;
    if(temp_v0>=0)
    {
        var_a0 = loaderCacheOpen;asm("" : : "r"(var_a0));
        var_a2 = 0;asm("" : "+r"(var_a2) : : "a1");
        var_a1 = var_a0;
loop_2:
        temp_v0_2 = *var_a0;
        var_a0+=2;previous=var_a1;
        if(temp_v0_2!=wanted)
        {
            var_a2 += 1;
            var_a1=previous+2;
            if (var_a2 >= 8)
            {
                var_s1_2 = NULL;
            }
            else
            {
                goto loop_2;
            }
        }
        else
        {
            var_s1_2 = var_a1;
        }
        *var_s1_2 = temp_v0;
        LoaderSysGetstat(arg0,&stats);asm("" : "+r"(temp_v0));
        var_a2_2 = 0;
        statValue = stats.st_size;
        asm("" : : "r"(statValue));
        var_a0_2 = loaderCacheSize;
        asm("" : "+r"(statValue) : "r"(var_a0_2));
        var_s1_2[1] = statValue;
        var_a1_2 = loaderCacheSize;
        if (temp_v0 >= 0)
            goto loop_9;
    }
block_8:
    return -1;
loop_9:
    {
        temp_v0_3 = *var_a0_2;
        var_a0_2+=2;previous=var_a1_2;
        if (temp_v0_3 != temp_v0)
        {
            var_a2_2 += 1;
            var_a1_2=previous+2;
            if (var_a2_2 >= 8)
            {
                var_v0 = NULL;
            }
            else
            {
                goto loop_9;
            }
        }
        else
        {
            var_v0 = var_a1_2;
        }
        asm("" : "+r"(var_v0));
        var_s1 = 0;
        if (var_v0 != NULL)
        {
            var_s1 = var_v0[1];
        }
        LoaderSysFClose(temp_v0);
        var_a2_3 = 0;
        var_a0_3 = loaderCacheClose;
        var_a1_3 = loaderCacheClose;
loop_17:
        temp_v0_4 = *var_a0_3;
        var_a0_3+=2;previous=var_a1_3;
        if (temp_v0_4 != temp_v0)
        {
            var_a2_3 += 1;
            var_a1_3=previous+2;
            if (var_a2_3 >= 8)
            {
                asm volatile("break 0");
            }
            else
            {
                goto loop_17;
            }
        }
        else
        {
            *var_a1_3 = -1;
        }
        return var_s1;
    }
}


__asm__(".globl func_4001C99C\nfunc_4001C99C=iosLoaderGetFileSizeName+0x3C");


s32 iosFileRead(char* arg0, void* buffer, s32 bytes)
{
    struct sce_stat stats;
    register s32* previous __asm__("v1");
    register s32 wanted __asm__("a3");
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_4;
    register s32 var_a2 asm("a2");
    s32 var_a2_3;
    s32* var_a0;
    s32* var_a0_3;
    register s32* var_a1 asm("a1");
    register s32* var_a1_3 asm("a1");
    s32* var_s1_2;

    s32 size;

    size = iosLoaderGetFileSizeName(arg0);
    temp_v0 = LoaderSysFOpen(arg0, 1, 0);
    wanted=-1;
    if(temp_v0>=0)
    {
        var_a0 = loaderCacheOpen;asm("" : : "r"(var_a0));
        var_a2 = 0;asm("" : "+r"(var_a2) : : "a1");
        var_a1 = var_a0;
loop_2:
        temp_v0_2 = *var_a0;
        var_a0+=2;previous=var_a1;
        if(temp_v0_2!=wanted)
        {
            var_a2 += 1;
            var_a1=previous+2;
            if (var_a2 >= 8)
            {
                var_s1_2 = NULL;
            }
            else
            {
                goto loop_2;
            }
        }
        else
        {
            var_s1_2 = var_a1;
        }
        *var_s1_2 = temp_v0;
        LoaderSysGetstat(arg0,&stats);asm("" : "+r"(temp_v0));
        var_s1_2[1] = stats.st_size;
        if (temp_v0 >= 0)
            goto reading;
    }
    return -1;
reading:
    {
        if (bytes >= size)
            LoaderSysFRead(temp_v0,buffer,size);
        else
            LoaderSysFRead(temp_v0,buffer,bytes);
        LoaderSysFClose(temp_v0);
        var_a2_3 = 0;
        var_a0_3 = loaderCacheClose;
        var_a1_3 = loaderCacheClose;
        asm(".p2align 3");
loop_17:
        temp_v0_4 = *var_a0_3;
        var_a0_3+=2;previous=var_a1_3;
        if (temp_v0_4 != temp_v0)
        {
            var_a2_3 += 1;
            var_a1_3=previous+2;
            if (var_a2_3 >= 8)
            {
                asm volatile("break 0");
            }
            else
            {
                goto loop_17;
            }
        }
        else
        {
            *var_a1_3 = -1;
        }
        return size;
    }
}

__asm__(".globl func_4001CB40\nfunc_4001CB40=iosFileRead+0x88");


s32 iosLoaderFOpen(s8* arg0, s32 arg1, s32 arg2)
{
    struct sce_stat stats;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 var_a2;
    s32* var_a0;
    s32* var_a1;
    s32* var_s0;register s32 wanted __asm__("a3");register s32*previous __asm__("v1");

    temp_v0 = LoaderSysFOpen(arg0, arg1, arg2);
    if(temp_v0<0)return temp_v0;
    wanted=-1;
    if (temp_v0 >= 0)
    {
        var_a0 = D_40081A28;
        var_a2 = 0;
        var_a1 = D_40081A28;
loop_2:
        temp_v0_2 = *var_a0;
        var_a0 += 2;
        previous=var_a1;
        if (temp_v0_2 != wanted)
        {
            var_a2 += 1;
            var_a1=previous+2;
            if (var_a2 >= 8)
            {
                var_s0 = NULL;
            }
            else
            {
                goto loop_2;
            }
        }
        else
        {
            var_s0 = var_a1;
        }
        var_s0[0] = temp_v0;
        LoaderSysGetstat(arg0, &stats);
        var_s0[1] = stats.st_size;
    }
    return temp_v0;
}

s32 iosLoaderFClose(s32 arg0)
{
    s32 temp_a3;
    s32 temp_v0;
    s32 var_a2;
    s32* var_a0;
    register s32*previous __asm__("v1");s32* var_a1;

    temp_a3 = LoaderSysFClose(arg0);
    var_a0 = D_40081A28;
    var_a2 = 0;
    var_a1 = D_40081A28;
loop_1:
    temp_v0 = *var_a0;
    var_a0 += 2;
    previous=var_a1;
    if (temp_v0 != arg0)
    {
        var_a2 += 1;
        var_a1=previous+2;
        if (var_a2 >= 8)
        {
            asm volatile("break 0");
        }
        else
        {
            goto loop_1;
        }
    }
    else
    {
        *var_a1 = -1;
    }
    return temp_a3;
}

s32 iosLoaderFRead(s32 fd, void* buffer, s32 count)
{
    return LoaderSysFRead(fd, buffer, count);
}

s32 iosLoaderGetFileSize(s32 arg0)
{
    s32 temp_v0;
    register s32 var_a3 __asm__("a3");
    s32 var_v0;
    s32*var_a1;
    register s32* var_a2 __asm__("a2");
    register s32* var_v1 __asm__("v1");

    var_a3 = 0;
    var_a1 = D_40081A28;
    var_a2 = D_40081A28;
loop_1:
    temp_v0 = *var_a1;
    var_a1 += 2;
    var_v1 = var_a2;
    if (temp_v0 != arg0)
    {
        var_a3 += 1;
        var_a2 = var_v1 + 2;
        if (var_a3 >= 8)
        {
            var_v1 = NULL;
        }
        else
        {
            goto loop_1;
        }
    }
    var_v0 = 0;
    if (var_v1 != NULL)
    {
        var_v0 = var_v1[1];
    }
    return var_v0;
}

void initDldSys(void)
{
    s32 var_v1;
    s32*var_v0;register s32 value __asm__("a0")=-1;

    var_v0 = D_40081A28;
    var_v1=7;
    var_v0 += 14;
    do
    {
        var_v1 -= 1;
        *var_v0=value;
        var_v0 -= 2;
    } while (var_v1 >= 0);
}

static inline char* lookupModuleRelocationName(struct t_xffEntPntHdr* xff,u32 target)
{
    register s32 i asm("t6");s32 offset;
    register struct t_xffRelocEnt* scan asm("t1");
    struct t_xffRelocEnt* base;
    register s32 count asm("v0")=xff->relocTabNrE;
    register s32 savedCount asm("t8");
    register s32 type4 asm("s3");
    register s32 type9 asm("t9");
    asm("" : "+r"(count));
    asm volatile("" : "=r"(i) : "r"(count));
    i=0;
    if(count>0) {
        scan=xff->relocTab;asm("" : "+r"(count) : "r"(scan));
        savedCount=count;asm("" : "+r"(savedCount));
        type4=4;type9=9;
        asm("" : : "r"(type4),"r"(type9));
        offset=0;base=scan;
        do {
            if(scan->type==type4 || scan->type==type9) {
                u32 j=0;
                if(scan->nrEnt!=0) {
                    struct t_xffRelocEnt* table=(void*)((char*)base+offset);
                    struct t_xffSymEnt* symbols=xff->symTab;
                    u32 n=table->nrEnt;
                    struct t_xffRelocAddrEnt* reloc=table->addr;
                    do {
                        struct t_xffSymEnt*symbol=symbols+(reloc->tyIx>>8);
                        if(symbol->sect!=0)goto nextReloc;
                        if((u32)xff->sectTab[table->sect].memPt+reloc->addr!=target)goto nextReloc;
                        return xff->symTabStr+symbol->nameOffs;
nextReloc:
                        j++;reloc++;
                    }while(j<n);
                }
            }
            i++;offset+=sizeof(*scan);scan++;
        }while(i<savedCount);
    }
    return NULL;
}
extern char D_40047148[];
s32 func_4001CDD8(char* moduleName,char* functionName,isysObj* module,u32 target)
{
 struct t_xffEntPntHdr*xff=((isysModuleObj*)module)->xff;
 struct t_xffSectEnt*section;
 s32 i;
 s32 count;
 char*name;
 if(xff){
  count=xff->sectNrE;i=0;
  if(count>0){section=xff->sectTab;do{
   if(section->memPt!=NULL && target>=(u32)section->memPt && target<(u32)section->memPt+section->size){
    if(moduleName)strcpy(moduleName,((isysExecModuleObj*)module)->name);
    if(functionName){
     name=lookupModuleRelocationName(xff,target);
     if(name){strcpy(functionName,name);return 1;}
     memcpy(functionName,D_40047148,37);
     return 2;
    }
    return 1;
   }
   i++;section++;
  }while(i<count);}
 }
 return 0;
}


extern char D_40047170[], D_400471C0[], D_40047220[], D_40047270[], D_400472C8[], D_40047308[], D_40047388[];
void func_4001CFE0(u32 arg0)
{
    char moduleName[256];char functionName[256];

    char*buf;
    isysLink* var_v0;
    isysObj* var_s2;
    s32 temp_v0;
    s32 var_s3;

    buf=moduleName;
    var_s2 = NULL;
    var_s3 = 0;
    for(var_v0=isysGroupForFirst(&executableModuleGroup,0);var_v0!=NULL;var_v0=isysGroupForNext(var_v0,0)){
        var_s2=var_v0->obj;
        temp_v0=func_4001CDD8(buf,functionName,var_s2,arg0);
        if(temp_v0!=0){var_s3=temp_v0;break;}
    }
    if (var_s2 != NULL)
    {
        isysGroupForExit(var_s2, 0);
    }
    if (var_s3 != 0)
    {
        if (var_s3 == 2)
        {
            LoaderSysPrintf(D_40047170, arg0);
            LoaderSysPrintf(D_400471C0);
            LoaderSysPrintf(D_40047220);
            LoaderSysPrintf(D_40047270, arg0);
            LoaderSysPrintf(D_400472C8, arg0, arg0);
        }
        iosJumpRecoverPoint(D_40047308, arg0,moduleName,functionName);
        return;
    }
    iosJumpRecoverPoint(D_40047388, arg0);
}
__asm__(".globl func_4001D108\nfunc_4001D108=func_4001CFE0+0x128");




static inline char* lookupRelocationName(struct t_xffEntPntHdr* xff,u32 target)
{
    register s32 i asm("s0");s32 offset;
    register struct t_xffRelocEnt* scan asm("t1");
    struct t_xffRelocEnt* base;
    register s32 count asm("v0")=xff->relocTabNrE;
    register s32 savedCount asm("t9");
    register s32 type4 asm("s3");
    register s32 type9 asm("s2");
    asm("" : "+r"(count));
    asm volatile("" : "=r"(i) : "r"(count));
    i=0;
    if(count>0) {
        scan=xff->relocTab;asm("" : "+r"(count) : "r"(scan));
        savedCount=count;asm("" : "+r"(savedCount));
        type4=4;type9=9;
        asm("" : : "r"(type4),"r"(type9));
        offset=0;base=scan;
        do {
            if(scan->type==type4 || scan->type==type9) {
                u32 j=0;
                if(scan->nrEnt!=0) {
                    struct t_xffRelocEnt* table=(void*)((char*)base+offset);
                    struct t_xffSymEnt* symbols=xff->symTab;
                    u32 n=table->nrEnt;
                    struct t_xffRelocAddrEnt* reloc=table->addr;
                    do {
                        struct t_xffSymEnt*symbol=symbols+(reloc->tyIx>>8);
                        if(symbol->sect!=0)goto nextReloc;
                        if((u32)xff->sectTab[table->sect].memPt+reloc->addr!=target)goto nextReloc;
                        return xff->symTabStr+symbol->nameOffs;
nextReloc:
                        j++;reloc++;
                    }while(j<n);
                }
            }
            i++;offset+=sizeof(*scan);scan++;
        }while(i<savedCount);
    }
    return NULL;
}
s32 func_4001D120(s32 target)
{
 register isysLink*link asm("t6");isysObj*obj=NULL;register s32 result asm("s0")=0;
 link=isysGroupForFirst(&symbolDefModuleGroup,0);
 if(link)do{obj=link->obj;result=(s32)lookupRelocationName(((isysModuleObj*)obj)->xff,target);if(result)break;link=isysGroupForNext(link,0);}while(link);
 if(obj)isysGroupForExit(obj,0);
 return result;
}



void DldExceptionByOriginAdr(s32 arg0)
{
    s32 temp_v0;

    temp_v0 = func_4001D120(arg0);
    if (temp_v0 != 0)
    {
        iosJumpRecoverPoint(D_40047400, temp_v0, arg0);
        return;
    }
    iosJumpRecoverPoint(D_400474A8, arg0);
}

s32 isysSearchCalledModuleFuncNameByAddr(s8* arg0, s8* arg1, u32 arg2)
{
    isysLink* var_v0;
    isysObj* var_s1;
    s32 temp_v0;
    s32 var_s5;

    var_s1 = NULL;
    var_s5 = 0;
    for (var_v0 = isysGroupForFirst(&executableModuleGroup, 0); var_v0 != NULL; var_v0 = isysGroupForNext(var_v0, 0))
    {
        var_s1 = var_v0->obj;
        temp_v0 = func_4001CDD8(arg0, arg1, var_s1, arg2);
        if(temp_v0 != 0){var_s5=temp_v0;break;}
    }
    if (var_s1 != NULL)
    {
        isysGroupForExit(var_s1, 0);
    }
    return var_s5;
}

void isysCreateExceptionThread(void)
{}

void func_4001D3A8(void)
{
    register u32 caller __asm__("ra");
    func_4001CFE0(caller - 8);
}
