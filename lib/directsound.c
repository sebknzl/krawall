/*
 * Krawall, XM/S3M Modplayer Library
 * Copyright (C) 2001-2005, 2013 Sebastian Kienzl
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License in COPYING for more details.
 */

#include "regs.h"
#include "directsound.h"

#if IWRAM_USAGE_SMALL || IWRAM_USAGE_MEDIUM
static s8 lBuffer[ DMABUFFERSIZE ] EWRAM __attribute__((__aligned__(4)));
static s8 rBuffer[ DMABUFFERSIZE ] EWRAM __attribute__((__aligned__(4)));
#else
static s8 lBuffer[ DMABUFFERSIZE ] IWRAM __attribute__((__aligned__(4)));
static s8 rBuffer[ DMABUFFERSIZE ] IWRAM __attribute__((__aligned__(4)));
#endif

static u32 nextSample EWRAM;
static u32 currMixBlock EWRAM;

static u32 active EWRAM;

volatile u8 dmaBlock EWRAM;
u8 dsStereo IWRAM;


u32 LONG_CALL getDmaAddress( s8 **left, s8 **right )
{
	if( !active )
		return 0;

	// only write to this half when the DMA
	// reads in the other half
	if( dmaBlock == ( currMixBlock >> 1 ) )
		return 0;

	*left = &lBuffer[ nextSample ];
	*right = &rBuffer[ nextSample ];

	u32 fit = SAMPLESPERFRAME;
	nextSample += fit;

	if( ++currMixBlock >= BUFFERBLOCKS ) {
		currMixBlock = 0;
		nextSample = 0;
	}

	return fit;
}

void dsInit( u32 stereo )
{
	dsStereo = stereo;

    // soundchip on
	SGCNT1 = ( 1 << 7 );

	// output ratio full range A/B | output to L on A | FIFO A reset | output to R on B | FIFO B reset
	SGCNT0_H = ( ( 3 << 2 ) | ( 1 << 9 ) | ( 1 << 11 ) | ( 1 << 12 ) | ( 1 << 15 ) );

	if( !dsStereo ) { // mono
		SGCNT0_H |= ( 1 << 8 ); // output A to R as well
		SGCNT0_H &= 0x0fff;
	}

	// dma's source-address
	DM1SAD = ( u32 )&lBuffer;
	DM2SAD = ( u32 )&rBuffer;

	// dma's dest -> ds a/b fifo
	DM1DAD = ( u32 )&SGFIFOA;
	DM2DAD = ( u32 )&SGFIFOB;

	// dest fixed | dma repeat on | transfer-type 32b | dma-transfer timing 3 (direct sound)
	DM1CNT_H = ( 2 << 5 ) | ( 1 << 9 ) | ( 1 << 10 ) | ( 3 << 12 ) | ( 1 << 15 );

	if( dsStereo ) {
		DM2CNT_H = DM1CNT_H;
	}

	/*
	 this is a little tricky:
	 we must reset the dma after DMABUFFERSIZE samples have been played
	 (=# TM0 has overflowed)
	 now gba's fifo is a little weird, it fills it's fifo-buffer (32 bytes) quite fast,
	 actually it has already transfered 32 bytes (=2 transfers a 16 byte) when only
	 2 samples have been played.
	 when the 3rd dma-transfer is done (=48 bytes) only 16 samples have been played
	 so the dma is 32 bytes ahead.
	 therefore we must reset the DMA 32 bytes earlier, this is where the +32 comes from.
	 the -3 is a few samples we wait to reset the dma so we don't reset it while it
	 is still transferring data.
	 kradInterrupt will reprogram the timer to
	 TM1D = 65536 - DMABUFFERSIZE/2
	 on first occurance.
	 (DMABUFFERSIZE/2 because we want to get a notification when half the buffer has
	  been played).
	*/
	TM1D = 65536 - (DMABUFFERSIZE/2) + 32 - 3;

	// enable | generate interrupt | count-up timing
	TM1CNT = ( 1 << 7 ) | ( 1 << 6 ) | ( 1 << 2 );

	// SAMPLEFREQ hz (2^24/f)
	TM0D = 65536 - (1<<24>>SAMPLEFREQEXP);

	currMixBlock = 2;
	nextSample = 2*SAMPLESPERFRAME;

	dmaBlock = 0;
}

void kradActivate()
{
	// allow timer1-interrupts
	INT_IE |= ( 1 << 4 );

	// start timer 0
	TM0CNT = ( 1 << 7 );

	active = 1;
}

void kradDeactivate()
{
	// disable timer 0
	TM0CNT &= ~( 1 << 7 );
	active = 0;
}

void dsDeInit()
{
	// disable the timer first
	kradDeactivate();

	// end DMA-repeat/Directsound-mode (see Programming Manual 12.4 (2-2))
	// "write in 32-bit blocks"

	/*
	DMA enabled flag : 1 (Enabled)
	DMA start timing : 00 (Start immediately)
	DMA transfer type : 1 (32 bit transfer mode)
	DMA repeat : 0 (OFF)
	Destination address control flag : 10 (Fixed)
	Other control bits : No change
    */

    //u32 set = 4 | ( ( ( 1 << 15 ) | ( 1 << 10 ) | ( 2 << 5 ) ) << 16 );
    u32 set = ( 4 << 16 ) | ( ( 1 << 15 ) | ( 1 << 10 ) | ( 2 << 5 ) );

	DM1CNT = set;
	if( dsStereo ) {
		DM2CNT = set;
	}

	kradInterruptUndoCodeMod();
}

