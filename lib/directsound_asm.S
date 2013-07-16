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

	.SECTION .iwram, "axw"
	.ARM

	.EXTERN dmaBlock
	// volatile u32 dmaBlock0 EWRAM;

	#define DM1CNT_H 	0x40000c6
	#define DM2CNT_H 	0x40000d2
	#define	TM1		0x4000104
	#include "directsound-def.h"

	.GLOBAL kradInterrupt, kradInterruptUndoCodeMod
kradInterrupt:
	ldr	r0, =dmaBlock
	ldrb	r1, [r0]
	tst	r1, #1		// wraparound?
	eor	r1, r1, #1	// toggle
	strb	r1, [r0]

Lnewend:
	// this instructions gets rewritten to "bxeq lr".
	// bx only if we don't have to reset the dma
	b	Lreprogram

	// yes, reset dma!

	// dma1 control
	ldr	r0, =DM1CNT_H
        ldrh	r1, [r0]
        // stop (bit 15)
        bic	r1, r1, #( 1 << 15 )
        strh	r1, [r0]
        // start
        orr	r1, r1, #( 1 << 15 )
        strh	r1, [r0]

        // dma2 control
        ldr	r0, =DM2CNT_H
        ldrh	r1, [r0]
        // active?
        tst	r1, #( 1 << 15 )

        // nah -> mono
        bxeq	lr

	// reset dma2 as well
        bic	r1, r1, #( 1 << 15 )
        strh	r1, [r0]
        orr	r1, r1, #( 1 << 15 )
        strh	r1, [r0]

        bx	lr

Lreprogram:
	// reprogram timer
	ldr	r0, =TM1
	ldr	r1, [r0]
	bic	r2, r1, #( 1 << 23 )
	// stop timer
	str	r2, [r0]

	// load 'new' size
	ldr	r2, =(65536-(DMABUFFERSIZE/2))
	// clear lower hword
	mov	r1, r1, lsr #16
	// upper hw = control, lower hw = count
	add	r1, r2, r1, lsl #16
	// reprogram and start (bit 23 is still set)
	str	r1, [r0]

	// we only have to reprogram the timer once so...
	// backup the op from Lnewend (for redoing our change later (shutdown)) and ...
	ldr	r1, Lnewend
	adr	r0, Lopbackup
	str	r1, [r0]
	// ... copy "bxeq lr" to Lnewend
	ldr	r1, Lop
	adr	r0, Lnewend
	str	r1, [r0]

	bx	lr


kradInterruptUndoCodeMod:
	ldr	r1, Lopbackup
	adr	r0, Lnewend
	str	r1, [r0]
	bx	lr

Lop:
	bxeq	lr

Lopbackup:	.word	0

	.POOL

	.END

