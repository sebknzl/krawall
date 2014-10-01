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
#include <cmath>
#include <cstring>
#include "Sample.h"
#include "Instrument.h"
#include "Mod.h"
#include "Exception.h"

deque<Sample *> Sample::samples;
int Sample::currentOffset;

//extern bool debug;


Sample::Sample() :
	data( 0 ), empty( true ), numReferenced( 0 ), referencedAs( 0 ),
	finetune( 0 ), freq( 0 ), relativeNote( 0 ),
	length( 0 ), loopBegin( 0 ), loopLength( 0 ),
	volDefault( 0 ), panDefault( 0 ), loop( 0 )
{
	samples.push_back( this );
}

Sample::~Sample() {
	if( data )
		delete [] data;
}

bool Sample::operator==( const Sample & p ) {
	if( ( length == p.length ) &&
		( loopBegin == p.loopBegin ) &&
		( loopLength == p.loopLength ) &&
//		( finetune == p.finetune ) &&
		( relativeNote == p.relativeNote ) &&
//		( freq == p.freq ) &&
		( abs( finetune - p.finetune ) <= 4 ) &&
		( volDefault == p.volDefault ) &&
		( loop == p.loop ) ) {
		for( int i = length - 1; i; i-- ) {
			if( data[ i ] != p.data[ i ] )
				return false;
		}
		return true;
	}
	return false;
}

void Sample::extendLoop()
{
	#define LOOPMINSIZE 512

	if( !loop )
		return;

	if( loopLength >= LOOPMINSIZE )
		return;

	u32 oldlength = loopLength;

	u8* ndata = new u8[ length + LOOPMINSIZE*2 ];
	memcpy( ndata, data, loopBegin );
	u8* cp = ndata + loopBegin;
	length -= loopLength;

	u32 ll = loopLength;
	loopLength = 0;

	do {
		memcpy( cp, data + loopBegin, ll );
		cp += ll;
		loopLength += ll;
		length += ll;
	} while( loopLength < LOOPMINSIZE );


	delete [] data;
	data = ndata;

	cout	<< "\t#" << ( int )referencedAs << " \"" << name << "\": "
			<< "extended loop from " << ( int )oldlength << " to " << ( int )loopLength << endl;
}

void Sample::cutLoop()
{
	u32 loopEnd = loopBegin + loopLength;
	if( loop && ( loopEnd < length ) ) {
		cout	<< "\t#" << ( int )referencedAs << " \"" << name << "\": "
				<< "cut beyond loopend (" << ( int )( length - loopEnd ) << " bytes)" << endl;
		length = loopEnd;
	}
}


void Sample::replaceSample( int s, int by )
{

	Instrument::replaceSample( s, by );

	for( int m = 0; m < Mod::modules.size(); m++ ) {
		if( !Mod::modules[ m ]->flagInstrumentBased ) {
			Mod::modules[ m ]->replaceInstrument( s, by );
		}
	}
}

void Sample::deleteSample( int s )
{
	delete samples[ s ];
	samples.erase( samples.begin() + s );

	Instrument::decreaseSample( s );

	for( int m = 0; m < Mod::modules.size(); m++ ) {
		if( !Mod::modules[ m ]->flagInstrumentBased ) {
			Mod::modules[ m ]->decreaseInstrument( s );
		}
	}

}


void Sample::optimize()
{
	cout << "Optimizing samples:" << endl;

	int i;
	for( i = 0; i < samples.size(); i++ ) {
		Sample &s = *samples[ i ];
		int fffffffff = samples.size();

		if( s.empty ) {
			//cerr << "\t#" << ( int )s.referencedAs << " \"" << s.name << "\": empty, removing" << endl;
			deleteSample( i );
			i--;
			continue;
		}

		if( !s.numReferenced ) {
			cerr << "\t#" << ( int )s.referencedAs << " \"" << s.name << "\": unused, removing" << endl;
			deleteSample( i );
			i--;
			continue;
		}
	}

	for( i = 0; i < samples.size(); i++ ) {
		samples[ i ]->calcFreq();
		samples[ i ]->cutLoop();
		samples[ i ]->extendLoop();
	}

	for( i = 0; i < samples.size(); i++ ) {
		for( int j = i + 1; j < samples.size(); j++ ) {
			if( *samples[ i ] == *samples[ j ] ) {
				cout	<< "\t#"
						<< ( int )samples[ i ]->referencedAs << " \"" << samples[ i ]->name << "\""
						<< " and #"
						<< ( int )samples[ j ]->referencedAs << " \"" << samples[ j ]->name << "\": removing duplicate"
						<< endl;
				replaceSample( j, i );
				deleteSample( j );
				j--;
			}
		}
	}

	cout << samples.size() << " sample(s)." << endl << endl;
}

