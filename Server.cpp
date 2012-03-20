#include "stdafx.h"
#include "Server.h"
#include "CalibrationWindow.h"
#include <osg/ArgumentParser>
#include <fstream>
#include "Skeleton.h"

#ifndef WIN32
#include <signal.h>
#endif

using namespace RuGKinectInterfaceServer;
using namespace std;

Server *Server::s_pInstance = NULL;

Server *Server::instance()
{
	if ( !s_pInstance )
		s_pInstance = new Server();

	return s_pInstance;
}

#ifdef WIN32

BOOL WINAPI Server::ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
		case CTRL_C_EVENT:
		   cout << "\nControl input CTRL + C received\n\n";
		   instance()->stop();
		   break;
		
		case CTRL_BREAK_EVENT:
			cout << "\nControl input CTRL + Break received\n\n";
			instance()->stop();
			break;

		case CTRL_CLOSE_EVENT:
			cout << "\nConsole is closing\n\n";
			instance()->stop();
			break;

    }

    return true;
}

#else

void Server::ConsoleHandler(int signal)
{
	if ( signal == SIGINT )
		cout << "\nInterupt signal received\n\n";
	else
		cout << "\nUnknown process signal received\n\n";

	instance()->stop();
}

#endif

// Singleton: private
Server::Server()
: m_bIsRunning(FALSE), m_pRecorder( NULL ), m_pPlayer( NULL ), m_pArguments( NULL ), 
  m_pFOXApp( NULL ), m_pCalibrationWindow( NULL ), m_mode( SERVE )
{
	m_pListener = new RequestListener(REQUEST_PORT);

	m_pNewVideoDataCondition	= new OpenThreads::Condition();
	m_pNewDepthDataCondition	= new OpenThreads::Condition();
	m_pNewSkeletonDataCondition = new OpenThreads::Condition();
	m_pServerStopCondition		= new OpenThreads::Condition();

#ifdef WIN32
	m_pConnector = new KinectConnector;

	if ( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) RuGKinectInterfaceServer::Server::ConsoleHandler, TRUE ) )
	{
		cerr << "\n### Could not register the console handler. ###\n";
		throw CONSOLE_HANDLER_REGISTRATION_ERROR;
	}
#else
	signal(SIGINT, Server::ConsoleHandler);
#endif
	cout << "\nStarting Kinect Interface Server\n";
}

Server::~Server(void)
{
	delete s_pInstance, m_pRecorder, m_pPlayer, 
		m_pNewVideoDataCondition, m_pNewDepthDataCondition, 
		m_pNewSkeletonDataCondition;

#ifdef WIN32
	delete m_pConnector;
#endif
}

void Server::checkFile(std::string const &sFileName, char const *mode)
{
	FILE *fp = fopen(sFileName.c_str(),mode);
	if( !fp ) 
	{
		cerr << "\n### You specified file " << sFileName << " but this file cannot be opened in mode '" << mode << "'. ###\n";
		throw INVALID_FILE;
	}
	fclose(fp);
}

void Server::addUsage(osg::ArgumentParser &arguments)
{
	// Calibrate
#ifdef WIN32
	arguments.getApplicationUsage()->addCommandLineOption(
		"--writeCalibration [file]",
		"Calibrate the Kinect image on three points and write the calibration to specified file");
#endif
	arguments.getApplicationUsage()->addCommandLineOption(
		"--useCalibration [file]",
		"Use the calibration recorded in specified file and adjust all Kinect coordinates before transmission");

	// record/play
    arguments.getApplicationUsage()->addCommandLineOption(
		"--recordTo [file]",
		"In addition to serving Kinect data, record it to given file");
	arguments.getApplicationUsage()->addCommandLineOption(
		"--playFrom [file]",
		"In stead of getting data from Kinect, replay and serve data from given file");
	arguments.getApplicationUsage()->addCommandLineOption(
		"--loop",
		"In combination with --playFrom: loop playback of data");
}

void Server::setCmdLineArguments(osg::ArgumentParser *arguments)
{
	m_pArguments = arguments;

	std::string sFileName = "";
	if (arguments->read("--writeCalibration", sFileName))
	{
		checkFile(sFileName,"w");
		writeCalibrationTo(sFileName);
	} 
	sFileName.clear();
	if (arguments->read("--useCalibration", sFileName))
	{
		checkFile(sFileName,"r");
		useCalibrationFile( sFileName );
	}
	sFileName.clear();
	if (arguments->read("--playFrom", sFileName))
	{
		checkFile(sFileName,"r");
		playFrom( sFileName, arguments->read("--loop") );
	}
	sFileName.clear();
	if (arguments->read("--recordTo", sFileName))
	{
		checkFile(sFileName,"w");
		recordTo(sFileName);
	}
}

void Server::start()
{
	if ( m_mode == CALIBRATE )
		startCalibration();
	else
		startServerThreads();
}

