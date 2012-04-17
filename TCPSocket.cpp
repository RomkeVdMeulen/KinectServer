#include "stdafx.h"

// Blatantly stolen from the osgrc codebase

#ifdef __sgi
#include <stdio.h>
#include <string.h>
#include <assert.h>
#else
#include <cstdio>
#include <cstring>
#include <cassert>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>

#ifndef WIN32
	#include <sys/ioctl.h>
	#include <sys/uio.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <net/if.h>
	#include <netdb.h>
#endif

#if defined(__linux)
    #include <unistd.h>
    #include <linux/sockios.h>
    #include <errno.h>
#elif defined(__FreeBSD__)
    #include <unistd.h>
    #include <sys/sockio.h>
#elif defined(__sgi)
    #include <unistd.h>
    #include <net/soioctl.h>
#elif defined(__CYGWIN__)
    #include <unistd.h>
#elif defined(__sun)
    #include <unistd.h>
    #include <sys/sockio.h>
#elif defined (__APPLE__)
    #include <unistd.h>
    #include <sys/sockio.h>
#elif defined (WIN32)
    #include <winsock.h>
    #include <cstdio>
	#define snprintf _snprintf
	#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) // Microsoft visual studio, version 2005 and higher.
	  #if defined( _WINSOCKAPI_ ) && !defined( _WINSOCK2API_ )
		#undef FD_SET //fix warning C4127: conditional expression is constant ; caused by FD_SET in winsock.h
		#define FD_SET(fd, set) { \
			if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
				((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=(fd); }
	  #endif
	#endif// Microsoft visual studio, version 2005 and higher.
#elif defined (__hpux__)
    #include <unistd.h>
#else
    #error Teach me how to build on this system
#endif

#ifdef WSAECONNRESET
	#define DBG_REPORTERR(proc,msg)		\
	{ \
		int errorcode = WSAGetLastError(); \
		if (errorcode != WSAECONNRESET) { \
			if (_winsock_error_strings.find(errorcode) != _winsock_error_strings.end()) \
			{ \
				cerr << "\n### " << proc << ": " << std::string(msg) << " - " << _winsock_error_strings[errorcode] << " ###\n"; \
			} \
			else \
			{ \
				cerr << "\n### " << proc << ": " << std::string(msg) << " - (Winsock error " << errorcode << ") ###\n"; \
			} \
		} \
	}
#else
	#define DBG_REPORTERR(proc,msg)		\
	{ \
		cerr << "\n### " << proc << ": " << std::string(msg) << " (error code " << _errorcode << ") ###\n"; \
	}
#endif

#include "TCPSocket.h"

using namespace RuGKinectInterfaceServer;
///////////////DEBUG
#ifdef RAW_TCP_DEBUG
void printtolog(const char *buffer, unsigned int amount) {
		unsigned int logged_bytes = 0;
		while (amount >= 16)	{
			logWARNING("%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]),(0xFF & buffer[logged_bytes + 10]),(0xFF & buffer[logged_bytes + 11]),
				(0xFF & buffer[logged_bytes + 12]),(0xFF & buffer[logged_bytes + 13]),(0xFF & buffer[logged_bytes + 14]),(0xFF & buffer[logged_bytes + 15]));
			amount -= 16;
			logged_bytes+=16;
		}
		switch (amount) {
			case 15:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]),(0xFF & buffer[logged_bytes + 10]),(0xFF & buffer[logged_bytes + 11]),
				(0xFF & buffer[logged_bytes + 12]),(0xFF & buffer[logged_bytes + 13]),(0xFF & buffer[logged_bytes + 14]));
				break;
			case 14:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]),(0xFF & buffer[logged_bytes + 10]),(0xFF & buffer[logged_bytes + 11]),
				(0xFF & buffer[logged_bytes + 12]),(0xFF & buffer[logged_bytes + 13]));
				break;
			case 13:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]),(0xFF & buffer[logged_bytes + 10]),(0xFF & buffer[logged_bytes + 11]),
				(0xFF & buffer[logged_bytes + 12]));
				break;
			case 12:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]),(0xFF & buffer[logged_bytes + 10]),(0xFF & buffer[logged_bytes + 11]));
				break;
			case 11:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]),(0xFF & buffer[logged_bytes + 10]));
				break;
			case 10:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]),(0xFF & buffer[logged_bytes + 9]));
				break;
			case 9:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]),
				(0xFF & buffer[logged_bytes + 8]));
				break;
			case 8:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]),(0xFF & buffer[logged_bytes + 7]));
				break;
			case 7:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]),(0xFF & buffer[logged_bytes + 6]));
				break;
			case 6:
				logWARNING("%02d: %2X%2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]),(0xFF & buffer[logged_bytes + 5]));
				break;
			case 5:
				logWARNING("%02d: %2X%2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]),
				(0xFF & buffer[logged_bytes + 4]));
				break;
			case 4:
				logWARNING("%02d: %2X%2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]),(0xFF & buffer[logged_bytes + 3]));
				break;
			case 3:
				logWARNING("%02d: %2X%2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]),(0xFF & buffer[logged_bytes + 2]));
				break;
			case 2:
				logWARNING("%02d: %2X%2X",amount,
				(0xFF & buffer[logged_bytes + 0]),(0xFF & buffer[logged_bytes + 1]));
				break;
			case 1:
				logWARNING("%02d: %2X",amount,
				buffer[logged_bytes]);
				break;
			case 0:
				logWARNING("%02d: -",amount);
				break;
		}
}
#endif
///////////////DEBUG END
TCPSocket::TCPSocket()
{
#ifdef WIN32
	if (!_winsock_initialized)
		initializeWinsock();
#endif

	_localAddress = 0;
	_localPort = 0;

    _remotePort = 0;
	_remoteAddress = 0;

	_realized = false;
	_bound = false;
	_listening = false;
	_connected = false;
}

TCPSocket::TCPSocket(socket_t socket, sockaddr_in_t remoteSockAddr, socklen_t remoteSockAddrLen)
{
	_socket = socket;

	memcpy (&_remoteSockAddr, &remoteSockAddr, remoteSockAddrLen);

	_realized = true;

	_connected = true;
	_bound = false;
	_listening = false;

	_localAddress = 0;
	_localPort = 0;

    _remotePort = 0;
	_remoteAddress = 0;
}

TCPSocket::~TCPSocket()
{
	if (_connected)
		disconnect();
	
	if (_realized)
		close();
}

bool
TCPSocket::realize()
{
#ifdef SOCKET_USE_ASSERTS
	assert (!_realized);
#else
	if (_realized)
	{
		cerr << "\n### TCPSocket::realize(): socket already realized! ###\n";
		return true;
	}
#endif

	if (!_realized)
	{
		if( (_socket = ::socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	    {
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::realize()", "Socket creation failed")
			return false;
	    }

	    _realized = true;
	}

	return true;
}

bool
TCPSocket::setBooleanSockOpt(int level, int option, bool value)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::setBooleanSockOpt(): socket not realized!";
		return false;
	}
#endif

#ifdef WIN32
    BOOL on = (value ? TRUE : FALSE);
	int res = ::setsockopt(_socket, level, option, (const char *)&on, sizeof(BOOL));
#else
	int on = (value ? 1 : 0);
	int res = ::setsockopt(_socket, level, option, &on, sizeof(int));
#endif

	if (SOCKERR(res))
	{
		DBG_REPORTERR("TCPSocket::setBooleanSockOpt()", "Setting socket option failed")
		_errorcode = ERRORCODE;
		return false;
	}

	return true;
}

bool
TCPSocket::setIntegerSockOpt(int level, int option, int value)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::setIntegerSockOpt(): socket not realized!";
		return false;
	}
