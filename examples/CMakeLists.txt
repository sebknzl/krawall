find_package( Gba REQUIRED )

add_subdirectory( modules )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR} ${GBA_INCLUDE_DIRS} )

foreach( EXAMPLE simple callback jinglesong interrupt )
	add_executable( ${EXAMPLE} ${EXAMPLE}.c )
	target_link_libraries( ${EXAMPLE} krawall modules ${GBA_LIBRARIES} )
	copytobin_gbafix( ${EXAMPLE} )
	install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.bin DESTINATION examples )
endforeach()
