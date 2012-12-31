# - Try to find mpg123
# Once done, this will define
#
#  MPG123_FOUND - system has mpg123
#  MPG123_INCLUDE_DIRS - the mpg123 include directories
#  MPG123_LIBRARIES - link these to use mpg123

# Include dir
FIND_PATH( MPG123_INCLUDE_DIR1 
  NAMES mpg123.h 
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  extern/mpg123/ports/MSVC++
  extern/mpg123/ports/Xcode
)

IF( MPG123_INCLUDE_DIR1 )
  SET( MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIRS} ${MPG123_INCLUDE_DIR1} )
ENDIF( MPG123_INCLUDE_DIR1 )

# Include dir (May not be necessary on all platforms)
FIND_PATH( MPG123_INCLUDE_DIR2 
  NAMES mpg123.h.in 
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  extern/mpg123/src/libmpg123
)

IF( MPG123_INCLUDE_DIR2 )
  SET( MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIRS} ${MPG123_INCLUDE_DIR2} )
ENDIF( MPG123_INCLUDE_DIR2 )

# MESSAGE( "MPG123_INCLUDE_DIR1: " ${MPG123_INCLUDE_DIR1} )
# MESSAGE( "MPG123_INCLUDE_DIR2: " ${MPG123_INCLUDE_DIR2} )
# MESSAGE( "MPG123_INCLUDE_DIRS: " ${MPG123_INCLUDE_DIRS} )

FIND_LIBRARY( MPG123_LIBRARIES 
  NAMES mpg123 libmpg123.lib
  HINTS
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 Release Debug
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  extern/mpg123/ports/MSVC++/2005
  extern/mpg123/ports/MSVC++/2008
  extern/mpg123/ports/MSVC++/2008clr
  extern/mpg123/ports/MSVC++/2010
)

SET( MPG123_FOUND 0 )
IF( MPG123_LIBRARIES AND MPG123_INCLUDE_DIRS )
  SET( MPG123_FOUND 1 )
  MESSAGE( STATUS "mpg123 found!" )
ELSE( MPG123_LIBRARIES AND MPG123_INCLUDE_DIRS )
  MESSAGE( STATUS "mpg123 not found..." )
ENDIF( MPG123_LIBRARIES AND MPG123_INCLUDE_DIRS )
