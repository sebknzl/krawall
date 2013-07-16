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

#include "mixer.h"
#include "directsound.h"
#include "mixer_private.h"
#include "tools.h"

static u32 cidCounter IWRAM;
u8 volMaster[] IWRAM = { 128, 128 };
u8 hqMode IWRAM = 0;
u8 hqRamp IWRAM = 1;

// ignore this.
//  0,  2,  5,  7, 10, 12, 14, 17, 19, 21, 23, 26, 28, 30, 32, 34,
// 36, 38, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 59, 61, 62, 64,

#define VALIDATECONTROL \
	MixChannel *chn = &channels[ c.channel ];	\
	if( chn->id.val != c.val ) \
		return 0;

void mixRampOut( struct MixChannel *c, s16 *dest, uint amount )
{
 	uint smp = c->rampCount <= amount ? c->rampCount : amount;

	// sample order in buffer: L L L L R R R R
	for( ; smp; smp -= 4 ) {
		*( dest + 0 ) += ( c->lvol * ( c->rampPos >> 4 ) ) >> 3;
		*( dest + 4 ) += ( c->rvol * ( c->rampPos >> 4 ) ) >> 3;
		dest++;
		c->rampPos += c->rampInc;
		*( dest + 0 ) += ( c->lvol * ( c->rampPos >> 4 ) ) >> 3;
		*( dest + 4 ) += ( c->rvol * ( c->rampPos >> 4 ) ) >> 3;
		dest++;
		c->rampPos += c->rampInc;
		*( dest + 0 ) += ( c->lvol * ( c->rampPos >> 4 ) ) >> 3;
		*( dest + 4 ) += ( c->rvol * ( c->rampPos >> 4 ) ) >> 3;
		dest++;
		c->rampPos += c->rampInc;
		*( dest + 0 ) += ( c->lvol * ( c->rampPos >> 4 ) ) >> 3;
		*( dest + 4 ) += ( c->rvol * ( c->rampPos >> 4 ) ) >> 3;
		dest++;
		c->rampPos += c->rampInc;

		c->rampCount -= 4;
		amount -= 4;
		dest += 4;
	}

	if( !c->rampCount ) {
		c->status = CHN_INACTIVE;
	}

	if( amount ) {
		mixBias( dest, amount, c->lvol, c->rvol );
	}
}

MIX_FUNCTION((* const mixPanTable[])) = {
	mixStereo,		mixLeft, 	mixStereo,		mixRight, 	mixCenter,		0,	0, 	mixRampOut,
	mixStereoHQ,	mixLeftHQ, 	mixStereoHQ,	mixRightHQ,	mixCenterHQ,	0,	0, 	mixRampOut
};


int IWRAM_CODE kramStop( chandle c )
{
	VALIDATECONTROL;

	if( !hqRamp ) {
		chn->status = CHN_INACTIVE;
		return 1;
	}

	// if ramping is enabled set to ramp-mixfunc if vol > 5
	if( chn->status == CHN_ACTIVE && chn->vol > 5 ) {
		chn->rampCount = 16;
		chn->rampPos = *chn->pos << 4;
		chn->rampInc = ( s16 )128 - *chn->pos;

		chn->mixFunc = 7;
		chn->inc = 0;
		chn->hq = 0;
		chn->pos = chn->start;
		chn->id.val = 0;
	}
	else {
		chn->status = CHN_INACTIVE;
	}

	return 1;
}

int IWRAM_CODE kramSetFreq( chandle c, uint freq )
{
	VALIDATECONTROL;
	SETFREQUENCY(freq);
	return 1;
}

int IWRAM_CODE kramSetVol( chandle c, uint vol )
{
	VALIDATECONTROL;
	SETVOLUME(vol);
	SETLRVOLUME;
	return 1;
}

int IWRAM_CODE kramSetPan( chandle c, int pan )
{
	VALIDATECONTROL;
	SETPANNING(pan);
	SETLRVOLUME;
	return 1;
}

int IWRAM_CODE kramSetPos( chandle c, uint pos )
{
	VALIDATECONTROL;
	if( chn->start + pos < chn->end ) {
		chn->pos = chn->start + pos;
		chn->frac = 0;
	}
	return 1;
}

chandle IWRAM_CODE kramPlayExt( const Sample *s, int sfx, chandle c, uint freq, uint vol, int pan )
{
	MixChannel *chn = &channels[ c.channel ];

	if( hqRamp ) {
		kramStop( c );
	}

	// find free channel
	if( chn->id.val != c.val ) {
		int i = 0;
		chn = &channels[ 0 ];

		while( chn->status && i < CHANNELNUM ) {
			i++; chn++;
		}

		if( i >= CHANNELNUM ) {
			return ( chandle ){ { .val = 0 } };
		}

		c.channel = i;
	}

	c.id = ++cidCounter;
	chn->sfx = sfx & 1;
	chn->id = c;


	SETQUALITY;
	SETPANNING(pan);
	SETVOLUME(vol);
	chn->inc = 0;
	SETFREQUENCY(freq);
	SETLRVOLUME;


	chn->hqs = s->hq;
	chn->pos = chn->start = s->data;
	chn->loopLength = s->loopLength;
	chn->end = s->end;
	chn->loop = s->loop;

	chn->frac = 0;
	chn->lstart = chn->end - chn->loopLength;
	chn->status = CHN_ACTIVE;

	return c;
}

chandle IWRAM_CODE kramPlay( const Sample *s, int sfx, chandle c )
{
	return kramPlayExt( s, sfx, c, s->c2Freq, s->volDefault, s->panDefault );
}

int IWRAM_CODE kramActive( chandle c )
{
	VALIDATECONTROL;
	return ( chn->status != 0 );
}

