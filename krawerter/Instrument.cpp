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
#include <iomanip>
#include <fstream>
#include "Instrument.h"
#include "Sample.h"
#include "Mod.h"
#include "Exception.h"

int Instrument::currentOffset;
deque<Instrument *> Instrument::instruments;

using namespace std;

Instrument::Instrument() : referencedAs( 0 ), numReferenced( 0 ), empty( true )
{
	instruments.push_back( this );
}

void Instrument::addReference( u8 note )
{
	numReferenced++;
/*
	if( ( note < 96 ) && ( samples[ note ] < Sample::samples.size() ) ) {
		Sample::samples[ samples[ note ] ]->numReferenced++;
	}
*/
	for( int i = 0; i < 96; i++ )
		Sample::samples[ samples[ i ] ]->numReferenced++;
}


void Instrument::replaceSample( int s, int by )
{
	for( int i = 0; i < instruments.size(); i++ ) {
		Instrument &ins = *instruments[ i ];
		for( int n = 0; n < 96; n++ ) {
			if( ins.samples[ n ] == s ) {
				ins.samples[ n ] = by;
			}
		}
	}
}

void Instrument::decreaseSample( int from )
{
	for( int i = 0; i < instruments.size(); i++ ) {
		Instrument &ins = *instruments[ i ];
		for( int n = 0; n < 96; n++ ) {
			if( ins.samples[ n ] > from ) {
				ins.samples[ n ]--;
			}
		}
	}
}

void Instrument::deleteInstrument( int i )
{
	delete instruments[ i ];
	instruments.erase( instruments.begin() + i );

	for( int m = 0; m < Mod::modules.size(); m++ ) {
		if( Mod::modules[ m ]->flagInstrumentBased ) {
			Mod::modules[ m ]->decreaseInstrument( i );
		}
	}
}

static void recalcEnvelope( Instrument::Env &e )
{
	if( e.loopStart == e.loopEnd ) {
		e.flags &= 3;	// disable loop
	}

	for( int i = 0; i < (e.num-1); i++ ) {
		int diff = ( e.nodes[ i + 1 ].inc - e.nodes[ i ].inc ) << 8;
		int xdiff = ( e.nodes[ i + 1 ].coord - e.nodes[ i ].coord );
		if( !xdiff ) {
			for( int j = i; j < e.num; j++ ) {
				e.nodes[ j ] = e.nodes[ j + 1 ];
			}
			if( e.loopStart > i )
				e.loopStart--;
			if( e.loopEnd >= i )
				e.loopEnd--;
			e.num--;
			i--;
			continue;
		}
		diff = diff / xdiff;
		e.nodes[ i ].coord = ( e.nodes[ i ].inc << 9 ) | e.nodes[ i ].coord;
		e.nodes[ i ].inc = ( signed short )diff;
	}
	e.nodes[ e.num-1 ].coord = ( e.nodes[ e.num-1 ].inc << 9 ) | e.nodes[ e.num-1 ].coord;

	if( e.flags & 4 ) {	// looped?
		e.num = e.loopEnd + 1;
	}
}


void Instrument::recalcEnvelopes()
{
	recalcEnvelope( envVol );
	recalcEnvelope( envPan );
}


void Instrument::optimize()
{
	cout << "Optimizing instruments:" << endl;

	for( int i = 0; i < instruments.size(); i++ ) {
		Instrument &ins = *instruments[ i ];

		if( ins.empty ) {
			cerr << "\t#" << ( int )ins.referencedAs << " \"" << ins.name << "\": empty, removing" << endl;
			deleteInstrument( i );
			i--;
			continue;
		}

		if( !ins.numReferenced ) {
			cerr << "\t#" << ( int )ins.referencedAs << " \"" << ins.name << "\": unused, removing" << endl;
			deleteInstrument( i );
			i--;
			continue;
		}
	}

	for( int i = 0; i < instruments.size(); i++ ) {
		instruments[ i ]->recalcEnvelopes();
	}

	cout << instruments.size() << " instrument(s)." << endl << endl;
}


