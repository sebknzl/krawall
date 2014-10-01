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
#include <cstring>
using namespace std;

#include "Exception.h"
#include "ModS3M.h"
#include "Pattern.h"
#include "Sample.h"
#include "Instrument.h"
#include "effects.h"

ModS3M::ModS3M( string filename, int globalVolume ) : Mod( filename, globalVolume )
{
	dataBackup = data;

	flagInstrumentBased = false;
	songRestart = 0;
	flagLinearSlides = false;

	nameSong = ( char * )data;
	cout << "\"" << nameSong << "\"" << endl;

	numOrders = MU16( data, 0x20 );
	numSamples = MU16( data, 0x22 );
	numPatterns = MU16( data, 0x24 );

	u16 flags = MU16( data, 0x26 );
	u16 cwt = MU16( data, 0x28 );
	u16 pfi = MU16( data, 0x2a );

	if( flags & 8 )
		flagVolOpt = true;
	else
		flagVolOpt = false;

	if( flags & 16 ) {
		cerr << "\tWarning: Ignoring Amiga limits" << endl;
	}

	if( flags & 64 || cwt == 0x1300 ) {
		flagFastVolSlides = true;
	}
	else {
		flagFastVolSlides = false;
	}

	if( pfi == 1 )
		flagSignedSamples = true;
	else
		flagSignedSamples = false;


	if( !volGlobal )
		volGlobal = MU8( data, 0x30 ) * 2;
	initialSpeed = MU8( data, 0x31 );
	initialBPM = MU8( data, 0x32 );

	bool flagPanSettings;

	if( MU8( data, 0x35 ) == 252 )
		flagPanSettings = true;
	else
		flagPanSettings = false;

	memcpy( channelSettings, ( char * )( data + 0x40 ), 32 );

	numChannels = 0;

	int i;
	for( i = 0; i < 32; i++ ) {
		if( channelSettings[ i ] < 8 ) {
			channelSettings[ i ] = 1;
			numChannels++;
		}
		else if( channelSettings[ i ] >= 8 && channelSettings[ i ] < 16 ) {
			channelSettings[ i ] = 2;
			numChannels++;
		}
		else {
			channelSettings[ i ] = 0;
		}
	}

	numChannels = 32;

	memcpy( order, ( char * )( data + 0x60 ), numOrders );
	data += 0x60 + numOrders;

	for( i = 0; i < numOrders; i++ ) {
		if( order[ i ] == 0xff ) {
			numOrders = i;
			break;
		}
	}

	u16 *insPtr = ( u16 * )data;
	u16 *patPtr = insPtr + numSamples;

	data += 2*numSamples + 2*numPatterns;

	if( flagPanSettings ) {
		memcpy( channelPan, data, 32 );
	}
	else {
		memset( channelPan, 0, sizeof( channelPan ) );
	}

	for( i = 0; i < 32; i++ ) {
		if( channelPan[ i ] & ( 1 << 5 ) ) {
			channelPan[ i ] &= 0xf;
		}
		else {
			//channelPan[ i ] = ( channelSettings[ i ] == 1 ) ? 0x3 : 0xc;
			channelPan[ i ] = ( channelSettings[ i ] == 1 ) ? 0x0 : 0xf;
		}

		if( channelPan[ i ] == 7 || channelPan[ i ] == 8 )
			channelPan[ i ] = 0;
		else
			channelPan[ i ] = channelPan[ i ] * 128 / 15 - 64;
	}

	sampleOffsetStart = Sample::currentOffset;
	for( i = 0; i < numSamples; i++ ) {
		data = dataBackup + 16*insPtr[ i ];
		readS3MSample( i );
	}

	for( i = 0; i < numPatterns; i++ ) {
		data = dataBackup + 16*patPtr[ i ];
		readS3MPattern( i );
	}

	Sample::currentOffset = Sample::samples.size();
}

ModS3M::~ModS3M() {}

static u8 eff_conv[] = {
	0,
	EFF_SPEED,
	EFF_PATTERN_JUMP,
	EFF_PATTERN_BREAK,
	EFF_VOLSLIDE_S3M,
	EFF_PORTA_DOWN_S3M,
	EFF_PORTA_UP_S3M,
	EFF_PORTA_NOTE,
	EFF_VIBRATO,
	EFF_TREMOR,
	EFF_ARPEGGIO,
	EFF_VOLSLIDE_VIBRATO,
	EFF_VOLSLIDE_PORTA,
	EFF_CHANNEL_VOL,
	EFF_CHANNEL_VOLSLIDE,
	EFF_OFFSET,
	EFF_PANSLIDE,
	EFF_RETRIG,
	EFF_TREMOLO,
	0,	// special
	EFF_BPM,
	EFF_FVIBRATO,
	EFF_GLOBAL_VOL,
	EFF_GLOBAL_VOLSLIDE,
	EFF_PAN,
	EFF_PANBRELLO,
	EFF_MARK
};
#define EFF_CONV_SIZE (sizeof(eff_conv)/sizeof(eff_conv[0]))

#define EFF_SPECIAL 19

