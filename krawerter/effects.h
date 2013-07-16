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

#ifndef __EFFECTS_H__
#define __EFFECTS_H__


#define EFF_NONE					0


#define EFF_SPEED					1
#define EFF_BPM						2
#define EFF_SPEEDBPM				3

#define EFF_PATTERN_JUMP			4
#define EFF_PATTERN_BREAK			5

#define EFF_VOLSLIDE_S3M			6
#define EFF_VOLSLIDE_XM				7

#define EFF_VOLSLIDE_DOWN_XM_FINE	8
#define EFF_VOLSLIDE_UP_XM_FINE		9

#define EFF_PORTA_DOWN_XM			10
#define EFF_PORTA_DOWN_S3M  		11
#define EFF_PORTA_DOWN_XM_FINE		12
#define EFF_PORTA_DOWN_XM_EFINE		13

#define EFF_PORTA_UP_XM				14
#define EFF_PORTA_UP_S3M			15
#define EFF_PORTA_UP_XM_FINE		16
#define EFF_PORTA_UP_XM_EFINE		17

#define EFF_VOLUME					18

#define EFF_PORTA_NOTE				19
#define EFF_VIBRATO					20
#define EFF_TREMOR					21
#define EFF_ARPEGGIO				22
#define EFF_VOLSLIDE_VIBRATO		23
#define EFF_VOLSLIDE_PORTA 			24
#define EFF_CHANNEL_VOL				25
#define EFF_CHANNEL_VOLSLIDE		26
#define EFF_OFFSET					27
#define EFF_PANSLIDE				28
#define EFF_RETRIG					29
#define EFF_TREMOLO					30


#define EFF_FVIBRATO				31
#define EFF_GLOBAL_VOL				32
#define EFF_GLOBAL_VOLSLIDE			33
#define EFF_PAN						34
#define EFF_PANBRELLO				35
#define EFF_MARK					36


#define EFF_GLISSANDO				37
#define EFF_WAVE_VIBR				38
#define EFF_WAVE_TREMOLO			39
#define EFF_WAVE_PANBRELLO			40
#define EFF_PATTERN_DELAYF			41
#define EFF_OLD_PAN					42
#define EFF_PATTERN_LOOP			43
#define EFF_NOTE_CUT				44
#define EFF_NOTE_DELAY				45
#define EFF_PATTERN_DELAY			46

#define EFF_ENV_SETPOS				47
#define EFF_OFFSET_HIGH				48

#define EFF_VOLSLIDE_VIBRATO_XM		49
#define EFF_VOLSLIDE_PORTA_XM		50

//#define EFF_VOLSLIDE_S3M_FAST		49
//#define EFF_PAN16					42

#endif
