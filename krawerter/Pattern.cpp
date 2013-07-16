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

#include <cstring>
#include "Pattern.h"
#include "Exception.h"

Pattern::Pattern( unsigned rows, unsigned cols ) : rows( rows ), cols( cols ), numReferenced( 0 ), referencedAs( 0 ), data( 0 )
{
	data = new atom[ rows * cols ];
	memset( data, 0, sizeof( atom ) * cols * rows );
}

Pattern::~Pattern()
{
	delete [] data;
}

Pattern::atom& Pattern::operator()( unsigned row, unsigned col )
{
	if( row >= rows || col >= cols )
		throw new Exception( "Pattern::operator(): out of bound" );
	else
		return data[ row * cols + col ];
}

Pattern::atom& Pattern::operator()( unsigned linear )
{
	if( linear >= rows*cols )
		throw new Exception( "Pattern::operator(): out of bound" );
	else
		return data[ linear ];
}


bool Pattern::empty( unsigned row, unsigned col )
{
	atom &a = (*this)( row, col );
	if( !a.effect && !a.effectop && !a.note && !a.instrument && !a.volume ) {
		return true;
	}
	return false;
}

unsigned Pattern::elements() { return rows*cols; }

// simple rle, similiar to those in s3m and xm
u8 * Pattern::compress( int & size, u16 * index )
{
	static u8 buffer[ 16384 ];
	u8 *b = buffer;


	memset( buffer, 0, 16384 );
	size = 0;

	for( int row = 0; row < rows; row++ ) {
		int col = 0;
		if( !( row & 3 ) ) { // eine mod 4 -reihe, index speichern
			index[ row >> 2 ] = b - buffer;
		}

		while( 1 ) {
			// ok, erste col suchen wo was zum speichern is
			while( ( col < cols ) &&
				empty( row, col ) ) {
				col++;
			}
			if( col == cols )
				break;

			Pattern::atom &a = (*this)( row, col );

			u8 follow = col;

			// gut, was gibz zum speichern
			if( a.note || a.instrument ) {
				follow |= 32;
			}
			if( a.volume ) {
				follow |= 64;
			}
			if( a.effect || a.effectop ) {
				follow |= 128;
			}

			*b++ = follow;
			if( follow & 32 ) {
				*b = a.note & 0x7f;

				if( a.instrument > 255 ) {
					*b |= 0x80;
				}

				b++;
				*b++ = a.instrument & 0xff;

				if( a.instrument > 255 ) {
					*b++ = a.instrument >> 8;
				}

				/*
				u16 note = 0;
				if( a.note )
					note = ( a.note & 0x7f ) << 9;
				note |= ( a.instrument & 0x1ff );
				*b++ = note >> 8;
				*b++ = note & 0xff;
				*/
			}
			if( follow & 64 ) {
				*b++ = a.volume;
			}
			if( follow & 128 ) {
				*b++ = a.effect;
				*b++ = a.effectop;
			}
			col++;
		}

		// next row
		*b++ = 0;
	}

	size = b - buffer;
	return buffer;
}
