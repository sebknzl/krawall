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

#ifndef __DIRECTSOUND_H__
#define __DIRECTSOUND_H__

#include "types.h"
#include "directsound-def.h"

void dsInit( u32 stereo );
void dsDeInit();

u32 getDmaAddress( s8 **left, s8 **right ) LONG_CALL;

//%b
//! Directsound Interrupt
/*!
This function resets the DMA and must be tied to the
Timer1-IRQ.
*/
void kradInterrupt();

//! Activate Krawall
/*!
You only need to call this if you have called kradDeactivate().
*/
void kradActivate();

//! Deactivate Krawall
/*!
You might want to deactivate Krawall in order to write savegames
and stuff like that. Calling this will stop all DMA-operations
(and thus sound-output) until resumed by kradActivate().
*/
void kradDeactivate();


//%e

extern u8 dsStereo;
extern void kradInterruptUndoCodeMod() LONG_CALL;

#endif

