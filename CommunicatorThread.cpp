#include "stdafx.h"
#include "CommunicatorThread.h"

using namespace RuGKinectInterfaceServer;

void CommunicatorThread::signalThreadReady()
{
	getCommunicator()->sendConnectionConfirmation();
}
