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

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

#include "Pattern.h"
#include "Exception.h"
#include "Mod.h"
#include "Instrument.h"
#include "Sample.h"

deque<Mod *> Mod::modules;
int Mod::counter;

Mod::Mod( string filename, int globalVolume ) :
	numSamples( 0 ), numInstruments( 0 ), sampleOffsetStart( 0 ), instrumentOffsetStart( 0 ),
	volGlobal( globalVolume )
{
	cout << "[" << filename << "]: ";

	if( !volGlobal )
		volGlobal = 128;
	_fdata = data = 0;
	numPatterns = 0;
	modules.push_back( this );
	nameFile = filename;
	nameFile.resize( nameFile.find_last_of( '.' ) );

	if( nameFile.find_last_of( '/' ) != string::npos ) {
		nameFile.erase( 0, nameFile.find_last_of( '/' ) + 1 );
	}
	if( nameFile.find_last_of( '\\' ) != string::npos ) {
		nameFile.erase( 0, nameFile.find_last_of( '\\' ) + 1 );
	}


	FILE *f = fopen( filename.c_str(), "rb" );
	if( !f ) {
		throw new Exception( "can't open file" );
	}

	fseek( f, 0, SEEK_END );
	long size = ftell( f );
	_fdata = new u8[ size ];
	fseek( f, 0, SEEK_SET );
	fread( _fdata, size, 1, f );
	fclose( f );
	data = _fdata;
}


bool Mod::canHandle( string filename ) { return true; }

Mod::~Mod()
{
	for( deque<Mod *>::iterator m = modules.begin(); m != modules.end(); m++ ) {
		if( *m == this ) {
			modules.erase( m );
			break;
		}
	}

	if( _fdata )
		delete [] _fdata;

	for( int i = 0; i < patterns.size(); i++ )
		delete patterns[ i ];
	patterns.clear();
}

void Mod::replaceOrder( int x, int by )
{
	for( int i = 0; i < numOrders; i++ ) {
		if( order[ i ] == x )
			order[ i ] = by;
	}
}


void Mod::deletePattern( int p )
{
	delete patterns[ p ];
	patterns.erase( patterns.begin() + p );
	numPatterns--;

	for( int i = p; i < numPatterns; i++ ) {
		replaceOrder( i + 1, i );
	}
}


void Mod::optimizePatterns()
{
	int i;

	for( i = 0; i < numOrders; i++ ) {
		if( order[ i ] >= 254 )
			 continue;
		if( order[ i ] >= patterns.size() ) {
			cerr << "\tNonexistant pattern " << ( int )order[ i ] << " in orderlist, NOT fixing (Bxx)!" << endl;
			throw new Exception( "Nonexistant pattern referenced" );
		}
		patterns[ order[ i ] ]->numReferenced++;
	}

	bool msg = false;
	for( i = 0; i < numPatterns; i++ ) {
		if( !patterns[ i ]->numReferenced ) {
			if( !msg ) {
				cout << "\tRemoving unused pattern(s): ";
				msg = true;
			}
			cout << ( int )patterns[ i ]->referencedAs << " ";
			deletePattern( i );
			i--;
		}
	}
	if( msg )
		cout << endl;
}

bool Mod::channelUsed( int c )
{
	for( int i = 0; i < numPatterns; i++ ) {
		for( int row = 0; row < patterns[ i ]->rows; row++ ) {
			if( !( patterns[ i ]->empty( row, c ) ) )
				return true;
		}
	}
	return false;
}

void Mod::copyChannel( int dst, int src )
{
	for( int i = 0; i < numPatterns; i++ ) {
		for( int row = 0; row < patterns[ i ]->rows; row++ ) {
			(*patterns[ i ])( row, dst ) = (*patterns[ i ])( row, src );
		}
	}
	channelSettings[ dst ] = channelSettings[ src ];
	channelSettings[ src ] = 0; // disable
	channelPan[ dst ] = channelPan[ src ];
}