#endif

	int res = ::setsockopt(_socket, level, option, (const char*)&value, sizeof(int));

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::setIntegerSockOpt()", "::setsockopt() failed")
		return false;
	}

	return true;
}

bool
TCPSocket::getBooleanSockOpt(int level, int option)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::getBooleanSockOpt(): socket not realized!";
		return false;
	}
#endif

#ifdef WIN32
	BOOL		optval;
	int			optlen;
#else
	int			optval;
	socklen_t	optlen;
#endif

	// FIXME: return value on error

	optlen = sizeof(optval);
	int res = ::getsockopt(_socket, level, option, (char*)&optval, &optlen);

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::getBooleanSockOpt()", "::getsockopt() failed")
		return false;			// XXX false means error here :-/
	}

	return (optval == 1);
}

int
TCPSocket::getIntegerSockOpt(int level, int option)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::getIntegerSockOpt(): socket not realized!";
		return false;
	}
#endif

	int		optval;
#ifdef WIN32
	int		optlen;
#else
	socklen_t	optlen;
#endif

	// FIXME: return value on error

	optlen = sizeof(optval);
	int res = ::getsockopt(_socket, level, option, (char*)&optval, &optlen);

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::getIntegerSockOpt()", "::getsockopt() failed")
		return -1;
	}

	return optval;
}

