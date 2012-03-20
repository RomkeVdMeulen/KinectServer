#include "stdafx.h"
#include "Communicator.h"
#include "Protocol.h"

// Based on code from the osgrc program

#ifdef __sgi
#include <assert.h>
#else
#include <cassert>
#endif

#include "StructuredStreamView.h"
#include "TCPSocket.h"

using namespace RuGKinectInterfaceServer;

Communicator::Communicator():	
	m_pSocket(NULL)
{			
	_sendBuffer = new MessageStream();	

	_receiveBufferSize = 2048;
	_receiveBuffer = (Byte*) malloc(_receiveBufferSize);

	_receiveBufferView = new StructuredStreamView(_receiveBuffer, _receiveBufferSize);
}

Communicator::~Communicator()
{	
	if ( isConnected() )
		disconnect();

	delete _receiveBufferView;
	free(_receiveBuffer);
    delete _sendBuffer;
}

bool Communicator::isConnected() const
{
	if ( !m_pSocket )
		return false;

	return m_pSocket->isConnected(); 
}

void  Communicator::setConnection(TCPSocket *socket)
{
	m_pSocket = socket;
}

bool Communicator::connect(std::string const &address, short port)
{
	assert (!isConnected());

	preConnect();
	
	m_pSocket = new TCPSocket();
	m_pSocket->realize();

	bool res = m_pSocket->connect(address, port);
		
	if (res)
	{
		//m_pSocket->setBlocking(false);
		postConnect();

		return true;
	}
	else
	{
		cerr << "\n### Could not connect(): error = " << m_pSocket->errorcode() << " ###\n";
	}

	m_pSocket->close();
	delete m_pSocket;

	return false;
}


void Communicator::disconnect()
{
	preDisconnect();

	m_pSocket->shutdown(true, true);
	m_pSocket->close();

	delete m_pSocket;
	m_pSocket = NULL;

	postDisconnect();
}


void Communicator::sendUpdates()
{
	Uint32 bufsize, bufsize_n;

	if ( !isConnected() )
		throw NOT_CONNECTED_EXCEPTION;
	
	if (m_pSocket->writable())
	{
		bufsize = _sendBuffer->size();		
		if (bufsize > 0)
		{
			bufsize_n = htonl(bufsize);
			m_pSocket->send((char*)&bufsize_n, sizeof(bufsize_n));

			m_pSocket->send((char*)_sendBuffer->getBuffer(), bufsize);

			_sendBuffer->clear();	
		}
	} // writable
}

#ifndef WSAECONNRESET
#define WSAECONNRESET -47
#endif

#define COMMUNICATOR_CONNECTION_PROBLEM(res) \
{ \
	if (res == -1 && m_pSocket->errorcode() != WSAECONNRESET) \
		cerr << "\n### Error on socket: " << m_pSocket->errorcode() << " ###\n"; \
	else if ( res == 0 || m_pSocket->errorcode() == WSAECONNRESET ) \
		cout << " * Client at " << m_pSocket->getRemoteAddress() << ':' << m_pSocket->getRemotePort() << " has disconnected\n"; \
		\
	disconnect(); \
	 \
	return; \
}

void Communicator::receiveUpdates()
{
	int		res;
	MESSAGE msg;
	Uint16	msgdatasize;
	Uint32	bufsize, bufsize_n;

	if ( !isConnected() )
		throw NOT_CONNECTED_EXCEPTION;
	
	while(m_pSocket->readable()) // Process all incoming messages at once
	{
		res = m_pSocket->recvAmount((char*)&bufsize_n, sizeof(bufsize_n));			

		if (res <= 0)		
			COMMUNICATOR_CONNECTION_PROBLEM(res)

		bufsize = ntohl(bufsize_n);			
		if (bufsize > _receiveBufferSize)
		{
			cout << " * Growing buffer to " << bufsize << " bytes\n";
			_receiveBuffer = (Byte*) realloc(_receiveBuffer, bufsize);
			_receiveBufferSize = bufsize;
		}

		res = m_pSocket->recvAmount((char*)_receiveBuffer, bufsize);					
		if (res > 0)
		{
			//cout << " * " << __FUNC__ << ": Received " << bufsize << " bytes\n";

			_receiveBufferView->setBuffer(_receiveBuffer, bufsize);
								
			while (_receiveBufferView->dataLeft())
			{
				msg = _receiveBufferView->getByte();

				// The number of bytes that follow.
				// This is only used for skipping ahead if an unknown message is encountered
				msgdatasize = _receiveBufferView->getUint16();

				if ( !processMessage(msg, msgdatasize) )
				{
					cerr << " * Skipping unknown message " << msg << " (datasize=" << msgdatasize << ")\n";
					_receiveBufferView->skipAhead(msgdatasize);
				}
			}
		} else
			COMMUNICATOR_CONNECTION_PROBLEM(res)
	}
}


void Communicator::startWriteMessage(MESSAGE msg, Byte size)
{
	if ( !isConnected() )
		throw NOT_CONNECTED_EXCEPTION;
	
	_sendBuffer->appendByte(msg);
	_sendBuffer->appendUint16(size);
}

void Communicator::sendConnectionConfirmation()
{
	if ( !isConnected() )
		throw NOT_CONNECTED_EXCEPTION;

	_sendBuffer->appendString(KINECT_CONNECTION_CONFIRMATION_MESSAGE);
	sendUpdates();
}
