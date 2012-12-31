SET( LIB_SOURCES
	audiobuffer.c
	audioformat.c
	biquad.c
	btoven.c
	comb.c
	error.c
	processing.c
	window.c
	kissfft/kiss_fft.c
	kissfft/kiss_fftr.c
)

INCLUDE_DIRECTORIES( 
	${BTOVEN_SOURCE_DIR}/inc
	${BTOVEN_SOURCE_DIR}/src/kissfft
)

ADD_LIBRARY( ${LIB_NAME} ${LIB_SOURCES} )

IF( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
	TARGET_LINK_LIBRARIES( ${LIB_NAME} m )
ENDIF( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

INSTALL( FILES ${BTOVEN_SOURCE_DIR}/inc/btoven.h DESTINATION include )
INSTALL( TARGETS ${LIB_NAME} ARCHIVE DESTINATION lib )