bool
TCPSocket::setBlocking(bool value)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::setBlocking(): socket not realized!";
		return false;
	}
#endif

	int res;

#ifdef WIN32
	unsigned long nonblockingmode = (value ? 0 : 1);
	res = ::ioctlsocket(_socket, FIONBIO, &nonblockingmode);
#else
	res = ::fcntl(_socket, F_GETFL, 0);
	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::setBlocking()", "Getting socket mode using ::fcntl() failed")
		return false;
	}

	// current flags now in 'res'

	if (value)
		// Set to blocking
		res = ::fcntl(_socket, F_SETFL, res | O_NONBLOCK);
	else
		// Set to non-blocking
		res = ::fcntl(_socket, F_SETFL, res & (~O_NONBLOCK));
#endif

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::setBlocking()", "Setting socket blocking mode failed")
		return false;
	}

	return true;
}

bool
TCPSocket::shutdown(bool forRead, bool forWrite)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::shutdown(): socket not connected!";
		return false;
	}
#endif

	int how;

#ifdef WIN32
	if (forRead && !forWrite)
		how = SD_RECEIVE;
	else if (forWrite && !forRead)
		how = SD_SEND;
	else
		how = SD_BOTH;
#else
	if (forRead && !forWrite)
		how = SHUT_RD;
	else if (forWrite && !forRead)
		how = SHUT_WR;
	else
		how = SHUT_RDWR;
#endif

	int res = ::shutdown(_socket, how);

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::shutdown()", "Socket shutdown failed")
		return false;
	}

	return true;
}

bool
TCPSocket::close()
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::close(): socket not realized!";
		return false;
	}
#endif

	int res;

#ifdef WIN32
	res = ::closesocket(_socket);
#else
	res = ::close(_socket);
#endif

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::close()", "Error returned when attempting to close socket")
		return false;
	}

	_realized = false;
	_bound = false;
	_listening = false;
	_connected = false;

	return true;
}

bool
TCPSocket::bind(const short localPort, const std::string& localAddress)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
	assert (!_bound);
	assert (localPort > 1024);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::bind(): socket not realized!";
		return false;
	}
	if (_bound)
	{
		cerr << "TCPSocket::bind(): socket already bound!";
		return false;
	}
	if (localPort <= 1024)
	{
		cerr << "TCPSocket::bind(): localport (" << localPort << ") <= 1024!";
		return false;
	}
