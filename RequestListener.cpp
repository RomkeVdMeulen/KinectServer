#include "stdafx.h"
#include "RequestListener.h"
#include "RequestProcessor.h"
#include "Server.h"

using namespace RuGKinectInterfaceServer;

RequestListener::RequestListener(const int port)
: ServerThread(), m_nPort( port ), m_pSocket( NULL )
{}

RequestListener::~RequestListener()
{
	if ( m_pSocket )
	{
		delete m_pSocket;
		m_pSocket = NULL;
	}
}

void RequestListener::run()
{
	m_pSocket = new TCPSocket();
	m_pSocket->realize();
#ifdef ALLOW_REMOTE_REQUESTS
	m_pSocket->bind(m_nPort);
#else
	m_pSocket->bind(m_nPort, "localhost");
#endif

	m_pSocket->listen();
	
	cout << " * Listening for requests on port " << m_nPort << "\n";
	
	while ( m_bContinueThread )
	{
		listenForRequest();
	}

	cout << " * No longer listening for requests\n";

	m_pSocket->close();
	delete m_pSocket;

	m_pSocket = NULL;
}

void RequestListener::listenForRequest()
{
	if ( !m_pSocket || !m_pSocket->readable() )
		// no request
		return;
	
	TCPSocket *activeSocket = m_pSocket->accept();
	if (activeSocket)
	{
		cout << "\n * Accepted incoming connection request from " << activeSocket->getRemoteAddress() << ':' << activeSocket->getRemotePort() << "\n";

		// Create a shortlived thread that waits untill the client specifies the type of connection
		// so that we can continue listening for threads uninterrupted
		RequestProcessor *pProcessor = new RequestProcessor(activeSocket);
		pProcessor->start();
	}
	else
		cerr << "\n### Accepting incoming request connection failed ###\n";
}
