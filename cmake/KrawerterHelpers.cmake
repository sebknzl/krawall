function( build_module_library )
	set( MODS_HEADERS instruments.h modules.h samples.h )
	set( MODS_S instruments.S samples.S )
	set( MODS_ABSOLUTE )
	
	foreach( MOD ${ARGN} )
		GET_FILENAME_COMPONENT( BASENAME ${MOD} NAME_WE )
		list( APPEND MODS_S ${BASENAME}.S )
		list( APPEND MODS_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR}/${MOD} )
	endforeach()

	set_source_files_properties( ${MODS_S} PROPERTIES LANGUAGE C ) 

	# specifying "krawerter" here works, because it is imported as a target
	# through the ImportExecutables.cmake in the top-level CMakeLists.txt
	add_custom_command(
		OUTPUT ${MODS_S} ${MODS_HEADERS}
		COMMAND krawerter
		ARGS ${MODS_ABSOLUTE}
		DEPENDS ${MODS}
	)

	add_library( modules ${MODS_S} )

endfunction()