void Instrument::output()
{
	cout << "Saving instruments: ";

	ofstream f( "instruments.S", ios::out );
	if( f.bad() ) {
		throw new Exception( "can't open file for writing" );
	}

	f	<< ".global instruments" << endl
		<< ".section .rodata" << endl << endl;

	int i;
	for( i = 0; i < instruments.size(); i++ ) {
		Instrument &ins = *instruments[ i ];
		f << ".align" << endl;
		f << "@ ======================================================================" << endl;
		f << "@ \"" << ins.name << "\"" << endl;
		f << "Linstrument" << i << ":" << endl;

		// 	unsigned short	samples[ 96 ];
		f << "@ samplemap" << endl;
		f << ".short ";
		for( int n = 0; n < 95; n++ ) {
			f << ( int )ins.samples[ n ] << ", ";
		}
		f << ( int )ins.samples[ 95 ] << endl;

		int env;

		f << "@ vol-envelope" << endl;
		f << ".short ";
		for( env = 0; env < 11; env++ ) {
			f << ( int )ins.envVol.nodes[ env ].coord << ", " << ( int )ins.envVol.nodes[ env ].inc << ", ";
		}
		f << ( int )ins.envVol.nodes[ 11 ].coord << ", " << ( int )ins.envVol.nodes[ 11 ].inc << endl;
		f << ".byte  "	<< ( int )(ins.envVol.num-1) << ", " << ( int )ins.envVol.sus << ", " << ( int )ins.envVol.loopStart << ", "
						<< ( int )ins.envVol.flags << endl;

		f << "@ pan-envelope" << endl;
		f << ".short ";
		for( env = 0; env < 11; env++ ) {
			f << ( int )ins.envPan.nodes[ env ].coord << ", " << ( int )ins.envPan.nodes[ env ].inc << ", ";
		}
		f << ( int )ins.envPan.nodes[ 11 ].coord << ", " << ( int )ins.envPan.nodes[ 11 ].inc << endl;
		f << ".byte  "	<< ( int )(ins.envPan.num-1) << ", " << ( int )ins.envPan.sus << ", " << ( int )ins.envPan.loopStart << ", "
						<< ( int )ins.envPan.flags << endl;

		f << "@ volfade" << endl;
		f << ".short " << ins.volFade << endl;
		f << "@ vibrato-parms" << endl;
		f << ".byte  " << ( int )ins.vibType << ", " << ( int )ins.vibSweep << ", " << ( int )ins.vibDepth << ", " << ( int )ins.vibRate << endl << endl;


		cout << "." << flush;
	}
	cout << endl;

	f << ".align" << endl;
	f << "instruments:" << endl << ".word ";
	for( i = 0; i < instruments.size(); i++ ) {
		f << "Linstrument" << i;
		if( !( ( i + 1 ) & 7 ) )
			f << endl << ".word ";
		else if( i < ( instruments.size() - 1 ) )
				f << ", ";

	}
	f << endl << endl;

	f.close();

	f.open( "instruments.h", ios::out );
	f	<< "#ifndef __INSTRUMENTS_H__" << endl
		<< "#define __INSTRUMENTS_H__" << endl << endl;
	for( i = 0; i < instruments.size(); i++ ) {
		Instrument &ins = *instruments[ i ];

		if( !ins.name.length() )
			continue;


		bool ok = true;
		for( int c = 0; c < ins.name.length(); c++ ) {
			if( !isalnum( ins.name[ c ] ) &&
				ins.name[ c ] != '_' )
				ok = false;
		}
		if( !ok )
			continue;

		string iname = ins.name;
		for( int u = 0; u < iname.length(); u++ )
			iname[ u ] = toupper( iname[ u ] );
		f << "#define INSTRUMENT_" << iname << " " << i << endl;
	}

	f << endl << "#endif" << endl << endl;
	f.close();

	if( instruments.size() >= 511 ) {
		cout << "Warning! More than 511 instruments exist." << endl
			 << "If instruments above 511 are used by modules you will run into troubles." << endl;
	}
}

void Instrument::free()
{
	for( int i = 0; i < instruments.size(); i++ ) {
		delete instruments[ i ];
	}
	instruments.clear();
}