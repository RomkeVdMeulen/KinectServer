#include "stdafx.h"
#include "Player.h"
#include "Server.h"
#include "Skeleton.h"
#include <fstream>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif

using namespace RuGKinectInterfaceServer;

Player::Player(std::string const &fileName, bool loop)
: ServerThread(), m_sFileName( fileName ), m_bLoop( loop ), m_pLatestSkeleton( NULL )
{}

Player::~Player()
{
	delete m_pLatestSkeleton;
}

unsigned Player::getClockInMs()
{
	return  ((float) clock() / CLOCKS_PER_SEC) * 1000;
}

void Player::run()
{
	m_pFilestream = new ifstream( m_sFileName.c_str(), ios::in | ios::binary );
	if ( !m_pFilestream )
	{
		cerr << "\n### Could not open " << m_sFileName << " for reading. ###\n";
		throw INVALID_FILE;
	}

	std::cout << " * Playing back Kinect data from " << m_sFileName << endl;

	unsigned loopTime = 0;
	char buffer[sizeof(Skeleton)];
	while ( m_bContinueThread && m_pFilestream->read( buffer, sizeof(Skeleton) ) )
	{
		m_ghSkeletonMutex.lock();

		delete m_pLatestSkeleton;
		m_pLatestSkeleton = NULL;
		m_pLatestSkeleton = new Skeleton( *reinterpret_cast<Skeleton const *>(buffer) );
		
		m_ghSkeletonMutex.unlock();

		while ( m_pLatestSkeleton->getTime() > static_cast<unsigned long>(getClockInMs() - loopTime) ) 
#ifdef WIN32
			Sleep(1);
#else
			usleep(1);
#endif

		Server::instance()->signalNewSkeletonDataAvailable();

		if ( m_bLoop && m_pFilestream->peek() == EOF )
		{
			//std::cout << " * All data from " << m_sFileName << " has been replayed: looping\n";
			m_pFilestream->seekg(0, ios::beg);
			m_pFilestream->clear();
			loopTime = getClockInMs();
		}
	}

	if ( !m_bContinueThread )
		std::cout << " * Stopping Kinect data playback\n";
	else if ( m_pFilestream->eof() )
		std::cout << " * All data from " << m_sFileName << " has been replayed\n\n";
	else
		cerr << "\n### Kinect data playback stopped for unknown reason ###\n";
}

Skeleton Player::getLatestSkeleton()
{ 
	m_ghSkeletonMutex.lock();
	
	Skeleton returnValue(*m_pLatestSkeleton);

	m_ghSkeletonMutex.unlock();

	return returnValue;
}
