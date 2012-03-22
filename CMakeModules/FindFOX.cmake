# Locate gdal
# This module defines
# FOX_LIBRARY
# FOX_FOUND, if false, do not try to link to gdal 
# FOX_INCLUDE_DIR, where to find the headers
#
# $FOX_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FOX_DIR
#
# Created by Robert Osfield. 

FIND_PATH(FOX_INCLUDE_DIR fx.h
	PATH_SUFFIXES
		/fox-1.6/
		/fox-1.6/include/
		/include/
		/include/fox-1.6/
	PATHS
		$ENV{FOX_DIR}
		$ENV{OSGDIR}
		$ENV{OSG_ROOT}
		~/Library/Frameworks/
		/Library/Frameworks/
		/usr/local/
		/usr/include/
		/usr/freeware/
		/opt/local/
		/opt/
		/sw/include/ # Fink
		/opt/csw/include/ # Blastwave
		[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/
		[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]../
		${CMAKE_CURRENT_SOURCE_DIR}/../
)

MACRO(FIND_FOX_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
		PATH_SUFFIXES
			/lib/
			/lib/Win32/
			/fox-1.6/lib/
			/fox-1.6/lib/Win32/
        PATHS
			$ENV{FOX_DIR}
			$ENV{OSGDIR}
			$ENV{OSG_ROOT}
			~/Library/Frameworks
			/Library/Frameworks
			/usr/local/lib
			/usr/lib
			/sw/lib
			/opt/local/lib
			/opt/csw/lib
			/opt/lib
			[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
			[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]../
			/usr/freeware/lib64
			${CMAKE_CURRENT_SOURCE_DIR}/../
    )

ENDMACRO(FIND_FOX_LIBRARY LIBRARY LIBRARYNAME)

FIND_FOX_LIBRARY(FOX_LIBRARY FOX-1.6)

SET(FOX_FOUND "NO")
IF(FOX_LIBRARY AND FOX_INCLUDE_DIR)
    SET(FOX_FOUND "YES")
ENDIF(FOX_LIBRARY AND FOX_INCLUDE_DIR)
