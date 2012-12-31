################################################################################
## $Id$
## Author: griffin.milsap@gmail.com, justin.renga@gmail.com
## Btoven
##
## Copyright (c) 2012, Authors
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## Redistributions of source code must retain the above copyright notice, this
## list of conditions and the following disclaimer.
##
## Redistributions in binary form must reproduce the above copyright notice,
## this list of conditions and the following disclaimer in the documentation
## and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
## THE POSSIBILITY OF SUCH DAMAGE.
################################################################################

# - Try to find btoven
# Once done this will define
#  BTOVEN_FOUND - System has btoven
#  BTOVEN_INCLUDE_DIRS - The btoven include directories
#  BTOVEN_LIBRARIES - The libraries needed to use btoven
#  BTOVEN_DEFINITIONS - Compiler switches required for using btoven

#Search for the include file...
FIND_PATH( BTOVEN_INCLUDE_DIRS btoven.h DOC "Path to btoven include directory."
	HINTS
	$ENV{BTOVEN_ROOT}
	PATH_SUFFIX include
	PATHS
	/usr
	/usr/local
)

FIND_LIBRARY( BTOVEN_LIBRARIES DOC "Absolute path to btoven library."
	NAMES libbtoven.a libbtoven btoven
	HINTS
	$ENV{BTOVEN_ROOT}
	PATH_SUFFIXES lib lib64 libs libs64 libs/Win32 libs/Win64
	PATHS
	/usr/local/lib
	/usr/lib
)

SET( BTOVEN_FOUND 0 )
IF( BTOVEN_LIBRARIES AND BTOVEN_INCLUDE_DIRS )
	SET( BTOVEN_FOUND 1 )
	MESSAGE( STATUS "btoven found!" )
ELSE( BTOVEN_LIBRARIES AND BTOVEN_INCLUDE_DIRS )
	MESSAGE( STATUS "btoven not found..." )
ENDIF( BTOVEN_LIBRARIES AND BTOVEN_INCLUDE_DIRS )