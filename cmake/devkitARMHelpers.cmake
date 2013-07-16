# http://gcc.gnu.org/onlinedocs/gcc/ARM-Options.html
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -fomit-frame-pointer -ffast-math -mcpu=arm7tdmi -mtune=arm7tdmi -mthumb -mthumb-interwork" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=gba.specs" )

function( copytobin_gbafix _TARGET )
	get_target_property( _TARGET_LOCATION ${_TARGET} LOCATION )
#	message( STATUS ${_TARGET_LOCATION} ${_TARGET} )
	set( _TARGET_BIN_LOCATION ${_TARGET_LOCATION}.bin )
	add_custom_command(
		TARGET ${_TARGET}
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${_TARGET_LOCATION} ${_TARGET_BIN_LOCATION}
		COMMAND ${GBAFIX_EXECUTABLE} ${_TARGET_BIN_LOCATION}
	)
endfunction()


