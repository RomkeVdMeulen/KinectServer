#pragma once

#include <OpenThreads/Mutex>

namespace RuGKinectInterfaceServer {

class DataMutex : public OpenThreads::Mutex
{

public:
	static const int EXCEPTION = 1379;

	virtual int lock()
	{
		if ( OpenThreads::Mutex::lock() != 0 ) {
			throw EXCEPTION;
		}
        return 0;
	}

	virtual int unlock()
	{
		if ( OpenThreads::Mutex::unlock() != 0 ) {
			throw EXCEPTION;
		}
        return 0;
	}
};

}
