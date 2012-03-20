// Defines the entry point for the Kinect Interface Server
// See ReadMe.txt

#include "stdafx.h"
#include "Server.h"
#include "Communicator.h"
#include "DataMutex.h"
#include <osg/ArgumentParser>

int closeWithCode(int code)
{
	if ( RuGKinectInterfaceServer::Server::isInitialized() && RuGKinectInterfaceServer::Server::instance()->isRunning() )
		RuGKinectInterfaceServer::Server::instance()->stop();

#ifdef _DEBUG
	if ( code != NORMAL_EXIT )
		cin.ignore();
#endif

	return code;
}

// Main procedure
int main(int argc, char **argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is "+PROGRAM_NAME);
#ifdef WIN32
    arguments.getApplicationUsage()->setCommandLineUsage("Kinect Interface Server.exe [options] ...");
#else
	arguments.getApplicationUsage()->setCommandLineUsage("Kinect Interface Server --playFrom <file> [options] ...");
#endif

	// help!
    arguments.getApplicationUsage()->addCommandLineOption(
		"-h or --help",
		"Display this information");

	RuGKinectInterfaceServer::Server::addUsage(arguments);

	// If user request help, show it
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return closeWithCode(NORMAL_EXIT);
    }

	bool needHelp = arguments.read("--help");

	cout << "\nWelcome to " << PROGRAM_NAME << " v" << VERSION << "!\n"
		 << COPYRIGHT_NOTICE << "\n\n"
		 << "Use CTRL + C for controlled program stop\n"
		 << "\n";

	try {

		RuGKinectInterfaceServer::Server *pServer = RuGKinectInterfaceServer::Server::instance();
		pServer->setCmdLineArguments(&arguments);

		pServer->start();

	} catch(int exitCode) {
		if ( exitCode == RuGKinectInterfaceServer::Communicator::NOT_CONNECTED_EXCEPTION )
		{
			cerr << "\nUncaught Communicator not connected exception: terminating program\n\n";
		} else if ( exitCode == RuGKinectInterfaceServer::DataMutex::EXCEPTION ) {
			cerr << "\nUncaught mutex exception: terminating program\n\n";
		} else {
			cerr << "\nUnrecoverable error (" << exitCode << "): terminating program\n\n";
		}
		return closeWithCode(exitCode);
	} catch(string exitMessage) {
		cerr << "\n" << exitMessage << "\nUnrecoverable error (" << UNSPECIFIED_ERROR << "): terminating program\n\n";
		return closeWithCode(UNSPECIFIED_ERROR);
	} catch(char *exitMessage) {
		cerr << "\n" << exitMessage << "\nUnrecoverable error (" << UNSPECIFIED_ERROR << "): terminating program\n\n";
		return closeWithCode(UNSPECIFIED_ERROR);
	}

	cout << "\n" << PROGRAM_NAME << " complete. Have a nice day!\n\n";

	return closeWithCode(NORMAL_EXIT);
}

