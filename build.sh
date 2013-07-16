#!/bin/sh

BUILDDIR=build

if [ "$1" = "-d" ]; then
	BUILDTYPE=Debug
	BUILDDIR=$BUILDDIR-debug
else
	BUILDTYPE=RelWithDebInfo
fi

cd `dirname $0`
if [ -d $BUILDDIR ]; then
	echo "error: directory '$BUILDDIR' exists"
	exit 1
fi

mkdir $BUILDDIR && cd $BUILDDIR
cmake ..
make
