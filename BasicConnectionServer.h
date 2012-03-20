#pragma once

#include "CommunicatorThread.h"
#include "BasicCommunicator.h"

namespace RuGKinectInterfaceServer
{

class BasicConnectionServer : public CommunicatorThread
{
	BasicCommunicator *m_pCommunicator;

public:
	BasicConnectionServer(TCPSocket *socket);
	~BasicConnectionServer();

	virtual void run();

protected:
	virtual Communicator *getCommunicator() { return m_pCommunicator; }
};

}
