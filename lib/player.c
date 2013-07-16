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

#include <string.h>
#include "player.h"
#include "types.h"
#include "tools.h"
#include "mtypes.h"
#include "mixer.h"
#include "player_tables.h"
#include "effects.h"

typedef int bool;
#define true 1
#define false 0
typedef u8 flag;


typedef struct {
	chandle	handle;
	s8 		volume;
	s8		cvolume;
	s8 		panning;
	u8		pos;

	s8		volumec;		// current volume & panning,
	s8		panningc;		// modified by tremolo, panbrello, tremor etc

	u8		effect;			// current effect

	s16 	note;			// last set note (index into periods[])
	s16		period;			// period
	s16		periodc;		// current period (+vibrato stuff)
	const Sample* 		sample;
	const Instrument* 	instrument;


	u8		leffect;		// last effect
	u8		effectop;		// current effectop

	// porta-to-note
	s16		portainc;		// porta-inc
	s16		portadest;		// dest-period for porta-to-note
	s16		glissnote;		// current glissando-note (->periods[])

	// vibrato
	u8		vibrpos;		// position in vibrato-table
	u8		vibrspeed;
	u8		vibrdepth;
	flag	vibrretrig;		// reset vibrpos on new note?
	const s16*	vibrtable;	// points into vibrato-table (sine, ramp, square, random)

	// tremolo
	u8		trempos;
	u8		tremspeed;
	u8		tremdepth;
	flag	tremretrig;
	const s16*	tremtable;

	// panbrello
	u8		panbpos;
	u8		panbspeed;
	u8		panbdepth;
	flag	panbretrig;

	union {		// we don't need panbtable in effchns[]
		const s16*	panbtable;
		ihandle id;
	};

	flag	glissando;		// glissando (=slide in halftone-steps)?

	// tremor
	u8		trmcnt;
	u8		trmon;
	u8		trmtot;

	// other params we must remember
	u8		volslide;
	u8		volslide_fine;
	u8		cvolslide;
	flag	volslidefine;


	u8		panslide;
	flag	panslidefine;

	u8		arpeggio;
	u8		arpeggiocnt;

	u8		porta;
	u8		porta_fine;
	u8		porta_efine;
	u8		retrig;

    flag	resetnote;		// reset frequency when effect's over? (arpeggio&vibrato&...)
    flag	playnote;		// will we play the note after the row-effects?
    flag	touched;		// has channel been touched while decoding pattern-data?

    // instrument-stuff:
	#define ENVFLAGS_ENABLED 1
	#define ENVFLAGS_SUSTAIN 2
	#define ENVFLAGS_LOOP 4

	#define	ENVSTATUS_DISABLED 0
	#define ENVSTATUS_STOPPED 1
	#define ENVSTATUS_SUSTAIN 2
	#define ENVSTATUS_ACTIVE 3
	#define ENVSTATUS_ACTIVE_NOSUSTAIN 4

	flag	i_active;

    u8		i_vol_status;
    s8		i_vol;
    u16		i_vol_pos;
    u8		i_vol_node;
    u8		i_vol_tick;
    u8		i_vol_ticktarget;

	flag	i_vol_fadeactive;
    s16		i_vol_fade;

    u8		i_pan_status;
    s8		i_pan;
    u16		i_pan_pos;
    u8		i_pan_node;
    u8		i_pan_tick;
    u8		i_pan_ticktarget;

	u8		effectVC;		// volumn-column effect op

	u8		i_vibrpos;
} MChannel;

#define MCHANNELAMOUNT 20

typedef struct {
	int mode;					// loop, song

	int status;
	#define STATUS_STOP 0
	#define STATUS_PAUSE 1
	#define STATUS_PLAY 2

	const Module *module;

	u16 tempo;
	u16 row;

	u8 speed;
	u8 tick;
	u8 order;
	s8 pGotoOrder;

	const u8* pData;

	s16 pGotoRow;

	s8 pLoopOrder;
	s8 pLoopCount;
	s8 pLoopActive;

	s8 volume;
	u8 volSlide;	// last effect param (global volslide)

	u8 vibrOfs;		// offset to start in vibratotable (0 for amiga, 32 for linear)

	flag instrumentBased;
	flag fastVolSlides;

	u16 (*calcPeriod)( unsigned, const Sample* );
	u32 (*calcFinalFreq)( unsigned p );

	int songStartOrder;

	const Pattern* currPattern;
	MChannel chns[ MCHANNELAMOUNT ];
} Player;



#ifdef IWRAM_USAGE_XL
static Player player IWRAM;
#else
static Player player EWRAM;
#endif
static Player playerBackup EWRAM;


