#pragma once
#include "Communicator.h"

namespace RuGKinectInterfaceServer
{

class BasicCommunicator : public Communicator
{
public:
	enum MESSAGES {
		TRANSMIT_NEW_SKELETON_DATA
	};

	BasicCommunicator();

	void sendSekeleton();
};

}