void Sample::output()
{
	cout << "Saving samples: ";

	ofstream f( "samples.S", ios::out );
	if( f.bad() ) {
		throw new Exception( "can't open file for writing" );
	}

	f	<< ".global samples" << endl
		<< ".section .rodata" << endl << endl;

	int i;
	for( i = 0; i < samples.size(); i++ ) {
		Sample &s = *samples[ i ];

		int hq = ( ( s.fileName[ 0 ] == '~' ) ? 1 : 0 );
		f << ".align" << endl;
		f << "@ ======================================================================" << endl;
		f << "@ \"" << s.name << "\"" << endl;
		f << "Lsample" << i << ":" << endl;

		f << ".word "	<< ( int )s.loopLength << ", "
						//<< ( int )s.length << endl;
						<<  "Lsample" << i << "_end" << endl;
		f << ".word "	<< ( int )s.freq << endl;
		f << ".byte "	<< ( int )s.finetune << ", " << ( int )s.relativeNote << endl;


		f << ".byte "	<< ( int )s.volDefault << ", " << ( int )s.panDefault << ", "
						<< ( int )s.loop << ", "
						<< ( int )hq << endl;

		f << ".byte ";
		int sz;
		for( sz = 0; sz < s.length; sz++ ) {
			f << ( int )s.data[ sz ];

			if( !( ( sz + 1 ) & 127 ) )
				f << endl << ".byte ";
			else {
				if( sz != (s.length-1) )
					f << ", ";
			}
		}

		// 17*4 cause the max inc in the mixer can be (rounded up) 17 and
		// we go 4 samples over the end in the worst case
		// +1 for interpolation

		f << endl << "Lsample" << i << "_end:" << endl;
		f << ".byte ";

#define SAMPLES_ADD (17*4+1)
		for( sz = 0; sz < SAMPLES_ADD ; sz++ ) { // soll <= sein
			switch( s.loop ) {
			case 0: f << ( int )128; break;
			case 1:	f << ( int )s.data[ s.loopBegin + sz ]; break;
			case 2: f << ( int )s.data[ s.length - 1 - sz ]; break;
			}

			if( sz < SAMPLES_ADD-1 )
				f << ", ";
		}


		f << endl << endl;

		if( hq )
			cout << "o" << flush;
		else
			cout << "." << flush;
	}
	cout << endl;

	// Samples
	f << ".align" << endl;
	f << "samples:" << endl << ".word ";
	for( i = 0; i < samples.size(); i++ ) {
		f << "Lsample" << i;
		if( !( ( i + 1 ) & 7 ) )
			f << endl << ".word ";
		else if( i < ( samples.size() - 1 ) )
				f << ", ";

	}
	f << endl << endl;

	f.close();

	f.open( "samples.h", ios::out );
	f	<< "#ifndef __SAMPLES_H__" << endl
		<< "#define __SAMPLES_H__" << endl << endl;
	for( i = 0; i < samples.size(); i++ ) {
		Sample &s = *samples[ i ];

		if( !s.name.length() )
			continue;

		bool ok = true;
		for( int c = 0; c < s.name.length(); c++ ) {
			if( !isalnum( s.name[ c ] ) &&
				s.name[ c ] != '_' )
				ok = false;
		}
		if( !ok )
			continue;

		string sname = s.name;
		for( int u = 0; u < sname.length(); u++ )
			sname[ u ] = toupper( sname[ u ] );
		f << "#define SAMPLE_" << sname << " " << i << endl;
	}

	f << endl << "#endif" << endl << endl;
	f.close();
}


void Sample::free()
{
	for( int s = 0; s < samples.size(); s++ ) {
		delete samples[ s ];
	}
	samples.clear();
}


inline int _round( double x ) {
	if( x >= 0 )
		return ( int )( x + .5 );
	else
		return ( int )( x - .5 );
}

void Sample::calcFreq()
{
	if( freq ) { // freq given, calc finetune/relative note
		findFreqApprox();
	}
	else { // freq not given
		// warum dividier i da finetune runter?
		// warum nit einfach relativeNote*128 und unten auch mal 128?
		// naja.
		int tune = relativeNote*64 + (finetune>>1);
		freq = _round( 8363 * pow( 2.0, ( double )tune / (12.0*64.0) ) );
	}
}


void Sample::findFreqApprox()
{
	if( freq == 8363 ) {
		finetune = 0;
		relativeNote = 0;
		return;
	}

	double ratio = freq/8363.0;
	int tune = _round(1536*log(ratio)/log(2));
	relativeNote = tune / 128;
	finetune = tune - relativeNote*128;

	// wtf, war i do bsoffn?
/*
	int tune = 0;
	double cfreq = 8363;
	double fact;
	int dir;
	double error = 10000000;
	int besttune = 0;

	if( freq >= 8363 ) {
		fact = pow( 2.0, 1.0/(12.0*32.0) );
		dir = 1;
	}
	else {
		fact = pow( 2.0, -1.0/(12.0*32.0) );
		dir = -1;
	}

	while( 1 ) {
		cfreq *= fact;

		double cerror = cfreq - ( double )freq;

		if( cerror < 0 )
			cerror = -cerror;

		if( cerror < error ) {
			error = cerror;
			besttune = tune;
		}

		if( cerror > error ) {
			break;
		}

		tune += dir;
	}


	relativeNote = tune / 32;
	finetune = ( tune % 32 ) * 4;
	*/
}