#endif

	_localPort = localPort;

   	if (localAddress.size() > 0)
   	{
		struct hostent *h;
		if( (h = gethostbyname(localAddress.c_str())) == NULL )
    	{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::bind()", "Cannot resolve an address for given hostname '"+localAddress+"'")
			_localAddress = 0;
			return false;
    	}

   		_localAddress = *((unsigned long *)h->h_addr);
   	}
   	else
   		_localAddress = htonl(INADDR_ANY);

	if (!setBooleanSockOpt(SOL_SOCKET, SO_REUSEADDR, true))
	{
		DBG_REPORTERR("TCPSocket::bind()", "Can't set SO_REUSEADDR flag on socket")
		return false;
	}

    _localSockAddr.sin_family = AF_INET;
   	_localSockAddr.sin_port = htons(_localPort);
	_localSockAddr.sin_addr.s_addr = _localAddress;

	int res = ::bind(_socket, (const sockaddr*)&_localSockAddr, sizeof(_localSockAddr));

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::bind()", "Could not bind to local address")
		return false;
	}

    _bound = true;

    return true;
}

bool
TCPSocket::listen(int backlog)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_bound);
	assert (!_connected);
#else
	if (!_bound)
	{
		cerr << "TCPSocket::listen(): socket is not bound!";
		return false;
	}
	if (_connected)
	{
		cerr << "TCPSocket::listen(): socket is connected!";
		return false;
	}
#endif

	int res = ::listen(_socket, backlog);

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::listen()", "Could not ::listen()")
		return false;
	}

	_listening = true;

	return true;
}

TCPSocket*
TCPSocket::accept()
{
#ifdef SOCKET_USE_ASSERTS
	assert (_bound);
	assert (_listening);
	assert (!_connected);
#else
	if (!_bound)
	{
		cerr << "TCPSocket::accept(): socket is not bound!";
		return NULL;
	}
	if (!_listening)
	{
		cerr << "TCPSocket::accept(): socket is not listening!";
		return NULL;
	}
	if (_connected)
	{
		cerr << "TCPSocket::accept(): socket is connected!";
		return NULL;
	}
#endif

	sockaddr_in_t	remoteSockAddr;

	socklen_t	len = sizeof(remoteSockAddr);

#ifdef WIN32
	SOCKET res = ::accept(_socket, (struct sockaddr *)&remoteSockAddr, &len);
#else
	int res = ::accept(_socket, (struct sockaddr *)&remoteSockAddr, &len);
#endif

#ifdef WIN32
	if (res == INVALID_SOCKET)
#else
	if (SOCKERR(res))
#endif
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::accept()", "Call to ::accept() failed")
		return NULL;
	}

	TCPSocket*	newSocket = new TCPSocket(res, remoteSockAddr, len);

	return newSocket;
}

bool
TCPSocket::connect(const std::string& destAddress, const short destPort)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
	assert (!_connected);

	assert (destAddress.size() > 0);
	assert (destPort > 1024);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::connect(const std::string&, const short): socket is not realized!";
		return false;
	}
	if (_connected)
	{
		cerr << "TCPSocket::connect(const std::string&, const short): socket is already connected!";
		return false;
	}

	if (destAddress.size() == 0)
	{
		cerr << "TCPSocket::connect(const std::string&, const short): empty destAddress given";
		return false;
	}
	if (destPort <= 1024)
	{
		cerr << "TCPSocket::connect(const std::string&, const short): destPort <= 1024";
		return false;
	}
#endif

	struct hostent *h;
	if( (h = gethostbyname(destAddress.c_str())) == NULL )
    {
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::connect(const std::string&, const short)", "Cannot resolve an address for given hostname '"+destAddress+"'");
		_remoteAddress = 0;
		return false;
    }

    _remoteAddress = *((unsigned long *)h->h_addr);
	_remotePort = destPort;

	_remoteSockAddr.sin_family = AF_INET;
	_remoteSockAddr.sin_addr.s_addr = _remoteAddress;
	_remoteSockAddr.sin_port = htons(_remotePort);

	unsigned char *ptr = (unsigned char *)&_remoteAddress;

	int res = ::connect(_socket, (struct sockaddr *)&_remoteSockAddr, sizeof(_remoteSockAddr));

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::connect(const std::string&, const short)", "Could not connect")
		return false;
	}

	_connected = true;

	return true;
}

bool
TCPSocket::connect(sockaddr_in_t addr)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_realized);
	assert (!_connected);
