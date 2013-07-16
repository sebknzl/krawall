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

#ifndef __MTYPES_H__
#define __MTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__ ((packed)) {
	unsigned long 	loopLength;
	unsigned char*	end;
	unsigned long	c2Freq;
	signed char		fineTune;
	signed char		relativeNote;
	unsigned char  	volDefault;
	signed char		panDefault;
	unsigned char  	loop;
	unsigned char	hq;
	signed char  	data[1];
} Sample;

typedef struct __attribute__ ((packed)) {
	unsigned short	coord, inc;
} EnvNode;

typedef struct __attribute__ ((packed)) {
	EnvNode			nodes[ 12 ];
	unsigned char	max;
	unsigned char	sus;
	unsigned char	loopStart;
	unsigned char	flags;
} Envelope;


typedef struct __attribute__ ((packed)) {
	unsigned short	samples[ 96 ];

	Envelope		envVol;
	Envelope		envPan;
	unsigned short	volFade;

	unsigned char	vibType;
	unsigned char	vibSweep;
	unsigned char	vibDepth;
	unsigned char	vibRate;
} Instrument;

typedef struct __attribute__ ((packed)) {
	unsigned short 	index[ 16 ];
	unsigned short	rows;
	unsigned char 	data[1];
} Pattern;

typedef struct __attribute__ ((packed)) {
	unsigned char 	channels;
	unsigned char 	numOrders;
	unsigned char	songRestart;
	unsigned char 	order[ 256 ];

	signed char 	channelPan[ 32 ];

	unsigned char 	songIndex[ 64 ];

	unsigned char 	volGlobal;

	unsigned char 	initSpeed;
	unsigned char 	initBPM;

	unsigned char	flagInstrumentBased;
	unsigned char	flagLinearSlides;
	unsigned char 	flagVolSlides;
	unsigned char 	flagVolOpt;
	unsigned char 	flagAmigaLimits;
	unsigned char	___padding;

	const Pattern* 	patterns[1];
} Module;

extern const Sample* const samples[];
extern const Instrument* const instruments[];

#ifdef __cplusplus
}
#endif

#endif

