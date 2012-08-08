# btoven
# Motalen 2011

# The C example requires mpg123 and portaudio to function
SET( EX_NAME btoven_c_example )
SET( EX_SOURCES btoven_c_example.c )

INCLUDE_DIRECTORIES( ${BTOVEN_SOURCE_DIR}/inc )
LINK_DIRECTORIES( ${BTOVEN_BINARY_DIR}/lib )

ADD_EXECUTABLE( ${EX_NAME} ${EX_SOURCES} )

TARGET_LINK_LIBRARIES( ${EX_NAME} ${LIB_NAME} mpg123 portaudio )
