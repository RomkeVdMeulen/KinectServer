#pragma once

#include "ServerThread.h"
#include "TCPSocket.h"

namespace RuGKinectInterfaceServer
{

class RequestProcessor : public ServerThread
{
	TCPSocket *pRequestSocket;

public:
	RequestProcessor(TCPSocket *socket) : pRequestSocket( socket ) {}

	virtual void run();
};

}