static int musicVolume EWRAM = 128;
static int musicVolumeTarget EWRAM;
static int musicVolumeFadeInc EWRAM;	// inc

/* if func2 != 0 it will only be called inbetween rows! */
typedef void (*EffFunc)( MChannel*, bool );
typedef struct {
	EffFunc func1;
	EffFunc func2;
	u8		inbet1;
} effectStruct;

static const effectStruct effects[];

typedef struct {
	EffFunc func;
	u8		inbet;
} effectStructVC;

static const effectStructVC effectsVC[];


static void (*callBack)( int, int ) EWRAM;

#define F_CLIP(x,y)	{ if(x<y) x=y; } //floor
#define C_CLIP(x,y)	{ if(x>y) x=y; } //ceiling


static void pDataSet()
{
	int row = player.row;
	player.currPattern = player.module->patterns[ player.module->order[ player.order ] ];
	if( !row ) {
		// go to start of pattern
		player.pData = player.currPattern->data;
	}
	else {
		// use index
		player.pData = &player.currPattern->data[ player.currPattern->index[ ( row >> 2 ) & 0xf ] ];

		int l = row & 3;

		// if index mod 4 != 0 we have too seek on a few rows
		while( l ) {
			u8 follow = *player.pData++;
			if( !follow ) {
				l--;
				continue;
			}

			if( follow & 32 ) {
				player.pData += 2;
			}
			if( follow & 64 ) {
				player.pData++;
			}
			if( follow & 128 ) {
				player.pData += 2;
			}
		}
	}
}

static void jingleDone();

static void advanceRow()
{
	// pattern jump?
	if( player.pGotoOrder >= 0 ) {
		player.order = player.pGotoOrder - 1;
		player.row = 8192;		// big enough, so the next if-statement is true
		player.pGotoOrder = -1;
	}

	// no, everything's normal
	if( ( ++player.row >= player.currPattern->rows ) ||		// pattern done?
		( player.pGotoRow >= 0 ) ) {						// or pattern break?

		bool markerFound = false;

		// jump over all '+++' in the orderlist
		while( player.module->order[ ++player.order ] == 254 && player.order < player.module->numOrders ) {
			markerFound = true;
		}

		if( markerFound && callBack )
			callBack( KRAP_CB_SONG, 0 );

		// beyond last order or over +++ (songmode!)?
		if( player.order >= player.module->numOrders ||					// last pattern
			( markerFound && ( player.mode & KRAP_MODE_SONG ) ) ) {		// or +++ in song-mode?

			player.order = player.songStartOrder;

			// not looped?
			if( !( player.mode & KRAP_MODE_LOOP ) ) {
				// jingle-mode? if so stop the jingle!
				if( player.mode & KRAP_MODE_JINGLE ) {
					jingleDone();
					return;
				}
				else {
					// nope, song's simply done, stop!
					krapStop();
					if( callBack )
						callBack( KRAP_CB_DONE, 0 );
					return;
				}
			}

			// it is looped! let the callback-subscriber know!
			if( callBack )
				callBack( KRAP_CB_LOOP, 0 );
		}

		player.row = 0;
		pDataSet();
	}

	// does row have to be altered? (patternbreak/patternjump)
	if( player.pGotoRow >= 0 ) {
		player.row = player.pGotoRow < player.currPattern->rows ? player.pGotoRow : 0;
		pDataSet();
		player.pGotoRow = -1;
	}
}

static u16 calcPeriodAmiga( unsigned n, const Sample* s )
{
//	int ret;
//	FastIntDivide( 8363 * table_periods[ n ], s->c2Freq, &ret );
//	return ret;
// this is faster:
	return ( table_periods[ n ] * table_finetune[ 32 + (s->fineTune>>2) ] ) >> 15;
}

static u32 calcFinalFreqAmiga( unsigned p )
{
	int ret;
	FastIntDivide( 8363*1712, p, &ret );
	return ret;
}

static u16 calcPeriodLinear( unsigned n, const Sample* s )
{
	return (n<<6) + 2*768 + (s->fineTune>>1);
}

static u32 calcFinalFreqLinear( unsigned p )
{
	int okt, rem;
	okt = ((p>>6)*0xaaab)>>19;	// same as div 768 (exact for all possible p's)
	rem = p - okt * 768;
	return table_linear[ rem ] >> ( 23 - okt );
}

// for arpeggio
static u32 calcFinalFreqFromNote( int n, const Sample* s )
{
//	int ret;
//	FastIntDivide( 1712 * s->c2Freq, table_periods[ n ], &ret );
//	return ret;
// this is faster:
	return calcFinalFreqLinear( calcPeriodLinear( n, s ) );
}

