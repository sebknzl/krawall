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

#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include "types.h"
#include <string>
#include <deque>

using namespace std;

class Sample {
public:
	Sample();
	~Sample();

	static void optimize();
	static void output();
	static void free();

	string fileName;
	string name;

	u32 length;
	u32 loopBegin;
	u32 loopLength;

	u32 freq;
	s8 finetune;
	s8 relativeNote;

	u8 volDefault;
	s8 panDefault;
	u8 loop;

	//	bool stereo;

	bool empty;
	u8 *data;

	bool operator==( const Sample & p );
	void extendLoop();
	void cutLoop();

	u16 referencedAs;
	u32 numReferenced;
	static deque<Sample *> samples;
	static int currentOffset;

	bool is16bit;		// only for converting (ModXM)

private:
	static void deleteSample( int s );
	static void replaceSample( int s, int by );


	void calcFreq();
	void findFreqApprox();
};

#endif
