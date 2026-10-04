#include "common.h"
#include "sdk/ee/libmc2.h"

typedef struct {
    s32 socket;
    s32 reserved[9];
} IosMcSocketRef;
extern IosMcSocketRef D_4007CE84[];
extern char D_40046B08[];
extern s32 LoaderSysPrintf(const char *, ...);

typedef struct {
    s32 code;
    s32 result;
    s32 reserved;
} IosMcErrorMapping;
extern IosMcErrorMapping D_40045608[];

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/mc", iosMcInit);

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/mc", iosMcGetInfo);

s32 iosMcChDir(s32 port, s32 slot, const char *path)
{
    IosMcErrorMapping *mapping;
    s32 code;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    code = sceMc2Chdir(D_4007CE84[port].socket, path, NULL) & 0xFFFF;
    result = 8;
    mapping = D_40045608;
    index = 0;
    do {
        if (code == mapping->code) {
            result = mapping->result;
            break;
        }
        index++;
        mapping++;
    } while (index < 22U);
    return result;
}

s32 iosMcMkDir(s32 port, s32 slot, const char *path)
{
    IosMcErrorMapping *mapping;
    s32 code;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    code = sceMc2Mkdir(D_4007CE84[port].socket, path) & 0xFFFF;
    result = 8;
    mapping = D_40045608;
    index = 0;
    do {
        if (code == mapping->code) {
            result = mapping->result;
            break;
        }
        index++;
        mapping++;
    } while (index < 22U);
    return result;
}

s32 iosMcCreateFile(s32 port, s32 slot, const char *path)
{
    IosMcErrorMapping *mapping;
    s32 code;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    code = sceMc2CreateFile(D_4007CE84[port].socket, path) & 0xFFFF;
    result = 8;
    mapping = D_40045608;
    index = 0;
    do {
        if (code == mapping->code) {
            result = mapping->result;
            break;
        }
        index++;
        mapping++;
    } while (index < 22U);
    return result;
}

s32 iosMcDeleteFile(s32 port, s32 slot, const char *path)
{
    IosMcErrorMapping *mapping;
    s32 code;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    code = sceMc2Delete(D_4007CE84[port].socket, path) & 0xFFFF;
    result = 8;
    mapping = D_40045608;
    index = 0;
    do {
        if (code == mapping->code) {
            result = mapping->result;
            break;
        }
        index++;
        mapping++;
    } while (index < 22U);
    return result;
}

s32 iosMcReadFile(s32 port, s32 slot, const char *path, void *data, s32 offset, s32 size)
{
    s32 code;
    s32 readResult;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    readResult = sceMc2ReadFile(D_4007CE84[port].socket, path, data, offset, size);
    if (readResult >= 0)
        return 0;
    code = readResult & 0xFFFF;
    result = 8;
    index = 0;
    do {
        if (code == D_40045608[index].code) {
            result = D_40045608[index].result;
            break;
        }
        index++;
    } while (index < 22U);
    return result;
}

s32 iosMcWriteFile(s32 port, s32 slot, const char *path, void *data, s32 offset, s32 size)
{
    s32 code;
    s32 readResult;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    readResult = sceMc2WriteFile(D_4007CE84[port].socket, path, data, offset, size);
    if (readResult >= 0)
        return 0;
    code = readResult & 0xFFFF;
    result = 8;
    index = 0;
    do {
        if (code == D_40045608[index].code) {
            result = D_40045608[index].result;
            break;
        }
        index++;
    } while (index < 22U);
    return result;
}

s32 iosMcFormat(s32 port, s32 slot)
{
    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    return sceMc2Format(D_4007CE84[port].socket);
}

asm(".p2align 3");

s32 iosMcUnformat(s32 port, s32 slot)
{
    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    return sceMc2Unformat(D_4007CE84[port].socket);
}

s32 iosMcGetDir(s32 port, s32 slot, const char *path, s32 mode, s32 maxEntries, SceMc2DirParam *entries, s32 *count)
{
    IosMcErrorMapping *mapping;
    s32 code;
    s32 result;
    u32 index;

    if (slot != 0)
        LoaderSysPrintf(D_40046B08);
    code = sceMc2GetDir(D_4007CE84[port].socket, path, mode, maxEntries, entries, count) & 0xFFFF;
    result = 8;
    mapping = D_40045608;
    index = 0;
    do {
        if (code == mapping->code) {
            result = mapping->result;
            break;
        }
        index++;
        mapping++;
    } while (index < 22U);
    return result;
}

INCLUDE_ASM("asm/KERNEL.XFF/nonmatchings/ios/mc", func_400167B0);