static inline void setChannelFreq( MChannel *c )
{
	c->periodc = c->period;
	kramSetFreq( c->handle, player.calcFinalFreq( c->period ) );
}

static inline void setChannelFreqH( MChannel *c, u32 p )
{
	c->periodc = p;
	kramSetFreq( c->handle, player.calcFinalFreq( p ) );
}
static inline void setChannelFreqNote( MChannel *c, u32 n )
{
 	c->periodc = calcPeriodLinear( n, c->sample );
 	kramSetFreq( c->handle, calcFinalFreqLinear( c->periodc ) );
}


static inline bool setChannelVol( MChannel *c )
{
	c->volumec = c->volume;
	return kramSetVol( c->handle, c->volumec * c->cvolume * c->i_vol >> 12 );
}
static inline bool setChannelVolH( MChannel *c )
{
	return kramSetVol( c->handle, c->volumec * c->cvolume * c->i_vol >> 12 );
}


static inline bool setChannelPan( MChannel *c )
{
	c->panningc = c->panning;
	s8 x = c->panningc + c->i_pan;
	F_CLIP( x, -64 );
	C_CLIP( x, 64 );
	return kramSetPan( c->handle, x );
}
static inline bool setChannelPanH( MChannel *c )
{
	s16 x = c->panningc + c->i_pan;
	F_CLIP( x, -64 );
	C_CLIP( x, 64 );
	return kramSetPan( c->handle, x );
}


static inline void setChannelPos( MChannel *c )
{
	kramSetPos( c->handle, ( u32 )c->pos << 8 );
}
static inline void setChannelPosH( MChannel *c, u32 pos )
{
	kramSetPos( c->handle, pos << 8 );
}


static inline void stopChannel( MChannel *c )
{
	c->i_active = false;
	kramStop( c->handle );
}


static inline bool i_init( MChannel *c )
{
	const register Instrument *ins = c->instrument;
	bool ret = false;
	c->i_active = false;
    c->i_vibrpos = 0;

	// volume-envelope?
	if( ins->envVol.flags & ENVFLAGS_ENABLED ) {
		c->i_vol_status = ENVSTATUS_ACTIVE;
    	c->i_vol_node = c->i_vol_tick = -1;
		c->i_vol_ticktarget = 0;

		c->i_vol_fade = 32767;
		c->i_vol_fadeactive = false;

		c->i_active = true;
		ret = true;
	}
	else {
		c->i_vol = 64;
		c->i_vol_status = ENVSTATUS_DISABLED;
	}

	// pan-envelope?
    if( ins->envPan.flags & ENVFLAGS_ENABLED ) {
		c->i_pan_status = ENVSTATUS_ACTIVE;
		c->i_pan_node = c->i_pan_tick = -1;
		c->i_pan_ticktarget = 0;
    	c->i_active = true;
	}
	else {
		c->i_pan = 0;
		c->i_pan_status = ENVSTATUS_DISABLED;
	}

	if( ins->vibRate ) {
		c->i_active = true;
	}

	return ret;
}

static inline void playChannelNote( MChannel *c )
{
	if( player.instrumentBased ) {
		i_init( c );
	}
	c->resetnote = false;

	c->handle =
    	kramPlayExt(
       		c->sample, 0, c->handle,
       		player.calcFinalFreq( c->period ),
       		c->volume * c->cvolume >> 6, c->panning
       	);

	if( c->pos )
    	setChannelPos( c );
}

static inline void i_release( MChannel *c )
{
	if( c->i_vol_status ) {
		c->i_vol_fadeactive = true;
		if( c->i_vol_status == ENVSTATUS_SUSTAIN )
			c->i_vol_status = ENVSTATUS_ACTIVE;
		else
			c->i_vol_status = ENVSTATUS_ACTIVE_NOSUSTAIN;

		// this is ok here, cause w/o a volume-envelope === will stop the sample
		if( c->i_pan_status ) {
			if( c->i_pan_status == ENVSTATUS_SUSTAIN )
				c->i_pan_status = ENVSTATUS_ACTIVE;
			else
				c->i_pan_status = ENVSTATUS_ACTIVE_NOSUSTAIN;
		}
	}
	else {
		stopChannel( c );
	}
}

