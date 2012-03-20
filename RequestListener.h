#pragma once

#include "TCPSocket.h"
#include "ServerThread.h"

namespace RuGKinectInterfaceServer {

class RequestListener : public ServerThread
{
public:
	RequestListener(const int port);
	~RequestListener();

	virtual void run();

private:
	const int m_nPort;

	TCPSocket *m_pSocket;

	void listenForRequest();
};

}
