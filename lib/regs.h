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

#ifndef __REGS_H__
#define __REGS_H__

#define MEM_REGISTERS 0x4000000

#define SG10_L		*( volatile u16* )( MEM_REGISTERS + 0x60 )
#define SG10_H		*( volatile u16* )( MEM_REGISTERS + 0x62 )
#define SG11		*( volatile u16* )( MEM_REGISTERS + 0x64 )

#define SG20		*( volatile u16* )( MEM_REGISTERS + 0x68 )
#define SG21		*( volatile u16* )( MEM_REGISTERS + 0x6c )

#define SG30_L		*( volatile u16* )( MEM_REGISTERS + 0x70 )
#define SG30_H		*( volatile u16* )( MEM_REGISTERS + 0x72 )
#define SG31		*( volatile u16* )( MEM_REGISTERS + 0x74 )

#define SG40		*( volatile u16* )( MEM_REGISTERS + 0x78 )
#define SG41		*( volatile u16* )( MEM_REGISTERS + 0x7c )

#define SGCNT0_L	*( volatile u16* )( MEM_REGISTERS + 0x80 )
#define SGCNT0_H	*( volatile u16* )( MEM_REGISTERS + 0x82 )
#define SGCNT1		*( volatile u16* )( MEM_REGISTERS + 0x84 )

#define SG_BIAS		*( volatile u16* )( MEM_REGISTERS + 0x88 )

#define SGWR0_L		*( volatile u16* )( MEM_REGISTERS + 0x90 )
#define SGWR0_H		*( volatile u16* )( MEM_REGISTERS + 0x92 )
#define SGWR1_L		*( volatile u16* )( MEM_REGISTERS + 0x94 )
#define SGWR1_H		*( volatile u16* )( MEM_REGISTERS + 0x96 )
#define SGWR2_L		*( volatile u16* )( MEM_REGISTERS + 0x98 )
#define SGWR2_H		*( volatile u16* )( MEM_REGISTERS + 0x9a )
#define SGWR3_L		*( volatile u16* )( MEM_REGISTERS + 0x9c )
#define SGWR3_H		*( volatile u16* )( MEM_REGISTERS + 0x9e )

#define SGFIFOA		*( volatile u32* )( MEM_REGISTERS + 0xa0 )
#define SGFIFOA_L	*( volatile u16* )( MEM_REGISTERS + 0xa0 )
#define SGFIFOA_H	*( volatile u16* )( MEM_REGISTERS + 0xa2 )
#define SGFIFOB		*( volatile u32* )( MEM_REGISTERS + 0xa4 )
#define SGFIFOB_L	*( volatile u16* )( MEM_REGISTERS + 0xa4 )
#define SGFIFOB_H	*( volatile u16* )( MEM_REGISTERS + 0xa6 )

#define INT_ME		*( volatile u16* )( MEM_REGISTERS + 0x208 )
#define INT_IE		*( volatile u16* )( MEM_REGISTERS + 0x200 )
#define INT_IF		*( volatile u16* )( MEM_REGISTERS + 0x202 )
#define INT_ADDR    *( volatile u32* )( 0x3007ffc )

// timers
	// timer 0
	#define TM0D		*( volatile u16* )( MEM_REGISTERS + 0x100 )
    #define TM0CNT		*( volatile u16* )( MEM_REGISTERS + 0x102 )
	// timer 1
	#define TM1D		*( volatile u16* )( MEM_REGISTERS + 0x104 )
	#define TM1CNT		*( volatile u16* )( MEM_REGISTERS + 0x106 )
	// timer 2
	#define TM2D		*( volatile u16* )( MEM_REGISTERS + 0x108 )
	#define TM2CNT		*( volatile u16* )( MEM_REGISTERS + 0x10a )
	// timer 3
	#define TM3D		*( volatile u16* )( MEM_REGISTERS + 0x10c )
	#define TM3CNT		*( volatile u16* )( MEM_REGISTERS + 0x10e )

// dma
	// channel 1
	#define DM1SAD		*( volatile u32* )( MEM_REGISTERS + 0xbc )
	#define DM1SAD_L	*( volatile u16* )( MEM_REGISTERS + 0xbc )
	#define DM1SAD_H	*( volatile u16* )( MEM_REGISTERS + 0xbe )
	#define DM1DAD		*( volatile u32* )( MEM_REGISTERS + 0xc0 )
	#define DM1DAD_L	*( volatile u16* )( MEM_REGISTERS + 0xc0 )
	#define DM1DAD_H	*( volatile u16* )( MEM_REGISTERS + 0xc2 )
	#define DM1CNT		*( volatile u32* )( MEM_REGISTERS + 0xc4 )
	#define DM1CNT_L	*( volatile u16* )( MEM_REGISTERS + 0xc4 )
	#define DM1CNT_H	*( volatile u16* )( MEM_REGISTERS + 0xc6 )
	// channel 2
	#define DM2SAD		*( volatile u32* )( MEM_REGISTERS + 0xc8 )
	#define DM2SAD_L	*( volatile u16* )( MEM_REGISTERS + 0xc8 )
	#define DM2SAD_H	*( volatile u16* )( MEM_REGISTERS + 0xca )
	#define DM2DAD		*( volatile u32* )( MEM_REGISTERS + 0xcc )
	#define DM2DAD_L	*( volatile u16* )( MEM_REGISTERS + 0xcc )
	#define DM2DAD_H	*( volatile u16* )( MEM_REGISTERS + 0xce )
	#define DM2CNT		*( volatile u32* )( MEM_REGISTERS + 0xd0 )
	#define DM2CNT_L	*( volatile u16* )( MEM_REGISTERS + 0xd0 )
	#define DM2CNT_H	*( volatile u16* )( MEM_REGISTERS + 0xd2 )

#endif

