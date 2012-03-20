#include "stdafx.h"
#include "BasicCommunicator.h"
#include "Server.h"
#include "Skeleton.h"

using namespace RuGKinectInterfaceServer;

BasicCommunicator::BasicCommunicator()
: Communicator()
{
}

void BasicCommunicator::sendSekeleton()
{
	Server *pServer = Server::instance();
	
	Skeleton skeleton = pServer->getLatestSkeleton();

	if ( pServer->hasCalibration() )
		skeleton.transform( pServer->getCalibration() );

	_sendBuffer->beginMessage(TRANSMIT_NEW_SKELETON_DATA);
	_sendBuffer->appendData(reinterpret_cast<void const *>(&skeleton), sizeof(Skeleton));
	_sendBuffer->writeMessage();

	sendUpdates();
}
