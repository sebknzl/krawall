cmake_minimum_required( VERSION 2.8 )

project( krawall )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
#include( SVNVersion )
include( ExternalProject )

# configuration
set( KRAWALL_BUILD_ALL_VARIANTS OFF CACHE BOOL "Build all library-variants (32kHz, 60 Hz retrace, IWRAM-usage, ...)" )
set( KRAWALL_DEFAULT_VARIANT "16k-60-medium" CACHE STRING "Default variant of the library to build (16k|32k)-(30|60)-(small|medium|large)" )


if( CMAKE_CROSSCOMPILING )
	include( devkitARMHelpers )

	# here we're importing the location of the krawerter-binary we're export()-ing below.
	# the "../../.." is not ideal, but it should work as long as nothing changes
	# in the ExternalProject_Add() below.
	# also see http://www.cmake.org/Wiki/CMake_Cross_Compiling#Using_executables_in_the_build_created_during_the_build
	include( ${CMAKE_BINARY_DIR}/../../../ImportExecutables.cmake )

	add_subdirectory( lib )
	add_subdirectory( examples )

else()
	add_subdirectory( krawerter )
	# export the krawerter-target, so we can use it in cross-compiling
	export( TARGETS krawerter FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake )

	set( CROSS_TARGET krawall-cross-gba )
	
	ExternalProject_Add(
		${CROSS_TARGET}
		PREFIX krawall

		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
		CMAKE_ARGS
			-DCMAKE_TOOLCHAIN_FILE:STRING=cmake/Toolchain-devkitARM.cmake
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
			-DKRAWALL_BUILD_ALL_VARIANTS=${KRAWALL_BUILD_ALL_VARIANTS}
			-DKRAWALL_DEFAULT_VARIANT=${KRAWALL_DEFAULT_VARIANT}
	)
	
	add_dependencies( ${CROSS_TARGET} krawerter )

	# In this extra-step we're deleting the build-stamp-file,
	# which causes cmake to rerun the build for this external project every time.
	# Otherwise, modifications in lib/ and examples/ will go undetected.
	# I would expect this to be the default behaviour, but for some reason it isn't.
	
	ExternalProject_Get_Property( ${CROSS_TARGET} STAMP_DIR )
	set( BUILD_STAMP_FILE ${STAMP_DIR}/${CROSS_TARGET}-build )

	#message( STATUS ${BUILD_STAMP_FILE} )

	ExternalProject_Add_Step( ${CROSS_TARGET} DELETE_BUILD_STAMP
		COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_STAMP_FILE}
		DEPENDEES install
	) 
endif()

