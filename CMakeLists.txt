IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
ELSE(WIN32)
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning: A critical CMake bug exists in 2.4.6 and below. Trying to build Universal Binaries will result in a compile error that seems unrelated. Either avoid building Universal Binaries by changing the CMAKE_OSX_ARCHITECTURES field to list only your architecture, or upgrade to the current CVS version of CMake or a newer stable version if it exists.")
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ELSE(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0 FATAL_ERROR)
    ENDIF(APPLE)
ENDIF(WIN32)

if(COMMAND cmake_policy)
    # Works around warnings libraries linked against that don't
    # have absolute paths (e.g. -lpthreads)
    cmake_policy(SET CMP0003 NEW)

    # Works around warnings about escaped quotes in ADD_DEFINITIONS
    # statements.
    cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

PROJECT(KinectDataServer)

set(APP_NAME KinectDataServer)

SET(CMAKE_MODULE_PATH "${KinectDataServer_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
INCLUDE(OsgMacroUtils)

IF(UNIX)
    # Some Unicies need explicit linkage to the Math library or the build fails.
    FIND_LIBRARY(MATH_LIBRARY m)
ENDIF(UNIX)

# Common to all platforms:
SET(OSG_DIR "" CACHE PATH "set to base osg install path")

SET(CMAKE_DEBUG_POSTFIX  "d")

FIND_PACKAGE(OSG)

FIND_PACKAGE(FOX)

FIND_PACKAGE(OpenThreads)

IF(NOT DEFINED LIB_POSTFIX)
    SET(LIB_POSTFIX "")
ENDIF(NOT DEFINED LIB_POSTFIX)

SET(GROUP_NETWORKING
BasicTypes.h
Communicator.h
Communicator.cpp
MessageStream.cpp
MessageStream.h
Protocol.h
SockUtil.cpp
SockUtil.h
StructuredStream.cpp
StructuredStream.h
StructuredStreamView.cpp
StructuredStreamView.h
TCPSocket.cpp
TCPSocket.h
)

SET(GROUP_HEADERS
BasicCommunicator.h
BasicConnectionServer.h
CalibrationWindow.h
CommunicatorThread.h
DataMutex.h
KinectConnector.h
NuiData.h
Player.h
Recorder.h
RequestListener.h
RequestProcessor.h
Server.h
ServerThread.h
Skeleton.h
stdafx.h
Vector3.h
)

SET(GROUP_SOURCES
BasicCommunicator.cpp
BasicConnectionServer.cpp
CalibrationWindow.cpp
CommunicatorThread.cpp
KinectConnector.cpp
main.cpp
Player.h
Recorder.cpp
RequestListener.cpp
RequestProcessor.cpp
Server.cpp
ServerThread.cpp
Skeleton.cpp
stdafx.cpp
Vector3.cpp
)

source_group("Networking" FILES ${GROUP_NETWORKING})
source_group("Header Files" FILES ${GROUP_HEADERS})
source_group("Source Files" FILES ${GROUP_HEADERS})

SET(TARGET_NAME KinectDataServer)
SET(TARGET_H
BasicCommunicator.h
BasicConnectionServer.h
BasicTypes.h
CalibrationWindow.h
Communicator.h
CommunicatorThread.h
DataMutex.h
KinectConnector.h
MessageStream.h
NuiData.h
Protocol.h
Player.h
Recorder.h
RequestListener.h
RequestProcessor.h
Server.h
ServerThread.h
Skeleton.h
SockUtil.h
stdafx.h
StructuredStream.h
StructuredStreamView.h
TCPSocket.h
Vector3.h
)
SET(TARGET_SRC
BasicCommunicator.cpp
BasicConnectionServer.cpp
CalibrationWindow.cpp
Communicator.cpp
CommunicatorThread.cpp
KinectConnector.cpp
main.cpp
MessageStream.cpp
Player.cpp
Recorder.cpp
RequestListener.cpp
RequestProcessor.cpp
Server.cpp
ServerThread.cpp
Skeleton.cpp
SockUtil.cpp
stdafx.cpp
StructuredStream.cpp
StructuredStreamView.cpp
TCPSocket.cpp
Vector3.cpp
CMakeLists.txt
ReadMe.txt
)

INCLUDE_DIRECTORIES(${FOX_INCLUDE_DIR} ${OSG_INCLUDE_DIRS} ${OPENTHREADS_INCLUDE_DIR})

set(TARGET_TARGETNAME)
set(TARGET_LABEL)

get_filename_component(FOX_LIBRARY_DIR ${FOX_LIBRARY} PATH)
get_filename_component(osgd.lib ${OSG_LIBRARY} PATH)

link_directories(${OSG_LIBRARY_DIR} ${FOX_LIBRARY_DIR})

SETUP_EXE(1)

LINK_WITH_DEBUG_POSTFIX(${TARGET_NAME} ${OSG_LIBRARY} ${OPENTHREADS_LIBRARY})

#LINK_WITH_DEBUG_POSTFIX(${TARGET_NAME} ${FOX_LIBRARY})
######## JUST LINK WITH FIXED NAME.
set(FOX_LIBRARY_DEBUG FOXD-1.6)

TARGET_LINK_LIBRARIES(${TARGET_NAME} optimized ${FOX_LIBRARY} debug ${FOX_LIBRARY_DEBUG})

IF (WIN32)
	set(KINECTAPI_DIR C:/Program Files/Microsoft SDKs/Kinect/v1.0)
	FIND_PACKAGE(KINECTAPI)
	get_filename_component(KINECTAPI_LIBRARY_DIR ${KINECTAPI_LIBRARY} PATH)
	INCLUDE_DIRECTORIES(${KINECTAPI_INCLUDE_DIR})
	LINK_EXTERNAL(${TARGET_NAME} ${KINECTAPI_LIBRARY})
ENDIF(WIN32)