#else
	if (!_realized)
	{
		cerr << "TCPSocket::connect(sockaddr_in_t): socket is not realized!";
		return false;
	}
	if (_connected)
	{
		cerr << "TCPSocket::connect(sockaddr_in_t): socket is already connected!";
		return false;
	}
#endif

    _remoteSockAddr = addr;
	_remotePort = ntohs(addr.sin_port);

	//_remoteSockAddr.sin_family = AF_INET;
	//_remoteSockAddr.sin_addr.s_addr = _remoteAddress;
	//_remoteSockAddr.sin_port = htons(_remotePort);

	// FIXME: doesn't work
	unsigned char *ptr = (unsigned char *)&(addr.sin_addr);

	int res = ::connect(_socket, (struct sockaddr *)&_remoteSockAddr, sizeof(_remoteSockAddr));

	if (SOCKERR(res))
	{
		_errorcode = ERRORCODE;
		DBG_REPORTERR("TCPSocket::connect(sockaddr_in_t)", "Could not connect")
		return false;
	}

	_connected = true;

	return true;
}

bool
TCPSocket::disconnect()
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::disconnect(): socket not connected!";
		return false;
	}
#endif

	if (_connected)
		return this->close();

	return false;
}

// Check if data can be read/written right now.
// For a socket in an unconnected state, readable() will return
// 1 if there is a connection waiting to be accepted.
//
// These functions always return immediately.
// Use waitUntil...() if you want to use a timeout value.
//
// Return values:
//   -1  an error occurred
//    0  not readable/writable (or timeout)
//    1  readable/writable

int
TCPSocket::readable()
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected || _bound);
#else
	if (!_connected && !_bound)
	{
		cerr << "TCPSocket::readable(): socket neither connected nor bound!";
		return -1;
	}
#endif

	fd_set			fdset;
	struct timeval	tv;
	int				res;

	if (_connected || (!_connected && _bound))
	{
		FD_ZERO(&fdset);
		FD_SET(_socket, &fdset);

		// Don't wait, let select() return immediately
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		/*
		select() return values:
			-1 		error occurred, errno set accordingly
			0  	    timeout expired before anything happened
			other	number of FDs in the set
		*/

		res = ::select(SELHIGHEST(_socket), &fdset, NULL, NULL, &tv);

		if (SOCKERR(res))
		{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::readable()", "::select() failed")
			return -1;
		}
		else if (res == 0)
		{
			// Nothing interesting happened before the timeout expired
			return 0;
		}

		return (FD_ISSET(_socket, &fdset) != 0);
	}

	return -1;
}

int
TCPSocket::writable()
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::writable(): socket not connected!";
		return false;
	}
#endif

	fd_set			fdset;
	struct timeval	tv;
	int				res;

	if (_connected)
	{
		FD_ZERO(&fdset);
		FD_SET(_socket, &fdset);

		// Don't wait, let select() return immediately
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		/*
		select() return values:
			-1 		error occurred, errno set accordingly
			0  	timeout expired before anything happened
			other	number of FDs in the set
		*/

		res = ::select(SELHIGHEST(_socket), NULL, &fdset, NULL, &tv);

		if (SOCKERR(res))
		{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::writable()", "::select() failed")
			return -1;
		}
		else if (res == 0)
		{
			return 0;
		}

		return (FD_ISSET(_socket, &fdset) != 0);
	}

	return -1;
}

// Wait until data can be read resp. written.
// The timeout parameter is in seconds.
// If timeout is set to 0, waiting will be indefinitely until data arrives or can be sent.
// Use poll...() if you want a function that returns immediately.
// The return value indicates if data was available or ready to be written
// before the timeout happened.

bool
TCPSocket::waitUntilReadable(int timeout)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected || _bound);
#else
	if (!_connected && !_bound)
	{
		cerr << "TCPSocket::waitUntilReadable(): socket not connected nor bound!";
		return false;
	}