void Server::startServerThreads()
{
	OpenThreads::Thread::Init();
	
	if ( !m_pPlayer )
	{
#ifndef WIN32
		cerr << "\n### Sorry, but use of live Kinect data is only available on Windows. Please specify a Kinect data file to use.###\n\n";
		m_pArguments->getApplicationUsage()->write(std::cout);
		throw INVALID_PROGRAM_USE;
#else
		// Start the Kinect connection
		m_pConnector->connect();
		m_pConnector->start();
#endif
	} else {
		// Play from data file
		m_pPlayer->start();
	}

	// Start listening for requests
	m_pListener->start();

	// Record data if needed
	if ( m_pRecorder )
		m_pRecorder->start();

	m_bIsRunning = TRUE;

	OpenThreads::Mutex *dummy = new OpenThreads::Mutex;
	m_pServerStopCondition->wait(dummy);

	m_bIsRunning = FALSE;

	cout << " * Server has stopped\n";
}

void Server::startCalibration()
{
#ifndef WIN32
	cerr << "\n### Sorry, the calibration window is only available on Windows ###\n";
	throw INVALID_PROGRAM_USE;
#else
	m_pFOXApp = new FXApp(PROGRAM_NAME,VENDOR);
	m_pFOXApp->init(m_pArguments->argc(),m_pArguments->argv());

	m_pCalibrationWindow = new CalibrationWindow(m_pFOXApp, m_sCalibrationFile);

	// Start the Kinect connection
	m_pConnector->connect();
	m_pConnector->start();
	
	m_pFOXApp->create();

	m_bIsRunning = true;
	m_pFOXApp->run();
	m_bIsRunning = false;

	stop();
#endif
}

void Server::stop()
{
	cout << "\nStopping Kinect Interface Server\n";

	if ( m_pFOXApp )
		 m_pFOXApp->stop();

	if ( m_pListener->isRunning() )
		m_pListener->stop();

	if ( m_pRecorder && m_pRecorder->isRunning() )
		m_pRecorder->stop();

#ifdef WIN32
	if ( m_pConnector->isConnected() )
		m_pConnector->disconnect();
	
	if ( m_pConnector->isRunning() )
		m_pConnector->stop();
#endif

	if ( m_pPlayer && m_pPlayer->isRunning() )
		m_pPlayer->stop();

	m_pServerStopCondition->broadcast();
}

void Server::recordTo(std::string const &file)
{
	m_pRecorder = new Recorder(file);
}

void Server::playFrom(std::string const &file, bool loop)
{
	m_pPlayer = new Player(file, loop);
}

void Server::writeCalibrationTo(std::string const &file)
{
	m_sCalibrationFile = file;
	m_mode = CALIBRATE;

#ifndef WIN32
	cerr << "\n\nSorry, but Kinect calibration is only available on Windows\n\n";
	throw INVALID_PROGRAM_USE;
#endif
}

void Server::useCalibrationFile(std::string const &file)
{
	m_sCalibrationFile = file;

	ifstream cabFileIn( file.c_str() );
	float calibration[16];
	for ( unsigned i = 0; i < 16; ++i )
		cabFileIn >> calibration[i];
	m_mxCalibration.set(calibration);

	cout << " * Calibration read from " << file << "\n";
}

#ifdef WIN32

DataMutex &Server::getVideoMutex()
{
	return m_pConnector->getVideoMutex();
}

void Server::waitForNewVideoData()
{
	m_pNewVideoDataCondition->wait(  &getVideoMutex() );
}

void Server::signalNewVideoDataAvailable()
{
	m_pNewVideoDataCondition->broadcast();

	if ( m_pFOXApp )
		m_pFOXApp->refresh();
}

DataMutex &Server::getDepthMutex()
{
	return m_pConnector->getDepthMutex();
}

void Server::waitForNewDepthData()
{
	m_pNewDepthDataCondition->wait(  &getDepthMutex() );
}

void Server::signalNewDepthDataAvailable()
{
	m_pNewDepthDataCondition->broadcast();

	if ( m_pFOXApp )
		m_pFOXApp->refresh();
}

#endif

void Server::waitForNewSkeletonData()
{
	m_pNewSkeletonDataCondition->wait( &getSkeletonMutex() );
}

DataMutex &Server::getSkeletonMutex()
{
	if ( m_pPlayer && m_pPlayer->isRunning() )
		return m_pPlayer->getSkeletonMutex();

#ifdef WIN32
	return m_pConnector->getSkeletonMutex();
#else
	throw INVALID_PROGRAM_USE;
#endif
}

void Server::signalNewSkeletonDataAvailable()
{
	m_pNewSkeletonDataCondition->broadcast();
}

Skeleton Server::getLatestSkeleton() const
{
	if ( m_pPlayer )
		return m_pPlayer->getLatestSkeleton();

#ifdef WIN32
	return getConnector()->getLatestSkeleton();
#else
	throw INVALID_PROGRAM_USE;
#endif
}
