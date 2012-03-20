#include "stdafx.h"
#include "RequestProcessor.h"
#include "BasicTypes.h"
#include "StructuredStreamView.h"
#include "Protocol.h"
#include "BasicConnectionServer.h"

using namespace RuGKinectInterfaceServer;

#define STOP_REQUEST_PROCESSING_WITH_ERROR { \
		if ( res == 0 ) \
			cout << " * Request ended prematurely\n"; \
		else \
			cout << " * Error or invalid request: request refused\n"; \
		pRequestSocket->disconnect(); \
		delete pRequestSocket; \
		return; \
	}

void RequestProcessor::run()
{
	// Wait for socket to be ready
	pRequestSocket->waitUntilReadable();
	
	// Read in request data
	int		res;
	Uint32	bufsize, bufsize_n;
	res = pRequestSocket->recvAmount((char*)&bufsize_n, sizeof(bufsize_n));
	if ( res <= 0 )
		STOP_REQUEST_PROCESSING_WITH_ERROR
	bufsize = ntohl(bufsize_n);

	Byte *receiveBuffer = (Byte *) malloc(bufsize);
	res = pRequestSocket->recvAmount((char*)receiveBuffer, bufsize);
	if ( res <= 0 )
		STOP_REQUEST_PROCESSING_WITH_ERROR

	StructuredStreamView receiveBufferView(receiveBuffer, bufsize);

	// We expect a certain opening message
	string expectedRequestMessage(KINECT_CONNECTION_REQUEST_MESSAGE);
	if ( !receiveBufferView.bytesLeft(expectedRequestMessage.size()) ) 
	{
		cout << " * Did not receive expected opening message\n";
		STOP_REQUEST_PROCESSING_WITH_ERROR
	}
	string actualMessage;
	receiveBufferView.getString(actualMessage);
	if ( actualMessage != expectedRequestMessage )
	{
		cout << " * Did not receive expected opening message\n";
		STOP_REQUEST_PROCESSING_WITH_ERROR
	}

	// Now please specify the type of connection you want
	int nConnectionType = (int) receiveBufferView.getByte();

	// Based on requested connection type, create a new server thread
	ServerThread *pServerThread = 0;

	switch ( nConnectionType )
	{
		case KINECT_INTERFACE_CONNECTION_BASIC:
			pServerThread = new BasicConnectionServer(pRequestSocket);
			break;

		default:
			cerr << "\n### Invalid request options: type " << nConnectionType << " is not a recognized connection type ###\n";
			STOP_REQUEST_PROCESSING_WITH_ERROR
	}

	pServerThread->start();

	free(receiveBuffer);

	return;
}
