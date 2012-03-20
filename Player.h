#pragma once

#include "ServerThread.h"
#include "DataMutex.h"
#include <iosfwd>

namespace RuGKinectInterfaceServer {

class Skeleton;

class Player : public ServerThread
{
	std::string m_sFileName;
	std::ifstream *m_pFilestream;

	bool m_bLoop;

	DataMutex m_ghSkeletonMutex;
	Skeleton *m_pLatestSkeleton;

public:
	Player(std::string const &fileName, bool loop = false);
	virtual ~Player();

	Skeleton getLatestSkeleton();

	virtual void run();

	DataMutex &getSkeletonMutex() { return m_ghSkeletonMutex; }

private:
	static unsigned getClockInMs();
};

}
