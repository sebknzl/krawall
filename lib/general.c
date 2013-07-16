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

#include "directsound.h"
#include "mixer.h"
#include "player.h"
#include "general.h"

void kragInit( int stereo )
{
	kramSetMasterVol( 128 );
	dsInit( stereo );
	kramResetChannels( 1 );	// needed if kragInit is called more than once, so stereo-channels are set to mono or vice versa
	kradActivate();
	krapInit();
}

void kragReset()
{
	dsDeInit();
}

#include "version.h"
const static char __id[] = "$Krawall v"KRAWALL_VERSION;

