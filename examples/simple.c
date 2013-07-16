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

/*

Example "simple":

This example shows how to play a song, SFX and instrument-SFX.

*/

#include <gba.h>
#include "krawall.h"
#include "modules/modules.h"
#include "modules/samples.h"
#include "modules/instruments.h"


int main()
{
	ihandle engine = 0;

	irqInit();
	irqSet( IRQ_TIMER1, kradInterrupt );
	// not really needed, as kragInit also enables IRQ_TIMER1
	irqEnable( IRQ_TIMER1 );
	REG_IME = 1;

	SetMode( MODE_0 | BG0_ON );

	kragInit( KRAG_INIT_STEREO );                   // init krawall
//	krapPlay( &mod_secondpm, KRAP_MODE_LOOP, 0 );   // play module
	krapPlay( &mod_tf_iesc, KRAP_MODE_LOOP, 0 );    // play module

	while( 1 ) {
		// wait for line 0
		while( !REG_VCOUNT );
		while( REG_VCOUNT );

		// rasterbar start
		BG_COLORS[0] = RGB8(58,110,165);

		krapInstProcess();   // update sfx-instruments
		kramWorker();        // do the stuff
		
		// rasterbar stop
		BG_COLORS[0] = 0;

		scanKeys();
		int keys_pressed = keysDown();
		int keys_released = keysUp();

		if( keys_pressed & KEY_A ) {
			// start instrument
			engine = krapInstPlay( instruments[ INSTRUMENT_ENGINE ], 72, engine );
		}
		if( keys_released & KEY_A ) {
			// release instrument when button is released
			krapInstRelease( engine );
		}

		if( keys_pressed & KEY_B ) {
			// play a sample, default is centered
			kramPlay( samples[ SAMPLE_MECH_KLACK ], 1, 0 );
		}

		if( keys_pressed & KEY_R ) {
			// start a sample and pan it far right
			chandle x = kramPlay( samples[ SAMPLE_HIT_GUIT ], 1, 0 );
			kramSetPan( x, KRAM_SP_RIGHT );
		}

		if( keys_pressed & KEY_L ) {
			// this is another way to do the same as above
			kramPlayExt( samples[ SAMPLE_HIT_SKIP ], 1, 0, samples[ SAMPLE_HIT_SKIP ]->c2Freq, 64, KRAM_SP_LEFT );
		}
	}

	return 0;
}

