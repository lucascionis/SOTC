#ifndef ISYS_MODULE_H
#define ISYS_MODULE_H

#include "ios/isys/obj.h"
#include "fl_xfftype.h"
#include "ios/hash.h"

typedef struct isysModuleRelocation {
    struct t_xffRelocEnt* table;
    s32 index;
} isysModuleRelocation;

typedef struct isysModuleRelocations {
    s32 count;
    isysModuleRelocation* entries;
} isysModuleRelocations;

typedef struct isysModuleImport {
    u32 hash;
    struct t_xffSymEnt* symbol;
    u32 previous;
    struct isysModuleImport* next;
} isysModuleImport;

typedef struct isysModuleObj {
    isysObj obj;
    struct t_xffEntPntHdr* xff;
    void* entryPoint;
    t_hashedSym* definedSymbols;
    isysModuleImport* importedSymbols;
    s32 flags;
    isysModuleRelocations relocations;
} isysModuleObj;

typedef struct isysExecModuleObj {
    isysModuleObj module;
    char* name;
    s32 loadedBytes;
} isysExecModuleObj;

s32 isysFlushModuleObj(isysModuleObj* module);
void func_4001C4F8(isysModuleObj* module);
void func_4001C5E8(isysExecModuleObj* module);
void isysRelocateModuleObj(isysModuleObj* module);
isysModuleObj* isysInitModuleObjExisted(isysModuleObj* module, struct t_xffEntPntHdr* xff, s32 defineSymbols, s32 resolveImports, s32 buildSymbols, s32 partition);
isysExecModuleObj* func_4001AD58(char* name, s32 size, s32 partition);
s32 func_4001AB68(isysExecModuleObj* module, char* name, struct t_xffEntPntHdr* xff);
isysModuleObj* func_4001A918(char* name, s32 size, s32 partition, s32 defineSymbols, s32 resolveImports, s32 buildSymbols);
isysModuleObj* isysCreateModuleObjExisted(struct t_xffEntPntHdr* xff, s32 workSize, s32 partition, s32 defineSymbols, s32 resolveImports, s32 buildSymbols, s32 relocatePartition);
isysExecModuleObj* func_4001AE88(char* name, struct t_xffEntPntHdr* xff, s32 partition);
void func_4001C8F0(void* partition, void* start, s32 size);
s32 isysDisposeModuleObjRelocationElement(isysModuleObj* module);
s32 isysSearchModuleFuncNameByAddr(char* moduleName, char* functionName, u32 address);
isysExecModuleObj* readEModule(char* name, s32 partition);
void* isysGetXffEntryPointPreRelocation(struct t_xffEntPntHdr* xff);
isysObj* isysSearchModuleSymbolByGroup(void** address, char* name, isysGroup* group, isysObj* exclude);
void isysMoveModuleObj(isysModuleObj* module, struct t_xffEntPntHdr* xff);
void isysLaunchModule(isysExecModuleObj* module);
s32 IosCdvdManagerSimulation(void);
void InitDld(void);
void isysWriteLinkerScriptFile(void* module);
void initDldSys(void);
s32 iosLoaderGetFileSize(s32 fd);
s32 isysResolveAllModuleObjOfGroup(isysGroup* group);
s32 isysSearchSymbol(void** address, u32 hash);
void IosLoadIrxSimulation(void);
void isysDumpExecutableModules(void);
void isysLaunchExecModule(void* module);
void DldExceptionByOriginAdr(s32 address);
s32 isysResolveAllModuleObjHasUndefSymbol(void);
s32 isysResolveAllProgramModule(void);
s32 isysSearchCalledModuleFuncNameByAddr(char* moduleName, char* functionName, u32 address);
void isysRemoveModuleObjGroup(isysObj* obj);
s32 isysResolveOneModuleObj(isysModuleObj* module);
s32 isysGetUndefModuleObjNum(void);
s32 isysGetSymdefModuleObjNum(void);
void func_4001C350(void* buffer, char* filename, s32 size);
struct t_xffEntPntHdr* func_4001C468(struct t_xffEntPntHdr* xff);
isysObj* loadtest(char* name, s32 partition);
s32 iosLoaderFOpen(char* name, s32 mode, s32 flags);
s32 iosLoaderFClose(s32 fd);
s32 iosLoaderGetFileSizeName(char* name);
s32 iosFileRead(char* name, void* buffer, s32 bytes);
char* IosGetMergeDataFileName(void);
s32 GetExecModuleSize(void);
u32 getModuleSize(isysModuleObj* module);
char* GetSymbolByIndex(isysModuleObj* module, s32 index);
s32 iosLoaderFRead(s32 fd, void* buffer, s32 count);
void* func_4001C220(s32 size, s32 alignment);
void* func_4001C248(s32 size);
void* func_4001C3B0(s32 size, s32 alignment);
void* func_4001C3F8(s32 size);
s32 isysGetXffWorkSize(struct t_xffEntPntHdr* xff);
s32 isysGetModuleObjMallocSize(isysModuleObj* module);

#endif
