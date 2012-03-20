#include "stdafx.h"

#ifndef SOCKUTIL_H
#define SOCKUTIL_H

// Blatantly stolen from the osgrc codebase

#ifndef NDEBUG
// Wether to use assert() to check a socket's state when a method is
// called (SOCKET_USE_ASSERTS defined) or wether to print an error
// message and return early (SOCKET_USE_ASSERTS not defined).
//#define SOCKET_USE_ASSERTS
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
	#pragma comment(lib, "wsock32.lib")

	#include <winsock.h>

	// XXX needs winsock 2 for these to be defined properly

	#ifndef SD_RECEIVE
	#define SD_RECEIVE      0x00
	#endif

	#ifndef SD_SEND
	#define SD_SEND         0x01
	#endif

	#ifndef SD_BOTH
	#define SD_BOTH         0x02
	#endif

	#include <map>
#elif defined(__sgi)
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <errno.h>
#else
	#include <netinet/in.h>
	#include <errno.h>
#endif

namespace RuGKinectInterfaceServer {

#ifdef WIN32

	typedef SOCKET socket_t;
	typedef SOCKADDR_IN sockaddr_in_t;
	typedef struct sockaddr sockaddr_t;
	typedef int socklen_t;
	
	#define SOCKERR(val)			((val) == SOCKET_ERROR)

	#define ERRORCODE				WSAGetLastError()
	#define NOERROR					0

	#define SELHIGHEST(s)			0

#else 
	
	// not WIN32

	typedef int socket_t;
	typedef struct sockaddr_in sockaddr_in_t;
	typedef struct sockaddr sockaddr_t;
	
	#define SOCKERR(val)			((val) == -1)

	#define ERRORCODE				errno
	#define NOERROR					0

	#define SELHIGHEST(s)			((s)+1)

#endif // ARCH

#ifdef WIN32
extern bool							_winsock_initialized;
extern std::map<int, std::string>	_winsock_error_strings;

extern bool							initializeWinsock();
#endif

}

#endif
