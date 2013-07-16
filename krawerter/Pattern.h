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

#ifndef __PATTERN_H__
#define __PATTERN_H__

#include "types.h"

class Pattern {
public:
	struct atom {
		u8 note;
		u16 instrument;
		u8 volume;
		u8 effect;
		u8 effectop;
	};

	atom* data;
	unsigned rows, cols;

	Pattern( unsigned rows, unsigned cols );
	~Pattern();
	atom& operator()( unsigned row, unsigned col );
	atom& operator()( unsigned linear );
	bool empty( unsigned row, unsigned col );

	u8 * compress( int & size, u16 * index );

	unsigned elements();

	u8 referencedAs;
	u8 numReferenced;
};



#endif
