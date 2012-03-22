# Locate OpenCV
# This module defines
# OPENCV_LIBRARY
# OPENCV_INCLUDE,
# Created by Pjotr Svetachov.
# Modified by Romke van der Meulen

IF(WIN32)

	FIND_PATH(OPENCV_INCLUDE opencv/cv.h
		PATH_SUFFIXES
			/build/include/
			/include/
		PATHS
			$ENV{OPENCV_DIR}
			$ENV{OPENCVDIR}
			"C:/Program Files/opencv/"
			"C:/Program Files (x86)/opencv/"
	)

	MACRO(FIND_OPENCV_LIBRARY MYLIBRARY MYLIBRARYNAME)

		FIND_LIBRARY(${MYLIBRARY}
			NAMES ${MYLIBRARYNAME}
			PATH_SUFFIXES
				/lib/
				/x86/vc10/lib/
			PATHS
				$ENV{OPENCV_DIR}
				$ENV{OPENCVDIR}
				${OPENCV_INCLUDE}/../
		)

	ENDMACRO(FIND_OPENCV_LIBRARY MYLIBRARY MYLIBRARYNAME)

ELSE(WIN32)

	FIND_PATH(OPENCV_INCLUDE cvconfig.h
		PATH_SUFFIXES
			/include/
			/opencv230/include/
			/opencv/include/
		PATHS
			~/Library/Frameworks
			/Library/Frameworks
			/usr/local
			/usr
			/sw # Fink
			/opt/local # DarwinPorts
			/opt/csw # Blastwave
			/opt
			/usr/freeware
			$ENV{OPENCV_DIR}
			$ENV{OPENCVDIR}
	)

	MACRO(FIND_OPENCV_LIBRARY MYLIBRARY MYLIBRARYNAME)

		FIND_LIBRARY(${MYLIBRARY}
			NAMES ${MYLIBRARYNAME}
			PATH_SUFFIXES
				/lib/
				/lib64/
				/opencv230/lib/
				/opencv230/lib64/
				/opencv/lib/
				/opencv/lib64/
			PATHS
				~/Library/Frameworks
				/Library/Frameworks
				/usr/local
				/usr
				/sw # Fink
				/opt/local # DarwinPorts
				/opt/csw # Blastwave
				/opt
				/usr/freeware
				$ENV{OPENCV_DIR}
				$ENV{OPENCVDIR}
		)

	ENDMACRO(FIND_OPENCV_LIBRARY MYLIBRARY MYLIBRARYNAME)

ENDIF(WIN32)

FIND_OPENCV_LIBRARY(OPENCV_LIBRARY_CORE			opencv_core231)
FIND_OPENCV_LIBRARY(OPENCV_LIBRARY_CALIB3D		opencv_calib3d231)

SET(OPENCV_FOUND "NO")
IF(OPENCV_LIBRARY_CORE AND OPENCV_INCLUDE)
    SET(OPENCV_FOUND "YES")
ENDIF(OPENCV_LIBRARY_CORE AND OPENCV_INCLUDE)
