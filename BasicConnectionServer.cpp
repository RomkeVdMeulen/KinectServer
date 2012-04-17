#include "stdafx.h"
#include "BasicConnectionServer.h"
#include "Server.h"

using namespace RuGKinectInterfaceServer;

BasicConnectionServer::BasicConnectionServer(TCPSocket *socket)
: CommunicatorThread()
{
	m_pCommunicator = new BasicCommunicator;
	m_pCommunicator->setConnection(socket);
}

BasicConnectionServer::~BasicConnectionServer()
{
	delete m_pCommunicator;
}

void BasicConnectionServer::run()
{
	signalThreadReady();

	cout << " * New basic connection established with " << m_pCommunicator->getSocket()->getRemoteAddress()
		 << ':' << m_pCommunicator->getSocket()->getRemotePort() << "\n"
		 << " * New server thread started\n\n";

	while ( m_bContinueThread )
	{
		m_pCommunicator->receiveUpdates();

		if ( !m_pCommunicator->isConnected() )
		{ 
			// client disconnected
			m_bContinueThread = false;
			break;
		}

		if ( !Server::instance()->waitForNewSkeletonData( 500 ) )
		{
			Server::instance()->getSkeletonMutex().unlock();
			continue;
		}

		try {
			m_pCommunicator->sendSekeleton();
		} catch ( int code ) {
			if ( code == Communicator::NOT_CONNECTED_EXCEPTION ) {
				cerr << "\n### Connection lost, closing server thread ###\n";
				m_bContinueThread = false;
			} else {
				throw code;
			}
		}

		Server::instance()->getSkeletonMutex().unlock();
	}

	if ( m_pCommunicator->getSocket() )
		cout << " * Closing server thread for connection to " << m_pCommunicator->getSocket()->getRemoteAddress()
			 << ':' << m_pCommunicator->getSocket()->getRemotePort() << "\n";
	else
		cout << " * Closing server thread\n";
}
