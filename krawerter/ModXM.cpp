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

#include <iostream>
#include <fstream>
using namespace std;

#include "Exception.h"
#include "ModXM.h"
#include "Pattern.h"
#include "Sample.h"
#include "Instrument.h"

#include "effects.h"

static u8 eff_conv[] = {
	EFF_ARPEGGIO,
	EFF_PORTA_UP_XM,
	EFF_PORTA_DOWN_XM,
	EFF_PORTA_NOTE,
	EFF_VIBRATO,
	EFF_VOLSLIDE_PORTA,
	EFF_VOLSLIDE_VIBRATO,
	EFF_TREMOLO,
	EFF_PAN,
	EFF_OFFSET,
	EFF_VOLSLIDE_XM,
	EFF_PATTERN_JUMP,
	EFF_VOLUME,
	EFF_PATTERN_BREAK,
	0,		// special1
	EFF_SPEEDBPM,
	EFF_GLOBAL_VOL,
	EFF_GLOBAL_VOLSLIDE,
EFF_CHANNEL_VOL,
EFF_CHANNEL_VOLSLIDE,
	0,		// MP: key off
	EFF_ENV_SETPOS,
	0,
	0,
	0,
	EFF_PANSLIDE,
	0,
	EFF_RETRIG,
	0,
	EFF_TREMOR,
	0,
	0,
	0,
	0,		// special2
	EFF_PANBRELLO,
	EFF_MARK
};
#define EFF_CONV_SIZE (sizeof(eff_conv)/sizeof(eff_conv[0]))

#define EFF_SPECIAL1 14
#define EFF_SPECIAL2 33

static u8 eff_conv_special1[] = {
	0,
	EFF_PORTA_UP_XM_FINE,
	EFF_PORTA_DOWN_XM_FINE,
	EFF_GLISSANDO,
	EFF_WAVE_VIBR,
	0,
	EFF_PATTERN_LOOP,
	EFF_WAVE_TREMOLO,
	0, //EFF_OLD_PAN,
	EFF_RETRIG,
	EFF_VOLSLIDE_UP_XM_FINE,
	EFF_VOLSLIDE_DOWN_XM_FINE,
	EFF_NOTE_CUT,
	EFF_NOTE_DELAY,
	EFF_PATTERN_DELAY,
};
#define EFF_CONV_SPECIAL1_SIZE (sizeof(eff_conv_special1)/sizeof(eff_conv_special1[0]))

static u8 eff_conv_special2[] = {
	0,
	EFF_PORTA_UP_XM_EFINE,
	EFF_PORTA_DOWN_XM_EFINE,
	0,
	0,
	EFF_WAVE_PANBRELLO,
	0,	// MP: fine pattern delay
	0,
	0,
	0,	// MP: sound control
	EFF_OFFSET_HIGH
};
#define EFF_CONV_SPECIAL2_SIZE (sizeof(eff_conv_special2)/sizeof(eff_conv_special2[0]))



static s8 round16to8( s16 x )
{
	float m = x;
	m /= 256;
	if( m >= 0 ) {
		m += .5;
	}
	else {
		m -= .5;
	}

	x = ( s16 )m;

	if( x > 127 )
		x = 127;
	if( x < -128 )
		x = -128;

	return ( s8 )x;
}

