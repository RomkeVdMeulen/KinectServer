#pragma once

// Blatantly stolen from the osgrc codebase

#include "SockUtil.h"

namespace RuGKinectInterfaceServer {

class TCPSocket  
{
public:

	TCPSocket();
	~TCPSocket();
	
	/*
	 * Initialization
	 */
		
	// create the socket
	bool 		realize();		
	
	// set socket to blocking yes/no
	// must be called after realize()
	bool 		setBlocking(bool value=true);		
	
	// set socket options
	int			getIntegerSockOpt(int level, int option);
	bool		getBooleanSockOpt(int level, int option);

	bool		setBooleanSockOpt(int level, int option, bool value);
	bool		setIntegerSockOpt(int level, int option, int value);
	
	// shutdown or close the socket
	bool 		shutdown(bool forRead=true, bool forWrite=true);
	bool 		close();
	
	// Bind the socket to a local address.
	// This function must be called after realize().
	// If the local address is not specified, INADDR_ANY is used.	
	bool 		bind(const short localPort, const std::string& localAddress="");

	// Prepare the socket for receiving an incoming connection.
	bool 		listen(int backlog=1);

	// Accept the incoming connection request.
	// Returns a new socket object for the accepted connection, or 
	// NULL in case an error occurred.
	TCPSocket*	accept();

	// Connect to the given remote address.
	// Returns wether connecting was successful
	bool 		connect(const std::string& destAddress, const short destPort);
	bool		connect(sockaddr_in_t addr);

	// If connected, shutdown the connection.
	bool 		disconnect();

	// Return wether the socket is in a connected state, i.e.
	// data can be sent to/read from it.
	bool 		isConnected() const			{ return _connected; }
	
	/* 
	 * Sending data
	 *
	 * only valid if connected
	 */
	
	// Send data over the connection
	// Returns the actual amount of data sent, or -1 in 
	// case an error occurred
	int 		send(const char* buffer, unsigned int datasize);

	// Send data over the connection. Doesn't return until all
	// data has been sent, or an error occurs.
	// Returns datasize on success (number of bytes sent) or -1 in
	// case an error occurred.
	int 		sendAll(const char* buffer, unsigned int datasize);
	
	/*
	 * Receiving data 
	 *
	 * only valid if connected
	 */

	// Receive data from the connection.
	// It's the caller's responsibility to ensure the provided
	// buffer is large enough.
	//
	// return value:
	//   -1  an error occurred
	//    0  peer performed shutdown
	//   >0  data received, value is the actual number of bytes received
	int 		recv(char* buffer, unsigned int buffersize);

	// Keep reading data until the requested amount has
	// been read. It's the caller's responsibility to ensure the
	// provided buffer is large enough.
	// Only use this on blocking sockets, as no select() is used
	// to determine when new data is available
	//
	// return value:
	//   -1		an error occurred
	//    0		peer performed shutdown
	//  amount	all data succesfully read
	int			recvAmount(char* buffer, unsigned int amount);
	
	/*
	 * Checking for data
	 */
	
	// Check if data can be read/written right now.
	// For a socket in an unconnected state, readable() will return
	// 1 if there is a connection waiting to be accepted.
	//
	// These functions always return immediately.
	// Use waitUntil...() if you want to use a timeout value.
	//
	// Return values:
	//   -1  an error occurred
	//    0  not readable/writable
	//    1  readable/writable
	int 		readable();
	int 		writable();
		
	// Wait until data can be read resp. written.
	// For a socket in an unconnected state, waitUntilReadable() will 
	// return true if there is a connection waiting to be accepted.
	//
	// The timeout parameter is in seconds. 
	// If timeout is set to 0, waiting will be indefinitely until data arrives or can be sent.
	// Use read/writeable() if you want a function that returns immediately.
	//
	// The return value indicates if data was available or ready to be written 
	// before the timeout happened.
	bool 		waitUntilReadable(int timeout=0);
	bool 		waitUntilWritable(int timeout=0);
	
	// Assumes data is available to be read and reads out the requested
	// amount. Returns the actual amount peeked in parameter 'size'.
	// Depending on the blocking flag of the socket, this function
	// may block if no data is available (but don't use that as a feature)
	bool 		peek(char *buffer, unsigned int& size);	
	
	/*
	 * Call errorcode() to find out what went wrong in case of errors.
	 * This function returns the value of errno or WSAGetLastError() 
	 * ___at the time the last method of this class was called___
	 */	
	int 		errorcode() const		{ return _errorcode; }	

	// Return the low-level socket handle
	socket_t	getHandle() const		{ return _socket; }

	std::string	getRemoteAddress() const;
	short getRemotePort() const { return _remoteSockAddr.sin_port; }

protected:

	// 'Wrap' a previously created socket. This constructor
	// is used by accept() to create a new TCPSocket instance.
	TCPSocket(socket_t s, sockaddr_in_t remoteSockAddr, socklen_t remoteSockAddrLen);
	
protected:

	int					_errorcode;

	bool				_realized;
	bool				_bound;
	bool				_listening;
	bool				_connected;				
	
	socket_t			_socket;	

	sockaddr_in_t 		_localSockAddr;			// set by bind()

	unsigned long		_localAddress;			// network byte-order
	short				_localPort;				// native byte-order

	sockaddr_in_t 		_remoteSockAddr;		// set by connect() and accept()

	unsigned long		_remoteAddress;			// network byte-order	
	short 				_remotePort;			// native byte-order
};

}	// namespace osgRC