void Mod::optimizeChannels()
{
	cout << "\tChecking channels: ";
	int i;
	for( i = 0; i < numChannels; i++ ) {
		if( channelSettings[ i ] && !channelUsed( i ) ) {
			cout << "x" << flush;
			channelSettings[ i ] = 0;
		}
		else {
			cout << "." << flush;
		}
	}
	cout << endl;

	int lastActive = -1;

	for( i = 31; i >= 0; i-- ) {
		if( channelSettings[ i ] ) {
			lastActive = i;
			break;
		}
	}

	if( lastActive < 0 ) {
		referenceAll();
		cout << "WARNING: File contains no patterndata! (keeping all samples and instruments for SFX)" << endl;
		return;
	}

	for( i = 0; i <= lastActive; i++ ) {
		if( !channelSettings[ i ] ) { // leerer channel
			cout << "copying channel #" << lastActive << " to #" << i << " (empty) " << endl;
			copyChannel( i, lastActive );
			lastActive--;
		}
	}

	numChannels = lastActive + 1;
	cout << ( int )numChannels << " channel(s)." << endl;
}



void Mod::countReferences()
{
#define SHOWPOSITION "(" << p << "," << ( l/numChannels ) << "," << ( l % numChannels ) << ")"

	for( int p = 0; p < patterns.size(); p++ ) {
		for( int l = 0; l < patterns[ p ]->elements(); l++ ) {
			Pattern::atom &a = (*patterns[ p ])( l );
			if( a.instrument ) {
				if( flagInstrumentBased ) {
					if( (a.instrument-1) < Instrument::instruments.size() &&
						!Instrument::instruments[ a.instrument-1 ]->empty ) {
						Instrument::instruments[ a.instrument-1 ]->addReference( a.note );
					}
					else {
						a.instrument = 0;
						cerr << "\tInvalid/empty instrument reference in " << SHOWPOSITION << endl;
					}
				}
				else {
					if( (a.instrument-1) < Sample::samples.size() &&
						!Sample::samples[ a.instrument-1 ]->empty ) {
						Sample::samples[ a.instrument-1 ]->numReferenced++;
					}
					else {
						a.instrument = 0;
						cerr << "\tInvalid/empty sample reference in " << SHOWPOSITION << endl;
					}
				}
			}
		}
	}
}



void Mod::replaceInstrument( int s, int by )
{
	s++;
	by++;

	for( int p = 0; p < patterns.size(); p++ ) {
		for( int l = 0; l < patterns[ p ]->elements(); l++ ) {
			Pattern::atom &a = (*patterns[ p ])( l );

			if( a.instrument == s )
				a.instrument = by;
		}
	}
}

void Mod::decreaseInstrument( int from )
{
	from++;

	for( int p = 0; p < patterns.size(); p++ ) {
		for( int l = 0; l < patterns[ p ]->elements(); l++ ) {
			Pattern::atom &a = (*patterns[ p ])( l );

			if( a.instrument > from )
				a.instrument--;
		}
	}
}




void Mod::optimize()
{
	countReferences();
	optimizePatterns();
	optimizeChannels();
	cout << endl;
}


