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

#ifndef __MODS3M_H__
#define __MODS3M_H__


#include "Mod.h"


class ModS3M : public Mod {
public:
	ModS3M( string filename, int globalVolume );
	~ModS3M();


	static bool canHandle( string filename );

private:
	bool flagSignedSamples;
	u8 *dataBackup;

	bool flagVolOpt;

	void readS3MSample( int ref );
	void readS3MPattern( int ref );
};


#endif
