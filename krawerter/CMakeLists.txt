set( TARGET krawerter )
 
add_executable( ${TARGET}
	main.cpp
	Exception.h
	Instrument.cpp Instrument.h
	Mod.cpp Mod.h
	ModS3M.cpp ModS3M.h
	ModXM.cpp ModXM.h
	Pattern.cpp Pattern.h
	Sample.cpp Sample.h
	effects.h
	types.h
)
