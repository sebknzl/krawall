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

Example "jinglesong":

This example shows how to use songs and jingles.

*/

#include <gba.h>
#include "krawall.h"
#include "modules/modules.h"
#include "modules/samples.h"
#include "modules/instruments.h"

int main()
{
	int buttonA, buttonB = 1;
	int rasterbar_color = ( 5 << 0 ) + ( 22 << 5 ) + ( 31 << 10 );

	irqInit();
	irqSet( IRQ_TIMER1, kradInterrupt );
	// not really needed, as kragInit also enables IRQ_TIMER1
	irqEnable( IRQ_TIMER1 );
	REG_IME = 1;

	SetMode( MODE_0 | BG0_ON );

	kragInit( KRAG_INIT_STEREO );					// init krawall
	krapPlay( &mod_secondpm, KRAP_MODE_LOOP, 0 );	// play module

	while( 1 ) {
		// wait for line 0
		while( !REG_VCOUNT );
		while( REG_VCOUNT );

		// rasterbar start
		BG_COLORS[0] = rasterbar_color;
		kramWorker();
		// rasterbar stop
		BG_COLORS[0] = 0;

		scanKeys();
		int keys_pressed = keysDown();

		if( keys_pressed & KEY_A ) {
			// start the jingle, see secondpm.s3m how the module is partioned,
			// the 5th partition is patterns 43, 44, 48, 49 and 66, thus we pass 4 here
			// (counting starts at 0).
			// also, we want these patterns looped forever, until button B is pressed
			krapPlay( &mod_secondpm, KRAP_MODE_JINGLE | KRAP_MODE_SONG | KRAP_MODE_LOOP, 4 );

			// change color here so we know we're in jingle mode
			if( rasterbar_color & 1 )
				rasterbar_color ^= 0xffff;
		}

		if( keys_pressed & KEY_B ) {
			// we've had enough of the jingle, resume normal playback
			krapStopJingle();

			// don't just resume playback, set volume to zero and fade it back in
			krapSetMusicVol( 0, 0 );
			krapSetMusicVol( 128, 1 );

			// change color back
			if( !( rasterbar_color & 1 ) )
				rasterbar_color ^= 0xffff;
		}
	}

	return 0;
}

