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

#ifndef __PLAYER_TABLES_H__
#define __PLAYER_TABLES_H__

#include "types.h"

#define TABLE_VIBRATO_LENGTH 64
#define TABLE_PERIODS_LENGTH 12*10

// vibrato/tremolo-tables
extern const s16 table_sine[];
extern const s16 table_ramp[];
extern const s16 table_square[];
extern const s16 table_rand[];
// fine-tune table (-8..7)
extern const u16 table_finetune[];
// period-table (amiga-style)
extern const u16 table_periods[];
// linear table (xm)
extern const u32 table_linear[];


#endif

