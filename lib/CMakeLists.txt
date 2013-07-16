set( TARGET krawall )

# variants:
# iwram-usage: -DIWRAM_USAGE_MEDIUM -DIWRAM_USAGE_SMALL -DDIWRAM_USAGE_LARGE
# 30hz-version: -DMODE30
# 32kHz: -DSAMPLEFREQEXP=15
# naming: <samplefreq>-<vrate>-<iwramusage>
# e.g. 16k-60-medium, 32k-30-large

# this converts names like in the example above to compiler-flags
function( variant_string_to_flags NAME RESULT_VAR )
	string( REPLACE "-" ";" VARIANT_ATTRS ${NAME} )
	list( GET VARIANT_ATTRS 0 SAMPLEFREQ )
	list( GET VARIANT_ATTRS 1 VFREQ )
	list( GET VARIANT_ATTRS 2 IWRAM_USAGE )

	if( SAMPLEFREQ MATCHES 32k )
		set( SAMPLEFREQ_FLAG -DSAMPLEFREQEXP=15 )
	endif()

	string( TOUPPER ${IWRAM_USAGE} IWRAM_USAGE_FLAG )
	set( IWRAM_USAGE_FLAG -DIWRAM_USAGE_${IWRAM_USAGE_FLAG} )

	if( VFREQ MATCHES 30 )
		SET( VFREQ_FLAG -DMODE30 )
	endif()

	set( ${RESULT_VAR} "${IWRAM_USAGE_FLAG} ${SAMPLEFREQ_FLAG} ${VFREQ_FLAG}" PARENT_SCOPE )
endfunction()


set( SOURCES_COMPILE_ARM mixer.arm.c mixer_private.arm.c )
set_source_files_properties( ${SOURCES_COMPILE_ARM} COMPILE_FLAGS -marm )

# We'll have .S-files run through gcc, not as.
# gcc pipes .S-files through cpp (which is what we need) and then runs as.
set( ASM_SOURCES directsound_asm.S mixer_func.S stub.S )
set_source_files_properties( ${ASM_SOURCES} PROPERTIES LANGUAGE C ) 
 
set( LIBRARY_SOURCE_FILES
	player.c player.h
	directsound.c directsound.h directsound-def.h
	general.c general.h
	mixer.c mixer.h
	player_tables.c player_tables.h
	player.c player.h
	
	${SOURCES_COMPILE_ARM}
	${ASM_SOURCES}
	
	regs.h effects.h mtypes.h player_effects.h tools.h types.h
	krawall.h
)

# default:
add_library( ${TARGET}
	${LIBRARY_SOURCE_FILES}
)

if( NOT KRAWALL_DEFAULT_VARIANT )
	set( KRAWALL_DEFAULT_VARIANT "16k-60-medium" )
endif()

message( STATUS "Building variant ${KRAWALL_DEFAULT_VARIANT}" )

variant_string_to_flags( "${KRAWALL_DEFAULT_VARIANT}" FLAGS )
set_target_properties( ${TARGET} PROPERTIES COMPILE_FLAGS ${FLAGS} )

set( KRAWALL_LIBRARIES ${TARGET} )

# should all variants be built?
if( KRAWALL_BUILD_ALL_VARIANTS )
	foreach( IWRAM_USAGE small medium large )
		foreach( SAMPLEFREQ 16k 32k )
			foreach( VFREQ 30 60 )
				# make up target name
				set( VTARGET krawall-${SAMPLEFREQ}-${VFREQ}-${IWRAM_USAGE} )
				add_library( ${VTARGET} ${LIBRARY_SOURCE_FILES} )
				# get flags and set them on this target
				variant_string_to_flags( "${VTARGET}" FLAGS )
				set_target_properties( ${VTARGET}
					PROPERTIES COMPILE_FLAGS ${FLAGS}
				)
				list( APPEND KRAWALL_LIBRARIES ${VTARGET} )
			endforeach()
		endforeach()
	endforeach()
endif()


# generate krawall.h from these headers
set( EXPORT_HEADERS general.h player.h mixer.h directsound.h )

string( REPLACE ";" "\\;" HEADER_FILES_ESCAPED "${EXPORT_HEADERS}" )
add_custom_command(
	OUTPUT krawall.h
	COMMAND ${CMAKE_COMMAND}
	ARGS -D FILES="${HEADER_FILES_ESCAPED}" -D OUTPUT=${CMAKE_CURRENT_BINARY_DIR}/krawall.h -P ${CMAKE_CURRENT_SOURCE_DIR}/genheader.cmake
	DEPENDS ${EXPORT_HEADERS} genheader.pre genheader.post genheader.cmake
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install( TARGETS ${KRAWALL_LIBRARIES} ARCHIVE DESTINATION lib )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/krawall.h DESTINATION include )
