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

#ifndef __GENERAL_H__
#define __GENERAL_H__

//%b
//! Init function.
/*!
Call this function once at startup.
\param stereo Whether Krawall should operate stereo (KRAG_INIT_STEREO) or not (KRAG_INIT_MONO)
*/
void kragInit( int stereo );
#define KRAG_INIT_MONO 0
#define KRAG_INIT_STEREO 1


//! Reset function.
/*!
This is only needed if you want to call kragInit() again, most likely because you want
to switch from mono to stereo or vice versa. Calling this while there is sound being output
will result in an audible (but harmless) hickup.
*/
void kragReset();

//%e

#endif