static void processRow()
{
	while( 1 ) {

		u8 follow = *player.pData++;
		if( !follow )
			break;

		u8 note = 0, vol = 0, eff = 0, effop = 0;
		u16 instrument = 0;
		flag setVol = false;
		flag sampleSet = false;

		MChannel *c = &player.chns[ follow & 0x1f ];
		c->touched = true;

		// note/sample
		if( follow & 32 ) {
			note = *player.pData++;
			instrument = *player.pData++;
			if( note & 128 ) {
				instrument |= ( *player.pData++ ) << 8;
				note &= 0x7f;
			}
		}
		// volume
		if( follow & 64 ) {
			vol = *player.pData++;
		}
		// effect
		if( follow & 128 ) {
			eff = *player.pData++;
			effop = *player.pData++;
		}

		c->leffect = c->effect;
		c->effect = eff;

		// was there an instrument? set vol & instrument/sample
		if( instrument ) {
			instrument--;
			if( player.instrumentBased ) {
				if( note ) {
					c->instrument = instruments[ instrument ];
					c->sample = samples[ c->instrument->samples[ note-1 ] ];
					c->volume = c->volumec = c->sample->volDefault;
					c->panning = c->panningc = c->sample->panDefault;
					setVol = true;
					sampleSet = true;
				}
			}
			else {
				c->sample = samples[ instrument ];
				c->volume = c->volumec = c->sample->volDefault;
				setVol = true;
			}
		}

		// volume
		if( vol ) {
			if( vol <= 0x60 ) {
				c->volume = vol - 0x10;
				C_CLIP( c->volume, 0x40 );
				c->volumec = c->volume;
				c->effectVC = 0;
				setVol = true;
			}
			else {
				// volume-column-effects
				c->effectVC = vol - 0x60;
				effectsVC[ c->effectVC >> 4 ].func( c, true );
			}
		}
		c->playnote = false;

        // note && sample set
        if( note && c->sample ) {
       		if( note-- == 0x7f ) { // note-off (postincrement -- on purpose)
       			i_release( c );
       		}
       		else {
				note += c->sample->relativeNote;

       			if( eff != EFF_PORTA_NOTE &&
       				eff != EFF_VOLSLIDE_PORTA ) {
	       			if( player.instrumentBased ) {
	       				if( !sampleSet )
    	   					c->sample = samples[ c->instrument->samples[ note ] ];
       					// c->instrument is always set at this point
       					//i_init( c );
       					// -> now in playChannelNote (so envs get delayed with NoteDelay, too ($#@%!!))
	       			}

       				c->note = c->glissnote = note;
       				c->period = c->periodc = c->portadest = player.calcPeriod( note, c->sample );
      				c->pos = 0;
      				c->trmcnt = 0;
       				c->playnote = true;
       			}
       			else {
       				c->portadest = player.calcPeriod( note, c->sample );
       			}
       		}
		}

		c->effectop = effop;

		// call onrow-effect
		if( eff ) {
			if( effects[ eff ].func1 )
				effects[ eff ].func1( c, true );
		}

		if( c->playnote ) {
			playChannelNote( c );
       	}
       	else if( setVol )
       			setChannelVol( c );
	}


	// now let's go over all channels again
	// (some might have not been touched because there was no pattern-data)
	int i;
	MChannel *c = &player.chns[ 0 ];
	for( i = player.module->channels; i; i-- ) {
		if( !c->touched ) {
			c->effectop = 0;
			c->leffect = c->effect;
			c->effect = 0;
			c->effectVC = 0;
		}
		else {
			c->touched = false;
		}

		// must the note be reset? (freq/vol/pan)
		if( c->resetnote && ( c->effect != c->leffect ) ) {
			setChannelFreq( c );
			setChannelVol( c );
			setChannelPan( c );
			c->resetnote = false;
		}

		c++;
	}

	advanceRow();
}


static void processInbetweenRow()
{
	int i;
	MChannel *c = &player.chns[ 0 ];
	for( i = player.module->channels; i; i--, c++ ) {
		// volume column effects
		if( c->effectVC ) {
			if( effectsVC[ c->effectVC >> 4 ].inbet )
				effectsVC[ c->effectVC >> 4 ].func( c, false );
		}
		// normal effects
		if( c->effect ) {
			if( effects[ c->effect ].inbet1 )
				effects[ c->effect ].func1( c, false );
			if( effects[ c->effect ].func2 )
				effects[ c->effect ].func2( c, false );
		}
	}
}

