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

	.GLOBAL	mixLeft, mixLeftHQ
	.GLOBAL mixRight, mixRightHQ
	.GLOBAL mixCenter, mixCenterHQ
	.GLOBAL mixSurround, mixSurroundHQ
	.GLOBAL mixStereo, mixStereoHQ
	.GLOBAL	mix16to8, mixClear
	.GLOBAL mixBias
	.GLOBAL mix16to8_patch

	.GLOBAL volSumLeft, volSumRight
	.GLOBAL volClipTable

	.SECTION .iwram,"axw",%progbits
	.ARM


// "struct"-definitions, see mixer_private.h
#define	CH_VOL 0
#define	CH_PAN (CH_VOL + 1)
#define	CH_ACTIVE (CH_PAN + 1)
#define	CH_LOOP (CH_ACTIVE + 1)
#define	CH_START (CH_LOOP + 1)
#define	CH_POS (CH_START + 4)
#define	CH_END (CH_POS + 4)
#define	CH_LOOPJUMPBACK (CH_END + 4)
#define	CH_INC (CH_LOOPJUMPBACK + 4)
#define	CH_ID (CH_INC + 4)
#define	CH_FRAC (CH_ID + 4)
#define	CH_LVOL (CH_FRAC + 2)
#define	CH_RVOL (CH_LVOL + 1)
#define	CH_HQ (CH_RVOL + 1)

#define DEAMP 5					// don't change, this is the size to fit the cliptable (defined below)
#define SRS 3					// amount every sample gets shifted down
#define SRSBITSHW (((1<<SRS)-1)<<16)		// bitmask to clear lsbits of upper word for shifting


@=================================================================
@ general register assignments for mix*
@=================================================================

#define CHN r0
#define DST r1
#define AMOUNT r2
#define LVOL r3
#define VOL r3
#define RVOL r4
#define SRC r5
#define INC r6
#define POS r7
#define IMM #




@=================================================================
@ mixLeft
@=================================================================
#define W1 r8
#define W2 r9
#define W3 r10
#define W4 r11
	.align
