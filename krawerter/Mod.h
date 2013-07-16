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

#ifndef __MOD_H__
#define __MOD_H__

#include <deque>
#include <string>
#include "types.h"
#include "Pattern.h"

using namespace std;

class Mod {
	friend class Sample;
	friend class Instrument;

public:
	Mod( string filename, int globalVolume );
	virtual ~Mod();
	static bool canHandle( string filename );

	static void free();

	void optimize();

	void replaceInstrument( int s, int by );
	void decreaseInstrument( int from );

	u8 * compressPattern( int pat, int & size, u16 * index );

	static void output();

	static Mod* create( string filename );

protected:
	void replaceOrder( int x, int by );
	void deletePattern( int p );
	void optimizePatterns();
	bool channelUsed( int c );
	void copyChannel( int dst, int src );
	void optimizeChannels();
	void countReferences();

	string nameFile;
	string nameSong;

//	u16 songLength;
	u16 songRestart;

	u16 numOrders;
	u16 numSamples;
	u16 numInstruments;
	u16 numPatterns;
	u16 numChannels;

	u8 volGlobal;
//	u8 volMaster;

	bool flagInstrumentBased;
	bool flagLinearSlides;
	u8 order[ 256 ];

	u8 initialSpeed;
	u8 initialBPM;

	/*
	bool flagVolOpt;
	bool flagStereo;
	*/
	bool flagFastVolSlides;


	u8 channelSettings[ 32 ];
	s8 channelPan[ 32 ];

	deque<Pattern *> patterns;

	static deque<Mod *> modules;

	u8* data;

	void outputFile();

	int sampleOffsetStart;		// are invalid as soon as samples/instruments get optimzed
	int instrumentOffsetStart;

private:
	u8* _fdata;
	static int counter;

	void referenceAll();		// SFX
};

#define MU8(x,y) ( *( u8* )( (char*)x + y ) )
#define MS8(x,y) ( *( s8* )( (char*)x + y ) )
#define MU16(x,y) ( *( u16* )( (char*)x + y ) )
#define MS16(x,y) ( *( s16* )( (char*)x + y ) )
#define MU32(x,y) ( *( u32* )( (char*)x + y ) )
#define MS32(x,y) ( *( s32* )( (char*)x + y ) )

#endif