static void processInstruments( MChannel *c, int amount )
{
	int i;

	for( i = amount; i; i--, c++ ) {

		if( !c->i_active )
			continue;

		const register Instrument *ins = c->instrument;

		// ==============================
		// volume envelopes/fadeout
		// ==============================
		if( c->i_vol_status ) {
			bool setVol = false;

			// fading?
			if( c->i_vol_fadeactive ) {
				setVol = true;
				c->i_vol_fade -= ins->volFade;
				if( c->i_vol_fade <= 0 ) {
					stopChannel( c );
					continue;
				}
			}

			// vol-envelope?
			if( c->i_vol_status >= ENVSTATUS_ACTIVE ) {
				const Envelope *env = &ins->envVol;

				setVol = true;
				c->i_vol_pos += env->nodes[ c->i_vol_node ].inc;

				// hit next node?
				if( ++c->i_vol_tick == c->i_vol_ticktarget ) {
					c->i_vol_node++;

					// sustain-node?
					if( ( env->flags & ENVFLAGS_SUSTAIN ) &&
						( c->i_vol_node == env->sus ) &&
						( c->i_vol_status != ENVSTATUS_ACTIVE_NOSUSTAIN ) ) {
						c->i_vol_status = ENVSTATUS_SUSTAIN;
					}

					// max node? check for loop!
					if( c->i_vol_node == env->max ) {
						if( env->flags & ENVFLAGS_LOOP ) {
							c->i_vol_node = env->loopStart;
							c->i_vol_tick = env->nodes[ c->i_vol_node ].coord & 0x1ff;
						}
						else {
							c->i_vol_status = ENVSTATUS_STOPPED;
							if( ( env->nodes[ c->i_vol_node ].coord >> 9 ) == 0 ) {
								stopChannel( c );
								continue;
							}
						}
					}

					c->i_vol_pos = ( env->nodes[ c->i_vol_node ].coord >> 9 ) << 8;
					c->i_vol_ticktarget = 1 + ( env->nodes[ c->i_vol_node + 1 ].coord & 0x1ff );
				}
			}

			if( setVol ) {
				c->i_vol = ( c->i_vol_pos * c->i_vol_fade ) >> 23;
				if( !setChannelVolH( c ) ) {	// sample is already done (kramSetVolume returned false)
					stopChannel( c );
					continue;
				}
			}
		}

		// ==============================
		// panning envelopes/fadeout
		// ==============================
		// this is a little copy'n'paste. too bad :(
		if( c->i_pan_status >= ENVSTATUS_ACTIVE ) {
			const Envelope *env = &ins->envPan;

			c->i_pan_pos += env->nodes[ c->i_pan_node ].inc;

			if( ++c->i_pan_tick == c->i_pan_ticktarget ) {
				c->i_pan_node++;

				if( ( env->flags & ENVFLAGS_SUSTAIN ) &&
					( c->i_pan_node == env->sus ) &&
					( c->i_pan_status != ENVSTATUS_ACTIVE_NOSUSTAIN ) ) {
					c->i_pan_status = ENVSTATUS_SUSTAIN;
				}

				if( c->i_pan_node == env->max ) {
					if( env->flags & ENVFLAGS_LOOP ) {
						c->i_pan_node = env->loopStart;
						c->i_pan_tick = 1 + ( env->nodes[ c->i_pan_node ].coord & 0x1ff );
					}
					else {
						c->i_pan_status = ENVSTATUS_STOPPED;
					}
				}

				c->i_pan_pos = ( env->nodes[ c->i_pan_node ].coord >> 9 ) << 8;
				c->i_pan_ticktarget = env->nodes[ c->i_pan_node + 1 ].coord & 0x1ff;
			}

			c->i_pan = ( ( c->i_pan_pos >> 8 ) - 32 ) << 1;

			if( !setChannelPanH( c ) ) {
				stopChannel( c );
				continue;
			}
		}

		// ==============================
		// sample vibrato
		// ==============================
		if( ins->vibRate ) {
			// vibr-sweep's not implemented
			// vibr-type not implemented (using table set in current channel)
			u16 vperiod = c->periodc + ( ( ins->vibDepth * c->vibrtable[ c->i_vibrpos >> 2 ] ) >> 8 );
			kramSetFreq( c->handle, player.calcFinalFreq( vperiod ) );
			c->i_vibrpos += ins->vibRate;
			c->i_vibrpos &= ( TABLE_VIBRATO_LENGTH << 2 ) - 1;	// =0xff
		}

	}
}


static inline void setGlobalVol()
{
	kramSetMusicVol( musicVolume * ( ( player.module ) ? ( player.module->volGlobal * player.volume ) : 128*64 ) >> 13 );
}

static void timerRoutine()
{
	if( ++player.tick >= player.speed ) { // row
		player.tick = 0;
		processRow();
	}
	else {
		processInbetweenRow();
	}

	if( player.instrumentBased ) {
		processInstruments( &player.chns[ 0 ], player.module->channels );
	}

	if( musicVolumeFadeInc ) {
		musicVolume += musicVolumeFadeInc;
		setGlobalVol();

		if( musicVolume == musicVolumeTarget ) {
			musicVolumeFadeInc = 0;
			if( callBack )
				callBack( KRAP_CB_FADE, musicVolume );
		}
	}

}