mixLeft:
	stmdb	sp!, {r4-r11}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	VOL, [CHN, #CH_LVOL]

	mov	AMOUNT, AMOUNT, lsr #2

0:
	ldrb	W1, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	add	W2, W1, W2, lsl #16		@ 1
	mul	W1, W2, VOL			@ 2

	bic	W3, W1, #SRSBITSHW		@ 1


	ldrb	W1, [SRC, POS, asr #16]
	add	POS, POS, INC

	ldrb	W2, [SRC, POS, asr #16]
	add	POS, POS, INC

	add	W2, W1, W2, lsl #16
	mul	W1, W2, VOL

	bic	W4, W1, #SRSBITSHW		@ = 18


	ldmia	DST, {W1, W2}			@ 4
	add	W1, W1, W3, lsr #SRS		@ 1
	add	W2, W2, W4, lsr #SRS		@ 1
	stmia	DST!, {W1, W2}			@ 3
	add	DST, DST, #8			@ 1


	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b				@ 3
						@ = 50/4 = 12.5

	add	SRC, SRC, POS, asr #16
        str	SRC, [CHN, #CH_POS]
        strh	POS, [CHN, #CH_FRAC]

	ldmia	sp!, {r4-r11}
	bx	lr


@=================================================================
@ mixRight
@=================================================================
	.align
mixRight:
	stmdb	sp!, {r4-r11}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	VOL, [CHN, #CH_RVOL]

	mov	AMOUNT, AMOUNT, lsr #2
	add	DST, DST, #8

	b	0b				@ jump to mixLeft


#undef W1
#undef W2
#undef W3
#undef W4



@=================================================================
@ mixCenter
@=================================================================
#define W1 r8
#define W2 r9
#define W3 r10
#define W4 r11
#define W5 r12
#define W6 r4
	.align
mixCenter:
	stmdb	sp!, {r4-r11}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	VOL, [CHN, #CH_LVOL]

	mov	AMOUNT, AMOUNT, lsr #2
0:
	ldrb	W1, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	add	W2, W1, W2, lsl #16		@ 1
	mul	W1, W2, VOL			@ 2

	bic	W5, W1, #SRSBITSHW		@ 1


	ldrb	W1, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	add	W2, W1, W2, lsl #16		@ 1
	mul	W1, W2, VOL			@ 2

	bic	W6, W1, #SRSBITSHW		@ 1


	ldmia	DST, {W1, W2, W3, W4}		@ 6
	add	W1, W1, W5, lsr #SRS		@ 1
	add	W3, W3, W5, lsr #SRS		@ 1
	add	W2, W2, W6, lsr #SRS
	add	W4, W4, W6, lsr #SRS		@ = 20
	stmia	DST!, {W1, W2, W3, W4}		@ 5

	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b				@ 3
						@ = 55/4 = 13.75

	add	SRC, SRC, POS, asr #16
        str	SRC, [CHN, #CH_POS]
        strh	POS, [CHN, #CH_FRAC]

	ldmia	sp!, {r4-r11}
	bx	lr


#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6


@=================================================================
@ mixStereo
@=================================================================
#define W1 r8
#define W2 r9
#define W3 r10
#define W4 r11
#define W5 r12
#define W6 lr
	.align
mixStereo:
	stmdb	sp!, {r4-r11, lr}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	LVOL, [CHN, #CH_LVOL]
	ldrb	RVOL, [CHN, #CH_RVOL]		@ load values

	mov	AMOUNT, AMOUNT, lsr #2

0:
	ldmia	DST, {W1, W2, W3, W4}		@ 6

	ldrb	W5, [SRC, POS, asr #16]		@ 6
	add	POS, POS, INC			@ 1

	ldrb	W6, [SRC, POS, asr #16]
	add	POS, POS, INC

	add	W5, W5, W6, lsl #16		@ 1
	mul	W6, W5, LVOL			@ 2
	bic	W6, W6, #SRSBITSHW		@ 1
	add	W1, W1, W6, lsr #SRS		@ 1

	mul	W6, W5, RVOL			@ 2
	bic	W6, W6, #SRSBITSHW		@ 1
	add	W3, W3, W6, lsr #SRS		@ 1

	ldrb	W5, [SRC, POS, asr #16]
	add	POS, POS, INC

	ldrb	W6, [SRC, POS, asr #16]
	add	POS, POS, INC			@ = 14

	add	W5, W5, W6, lsl #16
	mul	W6, W5, LVOL
	bic	W6, W6, #SRSBITSHW
	add	W2, W2, W6, lsr #SRS

	mul	W6, W5, RVOL
	bic	W6, W6, #SRSBITSHW
	add	W4, W4, W6, lsr #SRS		@ = 9

	stmia	DST!, {W1, W2, W3, W4}		@ 5

	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b				@ 3
						@ = 61/4 = 15.25

	add	SRC, SRC, POS, asr #16
        str	SRC, [CHN, #CH_POS]
        strh	POS, [CHN, #CH_FRAC]

	ldmia	sp!, {r4-r11, lr}
	bx	lr


#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6


@=================================================================
@ mixLeftHQ
@=================================================================
#define W1 r8
#define W2 r9
#define W3 r10
#define W4 r11
#define W5 r12
#define W6 r4
#define SRCI lr
	.align
mixLeftHQ:
	stmdb	sp!, {r4-r11, lr}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	VOL, [CHN, #CH_LVOL]

	mov	AMOUNT, AMOUNT, lsr #2

	add	SRCI, SRC, #1

0:
	ldrb	W1, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]	@ 6
	sub	W3, W3, W1			@ 1

	mov	W4, POS, lsl #16		@ 1
	mov	W4, W4, lsr #16			@ 1
	mul	W3, W4, W3			@ 2

	add	W1, W1, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1


	ldrb	W2, [SRC, POS, asr #16]
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W2

	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3

	add	W2, W2, W3, lsr #16
	add	POS, POS, INC			@ = 19


	add	W2, W1, W2, lsl #16		@ 1
	mul	W1, W2, VOL			@ 2

	bic	W5, W1, #SRSBITSHW		@ 1


	ldrb	W1, [SRC, POS, asr #16]
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W1

	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3

	add	W1, W1, W3, lsr #16
	add	POS, POS, INC			@ = 19


	ldrb	W2, [SRC, POS, asr #16]
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W2

	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3

	add	W2, W2, W3, lsr #16
	add	POS, POS, INC			@ = 19


	add	W2, W1, W2, lsl #16
	mul	W1, W2, VOL

	bic	W6, W1, #SRSBITSHW		@ = 4


	ldmia	DST, {W1, W2}			@ 4
	add	W1, W1, W5, lsr #SRS		@ 1
	add	W2, W2, W6, lsr #SRS		@ 1
	stmia	DST!, {W1, W2}			@ 3
	add	DST, DST, #8			@ 1

	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b				@ 3
						@ = 98/4 = 24.5


	add	SRC, SRC, POS, asr #16
        str	SRC, [CHN, #CH_POS]
        strh	POS, [CHN, #CH_FRAC]

	ldmia	sp!, {r4-r11, lr}
	bx	lr


@=================================================================
@ mixRightHQ
@=================================================================
	.align
mixRightHQ:
	stmdb	sp!, {r4-r11, lr}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	VOL, [CHN, #CH_RVOL]

	mov	AMOUNT, AMOUNT, lsr #2
	add	DST, DST, #8

	add	SRCI, SRC, #1

	b	0b				@ jump to mixLeftHQ



#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6
#undef SRCI

@=================================================================
@ mixCenterHQ
@=================================================================
#define W1 r8
#define W2 r9
#define W3 r10
#define W4 r11
#define W5 r12
#define W6 r4
#define SRCI lr
	.align
mixCenterHQ:
	stmdb	sp!, {r4-r11, lr}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	VOL, [CHN, #CH_LVOL]

	mov	AMOUNT, AMOUNT, lsr #2

	add	SRCI, SRC, #1
0:

	ldrb	W1, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W1			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W1, W1, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]	@ 6
	sub	W3, W3, W2			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W2, W2, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	add	W1, W1, W2, lsl #16		@ 1
	mul	W2, W1, VOL			@ 2
	bic	W5, W2, #SRSBITSHW		@ 1

	ldrb	W1, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W1			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W1, W1, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]	@ 6
	sub	W3, W3, W2			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W2, W2, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	add	W1, W1, W2, lsl #16		@ 1
	mul	W2, W1, VOL			@ 2
	bic	W6, W2, #SRSBITSHW		@ 1


	ldmia	DST, {W1, W2, W3, W4}		@ 6
	add	W1, W1, W5, lsr #SRS		@ 1
	add	W2, W2, W6, lsr #SRS		@ 1
	add	W3, W3, W5, lsr #SRS		@ 1
	add	W4, W4, W6, lsr #SRS		@ 1
	stmia	DST!, {W1, W2, W3, W4}		@ 5

	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b				@ 3

	add	SRC, SRC, POS, asr #16
        str	SRC, [CHN, #CH_POS]
        strh	POS, [CHN, #CH_FRAC]

	ldmia	sp!, {r4-r11, lr}
	bx	lr

#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6
#undef SRCI


@=================================================================
@ mixStereoHQ
@=================================================================
#define W1 r8
#define W2 r9
#define W3 r10
#define W4 r11
#define W5 r12
#define W6 r0
#define SRCI lr
#define W7 lr
	.align
mixStereoHQ:
	stmdb	sp!, {r4-r11, lr}
	ldr	SRC, [CHN, #CH_POS]
	ldr	INC, [CHN, #CH_INC]
	ldrh	POS, [CHN, #CH_FRAC]
	ldrb	LVOL, [CHN, #CH_LVOL]
	ldrb	RVOL, [CHN, #CH_RVOL]

	mov	AMOUNT, AMOUNT, lsr #2

	stmdb	sp!, {CHN}
0:
	add	SRCI, SRC, #1

	ldrb	W1, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W1			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W1, W1, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]	@ 6
	sub	W3, W3, W2			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W2, W2, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	add	W5, W1, W2, lsl #16		@ 1


	ldrb	W1, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]
	sub	W3, W3, W1			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W1, W1, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	ldrb	W2, [SRC, POS, asr #16]		@ 6
	ldrb	W3, [SRCI, POS, asr #16]	@ 6
	sub	W3, W3, W2			@ 1
	mov	W4, POS, lsl #16
	mov	W4, W4, lsr #16
	mul	W3, W4, W3			@ 2
	add	W2, W2, W3, lsr #16		@ 1
	add	POS, POS, INC			@ 1

	add	W6, W1, W2, lsl #16		@ 1


	ldmia	DST, {W1, W2, W3, W4}		@ 6

	mul	W7, W5, LVOL
	bic	W7, W7, #SRSBITSHW
	add	W1, W1, W7, lsr #SRS

	mul	W7, W5, RVOL
	bic	W7, W7, #SRSBITSHW
	add	W3, W3, W7, lsr #SRS

	mul	W7, W6, LVOL
	bic	W7, W7, #SRSBITSHW
	add	W2, W2, W7, lsr #SRS

	mul	W7, W6, RVOL
	bic	W7, W7, #SRSBITSHW
	add	W4, W4, W7, lsr #SRS

	stmia	DST!, {W1, W2, W3, W4}		@ 5

	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b				@ 3

	ldmia	sp!, {CHN}

	add	SRC, SRC, POS, asr #16
        str	SRC, [CHN, #CH_POS]
        strh	POS, [CHN, #CH_FRAC]

	ldmia	sp!, {r4-r11, lr}
	bx	lr




#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6
#undef W7
#undef SRCI



@=================================================================
@ mixBias( s16 *dest, uint amount, uint lvol, uint rvol );
@=================================================================
#define DEST r0
#undef AMOUNT
#define AMOUNT r1
#undef LVOL
#define LVOL r2
#undef RVOL
#define RVOL r3

	.align
mixBias:
	stmdb	sp!, {r4-r7}

	mov	LVOL, LVOL, lsl #7		@ *128
	mov	LVOL, LVOL, lsr #SRS
	add	LVOL, LVOL, LVOL, lsl #16	@ upper halfword

	mov	RVOL, RVOL, lsl #7
	mov	RVOL, RVOL, lsr #SRS
	add	RVOL, RVOL, RVOL, lsl #16

	mov	AMOUNT, AMOUNT, lsr #2

0:
	ldmia	DEST, {r4, r5, r6, r7} 	@ 6
	add	r4, r4, LVOL		@ 1
	add	r5, r5, LVOL		@ 1
	add	r6, r6, RVOL		@ 1
	add	r7, r7, RVOL		@ 1
	stmia	DEST!, {r4, r5, r6, r7}	@ 5

	subs	AMOUNT, AMOUNT, #1		@ 1
	bne	0b			@ 3
					@ = 19 / 4 = 4.75/sample

	ldmia	sp!, {r4-r7}
	bx	lr



@=================================================================
@ mixClear( s16 *dst, uint amount )
@=================================================================
#undef DST
#define DST r0
#undef AMOUNT
#define AMOUNT r1
        .align
mixClear:
	stmdb	sp!, { r4, r5 }
	mov	r2, #0
	mov     r3, #0
	mov	r4, #0
	mov	r5, #0

	str	r2, volSumLeft
	str	r2, volSumRight

	mov	AMOUNT, AMOUNT, lsr #2

       0:
	stmia	DST!, { r2-r5 }
	subs    AMOUNT, AMOUNT, #1
	bne     0b

	ldmia	sp!, { r4, r5 }
	bx      lr



@=================================================================
@ volSumLeft/Right
@=================================================================
volSumLeft:	.word	0
volSumRight:	.word	0


	.pool


@=================================================================
@ mix16to8( u16 *src, s8* left, s8 *right, uint amount );
@=================================================================
#undef SRC
#define SRC r0
#define LEFT r1
#define RIGHT r2
#undef AMOUNT
#define AMOUNT r3

#define W1 r4
#define W2 r5
#define W3 r6
#define W4 r7
#define VLEFT r8
#define VRIGHT r9
#define W5 r10
#define W6 r11
#define CLIPTABLE r12
	.align
mix16to8:
	stmdb	sp!, {r4-r11}

	ldr	VLEFT, volSumLeft
	mov	VLEFT, VLEFT, lsl #7	@ *128
	ldr	VRIGHT, volSumRight
	mov	VRIGHT, VRIGHT, lsl #7

	mov	VLEFT, VLEFT, lsr #SRS
	mov	VRIGHT, VRIGHT, lsr #SRS

	mov	AMOUNT, AMOUNT, lsr #2

	ldr	CLIPTABLE, =volClipTable

       0:
	ldmia	SRC!, {W1-W4}		@ 6
	@ = 6

	mov	W5, W1, lsr #16		@ 1: W5 -> upper 16 bit
	sub	W6, W1, W5, lsl #16	@ 1: W6 -> lower 16 bit

	sub	W5, W5, VLEFT		@ 1: remove bias
	sub	W6, W6, VLEFT		@ 1

       4:
	ldrb	W5, [CLIPTABLE, W5, asr #DEAMP]		@ EWRAM: 5 (IWRAM: 3)
	ldrb	W6, [CLIPTABLE, W6, asr #DEAMP]
	add	W1, W6, W5, lsl #8
        @ = 15

	mov	W5, W2, lsr #16
	sub	W6, W2, W5, lsl #16

	sub	W5, W5, VLEFT
	sub	W6, W6, VLEFT

       5:
	ldrb	W5, [CLIPTABLE, W5, asr #DEAMP]
	ldrb	W6, [CLIPTABLE, W6, asr #DEAMP]
	add	W6, W6, W5, lsl #8
	@ = 15

	add	W1, W1, W6, lsl #16	@ 1
	str	W1, [LEFT], #4		@ 2
	@ = 3

	mov	W5, W3, lsr #16
	sub	W6, W3, W5, lsl #16

	sub	W5, W5, VRIGHT
	sub	W6, W6, VRIGHT
       6:
	ldrb	W5, [CLIPTABLE, W5, asr #DEAMP]
	ldrb	W6, [CLIPTABLE, W6, asr #DEAMP]
	add	W3, W6, W5, lsl #8
	@ = 15

	mov	W5, W4, lsr #16
	sub	W6, W4, W5, lsl #16

	sub	W5, W5, VRIGHT
	sub	W6, W6, VRIGHT
       7:
	ldrb	W5, [CLIPTABLE, W5, asr #DEAMP]
	ldrb	W6, [CLIPTABLE, W6, asr #DEAMP]
	add	W6, W6, W5, lsl #8
	@ = 15

	add	W3, W3, W6, lsl #16
	str	W3, [RIGHT], #4
	@ = 3

	subs    AMOUNT, AMOUNT, #1	@ 1
	bne     0b			@ 3

					@ = 76 / 4 = 19/sample

	ldmia	sp!, {r4-r11}

	bx      lr


	.align

@=================================================================
@ void mix16to8_patch( uint deamp ) LONG_CALL;
@=================================================================
@ this func patches the
@	ldrb	W5, [CLIPTABLE, W5, asr #DEAMP]
@ instructions from above, changing the DEAMP.
/*
amnt tp
11  7 65 4
xxxxx xx 0
*/

mix16to8_patch:
	adr	r1, 4b
	ldr	r1, [r1]
	adr	r2, 4b+4
	ldr	r2, [r2]
	bic	r1, r1, #0b111110000000
	bic	r2, r2, #0b111110000000
	orr	r1, r1, r0, lsl #7
	orr	r2, r2, r0, lsl #7

	adr	r0, 4b
	str	r1, [r0]
	str	r2, [r0, #4]
	adr	r0, 5b
	str	r1, [r0]
	str	r2, [r0, #4]
	adr	r0, 6b
	str	r1, [r0]
	str	r2, [r0, #4]
	adr	r0, 7b
	str	r1, [r0]
	str	r2, [r0, #4]

	bx	lr

	.pool

@=================================================================
@ volclipTable
@=================================================================
#if IWRAM_USAGE_SMALL || IWRAM_USAGE_MEDIUM
	.section .ewram, "aw"
#endif
		.skip	1024, 0
volClipTable:	.skip   1024, 0

	.END

