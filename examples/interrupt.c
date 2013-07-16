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

Example "interrupt":

Same as the "simple" example, except that the work is done
in the vblank-interrupt. Also the "mutex" as suggested in
appendix A is implemented.

*/

#include <gba.h>
#include "krawall.h"
#include "modules/modules.h"
#include "modules/samples.h"
#include "modules/instruments.h"


volatile int gamelogicRunning = 0;
volatile int kramWorkerCallNeeded = 0;

// vblank-interrupt
void vblank()
{
	if( !gamelogicRunning ) {
		kramWorkerCallNeeded = 0;
		krapInstProcess();      // update sfx-instruments
		kramWorker();			// do the stuff
	}
	else {
		kramWorkerCallNeeded = 1;
	}
}


int main()
{
	int i;
	ihandle engine = 0;

	irqInit();
	irqSet( IRQ_TIMER1, kradInterrupt );
	irqSet( IRQ_VBLANK, vblank );
	
	// not really needed, as kragInit also enables IRQ_TIMER1
	irqEnable( IRQ_TIMER1 | IRQ_VBLANK );
	REG_IME = 1;

	kragInit( KRAG_INIT_STEREO );					// init krawall
	krapPlay( &mod_secondpm, KRAP_MODE_LOOP, 0 );	// play module

	while( 1 ) {

		// do stuff, it can take as long as you want,
		// because kramWorker() is called in the interrupt
		for( i = 0; i < 100000; i++ );

		// here comes the critical region, where calls to
		// the Krawall-API are allowed
		// NOTE that the critical region MUST NOT take longer
		// than one frame, (or two frames when using the 30Hz Krawall-version)
		// because otherwise kramWorker() does not get called often enough!
		gamelogicRunning = 1;
		{
			scanKeys();
			int keys_pressed = keysDown();
			int keys_released = keysUp();
	
			if( keys_pressed & KEY_A ) {
				// play a sample, default is centered
				kramPlay( samples[ SAMPLE_MECH_KLACK ], 1, 0 );
			}
				
			// now check if vblank was trying to call kramWorker(),
			// if it wasn't able to, do it here
			if( kramWorkerCallNeeded ) {
				krapInstProcess();      // update sfx-instruments
				kramWorker();			// do the stuff
			}
			
		}
		gamelogicRunning = 0;

	}

	return 0;
}