#endif

	fd_set			fdset;
	struct timeval	tv;
	int				res;

	if (_connected || (!_connected && _bound))
	{
		FD_ZERO(&fdset);
		FD_SET(_socket, &fdset);

		if (timeout > 0)
		{
    		tv.tv_sec = timeout;
    		tv.tv_usec = 0;

			res = ::select(SELHIGHEST(_socket), &fdset, NULL, NULL, &tv);
		}
		else
		{
			res = ::select(SELHIGHEST(_socket), &fdset, NULL, NULL, NULL);
		}

		if (SOCKERR(res))
		{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::waitUntilReadable()", "::select() failed")
			return false;	// FIXME
		}
        _errorcode = NOERROR;
		if (res == 0)
		{
			// timeout expired
			return false;
		}

		// no need to check result set, res is 1 when we get here (i.e. our socket is returned in the set)
		return true;
	}

	return false;
}

bool
TCPSocket::waitUntilWritable(int timeout)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::waitUntilWritable(): socket not connected!";
		return false;
	}
#endif

	fd_set			fdset;
	struct timeval	tv;
	int				res;

	if (_connected)
	{
		FD_ZERO(&fdset);
		FD_SET(_socket, &fdset);

		if (timeout > 0)
		{
    		tv.tv_sec = timeout;
    		tv.tv_usec = 0;

			res = ::select(SELHIGHEST(_socket), NULL, &fdset, NULL, &tv);
		}
		else
		{
			res = ::select(SELHIGHEST(_socket), NULL, &fdset, NULL, NULL);
		}

		if (SOCKERR(res))
		{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::waitUntilWritable()", "::select() failed")
			return false;	// FIXME
		}

		if (res == 0)
		{
			// timeout expired
			return false;
		}

		// no need to check result set, res is 1 when we get here (i.e. our socket is returned in the set)
		return true;
	}

	return false;
}

// Assumes data is available to be read and reads out the requested
// amount. Returns the actual amount peeked in size.
// Depending on the blocking flag of the socket, this function
// may block if no data is available (but don't use that as a feature)
bool
TCPSocket::peek(char *buffer, unsigned int& size)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::peek(): socket not connected!";
		return false;
	}
#endif

	int	res;

	if (_connected)
	{
		res = ::recv (_socket, (char*)buffer, size, MSG_PEEK);

#ifdef WIN32
		// winsock reports an error when we peek fewer bytes than are present in the
		// reception buffer. we ignore the error here
		if (WSAGetLastError() != WSAEMSGSIZE && SOCKERR(res))
#else
		if (SOCKERR(res))
#endif
		{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::peek()", "::recv() failed")
			return false;
		}
		else if (res == 0)
		{
			_errorcode = NOERROR;
			return false;
		}

		size = res;
		return true;
	}

	return false;
}


// receive data.
//
// return value:
//   -1  an error occurred
//    0  peer performed shutdown
//   >0  the actual number of bytes received
int
TCPSocket::recv(char *buffer, unsigned int buffersize)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::recv(): socket not connected!";
		return -1;
	}
#endif

#ifdef WIN32
	int	res;
#else
	ssize_t	res;
#endif

	if (_connected)
	{
		res = ::recv (_socket, (char*)buffer, buffersize, 0);

		if (SOCKERR(res))
		{
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::recv()", "::recv() failed")
			return -1;
		}
		else if (res == 0)
		{
			_errorcode = NOERROR;
			return 0;
		}
///////////////DEBUG
#ifdef RAW_TCP_DEBUG
		logWARNING("recv");
		printtolog(buffer, res);
#endif
///////////////DEBUG END
		return res;
	}

	return -1;
}

// keep reading data until the requested amount has
// been read. it's the callers responsibility to ensure the
// buffer is large enough.
// only use this on blocking sockets, as no select() is used
// to determine when new data is available
//
// return value:
//   -1		an error occurred
//    0		peer performed shutdown
//  amount	all data succesfully read
int
TCPSocket::recvAmount(char* buffer, unsigned int amount)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
	assert (amount > 0);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::recvAmount(): socket not connected!";
		return -1;
	}
	if (amount == 0)
	{
		cerr << "TCPSocket::recvAmount(): amount is zero!";
		return -1;
	}
