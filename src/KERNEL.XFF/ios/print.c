#include "common.h"
#include "gcc/stdarg.h"
#include "gcc/stdio.h"
#include "usbSerialSys.h"

s32 iosSPrintf(char* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    return vsprintf(buffer, format, args);
}

void iosUsbSerialPrintf(const char* format, ...)
{
    char buffer[0x100];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    usbSerialSysPutString(buffer);
}

void iosCreatePrintMgr(void) {
}

extern s32 D_40045810;

s32* iosGetPrintfMode(void)
{
    return &D_40045810;
}

asm(".align 3");
