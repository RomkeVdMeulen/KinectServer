#pragma once

#include "ServerThread.h"
#include <iosfwd>

namespace RuGKinectInterfaceServer {
	
class Recorder : public ServerThread
{
	std::string m_sFileName;
	std::ofstream *m_pFilestream;

public:
	Recorder(std::string const &fileName);
	virtual ~Recorder();

	virtual void run();

private:
	void storeCurrentSkeleton();
};

}