ModXM::ModXM( string filename, int globalVolume ) : Mod( filename, globalVolume )
{

	nameSong = ( char * )data + 17;
	if( nameSong.length() > 20 )
		nameSong.resize( 20 );

	cout << "\"" << nameSong << "\"" << endl;

	flagInstrumentBased = true;
	memset( channelPan, 0, sizeof( channelPan ) );

//	printf( "%x", MU16( data, 58 ) );
// warn version

	u32 headerSize = MU32( data, 60 );
	numOrders = MU16( data, 64 );
	songRestart = MU16( data, 66 );

	numChannels = MU16( data, 68 );

	memset( channelSettings, 0, sizeof( channelSettings ) );
	memset( channelSettings, 1, sizeof( channelSettings[ 0 ] ) * numChannels );

	if( numChannels > 32 )
		throw new Exception( "32 channels max!" );

	numPatterns = MU16( data, 70 );
	numInstruments = MU16( data, 72 );

	flagLinearSlides = MU16( data, 74 ) & 1;

	initialSpeed = MU16( data, 76 );
	initialBPM = MU16( data, 78 );

	memcpy( order, &data[ 80 ], 256 );


	u8* pattern = data + 60 + headerSize;

	for( int i = 0; i < numPatterns; i++ ) {
		headerSize = MU32( pattern, 0 );
		u16 rows = MU16( pattern, 5 );
		u16 size = MU16( pattern, 7 );

		pattern += headerSize;
		u8* decomp = pattern;
		pattern += size;

		Pattern *p = new Pattern( rows, numChannels );
		patterns.push_back( p );
		p->referencedAs = i;

		if( !size )
			continue;

		int channel = 0, row = 0;

		while( row < rows ) {
			Pattern::atom &a = (*p)( row, channel );

			if( *decomp & 128 ) {
				u8 follow = *decomp++;

				if( follow & 1 ) {
					a.note = *decomp++;
					if( a.note == 97 )
						a.note = 127;
				}
				if( follow & 2 )
					a.instrument = *decomp++ + Instrument::currentOffset;
				if( follow & 4 )
					a.volume = *decomp++;
				if( follow & 8 )
					a.effect = *decomp++;
				if( follow & 16 )
					a.effectop = *decomp++;
			}
			else {
				a.note = *decomp++;
				a.instrument = *decomp++ + Instrument::currentOffset;
				a.volume = *decomp++;
				a.effect = *decomp++;
				a.effectop = *decomp++;
			}

			bool warn = false;
			u8 eff = a.effect;

			if( a.effect == EFF_SPECIAL1 ) {
				if( ( a.effectop >> 4 ) >= EFF_CONV_SPECIAL1_SIZE ) {
					a.effect = 0;
					a.effectop = 0;
					warn = true;
				}
				else {
					a.effect = eff_conv_special1[ a.effectop >> 4 ];
					a.effectop &= 0xf;
					if( !a.effect )
						warn = true;
				}
			}
			else if( a.effect == EFF_SPECIAL2 ) {
				if( ( a.effectop >> 4 ) >= EFF_CONV_SPECIAL2_SIZE ) {
					a.effect = 0;
					a.effectop = 0;
					warn = true;
				}
				else {
					a.effect = eff_conv_special2[ a.effectop >> 4 ];
					a.effectop &= 0xf;
					if( !a.effect )
						warn = true;
				}
			}
			else {
				if( a.effect >= EFF_CONV_SIZE ) {
					a.effect = 0;
					a.effectop = 0;
					warn = true;
				}
				else {
					if( a.effect || ( !a.effect && a.effectop ) ) {	// sonderfall: arpeggio
						a.effect = eff_conv[ a.effect ];
						if( !a.effect )
							warn = true;
					}
				}
			}

			if( a.effect && !flagLinearSlides ) { // nasty: porta up mit porta down vertauschen fuer linear/amiga
				switch( a.effect ) {
				case EFF_PORTA_UP_XM:
					a.effect = EFF_PORTA_DOWN_XM;
					break;
				case EFF_PORTA_UP_XM_FINE:
					a.effect = EFF_PORTA_DOWN_XM_FINE;
					break;
				case EFF_PORTA_UP_XM_EFINE:
					a.effect = EFF_PORTA_DOWN_XM_EFINE;
					break;
				case EFF_PORTA_DOWN_XM:
					a.effect = EFF_PORTA_UP_XM;
					break;
				case EFF_PORTA_DOWN_XM_FINE:
					a.effect = EFF_PORTA_UP_XM_FINE;
					break;
				case EFF_PORTA_DOWN_XM_EFINE:
					a.effect = EFF_PORTA_UP_XM_EFINE;
					break;
				}
			}

			if( a.effect == EFF_PAN ) {
				a.effectop = ( ( int )a.effectop * 128 ) / 0xff;
			}

			if( a.effect == EFF_SPEEDBPM && !a.effectop ) {
				a.effect = 0;
			}


			if( warn ) {
				cerr << "\tInvalid/unimplemented effect in (" << i << "," << ( int )row << "," << ( int )channel << ")!" << endl;
			}

			if( ++channel >= numChannels ) {
				channel = 0;
				row++;
			}

		}
	}

	data = pattern;

	instrumentOffsetStart = Instrument::currentOffset;
	sampleOffsetStart = Sample::currentOffset;

	for( int insL = 0; insL < numInstruments; insL++ ) {

		Instrument *ins = new Instrument();
		ins->referencedAs = Instrument::currentOffset++;
		ins->numReferenced = 0;

		u32 insHeaderSize = *( u32 * )data;
		ins->name = ( char * )( data + 4 );
		if( ins->name.length() > 22 )
			ins->name.resize( 22 );

		int numSamples = MU16( data, 27 );

		if( !numSamples ) {
			ins->empty = true;
			data += insHeaderSize;
		}
		else {
			ins->empty = false;

			u32 smpHeaderSize = MU32( data, 29 );

			for( int i = 0; i < 96; i++ ) {
				ins->samples[ i ] = MU8( data, 33 + i );
			}

			memcpy( ins->envVol.nodes, data + 129, sizeof( ins->envVol.nodes ) );
			memcpy( ins->envPan.nodes, data + 177, sizeof( ins->envPan.nodes ) );
			ins->envVol.num = MU8( data, 225 );
			ins->envPan.num = MU8( data, 226 );
			ins->envVol.sus = MU8( data, 227 );
			ins->envVol.loopStart = MU8( data, 228 );
			ins->envVol.loopEnd = MU8( data, 229 );
			ins->envPan.sus = MU8( data, 230 );
			ins->envPan.loopStart = MU8( data, 231 );
			ins->envPan.loopEnd = MU8( data, 232 );
			ins->envVol.flags = MU8( data, 233 );
			ins->envPan.flags = MU8( data, 234 );

			ins->vibType = MU8( data, 235 );
			ins->vibSweep = MU8( data, 236 );
			ins->vibDepth = MU8( data, 237 );
			ins->vibRate = MU8( data, 238 );
			if( !ins->vibDepth || !ins->vibRate )
				ins->vibDepth = ins->vibRate = 0;
			ins->volFade = MU16( data, 239 ) ;

			data += insHeaderSize;

			int realSamples = 0;
			for( int j = 0; j < numSamples; j++ ) {
				if( !MU32( data, 0 ) ) { // empty/16b sample, don't load
					for( int i = 0; i < 96; i++ ) { // rewrite note->sample table
						if( ins->samples[ i ] > j )
							ins->samples[ i ]--;
					}
				}
				else {
					Sample *s = new Sample();
					this->numSamples++;
					s->length = MU32( data, 0 );
					s->empty = false;
					s->loopBegin = MU32( data, 4 );
					s->loopLength = MU32( data, 8 );
					s->volDefault = MU8( data, 12 );
					s->finetune = MS8( data, 13 );
					s->loop = MU8( data, 14 ) & 3;
					s->is16bit = ( MU8( data, 14 ) & ( 1 << 4 ) ) != 0;
					s->panDefault = ( int )( MU8( data, 15 ) - 128 ) / 2;
					s->relativeNote = MS8( data, 16 );
					s->name = ( char * )data + 18;
					if( s->name.length() > 22 )
						s->name.resize( 22 );
					s->fileName = s->name;
					s->referencedAs = Sample::currentOffset + realSamples++;
					s->numReferenced = 0;
				}

				data += smpHeaderSize;
			}


			if( realSamples ) { // are there any _real_ samples for this instrument?

				int msg = 0;
				for( int i = 0; i < 96; i++ ) {
					if( ins->samples[ i ] >= realSamples )  {
						switch( msg ) {
						case 0:
							cerr << "\t#" << ( int )ins->referencedAs << " \"" << ins->name << "\": invalid/empty sample-reference note " << i;
							msg++;
							break;
						case 1:
						case 2:
						case 3:
							cerr << "," << i;
							msg++;
							break;
						case 4:
							cerr << ",...";
							msg++;
							break;
						}
						ins->samples[ i ] = 0;
					}
					ins->samples[ i ] += Sample::currentOffset;
				}
				if( msg )
					cerr << endl;

				// load samples
				for( int j = 0; j < realSamples; j++ ) {
					Sample *s = Sample::samples[ Sample::currentOffset + j ];
					s->data = new u8[ s->length ];

					if( !s->is16bit ) {
						s8 val = 0;
						for( int i = 0; i < s->length; i++ ) {
							val += ( s8 )data[ i ];
							s->data[ i ] = ( u8 )( ( int )val + 128 );
						}
						data += s->length;
					}
					else {
						cerr << "\t#" << ( int )ins->referencedAs << " \"" << s->name << "\": 16bit, converting (ouch)" << endl;
						s16 val = 0;
						s->length >>= 1;
						s->loopBegin >>= 1;
						s->loopLength >>= 1;
						for( int i = 0; i < s->length; i++ ) {
							val += ( s16 )( ( s16 * )data )[ i ];
							s->data[ i ] = ( u8 )( ( int )round16to8( val ) + 128 );
						}
						data += (s->length<<1);
					}
				}

				Sample::currentOffset += realSamples;
				numSamples += realSamples;
			}
			else { // if( realSamples )
				ins->empty = true;
			}
		}


	}
}

ModXM::~ModXM() {}

bool ModXM::canHandle( string filename )
{
	ifstream f( filename.c_str(), ios::in | ios::binary );

	if( f.bad() )
		return false;

	f.seekg( 0x25, ios::beg );
	char sign;
	f.read( &sign, 1 );

	if( sign == 0x1a )
		return true;
	else
		return false;
}