#endif

	int res;
	unsigned int received;

	if (_connected && amount > 0)
	{
		received = 0;
		while (received < amount)
		{
			res = ::recv (_socket, (char*)buffer + received, amount - received, 0);

			if (SOCKERR(res))
			{
#ifdef WSAECONNABORTED
				if ( WSAGetLastError() == WSAECONNABORTED )
				{
					// Connection was closed in the mean time
					disconnect();
					return -2;
				}
#endif

				_errorcode = ERRORCODE;
				DBG_REPORTERR("TCPSocket::recvAmount()", "::recv() failed")
				return -1;
			}
			else if (res == 0)
			{
				_errorcode = NOERROR;
				return 0;
			}

			received += res;
		}

		// this assert is not between #ifdef SOCKET_USE_ASSERTS .. #endif because
		// something is really wrong when it fails
		assert (received == amount);

///////////////DEBUG
#ifdef RAW_TCP_DEBUG
		logWARNING("recvAmount");
		printtolog(buffer, amount);
#endif
///////////////DEBUG END
		return received;
	}

	return -1;
}

// send data over the connection
int
TCPSocket::send(const char *buffer, unsigned int datasize)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
	assert (datasize > 0);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::send(): socket not connected!";
		return -1;
	}
	if (datasize == 0)
	{
		cerr << "TCPSocket::send(): datasize is zero!";
		return -1;
	}
#endif

	int	res;

	if (_connected && datasize > 0)
	{
		res = ::send (_socket, (char*)buffer, datasize, 0);

		if (SOCKERR(res))
		{
#ifdef WSAECONNABORTED
			if ( WSAGetLastError() == WSAECONNABORTED )
			{
				// Connection was closed in the mean time
				disconnect();
				return -2;
			}
#endif
			_errorcode = ERRORCODE;
			DBG_REPORTERR("TCPSocket::send()", "::send() failed")
			return -1;
		}
///////////////DEBUG
#ifdef RAW_TCP_DEBUG
		logWARNING("send");
		printtolog(buffer, datasize);
#endif
///////////////DEBUG END
		return res;
	}

	return -1;
}

// send data over the connection, making sure all data is sent
int
TCPSocket::sendAll(const char *buffer, unsigned int datasize)
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
	assert (datasize > 0);
#else
	if (!_connected)
	{
		cerr << "TCPSocket::sendAll(): socket not connected!";
		return -1;
	}
	if (datasize == 0)
	{
		cerr << "TCPSocket::sendAll(): datasize is zero!";
		return -1;
	}
#endif

	if (_connected)
	{
		int				res;
		unsigned int	offset = 0;
		unsigned int	dataleft = datasize;

		while (dataleft > 0)
		{
			res = ::send (_socket, ((char*)buffer)+offset, datasize, 0);

			if (SOCKERR(res))
			{
				_errorcode = ERRORCODE;
				DBG_REPORTERR("TCPSocket::sendAll()", "::send() failed")
				return -1;
			}

			dataleft -= res;
			offset += res;
		}

///////////////DEBUG
#ifdef RAW_TCP_DEBUG
		logWARNING("sendAll");
		printtolog(buffer, datasize);
#endif
///////////////DEBUG END
		return datasize;
	}

	return -1;
}

std::string
TCPSocket::getRemoteAddress() const
{
#ifdef SOCKET_USE_ASSERTS
	assert (_connected);
#else
	if (!_connected)
	{
		cerr << "\n### TCPSocket::getRemoteAddress(): socket not connected! ###\n";
		return std::string();
	}
#endif

	unsigned char	*ptr = (unsigned char *)&(_remoteSockAddr.sin_addr);
	char			buf[256];

	snprintf(buf, 256, "%u.%u.%u.%u", ptr[0], ptr[1], ptr[2], ptr[3]);
	return std::string(buf);
}
