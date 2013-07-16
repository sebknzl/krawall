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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "mtypes.h"
#include "mixer.h"

typedef chandle ihandle;

//%b
//! Start music
/*!
\param m Pointer to module
\param mode is one or more of:
			- KRAP_MODE_LOOP	Loop module
			- KRAP_MODE_SONG	Enable song-mode
			- KRAP_MODE_JINGLE	Play module as jingle
\param song Song of module to play
\sa krapStop
*/
void krapPlay( const Module *m, int mode, int song );
#define KRAP_MODE_LOOP 1
#define KRAP_MODE_SONG 2
#define KRAP_MODE_JINGLE 4

//! Stop music
/*!
Immediately stops playback of music.
\sa krapPlay
*/
void krapStop();

//! Stop a playing jingle
/*!
This will immediately stop a playing jingle
and resume playback of the old song.
Note that if no jingle is playing this function will do nothing.
The callback will immediately get called with KRAP_CB_JDONE if set.
\sa krapPlay
\sa krapCallback
*/
void krapStopJingle();

//! Install callback
/*!
Installs a callback. The callback should return as quickly as possible.
When the callback gets called the first numeric parameter
describes the event, the second numeric parameter (if any)
is the parameter to the event.
The events are as following:
	- KRAP_CB_FADE		Destination volume has been reached
	- KRAP_CB_DONE		Module is done (also when KRAP_MODE_LOOP)
	- KRAP_CB_MARK		Mark-Effect Zxx (xx in param 2)
	- KRAP_CB_SONG		Song-boundary hit (+++-Marker)
	- KRAP_CB_JDONE		Jingle is done
\sa krapSetMusicVol
*/
void krapCallback( void (*func)( int, int ) );
#define KRAP_CB_FADE 1	// fading done (see krapSetMusicVol)
#define KRAP_CB_DONE 2	// module stop
#define KRAP_CB_MARK 3	// mark-effect occured in pattern (op in param 2)
#define KRAP_CB_SONG 4	// played over a song-boundary (++)
#define KRAP_CB_JDONE 5 // jingle done
#define KRAP_CB_LOOP 6  // module/song restarts because looped

//! Pause music
/*!
Pauses all currently active channels. You still can play SFX's.
The paused channels will be frozen until krapUnpause() gets called.
\param sfx If true pause sfx as well; if false pause music only
\sa krapUnpause
*/
void krapPause( int sfx );

//! Unpause music
/*!
Reactivates all channels that have been paused with krapPause()
\sa krapPause
*/
void krapUnpause();

//! Get Pause status
/*!
Returns whether playback is currently paused or not
\return True if paused
\sa krapPause
\sa krapUnpause
*/
int krapIsPaused();

//! Set music volume
/*!
You can either set the music volume immediately or fade slowly
to the specified volume. The fadespeed depends on the speed
of the currently active module.
If module is paused then volume is always set immediately.
If a callback is installed it will get triggered when fading is done.
The volume given will directly scale the global volume set in the S3M.
\param vol Music volume (0..128)
\param fade If true fade, if false set immediately
\sa kramSetSFXVol
\sa krapCallback
*/
void krapSetMusicVol( unsigned int vol, int fade );

//! Get music volume
/*!
Returns volume as set by krapSetMusicVol()
\sa krapSetMusicVol
\return volume
*/
unsigned int krapGetMusicVol();

//! Play an instrument as SFX
/*!
Plays an instrument as an SFX. If you use this, be sure to call
krapInstProcess() periodically, this is where the envelopes get
processed.
\param ins Pointer to instrument (instruments[])
\param note 0 (C-0) .. 95 (B-7)
\param old Old handle, will be recycled if given
\sa kramPlay
\sa krapInstRelease
\sa krapInstStop
\sa krapInstProcess
\return Instrument handle, zero if no channel/instrument could be allocated
*/
ihandle krapInstPlay( const Instrument *ins, int note, ihandle old );

//! Release a playing instrument
/*!
Releases a playing instrument if either still playing or in sustain-mode.
\param i Handle as returned by krapInstPlay()
\return true if successful
\sa krapInstPlay
*/
int krapInstRelease( ihandle i );

//! Stop a playing instrument
/*!
Will immediately stop a playing instrument
\param i Handle as returned by krapInstPlay()
\return true if successful
\sa krapInstPlay
*/
int krapInstStop( ihandle i );

//! Process instrument-sfx envelopes
/*!
If you use instruments for sfx you should call this periodically.
Once a frame is quite good idea.
*/
void krapInstProcess();

//! Check if an instrument-handle is still valid
/*!
This is similiar to kramHandleValid() but works
for an instrument-handle. It will return false
if an instrument has already stopped playing (one-shot sample).
\param i handle to check
\return true if valid, false if invalid
\sa kramHandleValid
*/
int krapInstHandleValid( ihandle i );

//! Set a channel's volume
/*!
This will set a channel's volume.
S3M-effects Mxx and Nxx will override this value.
krapPlay will reset all channel's volume to 64.
\param channel mod-channel (0..#channels of current mod)
\param vol volume (0..64)
\sa krapGetChannelVol
*/
void krapSetChannelVol( unsigned int channel, unsigned int vol );

//! Get a channel's volume
/*!
This will get a channel's volume as either set by
krapSetChannelVol() or S3M-effects Mxx and Nxx.
\param channel mod-channel (0..#channels of current mod)
\return volume
\sa krapSetChannelVol
*/
unsigned int krapGetChannelVol( unsigned int channel );

//%e

// semi-private stuff
void krapInit();

#endif

