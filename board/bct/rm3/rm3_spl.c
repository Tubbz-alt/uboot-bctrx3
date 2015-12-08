/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <spl.h>
#include "mmdc.h"

DECLARE_GLOBAL_DATA_PTR;


//DPR added to satisfy SPL build
int mxs_dma_init_channel(int chan)
{
	return 0;
}

void mxs_dma_init(void)
{

}

/* Run a pattern over the entire memory range, fill it up first
 * then checking the actual content in a second pass.
 * Location N should hold the value "PATTERN+N".
 * This is done three times, accessing memory by 32/16/8 bits.
 * Can either stop at the first occurrence of an error, or keep
 * going [which is useful to find *which* lines are actually shorted]
 */

#define MEMTESTPASS		0
#define MEMTESTFAIL		1

u32 MemoryTest1(u32 Start, u32 nBytes, u32 Pattern, u32 StopAtFirstError)
{

	u32 *pW32, A32;
    u8 *pW8, A8;
    u32 i;
	u32 starttime = 0;
	u32 currenttime = 0;

	//Test by words, write then readback

#define pattern(_i_)  (u32)((Pattern + (_i_)))

    printf("Testing in 32 bits at [x%x..x%x] with base pattern %x\r\n", Start,Start+nBytes,Pattern);
    
	
	pW32 = (u32 *) Start;
    for (i = 0; i < nBytes/4; i++) 
	{
        pW32[i] = pattern(i);
    }

    for (i = 0; i < nBytes/4; i++) 
	{
        A32 = pW32[i];
        if (A32 != pattern(i)) {
            printf("DWORD[%d] mismatch, got 0x%x (!= 0x%x)\r\n",i, A32, pattern(i));
            if (StopAtFirstError)
			{
                return MEMTESTFAIL;
			}
        }
    }
#undef pattern

    return MEMTESTPASS;
}


/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/*
	 * Zero out global data:
	 *  - this shoudl be done by crt0.S
	 *  - failure to zero it will cause i2c_setup to fail
	 */
	memset((void *)gd, 0, sizeof(struct global_data));

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	mmdc_do_write_level_calibration();
	mmdc_do_dqs_calibration();

	MemoryTest1(0x10000000, 0x00100000, 0x00000000, 0);

	printf("Memory Test Complete\r\n");

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);


	get_clocks();

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{

}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

u32 spl_boot_mode(void)
{
	return MMCSD_MODE_FAT;
}