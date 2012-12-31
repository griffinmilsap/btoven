# - Try to find Portaudio
# Once done, this will define
#
#  PORTAUDIO_FOUND - system has Portaudio
#  PORTAUDIO_INCLUDE_DIRS - the Portaudio include directories
#  PORTAUDIO_LIBRARIES - link these to use Portaudio

# Include dir
FIND_PATH( PORTAUDIO_INCLUDE_DIRS
  NAMES portaudio.h
  PATH_SUFFIXES include
  PATHS
  ../extern/portaudio
)

# Finally the library itself
FIND_LIBRARY( PORTAUDIO_LIBRARIES
  NAMES portaudio portaudio_x86
  PATH_SUFFIXES
  lib
  build/msvc/Win32/Debug
  build/msvc/Win32/Release
  bin/Win32/Debug
  bin/Win32/Release
  PATHS
  ../extern/portaudio
)

SET( PORTAUDIO_FOUND 0 )
IF( PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS )
  SET( PORTAUDIO_FOUND 1 )
  MESSAGE( STATUS "Portaudio found!" )
ELSE( PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS )
  MESSAGE( STATUS "Portaudio not found..." )
ENDIF( PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS )