// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define PROGRAM_NAME "RuG Kinect Interface - Server"
#define VENDOR "RuG Visualization Centre"
#define WINDOW_TITLE PROGRAM_NAME
#define VERSION "1.0.0"
#define COPYRIGHT_NOTICE "Copyright 2012 Romke van der Meulen"

#define APPLY_SMOOTHING 1

#define REQUEST_PORT 1986
#define ALLOW_REMOTE_REQUESTS 1
#define LISTEN_FOR_REQUESTS_EVERY_MS 10
#define LISTEN_FOR_KINECT_SIGNALS_EVERY_MS 100
#define SERVER_THREADS_LISTEN_FOR_SIGNALS_EVERY_MS 100

// Exit values
#define NORMAL_EXIT 0								// Program completed normally
#define UNSPECIFIED_ERROR 1							// Some unspecified error occurred, see error stream
#define MULTIPLE_INSTANCES_RUNNING 2				// Only one instance of the program should run at any time
#define CONSOLE_HANDLER_REGISTRATION_ERROR 3		// Could not register the console handler
#define KINECT_CONNECTION_ERROR 4					// Could not connect to the Kinect sensor
#define INVALID_FILE 5								// A file specified through command arguments is invalid
#define INVALID_PROGRAM_USE 6						// Invalid use of the program (e.g. trying to get live data on non-windows platform)

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <string>
using namespace std;

#ifdef WIN32
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include "windows.h"
#include <ole2.h>
#include <tchar.h>
#else

typedef void * LPVOID;
typedef void * HANDLE;
typedef unsigned long DWORD;
typedef float FLOAT;
typedef bool BOOL;
#define WINAPI


#endif
