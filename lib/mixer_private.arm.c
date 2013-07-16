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
#include "tools.h"
#include "mixer_private.h"

static s16 mixBuffer[ SAMPLESPERFRAME*2 ] IWRAM __attribute__((__aligned__(4)));	// *2 -> stereo

void (* volatile soundTimerFunc)( void ) EWRAM = 0;
u16 soundTimerSpeed EWRAM = 32768;
u16 soundTimerCount EWRAM = 32768;

// a guess how many samples still fit
#define SAMPLEINCGUESS		( todo * ( ( c->inc >> 16 ) + 1 ) )
#define SAMPLEINCGUESSNEG   ( todo * ( c->inc >> 16 ) )

#define LOOP_NORMAL     1
#define LOOP_BIDIR 		2

// 'does' the loop
#define DOLOOP								\
  	if( c->loop == LOOP_BIDIR ) {			\
   		c->inc = -c->inc;					\
	}										\
   	else {									\
		c->pos -= c->loopLength;			\
	}


// amount % 4 must be 0!
static void IWRAM_CODE mixReal( s8 *lbuffer, s8 *rbuffer, uint amount ) 
{
	s16 *dest;

	// clear buffer
	mixClear( mixBuffer, amount );

	int i;
	MixChannel* c = &channels[ 0 ];
	for( i = CHANNELNUM; i; i--, c++ ) {
		if( c->status != CHN_ACTIVE )
			continue;

		int todo = amount;

		// 0-vol optimization
		if( !c->vol ) {
			u32 sampleinc = SAMPLEINCGUESS;
			if( c->loop ) {
				if( c->loop == LOOP_BIDIR ) {
					if( c->inc >= 0 ) {
						c->pos += sampleinc;
						// if beyond end, move back into loop and change direction/position accordingly
						int block;
						for( block = 0; c->pos >= c->end; c->pos -= c->loopLength, block++ );
						if( block & 1 ) { // odd
							c->pos = c->end - ( c->pos - c->lstart );
							c->inc = -c->inc;
						}
					}
					else {	// same as above but when direction is negative
						c->pos += sampleinc;
						int block;
						for( block = 0; c->pos < c->lstart; c->pos += c->loopLength, block++ );
						if( block & 1 ) { // odd
							c->pos = c->lstart + ( c->end - c->pos );
							c->inc = -c->inc;
						}
					}
				}
				else {
					c->pos += sampleinc;
					// beyond end?
					if( c->pos >= c->end ) {
						for( ; c->pos >= c->end; c->pos -= c->loopLength );
					}
				}
			}
			else { // simple case, unlooped: either stop or continue
				if( c->pos + sampleinc >= c->end ) {
					c->status = CHN_INACTIVE;
				}
				else {
					c->pos += sampleinc;
				}
			}
			continue;
		}

    	dest = mixBuffer;

    	volSumLeft += c->lvol;
    	volSumRight += c->rvol;
    	// determine mix-func to be used
    	MIX_FUNCTION((*mixFunc)) = mixPanTable[ c->mixFunc + c->hq ];

      	int fitenough;
      	int fit, rem;

      keepmixing:
      	if( c->inc >= 0 ) {
      		// can we just mix? is the sample long enough?
      		if( !( fitenough = ( c->pos + SAMPLEINCGUESS < c->end ) ) ) {
      			// no, calculate exactly how many samples will fit
				FastIntDivideRem(
					( ( ( c->end - c->pos ) << 14 ) - ( c->frac >> 2 ) ), // '/'
					( c->inc >> 2 ),
					&fit, &rem );
      		}
      	}
      	else {
      		if( !( fitenough = ( c->pos + SAMPLEINCGUESSNEG > c->lstart ) ) ) {

				FastIntDivideRem(
					( ( ( c->pos - c->lstart ) << 14 ) + ( c->frac >> 2 ) ), // '/'
					( (-c->inc) >> 2 ),
					&fit, &rem );
      		}
      	}

		if( fitenough ) {
			// we can mix without hitting a loop-end, do that!
			mixFunc( c, dest, todo );
		}
		else {
			if( rem )
				fit++;

			// this can happen as well (guessing above isn't that accurate)
			if( fit > todo ) {
				mixFunc( c, dest, todo );
			}
			else { // fit <= todo
				// align fit to 4
				if( fit & 3 )
					fit = ( ( fit >> 2 ) + 1 ) << 2;

				if( fit < todo ) {
					if( fit ) {
				    	mixFunc( c, dest, fit );

				    	todo -= fit;
						dest += 2*fit;
					}

				    if( c->loop ) {
				        DOLOOP;

						if( todo ) {
							goto keepmixing;
						}
				    }
				    else {
				    	c->status = CHN_INACTIVE;
				    	mixBias( dest, todo, c->lvol, c->rvol );
				    }
				}
				else {	// fit == todo (not >todo since todo is aligned to 4 as well)
					mixFunc( c, dest, fit );
					if( c->loop ) {
						DOLOOP;
					}
					else
						c->status = CHN_INACTIVE;
				}
			}
		}
	}

	// downsample
	mix16to8( mixBuffer, lbuffer, rbuffer, amount );
}


int IWRAM_CODE kramWorker()
{
	s8 *lbuffer, *rbuffer;
	uint amount = getDmaAddress( &lbuffer, &rbuffer );

	if( !amount ) {
		return 0;
	}

	// mixReal's amount muss immer %4 ==0 sein
	// soundTimerCount & amount von getDmaAddress sind immer durch 4 teilbar!
	do {
		if( ( soundTimerCount <= amount ) && soundTimerFunc ) {
			mixReal( lbuffer, rbuffer, soundTimerCount );
			amount -= soundTimerCount;
			lbuffer += soundTimerCount;
			rbuffer += soundTimerCount;
			soundTimerFunc();
			soundTimerCount = soundTimerSpeed;
		}
		else {
			mixReal( lbuffer, rbuffer, amount );
			soundTimerCount -= amount;
			break;
		}
	} while( amount );

/*
	// this version saves around a percent, doesn't have very good timing tho.
	mixReal( lbuffer, rbuffer, amount );

	do {
		if( amount >= soundTimerCount ) {
			amount -= soundTimerCount;
			if( soundTimerFunc )
				soundTimerFunc();
			soundTimerCount = soundTimerSpeed;
		}
		else {
			soundTimerCount -= amount;
			break;
		}
	} while( amount );
*/
	return 1;
}


