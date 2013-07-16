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

#ifndef __MIXER_H__
#define __MIXER_H__

#include "types.h"
#include "mtypes.h"

typedef struct {
	union {
		u32 val;
		struct {
			unsigned channel:8;
			unsigned id:24;
		};
	};
} chandle;

//%b

//! Worker procedure
/*!
This is where the actual work is done, you *MUST* call
this once per frame after kraInit() to get sound
\sa kragInit()
\return True if actual work has been done
*/
int kramWorker() LONG_CALL;

//! Get number of currently active channels
/*!
Returns number of currently active channels.
\return Number of currently active channels
*/
int kramGetActiveChannels();

//! Set quality mode
/*!
This sets the quality mode of the mixing routines.
KRAM_QM_NORMAL is the default, KRAM_QM_MARKED only plays the marked samples (see docs)
in HQ and KRAM_QM_HQ plays everything in HQ.
The flag KRAM_QM_RAMP_OFF (must be OR'd) DISABLES stop-ramping. Until version 20040707
you had to enable it explicitly, now it must be disabled explicitly. Hence the flag
KRAM_QM_RAMP has lost it's meaning.
Especially looped samples that get stopped abruptly might cause pops.
Stop-ramping removes these pops at the cost of a little more CPU.
*/
void kramQualityMode( int );
#define KRAM_QM_NORMAL 		0
#define KRAM_QM_MARKED 		1
#define KRAM_QM_HQ 			2
#define KRAM_QM_RAMP		16
#define KRAM_QM_RAMP_OFF	32

//! Play a sample
/*!
Plays a sample with it's C2 (neutral) frequency.
\param s Pointer to sample
\param sfx Whether sample to play is an SFX
\param c Old handle, will be recycled if given
\sa kramPlayExt()
\return Channel handle
*/
chandle kramPlay( const Sample *s, int sfx, chandle c ) LONG_CALL;

//! Play a sample Ext
/*!
Just like kramPlay, but all of the attribs can be specified.
\param s Pointer to sample
\param sfx Whether sample to play is an SFX
\param c Old handle, will be recycled if given
\param freq Frequency in hertz to play sample at
\param vol Volume to play sample with (0..64)
\param pan Panning to play sample with (-64..64)
\sa kramPlay()
\return Channel handle
*/
chandle kramPlayExt( const Sample *s, int sfx, chandle c, unsigned int freq, unsigned int vol, int pan ) LONG_CALL;

//! Stop a channel
/*!
Stops playback of a channel.
Note that if the channel has already stopped this call will not do anything and return false.
\param c Channel handle
\return true if successful
*/
int kramStop( chandle c ) LONG_CALL;

//! Set frequency
/*!
Sets frequency of an active channel.
Note that if the channel has already stopped this call will not do anything and return false.
\param c Channel handle
\param freq Frequency in hertz
\return true if successful
*/
int kramSetFreq( chandle c, unsigned int freq ) LONG_CALL;

//! Set volume
/*!
Sets volume of an active channel.
Note that if the channel has already stopped this call will not do anything and return false.
\param c Channel handle
\param vol Volume (0..64)
\return true if successful
*/
int kramSetVol( chandle c, unsigned int vol ) LONG_CALL;

//! Set panning
/*!
Sets the panning-position of an active channel.
Note that if the channel has already stopped this call will not do anything and return false.
\param c Channel handle
\param pan Panning (-64..0..64), KRAM_SP_LEFT, KRAM_SP_RIGHT, KRAM_SP_CENTER
\return true if successful
*/
int kramSetPan( chandle c, int pan ) LONG_CALL;
#define KRAM_SP_LEFT -64
#define KRAM_SP_CENTER 0
#define KRAM_SP_RIGHT 64
//@}

//! Set Position
/*!
Sets the sample-position of an active channel.
Note that if the channel has already stopped this call will not do anything and return false.
\param c Channel handle
\param pos Sample offset to set
\return true if successful
*/
int kramSetPos( chandle c, unsigned int pos ) LONG_CALL;

//! Set SFX volume
/*!
Sets the volume of all active and future sfx.
\param vol Volume (0..128)
*/
void kramSetSFXVol( unsigned int vol );

//! Get SFX volume
/*!
Returns volume as set with kramSetSFXVol().
\sa kramSetSFXVol
\return volume
*/
unsigned int kramGetSFXVol();

//! Set Master Clip volume
/*!
Sets the clipping curve's steepness.
128 is the default value, setting a neutral clipping curve.
Values below 128 (down to 16) can be used to reduce distortion (volume) if the output is too high.
Values above 128 will give you additional gain but also reduce the quality because
information is lost, don't do this.
Additionally you can OR the volume with one of the parameters KRAM_MV_CHANNELS32, KRAM_MV_CHANNELS16
or KRAM_MV_CHANNELS8. KRAM_MV_CHANNELS32 is the default -- specifying one of the other values
will give you additional gain. However as the parameter says you should not use more than the amount
of channels then - otherwise you might get unpredictable clicks/distortion.
\param vol 128 is default, everything below/above changes the clipping curve. OR with KRAM_MV_CHANNELS16 or KRAM_MV_CHANNELS8 if appropriate
*/
void kramSetMasterVol( unsigned int vol );
#define KRAM_MV_CHANNELS32 ( 5 << 16 )
#define KRAM_MV_CHANNELS16 ( 4 << 16 )
#define KRAM_MV_CHANNELS8 ( 3 << 16 )

//! Stops all active SFX channels
/*!
Stops all currently active SFX channels. Paused SFX channels are not stopped.
*/
void kramStopSFXChannels();

//! Check whether a handle is still valid
/*!
Checks if chandle is still a valid handle. A handle will
get invalidated if for example a one-shot sample ends.
\param c handle to check
\return true if valid, false if invalid
*/
int kramHandleValid( chandle c );

//! Get frequency
/*!
Get frequency of a channel.
THIS FUNCTION DOES NOT CHECK FOR HANDLE-VALIDITY!
\param c handle
\return current frequency
*/
unsigned int kramGetFreq( chandle c );

//! Get volume
/*!
Get volume of a channel.
THIS FUNCTION DOES NOT CHECK FOR HANDLE-VALIDITY!
\param c handle
\return current volume
*/
unsigned int kramGetVol( chandle c );

//! Get panning
/*!
Get panning of a channel.
THIS FUNCTION DOES NOT CHECK FOR HANDLE-VALIDITY!
\param c handle
\return current panning
*/
int kramGetPan( chandle c );

//! Get position
/*!
Get position of a channel.
THIS FUNCTION DOES NOT CHECK FOR HANDLE-VALIDITY!
\param c handle
\return current position
*/
unsigned int kramGetPos( chandle c );

//%e
//////////////////////////////////////////////////////////////////////////

// semi-private functions
// set volume of all SFX's (0..128)
void kramSetMusicVol( uint vol );
void kramSetSoundTimer( void (*func)() );
void kramSetSoundTimerBPM( int bpm );
void kramPauseChannels( int sfx, int slot );
#define KRAM_PAUSE_SLOT1	2		// normal pause
#define	KRAM_PAUSE_SLOT2	3		// normal pause for jingle
#define	KRAM_PAUSE_SLOT3	4		// pause slot for non-jingle voices

void kramUnpauseChannels( int slot );
void kramStopChannels( int slot );

void kramResetChannels( int sfx );	// resets vol&pan

int kramActive( chandle c ) LONG_CALL;

#endif

