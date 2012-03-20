#pragma once

#include "ServerThread.h"
#include "Communicator.h"

namespace RuGKinectInterfaceServer
{

class CommunicatorThread : public ServerThread
{
	virtual void run() = 0;

protected:
	// Child classes should call signalThreadReady() when ready
	void signalThreadReady();

	virtual Communicator *getCommunicator() = 0;
};

}
