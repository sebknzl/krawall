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

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <list>
#include <cstring>
#include <iomanip>
using namespace std;

#include <assert.h>

#include "types.h"
#include "Sample.h"
#include "Instrument.h"
#include "Exception.h"

//bool debug = false;

#include "ModXM.h"

#define CONVERTER_VERSION "1.0"

int main( int argc, char **argv )
{
	cout <<
		"-============================================================-\n" \
		" Krawall converter "CONVERTER_VERSION"\n" \
		" Copyright (c) 2001-2005, 2013 Sebastian Kienzl <seb@knzl.de>\n" \
		" See COPYING for license terms\n" \
		"-============================================================-\n" << endl;
		
	if( argc < 2 ) {
		cerr << "Usage: " << argv[ 0 ] << " file1[:vol] file2[:vol] file3[:vol] ..." << endl << endl;
		return 0;
	}

	int startarg = 1;
	/*
	if( !strcmp( argv[ 1 ], "-d" ) ) {
		startarg++;
		debug = true;
	}
	*/
	int i;
	for( i = startarg; i < argc; i++ ) {
		Mod *m = 0;
		try {
			m = Mod::create( argv[ i ] );
			if( m )
				m->optimize();
		}
		catch( Exception * e ) {
			if( m )
				delete m;
			cout << "FATAL: " << e->getMsg() << endl << endl;
		}
	}

	try {
		cout << endl;
		Sample::optimize();
		Instrument::optimize();
		Sample::output();
		Instrument::output();
		Mod::output();
		cout << endl;
	}
	catch( Exception * e ) {
		cout << "FATAL: " << e->getMsg() << endl << endl;
	}

	Mod::free();
	Sample::free();
	Instrument::free();
	return 0;

}
