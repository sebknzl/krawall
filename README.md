![Krawall](docs/krawall-logo.png?raw=true)

Krawall
=======

Krawall is an XM/S3M Modplayer for the Gameboy Advance (GBA).

It used to be a commercial product, licensed for games like "Lord of the Rings", "Spiderman", "The Sims" and others.

Its main strengths is a very faithful XM/S3M-implementation and fast and high-quality mixing-routines.


Status
------

I've rewritten the build-system from the original version and ported everything to the toolchain used by
devkitPro. Building works on OSX (and probably any Linux) and the examples work on an emulator,
I've tried a Visual Boy Advance-port on OSX.

See TODO.txt for what needs to be done.


Building / Usage
----------------

Requirements:

 * CMake, version >= 2.8 (2.6 may work, but not tested)
 * devkitPro from http://devkitpro.org/, I've used the [automated installers](http://sourceforge.net/projects/devkitpro/files/Automated%20Installer/) on OSX
 * C++-Compiler (clang, g++, Visual Studio Express, ...) to build krawerter (MOD-converter)
 
Make sure you have the DEVKITPRO/DEVKITARM environment variables set.
 
Then, starting build.sh should be all you need on OSX and probably Linux.
I haven't tried building under Windows yet, it will probably require some tweaking of the CMakeLists. Contact me if you need help.

The following CMake-variables can be configured ("ccmake build" or GUI):

 * KRAWALL\_DEFAULT\_VARIANT (default "16k-60-medium"): This is the default variant of the library that will be built.
  * The first parameter can be "16k" or "32k", which refers to the frequency Krawall will mix at.
  * The second parameter can be "60" or "30", which is the rate (Hz) at which kramWorker() will be called.
  * The third parameter can be "small", "medium" or "large" and refers to the amount of IWRAM that will be used.
    "large" has the best performance, but uses the most IWRAM.
 * KRAWALL\_BUILD\_ALL\_VARIANTS (default OFF): If enabled, all possible variants will be built.

You'll find the build-results in the directories "krawall" and "krawerter" in the "build"-directory.
The examples in "krawall/examples" should be ready to run on an emulator.


License
-------

Krawall is covered by the [LGPL v2.1](http://www.gnu.org/licenses/lgpl-2.1.html), see COPYING for the full text. 

    Krawall, XM/S3M Modplayer Library
    Copyright (C) 2001-2005, 2013 Sebastian Kienzl
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
