find_path(
	GBA_INCLUDE_DIR
	gba.h
	$ENV{DEVKITPRO}/libgba/include
)

find_library(
	GBA_LIBRARY
	gba
	PATHS $ENV{DEVKITPRO}/libgba/lib
)

find_program(
	GBAFIX_EXECUTABLE
	gbafix
	PATHS $ENV{DEVKITARM}/bin
)

set( GBA_LIBRARIES ${GBA_LIBRARY} )
set( GBA_INCLUDE_DIRS ${GBA_INCLUDE_DIR} )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
	LibGba
	REQUIRED_VARS GBA_LIBRARY GBA_INCLUDE_DIR GBAFIX_EXECUTABLE
)

mark_as_advanced( GBA_INCLUDE_DIR GBA_LIBRARY GBAFIX_EXECUTABLE )
