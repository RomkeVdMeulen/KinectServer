#pragma once

#include "TCPSocket.h"
#include "MessageStream.h"
#include "StructuredStreamView.h"

namespace RuGKinectInterfaceServer
{
	
class Communicator
{
public:
	typedef Byte MESSAGE;

	static const int NOT_CONNECTED_EXCEPTION = 192837465;

	Communicator();
	virtual ~Communicator();

	void setConnection(TCPSocket *socket);

	bool connect(const std::string &address, short port);
	void disconnect();
	bool isConnected() const;

	void sendUpdates();
	void receiveUpdates();
	
	void sendConnectionConfirmation();

	TCPSocket*	getSocket()				{ return m_pSocket; }
	
protected:
	// Child classes may override for specific behavior:
	virtual bool processMessage(MESSAGE message, Uint16 datasize) { return false; }
	virtual void preConnect() {}
	virtual void postConnect() {}
	virtual void preDisconnect() {}
	virtual void postDisconnect() {}

	// and may use:
	void startWriteMessage(MESSAGE msg, Byte size);

protected:
	TCPSocket*				m_pSocket;	

	MessageStream*			_sendBuffer;

	StructuredStreamView*	_receiveBufferView;
	Byte*					_receiveBuffer;
	Uint16					_receiveBufferSize;

};

}
