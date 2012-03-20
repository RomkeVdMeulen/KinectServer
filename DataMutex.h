#pragma once

#include <OpenThreads/ReentrantMutex>

namespace RuGKinectInterfaceServer {

class DataMutex : public OpenThreads::ReentrantMutex
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
		int res = OpenThreads::Mutex::unlock();
		if ( res < 0 ) {
			throw EXCEPTION;
		}
        return 0;
	}
};

}
