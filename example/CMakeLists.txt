# btoven

# The C example requires mpg123 and portaudio to function
SET( EX_NAME btoven_c_example )
SET( EX_SOURCES btoven_c_example.c )

IF( MPG123_FOUND AND PORTAUDIO_FOUND )
	FIND_PACKAGE( BTOVEN )
	IF( BTOVEN_FOUND )
		MESSAGE( STATUS "Found btoven installed in path; using installed copy" )
		INCLUDE_DIRECTORIES( ${BTOVEN_INCLUDE_DIRS} )
	ELSE( BTOVEN_FOUND )
		MESSAGE( STATUS "Did not find btoven installed in path.  Try building the INSTALL target!" )
		INCLUDE_DIRECTORIES( ${BTOVEN_SOURCE_DIR}/inc )
		LINK_DIRECTORIES( ${BTOVEN_BINARY_DIR}/lib )
	ENDIF( BTOVEN_FOUND )
	
	INCLUDE_DIRECTORIES( ${MPG123_INCLUDE_DIRS} ${PORTAUDIO_INCLUDE_DIRS} )

	ADD_EXECUTABLE( ${EX_NAME} ${EX_SOURCES} )

	TARGET_LINK_LIBRARIES( ${EX_NAME} ${LIB_NAME} ${MPG123_LIBRARIES} ${PORTAUDIO_LIBRARIES} )
ELSE( MPG123_FOUND AND PORTAUDIO_FOUND )
	MESSAGE( STATUS "Could not find both MPG123 and Portaudio; " )
	MESSAGE( STATUS "Build files for " ${EX_NAME} " will not be generated." )
ENDIF( MPG123_FOUND AND PORTAUDIO_FOUND )
