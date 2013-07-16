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

#ifndef __MIXER_PRIVATE_H__
#define __MIXER_PRIVATE_H__

#include "types.h"
#include "directsound.h"
#include "mixer.h"

#define MIX_FUNCTION(name) void name( struct MixChannel *c, s16 *dest, uint amount )
// we don't need LONG_CALL for those cause these functions are called via a
// pointer anyway

// DO NOT CHANGE ANY OF THIS W/O CHANGING THE ASM-CODE IN mixer_func.s !!!
struct MixChannel {
	u8	vol;
	s8	pan;
	u8	status;
	#define CHN_INACTIVE	0
	#define CHN_ACTIVE		1
	// status >=2 --> pause-slots

	u8  loop;

	const u8 *start;
	const u8 *pos;
	const u8 *end;
	u32 loopLength;

	s32	inc;
	chandle	id;
	u16 frac;

	u8 lvol;
	u8 rvol;
	// ======================
	u8 hq;
	u8 mixFunc;		// index into mixPanTable
	u8 hqs;		// current sample a HQ sample?

	union {
		u8 sfx;			// is sfx?
		u8 rampCount;	// ramping count
	};

	union {
		const u8 *lstart;
		struct {
			s16 rampPos;	// ramping position
			s16	rampInc;	// ramping increment
		};
	};
};

typedef struct MixChannel MixChannel;

#define CHANNELNUM 32
MIX_FUNCTION(mixLeft);
MIX_FUNCTION(mixLeftHQ);
MIX_FUNCTION(mixRight);
MIX_FUNCTION(mixRightHQ);
MIX_FUNCTION(mixCenter);
MIX_FUNCTION(mixCenterHQ);
MIX_FUNCTION(mixStereo);
MIX_FUNCTION(mixStereoHQ);

#define MIXMONO mixLeft


// min tempo for the mods is 32, it's ok to be lower
// (*24 is mod-tempo to bpm)
#define BPM_MIN (16*24)


// amount is amount of samples!
// -> amount has to be aligned to 4!
void mixBias( s16 *dest, uint amount, uint lvol, uint rvol ) LONG_CALL;
void mixClear( s16 *dst, uint amount ) LONG_CALL;
void mix16to8( s16 *src, s8* left, s8 *right, uint amount ) LONG_CALL;
void mix16to8_patch( uint deamp ) LONG_CALL;

extern MixChannel channels[ CHANNELNUM ];

extern int volSumLeft;
extern int volSumRight;
extern s8 volClipTable[];

extern void (* volatile soundTimerFunc)();
extern u16 soundTimerSpeed;
extern u16 soundTimerCount;

extern u8 volMaster[];
extern u8 hqMode;
extern u8 hqRamp;

extern MIX_FUNCTION((* const mixPanTable[]));

// some macros used by both mixer.c and mixer.arm.c:

// lvol, rvol: 0..64
#define SETLRVOLUME \
	u32 __temp = chn->vol * volMaster[ chn->sfx ];		\
	chn->lvol = ( __temp * ( 64 - chn->pan ) ) >> 13;	\
	chn->rvol = ( __temp * ( 64 + chn->pan ) ) >> 13;

#define SETQUALITY \
	chn->hq = ( ( hqMode == KRAM_QM_HQ ) ? 1 : chn->hqs & hqMode ) << 3;

#define SETFREQUENCY(x) if( chn->inc >= 0 ) chn->inc = ( ( x >> ( SAMPLEFREQEXP - 14 ) ) << 2 ); else chn->inc = -( ( x >> ( SAMPLEFREQEXP - 14 ) ) << 2 );
#define SETVOLUME(x) 	chn->vol = x;
#define SETPANNING(x) \
	if( dsStereo ) { \
		/* 64>>6 = 1, 0..63>>6 = 0 */ \
		chn->mixFunc = (x==0) ? 4 : ( (x) > 0 ? ((x) >> 6) + 2 : (-x) >> 6 ); \
		chn->pan = x; \
	} \
	else { \
		chn->mixFunc = 1; \
		chn->pan = 0; \
	}


#endif

