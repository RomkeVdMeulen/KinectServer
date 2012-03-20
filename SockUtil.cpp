#include "stdafx.h"
#include "SockUtil.h"

// Blatantly stolen from the osgrc codebase

#include <map>

#ifdef WIN32
bool						RuGKinectInterfaceServer::_winsock_initialized = false;
std::map<int, std::string>	RuGKinectInterfaceServer::_winsock_error_strings;

bool
RuGKinectInterfaceServer::initializeWinsock()
{
	// initialize Winsock on windows
	if (!_winsock_initialized)
	{
		WORD version = MAKEWORD(2, 0);
		WSADATA wsaData;
		
		if (WSAStartup(version, &wsaData) != 0)
		{
			cerr << "RuGKinectInterfaceServer::initializeWinsock(): Error initializing Winsock: %d\n", WSAGetLastError();
			return false;
		}
		else
			_winsock_initialized = true;

		_winsock_error_strings[WSAEFAULT] = "(WSAEFAULT) Bad address";
		_winsock_error_strings[WSAEINVAL] = "(WSAEINVAL) Invalid argument";
		_winsock_error_strings[WSAEWOULDBLOCK] = "(WSAEWOULDBLOCK) Resource temporarily unavailable";
		_winsock_error_strings[WSAEINPROGRESS] = "(WSAEINPROGRESS) Operation now in progress";
		_winsock_error_strings[WSAEALREADY] = "(WSAEALREADY) Operation already in progress";
		_winsock_error_strings[WSAEMSGSIZE] = "(WSAEMSGSIZE) Message too long";
		_winsock_error_strings[WSAEADDRINUSE] = "(WSAEADDRINUSE) Address already in use";
		_winsock_error_strings[WSAENETUNREACH] = "(WSAENETUNREACH) Network is unreachable";
		_winsock_error_strings[WSAENETRESET] = "(WSAENETRESET) Network dropped connection on reset";
		_winsock_error_strings[WSAECONNABORTED] = "(WSAECONNABORTED) Software caused connection abort";
		_winsock_error_strings[WSAECONNRESET] = "(WSAECONNRESET) Connection reset by peer";
		_winsock_error_strings[WSAECONNREFUSED] = "(WSAECONNREFUSED) Connection refused";
	}

	return true;
}
#endif	
