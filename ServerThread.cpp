#include "stdafx.h"
#include "ServerThread.h"

using namespace RuGKinectInterfaceServer;

ServerThread::ServerThread()
: m_bContinueThread( true )
{}

void ServerThread::stop()
{
	m_bContinueThread = false;
}
