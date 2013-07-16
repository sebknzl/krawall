#message( STATUS ${OUTPUT} )

file( READ genheader.pre PRE )
file( READ genheader.post POST )

file( WRITE ${OUTPUT} "${PRE}" )

foreach( FILE ${FILES} )
	file( READ ${FILE} DATA )
	string( REGEX REPLACE ".*%b(.*)//%e.*" "\\1" HAM "${DATA}" )
	file( APPEND ${OUTPUT} "${HAM}" )
endforeach()

file( APPEND ${OUTPUT} "${POST}" )
