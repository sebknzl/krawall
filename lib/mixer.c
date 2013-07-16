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
#include "mixer_private.h"
#include "tools.h"
#include "directsound.h"

#if IWRAM_USAGE_SMALL
MixChannel channels[ CHANNELNUM ] EWRAM;
#else
MixChannel channels[ CHANNELNUM ] IWRAM;
#endif

static u16 soundTimerCountBackup EWRAM;

void kramSetSoundTimer( void (*func)() )
{
	soundTimerFunc = func;
	if( !func ) {
	 	soundTimerCountBackup = soundTimerCount;
	}
	else {
	 	soundTimerCount = soundTimerCountBackup;
	}
}

void kramSetSoundTimerBPM( int bpm )
{
	if( bpm < BPM_MIN )
		bpm = BPM_MIN;

	int x;
	FastIntDivide( 15 * SAMPLEFREQ, bpm, &x );
	// aligned to 4
	x <<= 2;
	soundTimerSpeed = x;
	soundTimerCountBackup = soundTimerCount = soundTimerSpeed;
}

void kramResetChannels( int sfx )
{
	int i;
	MixChannel *chn;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ ) {
		if( chn->status && chn->sfx == sfx ) {
			SETPANNING(chn->pan);
			SETLRVOLUME;
		}
	}
}


void kramSetMusicVol( uint vol )
{
	volMaster[ 0 ] = vol > 255 ? 255 : vol;
	kramResetChannels( 0 );
}

void kramSetSFXVol( uint vol )
{
	volMaster[ 1 ] = vol > 255 ? 255 : vol;
	kramResetChannels( 1 );
}

uint kramGetSFXVol()
{
	return volMaster[ 1 ];
}

void kramSetMasterVol( uint vol )
{
	int i, v, x;

	if( ( vol >> 16 ) == 0 ) {
		// no KRAM_MV_xx set, default to 5
		mix16to8_patch( 5 );
	}
	else {
		mix16to8_patch( ( vol >> 16 ) );
	}

	vol &= 0xff;

	// calc clip-table
	v = 0;
	for( i = 0; i < 1024; i++ ) {
		x = v >> 7;
		volClipTable[ i ] = ( x > 127 ) ? 127 : x;
		volClipTable[ -i ] = ( x > 128 ) ? -128 : -x;
		v += vol;
	}
	x = v >> 7;
	// volClipTable[ -1024 ] makes GCC 3.0.2 generate an invalid constant, leave it like this
	volClipTable[ -i ] = ( x > 128 ) ? -128 : -x;

}

int kramGetActiveChannels()
{
	int ret = 0;
	int i;
	MixChannel *chn;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ )
		if( chn->status == CHN_ACTIVE )
			ret++;
	return ret;
}

void kramPauseChannels( int sfx, int slot )
{
	int i;
	MixChannel *chn;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ ) {
		if( ( chn->status == CHN_ACTIVE ) &&
			( sfx || !( chn->sfx ) ) )
			chn->status = slot;
	}
}

void kramUnpauseChannels( int slot )
{
	int i;
	MixChannel *chn;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ )
		if( chn->status == slot )
			chn->status = CHN_ACTIVE;
}

void kramStopChannels( int slot )
{
	int i;
	MixChannel *chn;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ )
		if( chn->status == slot )
			chn->status = CHN_INACTIVE;
}

void kramStopSFXChannels()
{
	int i;
	MixChannel *chn;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ ) {
		if( chn->sfx && chn->status == CHN_ACTIVE ) {
			chn->status = CHN_INACTIVE;
		}
	}
}

void kramQualityMode( int m )
{
	int i;
	MixChannel *chn;

	if( m & KRAM_QM_RAMP_OFF ) {
		hqRamp = 0;
	}
	else {
		hqRamp = 1;
	}

/*
	if( m & KRAM_QM_RAMP ) {
		hqRamp = 1;
	}
	else {
		hqRamp = 0;
	}
*/
	m &= 0xf;

	hqMode = m > 2 ? 0 : m;
	for( i = CHANNELNUM, chn = &channels[ 0 ]; i; i--, chn++ ) {
		if( chn->status != CHN_INACTIVE ) {
			SETQUALITY;
		}
	}
}


int kramHandleValid( chandle c )
{
	return ( channels[ c.channel ].id.val == c.val ) && channels[ c.channel ].status ? 1 : 0;
}

uint kramGetFreq( chandle c )
{
	uint ret = ( channels[ c.channel ].inc >> 2 ) << ( SAMPLEFREQEXP - 14 );
	return ( ret < 0 ) ? -ret : ret;
}

uint kramGetVol( chandle c )
{
	return channels[ c.channel ].vol;
}

int kramGetPan( chandle c )
{
	return channels[ c.channel ].pan;
}

uint kramGetPos( chandle c )
{
	return channels[ c.channel ].pos - channels[ c.channel ].start;
}