void Mod::outputFile()
{
	ofstream f( ( nameFile + ".S" ).c_str(), ios::out );

	f	<< ".global mod_" << nameFile << endl
		<< ".section .rodata" << endl << endl;

	char xx[ 256 ];
	sprintf( xx, "LModule%02x", counter++ );
	string dataPrefix = xx;

	f << "@ song: \"" << nameSong << "\"" << endl << endl;

	// we'll just write the first 16 index'es tho.
	u16 index[ 64 ];
	int size;
	u8 *data;

	int i;
	for( i = 0; i < numPatterns; i++ ) {
		f << ".align" << endl;
		f << dataPrefix << "Pattern" << i << ":" << endl;

		data = patterns[ i ]->compress( size, index );

		f << ".short ";
		for( int in = 0; in < 16; in++ ) {
			f << index[ in ];
			if( in < 15 )
				f << ", ";
		}
		f << endl;

		f << ".short " << patterns[ i ]->rows << endl;

		f << ".byte ";
		for( int sz = 0; sz < size; sz++ ) {
			f << ( int )data[ sz ];
			if( !( ( sz + 1 ) & 63 ) )
				f << endl << ".byte ";
			else if( sz < ( size - 1 ) )
					f << ", ";

		}
		f << endl << endl << endl;
	}

	// Module
	f << ".align" << endl << "mod_" << nameFile << ":" << endl;
	f << ".byte " << ( int )numChannels << " @ # channels" << endl;
	f << ".byte " << numOrders << ", " << songRestart << " @ orders, songrestart, orderlist:" << endl;
	f << ".byte ";
	for( i = 0; i < numOrders; i++ ) {
		f << ( int )order[ i ];
		if( i < numOrders - 1 )
			f << ", ";
	}
	f << endl;
	f << ".skip " << ( 256 - numOrders ) << ", 0" << endl;

	f << "@ panlist:" << endl;
	f << ".byte ";
	for( i = 0; i < numChannels; i++ ) {
		f << ( int )channelPan[ i ];
		if( i < numChannels - 1 )
			f << ", ";
	}
	f << endl;
	f << ".skip " << ( 32 - numChannels ) << ", 0" << endl;

	u8 songIndex[ 64 ];
	int indx = 1;
	memset( songIndex, 0, 64 );
	bool seenMarker = false;
	for( i = 0; i < numOrders; i++ ) {
		if( order[ i ] == 254 )
			seenMarker = true;
		else {
			if( seenMarker ) {
				songIndex[ indx++ ] = i;
				seenMarker = false;
			}
		}
	}

	f << "@ songindex: " << endl << ".byte ";
	for( i = 0; i < 64; i++ ) {
		f << ( int )songIndex[ i ];
		if( i < 64 - 1 )
			f << ", ";
	}
	f << endl;


	f << ".byte " << ( int )volGlobal << " @ volglobal" << endl;
	f << ".byte " << ( int )initialSpeed << ", " << ( int )initialBPM << " @ speed/bpm" << endl;
	f << ".byte " << ( int )flagInstrumentBased << ", " << ( int )flagLinearSlides << ", " << ( int )flagFastVolSlides << ", 0, 0, 0" << endl;
		//( int )flagVolOpt << ", " << ( int )flagVolSlides << ", " << ( int )flagAmigaLimits << " @ volopt/volslides/amigalim" << endl;

	f << ".word ";
	for( i = 0; i < numPatterns; i++ ) {
		f << dataPrefix << "Pattern" << i;
		if( !( ( i + 1 ) & 3 ) )
			f << endl << ".word ";
		else if( i < ( numPatterns - 1 ) )
				f << ", ";
	}
	f << endl << endl;
}

void Mod::output()
{
	ofstream ms( "modules.h", ios::out );
	ms << "#ifndef __MODULES_H__" << endl;
	ms << "#define __MODULES_H__" << endl << endl;
//	ms << "#include \"mtypes.h\"" << endl << endl;

	cout << "Saving modules: " << flush;

	for( int m = 0; m < modules.size(); m++ ) {
		modules[ m ]->outputFile();
		if( m ) {
			cout << ", ";
		}
		cout << modules[ m ]->nameFile << flush;

		ms << "extern const Module mod_" << modules[ m ]->nameFile << ";" << endl;
	}

	cout << endl;
	ms << endl << "#endif" << endl << endl;
}


#include "ModXM.h"
#include "ModS3M.h"

Mod* Mod::create( string filename )
{
	try {
		unsigned vol = 0;
		int pos;

		if( ( pos = filename.find_first_of( ':' ) ) > 0 ) {
			string volS = filename.substr( pos + 1 );
			vol = atoi( volS.c_str() );
			if( vol > 255 )
				vol = 255;
			filename.resize( pos );
		}
		ifstream x( filename.c_str(), ios::in | ios::binary );
		if( !x.good() ) {
			cerr << "Can't open file \"" << filename << "\"!" << endl;
			return 0;
		}
		x.close();

		if( ModXM::canHandle( filename ) )
			return new ModXM( filename, vol );
		if( ModS3M::canHandle( filename ) )
			return new ModS3M( filename, vol );

		cerr << "Can't handle file \"" << filename << "\"!" << endl << endl;
		return 0;
	}
	catch( Exception *e ) {
		throw e;
	}
}

void Mod::free()
{
	while( modules.size() ) {
		delete modules[ 0 ];
	}
	modules.clear();
}

void Mod::referenceAll()
{
	int i;
	for( i = sampleOffsetStart; i < (sampleOffsetStart+numSamples); i++ ) {
		Sample &s = *Sample::samples[ i ];
		if( !s.empty ) {
			s.numReferenced++;
		}
	}

	for( i = instrumentOffsetStart; i < (instrumentOffsetStart+numInstruments); i++ ) {
		Instrument &ins = *Instrument::instruments[ i ];
		if( !ins.empty ) {
			ins.numReferenced++;
		}
	}
}