static u8 eff_conv_special[] = {
	0,
	EFF_GLISSANDO,
	0,
	EFF_WAVE_VIBR,
	EFF_WAVE_TREMOLO,
	EFF_WAVE_PANBRELLO,
	0,	// mp: fine pattern delay
	0,
	EFF_OLD_PAN,
	0,	// mp: sound control
	EFF_OFFSET_HIGH,
	EFF_PATTERN_LOOP,
	EFF_NOTE_CUT,
	EFF_NOTE_DELAY,
	EFF_PATTERN_DELAY
};
#define EFF_CONV_SPECIAL_SIZE (sizeof(eff_conv_special)/sizeof(eff_conv_special[0]))


void ModS3M::readS3MPattern( int ref )
{
	Pattern &p = *( new Pattern( 64, numChannels ) );
	p.referencedAs = ref;
	patterns.push_back( &p );

	data += 2; // skip length

	u8 *bp = data;
	u8 row = 0;

	while( row < 64 ) {
		if( !*bp ) {
			bp++;
			row++;
			continue;
		}

		u8 col = *bp & 31;
		u8 follow = *bp++;

		Pattern::atom &a = p( row, col );

		if( follow & 32 ) {
			a.note = *bp++;

			switch( a.note ) {
			case 255: // leere note
				a.note = 0;
				break;
			case 254: // note-off
				a.note = 127;
				break;
			default:
				u8 note = a.note;
				a.note = ( ( ( note ) >> 4 ) * 12 + ( ( note ) & 0xf ) ) + 1;
				break;
			}

			a.instrument = *bp++;

			if( a.instrument ) {
				a.instrument += Sample::currentOffset;
			}
		}
		if( follow & 64 ) {
			a.volume = *bp++;
			if( a.volume & 0x80 ) {	// pan
				a.volume &= 0x7f;
				a.volume = a.volume * 15 / 64;
				if( a.volume == 8 )
					a.volume = 7;
				a.volume += 0xc0;
			}
			else {
				a.volume += 0x10;
			}
		}
		if( follow & 128 ) {
			a.effect = *bp++;
			a.effectop = *bp++;

			bool warn = false;

			u8 eff = a.effect;
			if( a.effect == EFF_SPECIAL ) {
				if( ( a.effectop >> 4 ) >= EFF_CONV_SPECIAL_SIZE ) {
					a.effect = 0;
					a.effectop = 0;
					warn = true;
				}
				else {
					a.effect = eff_conv_special[ a.effectop >> 4 ];
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
					a.effect = eff_conv[ a.effect ];
					if( !a.effect )
						warn = true;
				}
			}
			// convert old S8-pan to X-pan
			if( a.effect == EFF_OLD_PAN ) {
				a.effect = EFF_PAN;
				a.effectop = a.effectop * 128 / 15;
			}

			if( a.effect == EFF_SPEED && !a.effectop ) {
				a.effect = 0;
			}

/*
			if( ( a.effect == EFF_VOLSLIDE_S3M ) && flagFastVolSlides ) {
				a.effect = EFF_VOLSLIDE_S3M_FAST;
			}
*/
			if( warn ) {
				cerr << "\tInvalid/unimplemented effect in (" << ref << "," << ( int )row << "," << ( int )col << ")!" << endl;
			}
		}
	}
}



void ModS3M::readS3MSample( int ref )
{
	Sample *s = new Sample();
	s->referencedAs = Sample::currentOffset + ref;

	s->name = ( char * )( data + 0x30 );

	if( MU8( data, 0 ) != 1 ) {
		s->empty = true;
		return;
	}

	u8 flags = MU8( data, 0x1f );

	if( flags & 2 || flags & 4 ) {
		cerr << "\t#" << s->numReferenced << " \"" << s->name << "\": ignoring because either stereo or 16bit" << endl;
		s->empty = true;
		return;
	}

	s->empty = false;

	s->fileName = ( char * )( data + 1 );
	if( s->fileName.length() > 12 )
		s->fileName.resize( 12 );

	u32 sampPos = 0;

	sampPos = MU32( data, 0xd );
	sampPos &= 0xffffff;
	sampPos = ( sampPos >> 8 ) | ( ( sampPos & 0xff ) << 16 );
	sampPos <<= 4;

	s->length = MU32( data, 0x10 );
	u32 loopEnd = MU32( data, 0x18 );
	s->loopBegin = MU32( data, 0x14 );
	s->loopLength = loopEnd - s->loopBegin;

	s->volDefault = MU8( data, 0x1c );

	if( !s->length ) {
		s->empty = true;
		return;
	}

	if( flags & 1 ) {
		s->loop = 1;
	}
	else {
		s->loop = 0;
	}
	if( ( s->loopBegin >= s->length ) ||
		( s->loopLength == 0 ) ||
		( s->loopBegin + s->loopLength > s->length ) ) {
		s->loop = 0;
	}

	if( !s->loop ) {
		s->loopBegin = 0;
		s->loopLength = 0;
	}

	s->freq = MU32( data, 0x20 );
	s->data = new u8[ s->length ];
	memcpy( s->data, ( char * )( dataBackup + sampPos ), s->length );

	if( flagSignedSamples ) {
		for( int i = 0; i < s->length; i++ ) {
			s->data[ i ] = ( s->data[ i ] + 128 );
		}
	}
}


bool ModS3M::canHandle( string filename )
{
	ifstream f( filename.c_str(), ios::in | ios::binary );

	if( f.bad() )
		return false;

	f.seekg( 0x1c, ios::beg );
	char sign;
	f.read( &sign, 1 );

	if( sign == 0x1a )
		return true;
	else
		return false;

}