static void setTempo( int tempo )
{
	player.tempo = tempo;
	kramSetSoundTimerBPM( 24 * tempo );
}

void krapStop()
{
	if( player.status == STATUS_STOP )
		return;

	player.status = STATUS_STOP;
	kramSetSoundTimer( 0 );

	int i;
	for( i = 0; i < player.module->channels; i++ ) {
		stopChannel( &player.chns[ i ] );
	}
}

void krapPause( int sfx )
{
	if( player.status != STATUS_PLAY )
		return;

	player.status = STATUS_PAUSE;
	kramPauseChannels( sfx, KRAM_PAUSE_SLOT1 + ( player.mode & KRAP_MODE_JINGLE ? 1 : 0 ) );
	kramSetSoundTimer( 0 );
}

void krapUnpause()
{
	if( player.status != STATUS_PAUSE )
		return;

	player.status = STATUS_PLAY;
	kramUnpauseChannels( KRAM_PAUSE_SLOT1 + ( player.mode & KRAP_MODE_JINGLE ? 1 : 0 ) );
	kramSetSoundTimer( timerRoutine );
}

int krapIsPaused()
{
	return player.status == STATUS_PAUSE;
}

void krapCallback( void (*func)( int, int ) )
{
	callBack = func;
}


void krapSetMusicVol( uint vol, int fade )
{
	if( fade && player.status == STATUS_PLAY ) {
		if( vol != musicVolume ) {
			musicVolumeTarget = vol;
			musicVolumeFadeInc = ( musicVolumeTarget < musicVolume ) ? -1 : 1;
		}
	}
	else {
		musicVolumeFadeInc = 0;
		musicVolume = musicVolumeTarget =  vol;
		setGlobalVol();
	}
}

uint krapGetMusicVol()
{
	return musicVolumeTarget;
}

void krapSetChannelVol( uint channel, uint vol )
{
	if( channel >= MCHANNELAMOUNT )
		return;

	C_CLIP( vol, 64 );
	player.chns[ channel ].cvolume = vol;
}

uint krapGetChannelVol( uint channel )
{
	if( channel >= MCHANNELAMOUNT )
		return 0;

	return player.chns[ channel ].cvolume;
}


void krapPlay( const Module *m, int mode, int song )
{
	// stop any paused channels
	kramStopChannels( KRAM_PAUSE_SLOT1 );
	kramStopChannels( KRAM_PAUSE_SLOT2 );

	if( mode & KRAP_MODE_JINGLE ) {
		if( player.status == STATUS_STOP ) {
			mode &= ~KRAP_MODE_JINGLE;
		}
		else {
			if( !( player.mode & KRAP_MODE_JINGLE ) ) {
				kramPauseChannels( 0, KRAM_PAUSE_SLOT3 );
				playerBackup = player;
			}
			else {
				krapStop();
			}
		}
	}
	else {
		// if we're in jingle-mode then stop paused channels of the interrupted song
		if( player.mode & KRAP_MODE_JINGLE ) {
			kramStopChannels( KRAM_PAUSE_SLOT3 );
		}
		krapStop();
	}

	player.mode = mode;
	player.module = m;
	player.tick = player.speed = player.module->initSpeed;

	player.volume = 64;
	player.volSlide = 0;
	krapSetMusicVol( musicVolume, 0 );
	setTempo( player.module->initBPM );

	if( song >= 64 )
		song = 0;

	player.order = player.songStartOrder = player.module->songIndex[ song ];
	if( !( player.mode & KRAP_MODE_SONG ) ) {
		player.songStartOrder = player.module->songRestart;
	}

	player.row = 0;
	pDataSet();

	player.pGotoOrder = player.pGotoRow = -1;
	player.pLoopOrder = player.pLoopCount = player.pLoopActive = 0;

	player.instrumentBased = player.module->flagInstrumentBased;
	player.fastVolSlides = player.module->flagVolSlides;

	if( player.instrumentBased ) {
		player.vibrOfs = TABLE_VIBRATO_LENGTH >> 1;
	}
	else {
		player.vibrOfs = 0;
	}

	if( player.module->flagLinearSlides ) {
		player.calcPeriod = calcPeriodLinear;
		player.calcFinalFreq = calcFinalFreqLinear;
	}
	else {
		player.calcPeriod = calcPeriodAmiga;
		player.calcFinalFreq = calcFinalFreqAmiga;
	}

	bzero( ( void * )player.chns, sizeof( player.chns ) );

	int i;
	for( i = 0; i < player.module->channels; i++ ) {
		player.chns[ i ].vibrpos = player.vibrOfs;
		player.chns[ i ].vibrretrig = 1;
		player.chns[ i ].vibrtable = table_sine;
		player.chns[ i ].tremretrig = 1;
		player.chns[ i ].tremtable = table_sine;
		player.chns[ i ].panbretrig = 0;
		player.chns[ i ].panbtable = table_sine;
		player.chns[ i ].sample = 0;
		player.chns[ i ].instrument = 0;
		player.chns[ i ].panning = player.module->channelPan[ i ];
		player.chns[ i ].i_vol = 64;
		player.chns[ i ].i_pan = 0;
		player.chns[ i ].cvolume = 64;
	}

	player.status = STATUS_PLAY;
	kramSetSoundTimer( timerRoutine );
}


