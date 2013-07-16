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

Example "callback":

This example shows how to use callbacks, pause and fade a song.

*/

#include <gba.h>
#include "krawall.h"
#include "modules/modules.h"
#include "modules/samples.h"
#include "modules/instruments.h"


volatile int rasterbar_color = ( 5 << 0 ) + ( 22 << 5 ) + ( 31 << 10 );


// this is the callback-function
void callback( int event, int param )
{
	switch( event ) {
	case KRAP_CB_MARK:
		rasterbar_color = 	( ( param & 0x30 ) << 9 ) |
							( ( param & 0x0c ) << 6 ) |
							( ( param & 0x03 ) << 3 );
		break;
	case KRAP_CB_FADE:
		// fading to silence done, pause!
		if( param == 0 ) {
			krapPause( 1 );
		}
		break;
	case KRAP_CB_DONE:
		// play it again
		// (could be done with parameter KRAP_MODE_LOOP, too)
		krapPlay( &mod_secondpm, 0, 0 );
		break;
	default:
		break;
	}

}


int main()
{
	int fadeToggle = 1;

	irqInit();
	irqSet( IRQ_TIMER1, kradInterrupt );
	// not really needed, as kragInit also enables IRQ_TIMER1
	irqEnable( IRQ_TIMER1 );
	REG_IME = 1;

	SetMode( MODE_0 | BG0_ON );

	kragInit( KRAG_INIT_STEREO );       // init krawall
	krapCallback( callback );
	krapPlay( &mod_secondpm, 0, 0 );    // play module

	while( 1 ) {
		// wait for line 0
		while( !REG_VCOUNT );
		while( REG_VCOUNT );

		// rasterbar start
		BG_COLORS[0] = rasterbar_color;
		kramWorker();			// do the stuff
		// rasterbar stop
		BG_COLORS[0] = 0;

		scanKeys();
		int keys_pressed = keysDown();

		if( keys_pressed & KEY_A ) {
			if( krapIsPaused() ) {
				// if resuming, force volume to max
				krapSetMusicVol( 128, 0 );
				krapUnpause();
			}
			else
				krapPause( 1 );
		}

		if( keys_pressed & KEY_B ) {
			if( fadeToggle ) {
				// see callback function, when silence is reached
				// the tune is paused
				krapSetMusicVol( 0, 1 );
			}
			else {
				krapSetMusicVol( 128, 1 );
			}
			fadeToggle ^= 1;
		}

	}

	return 0;
}

