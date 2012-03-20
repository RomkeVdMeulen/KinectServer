#include "stdafx.h"
#include "Recorder.h"
#include "Server.h"
#include <fstream>
#include "Skeleton.h"

using namespace RuGKinectInterfaceServer;

Recorder::Recorder(std::string const &fileName)
: ServerThread(), m_sFileName( fileName ), m_pFilestream( NULL )
{}

Recorder::~Recorder()
{
	if ( !m_pFilestream )
		return;

	if ( m_pFilestream->is_open() )
		m_pFilestream->close();

	delete m_pFilestream;
}

void Recorder::run()
{
	m_pFilestream = new ofstream( m_sFileName.c_str(), ios::binary | ios::out | ios::trunc );	// file is truncated before write (alternative: `app' for append)
	if ( !m_pFilestream )
	{
		cerr << "\n### Could not open " << m_sFileName << " for writing. ###\n";
		throw INVALID_FILE;
	}

	Server *pServer = Server::instance();

	cout << " * Recording Kinect data to " << m_sFileName << "\n";

	while ( m_bContinueThread )
	{
		pServer->waitForNewSkeletonData();

		storeCurrentSkeleton();

		pServer->getSkeletonMutex().unlock();
	}

	cout << " * Writing to " << m_sFileName << "...   ";

	m_pFilestream->close();

	cout << "done.\n";

	delete m_pFilestream;
	m_pFilestream = NULL;
}

void Recorder::storeCurrentSkeleton()
{
	Skeleton skeleton(Server::instance()->getLatestSkeleton());
	m_pFilestream->write(reinterpret_cast<char const *>(&skeleton),sizeof(Skeleton));
}