static void jingleDone()
{
	krapStop();
	player = playerBackup;
	setTempo( player.tempo );
	krapSetMusicVol( musicVolume, 0 );
	if( player.status == STATUS_PLAY ) {
		kramSetSoundTimer( timerRoutine );
		kramUnpauseChannels( KRAM_PAUSE_SLOT3 );
	}
	if( callBack )
		callBack( KRAP_CB_JDONE, 0 );
}

void krapStopJingle()
{
	if( player.mode & KRAP_MODE_JINGLE ) {
		jingleDone();
	}
}

//==============================================================
// instrument-sfx-stuff
//==============================================================

static u32 iidCounter IWRAM;
static MChannel effchns[ 6 ] EWRAM;
#define EFFCHNSAMOUNT (sizeof(effchns)/sizeof(effchns[0]))


ihandle krapInstPlay( const Instrument *ins, int note, ihandle old )
{
	MChannel* c = &effchns[ old.channel ];

	if( c->id.val != old.val ) {
		c = &effchns[ 0 ];
		int i = 0;

		// don't check c->i_active because it is not set
		// for instruments that do not have an envelope!
		// always check kramActive( c->handle )
		while( kramActive( c->handle ) && i < EFFCHNSAMOUNT ) {
			i++; c++;
		}

		if( i >= EFFCHNSAMOUNT )
			return ( ihandle ){ { .val = 0 } };

		old.channel = i;
	}

	old.id = ++iidCounter;
	c->id = old;

	c->instrument = ins;
	c->sample = samples[ ins->samples[ note ] ];
	c->volume = c->volumec = c->sample->volDefault;
	c->panning = c->panningc = c->sample->panDefault;
	c->period = c->periodc = calcPeriodLinear( note, c->sample );

	if( i_init( c ) )
		c->handle =	kramPlayExt( c->sample, 1, c->handle, calcFinalFreqLinear( c->period ),	0, c->panning );
	else
		c->handle =	kramPlayExt( c->sample, 1, c->handle, calcFinalFreqLinear( c->period ),	c->volume, c->panning );

	if( !c->handle.val ) {
		c->id.val = 0;
		return ( ihandle ){ { .val = 0 } };
	}

	return old;
}

#define VALIDATECONTROL \
	MChannel *c = &effchns[ i.channel ];	\
	if( c->id.val != i.val ) \
		return 0;

int krapInstRelease( ihandle i )
{
	VALIDATECONTROL;
	i_release( c );
	return 1;
}

int krapInstStop( ihandle i )
{
	VALIDATECONTROL;
	stopChannel( c );
	return 1;
}

void krapInstProcess()
{
	processInstruments( &effchns[ 0 ], EFFCHNSAMOUNT );
}

int krapInstHandleValid( ihandle i )
{
	VALIDATECONTROL;
	return kramHandleValid( c->handle );
}

void krapInit()
{
	MChannel *c = &effchns[ 0 ];

	int i;
	for( i = EFFCHNSAMOUNT; i; i--, c++ ) {
		c->vibrtable = table_sine;
		c->cvolume = 64;
	}

}



#include "player_effects.h"

