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

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include <string>
#include <deque>
using namespace std;

#include "types.h"

class Instrument {
public:
	Instrument();

	static void optimize();
	static void output();

	static void free();

	static void replaceSample( int s, int by );
	static void decreaseSample( int from );
	static void deleteInstrument( int i );


	string name;

	u16 samples[ 96 ];

	struct EnvNode {
		u16 coord, inc;
	};

	struct Env {
		EnvNode	nodes[ 12 ];
		u8 num;
		u8 sus;
		u8 loopStart;
		u8 loopEnd;
		u8 flags;
	};

	Env	envVol;
	Env	envPan;
/*
	env envVol[ 12 ];
	u8 envVolNum;
	u8 envVolSus;
	u8 envVolLoopStart;
	u8 envVolLoopEnd;
	u8 envVolFlags;

	env envPan[ 12 ];
	u8 envPanNum;
	u8 envPanSus;
	u8 envPanLoopStart;
	u8 envPanLoopEnd;
	u8 envPanFlags;
*/
	u8 vibType;
	u8 vibSweep;
	u8 vibDepth;
	u8 vibRate;

	u16 volFade;


	bool empty;

//	static void sanityCheck();
	void addReference( u8 note );

	u16 referencedAs;
	u32 numReferenced;
	static deque<Instrument *> instruments;
	static int currentOffset;

protected:
	void recalcEnvelopes();
};

#endif
