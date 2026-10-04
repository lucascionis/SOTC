#include "common.h"
#include "sdk/ee/eeregs.h"

extern void iosWaitDMA(void);

void iosSrpWaitDMA(void)
{
    asm("sync");
    iosWaitDMA();
}

void iosMemToSprDmaAdrCopy(u32 address, u32 count, u32 scratchAddress)
{
    *D_PCR = -0x400;
    *D_STAT = 0x200;
    *D9_MADR = address;
    *D9_QWC = count;
    *D9_SADR = scratchAddress;
    *D_PCR |= 0x200;
    *D9_CHCR = 0x101;
}

void iosSprToMemDmaAdrCopy(u32 address, u32 count, u32 scratchAddress)
{
    *D_PCR = -0x400;
    *D_STAT = 0x100;
    *D8_MADR = address;
    *D8_QWC = count;
    *D8_SADR = scratchAddress;
    *D_PCR |= 0x100;
    *D8_CHCR = 0x100;
}

asm(".align 3");