static const effectStruct effects[] = {
	{ 0, 				0,				0 },
	{ eff_speed,		0,				0 },	// EFF_SPEED
	{ eff_bpm,			0,				0 },	// EFF_BPM
	{ eff_speedbpm,		0,				0 },	// EFF_SPEEDBPM
	{ eff_patt_jump,	0,				0 },	// EFF_PATTERN_JUMP
	{ eff_patt_break,	0,				0 },	// EFF_PATTERN_BREAK				5
	{ eff_volslide_s3m,	0,				1 },	// EFF_VOLSLIDE_S3M
	{ eff_volslide_xm,	0,				1 },	// EFF_VOLSLIDE_XM
	{ eff_volslide_df,	0,				0 },	// EFF_VOLSLIDE_DOWN_XM_FINE
	{ eff_volslide_uf,	0,				0 },	// EFF_VOLSLIDE_UP_XM_FINE
	{ eff_portadown_xm,	0,				1 },	// EFF_PORTA_DOWN_XM				10
	{ eff_portadown_s3m,0,				1 },	// EFF_PORTA_DOWN_S3M
	{ eff_portadown_f,	0,				0 },	// EFF_PORTA_DOWN_XM_FINE
	{ eff_portadown_ef,	0,				0 },	// EFF_PORTA_DOWN_XM_EFINE
	{ eff_portaup_xm,	0,				1 },	// EFF_PORTA_UP_XM
	{ eff_portaup_s3m,	0,				1 },	// EFF_PORTA_UP_S3M					15
	{ eff_portaup_f,	0,				0 },	// EFF_PORTA_UP_XM_FINE
	{ eff_portaup_ef,	0,				0 },	// EFF_PORTA_UP_XM_EFINE
	{ eff_volume,		0,				0 },	// EFF_VOLUME
	{ eff_portanote,	0,				1 },	// EFF_PORTA_NOTE
	{ eff_vibrato,		0,				1 },	// EFF_VIBRATO						20
	{ eff_tremor,		0,				1 },	// EFF_TREMOR
	{ eff_arpeggio,		0,				1 },	// EFF_ARPEGGIO
	{ eff_volslide_s3m,	eff_vibrato,	1 },	// EFF_VOLSLIDE_VIBRATO
	{ eff_volslide_s3m,	eff_portanote,	1 },	// EFF_VOLSLIDE_PORTA
	{ eff_cvolume,		0,				0 },	// EFF_CHANNEL_VOL					25
	{ eff_cvolslide,	0,				1 },	// EFF_CHANNEL_VOLSLIDE
	{ eff_offset,		0,				0 },	// EFF_OFFSET
	{ eff_panslide,		0,				1 },	// EFF_PANSLIDE
	{ eff_retrig,		0,				1 },	// EFF_RETRIG
	{ eff_tremolo,		0,				1 },	// EFF_TREMOLO						30
	{ eff_fvibrato,		0,				1 },	// EFF_FVIBRATO
	{ eff_gvolume,		0,				0 },	// EFF_GLOBAL_VOL
	{ eff_gvolslide,	0,				1 }, 	// EFF_GLOBAL_VOLSLIDE
	{ eff_pan,			0,				0 },	// EFF_PAN
	{ eff_panbrello,	0,				1 }, 	// EFF_PANBRELLO					35
	{ eff_mark,			0,				0 }, 	// EFF_MARK
	{ eff_glissando,	0,				0 },	// EFF_GLISSANDO
	{ eff_wave_vibr,	0,				0 },	// EFF_WAVE_VIBR
	{ eff_wave_trem,	0,				0 },	// EFF_WAVE_TREMOLO
	{ eff_wave_panb,	0,				0 },	// EFF_WAVE_PANBRELLO				40
	{ 0,				0,				0 },	// EFF_PATTERN_DELAYF			(!)
	{ 0,				0,				0 },	// EFF_OLD_PAN					(!) converted to EFF_PAN
	{ eff_patternloop,	0,				0 },	// EFF_PATTERN_LOOP
	{ eff_notecut,		0,				1 },	// EFF_NOTE_CUT
	{ eff_notedelay,	0,				1 },	// EFF_NOTE_DELAY					45
	{ 0,				0,				0 },	// EFF_PATTERN_DELAY			(*)
	{ 0,				0,				0 },	// EFF_ENV_SETPOS
	{ 0,				0,				0 },	// EFF_OFFSET_HIGH
	{ eff_volslide_xm,	eff_vibrato,	1 },	// EFF_VOLSLIDE_VIBRATO_XM
	{ eff_volslide_xm,	eff_portanote,	1 },	// EFF_VOLSLIDE_PORTA_XM			50
};


static const effectStructVC effectsVC[] = {
	{ eff_VC_volslide_down,		1 },	// $60-$6f   Volume slide down
	{ eff_VC_volslide_up,		1 },	// $70-$7f   Volume slide up
	{ eff_VC_fvolslide_down,	0 },	// $80-$8f   Fine volume slide down
	{ eff_VC_fvolslide_up,		0 },	// $90-$9f   Fine volume slide up
	{ eff_VC_vibrato_setspeed,	0 },	// $a0-$af   Set vibrato speed
	{ eff_VC_vibrato,			1 },	// $b0-$bf   Vibrato
	{ eff_VC_pan,				0 },	// $c0-$cf   Set panning
	{ eff_VC_panslide_left,		1 },	// $d0-$df   Panning slide left
	{ eff_VC_panslide_right,	1 },	// $e0-$ef   Panning slide right
	{ eff_VC_portanote,			1 }		// $f0-$ff   Tone porta
};

