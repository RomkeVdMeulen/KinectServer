#pragma once

#include <OpenThreads/Thread>

namespace RuGKinectInterfaceServer
{

// Adds to the openthreads thread the possibility for gracefully stopping a thread
class ServerThread : public OpenThreads::Thread
{
protected:
	bool m_bContinueThread;

public:
	ServerThread();

	virtual void run() = 0;

	void stop();
};

}
