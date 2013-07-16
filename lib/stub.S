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

@ stubs for gcc < 3 (doesn't know long_call)
	.TEXT
	.THUMB

        .ALIGN 2

	.GLOBAL kramWorkerSTUB
        .THUMB_FUNC
kramWorkerSTUB:
	ldr 	r3, =kramWorker
	bx	r3

	.GLOBAL kramPlaySTUB
	.THUMB_FUNC
kramPlaySTUB:
	ldr	r3, =kramPlay
	bx	r3

	.GLOBAL kramPlayExtSTUB
	.THUMB_FUNC
kramPlayExtSTUB:
	bx	pc
	nop
	.ARM
	ldr	ip, =kramPlayExt
	bx	ip
	.THUMB

	.GLOBAL kramStopSTUB
	.THUMB_FUNC
kramStopSTUB:
	ldr	r3, =kramStop
	bx	r3

	.GLOBAL kramSetFreqSTUB
	.THUMB_FUNC
kramSetFreqSTUB:
	ldr	r3, =kramSetFreq
	bx	r3

	.GLOBAL kramSetVolSTUB
	.THUMB_FUNC
kramSetVolSTUB:
	ldr	r3, =kramSetVol
	bx	r3

	.GLOBAL kramSetPanSTUB
	.THUMB_FUNC
kramSetPanSTUB:
	ldr	r3, =kramSetPan
	bx	r3

	.GLOBAL kramSetPosSTUB
	.THUMB_FUNC
kramSetPosSTUB:
	ldr	r3, =kramSetPos
	bx	r3

	.END

