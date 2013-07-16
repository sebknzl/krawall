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

#ifndef __DIRECTSOUNDDEF_H__
#define __DIRECTSOUNDDEF_H__

// 13:  8192hz
// 14: 16384hz (*)
// 15: 32768hz
#ifndef SAMPLEFREQEXP
# define SAMPLEFREQEXP 14
#endif
//#undef MODE30	// 30hz mode?

#define SAMPLEFREQ (1<<SAMPLEFREQEXP)
#if SAMPLEFREQEXP < 13 || SAMPLEFREQEXP > 15
#error "invalid samplefreq given!"
#endif

/*
dmabuffer-size must be divisible by 16, cause 4 words
get transfered each!

samplerate: 8192/16384/32768hz
v-rate    : 59.727hz

8192hz : 137.16	=>	140	(4 blocks)	=>	2*4*140 = 1120
2.84 samples too much/frame => skip ~ every 50 frames

16384hz: 274.32 =>	276	(4 blocks)	=>	2*4*276 = 2208
1.68 samples too much/frame => skip ~ every 164 frames

32768hz: 548.63 =>	552	(4 blocks)	=>	2*4*552 = 4416
3.37 samples too much/frame => skip ~ every 163 frames

-- 30 hz:

08KHz : -> 16KHz/60Hz
16KHz : -> 32KHz/60Hz
32Khz : 1097.26		1104		-> skip 163

*/

// DO NOT CHANGE
#define BUFFERBLOCKS 4

#if ((SAMPLEFREQ == 32768) && defined(MODE30))
	#define SAMPLESPERFRAME 1104
#elif (SAMPLEFREQ == 32768) || ((SAMPLEFREQ == 16384) && defined(MODE30))
	#define SAMPLESPERFRAME 552
#elif (SAMPLEFREQ == 16384) || ((SAMPLEFREQ == 8192) && defined(MODE30))
	#define SAMPLESPERFRAME 276
#elif (SAMPLEFREQ == 8192) && !defined(MODE30)
	#define SAMPLESPERFRAME 140
#else
	#error "SAMPLEFREQ/MODE30 invalid"
#endif

#define DMABUFFERSIZE (BUFFERBLOCKS*SAMPLESPERFRAME)

#endif

