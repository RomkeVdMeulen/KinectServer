#pragma once

#include "KinectConnector.h"
#include "RequestListener.h"
#include "Recorder.h"
#include "Player.h"
#include <string>
#include <fx.h>
#include <osg/Matrix>
#include <OpenThreads/Condition>

namespace osg {
	class ArgumentParser;
}

namespace RuGKinectInterfaceServer {

class Skeleton;
class CalibrationWindow;
class DataMutex;

class Server
{
public:
	enum CONNECTION_TYPE {
		BASIC
	};
	
	static Server *instance();
	static bool isInitialized() { return s_pInstance != 0; }
	~Server();

	// Arguments
	static void addUsage(osg::ArgumentParser &arguments);
	void setCmdLineArguments(osg::ArgumentParser *arguments);
	osg::ArgumentParser *getCmdLineArguments() { return m_pArguments; }

	// Run
	void start();
	void stop();
	bool isRunning() const { return m_bIsRunning; }

	// Record to/play from file
	void recordTo(std::string const &file);
	void playFrom(std::string const &file, bool loop = false);

	// Calibration
	bool hasCalibration() const { return m_sCalibrationFile.size() > 0; }
	void writeCalibrationTo(std::string const &file);
	void useCalibrationFile(std::string const &file);
	osg::Matrix const &getCalibration() { return m_mxCalibration; }

#ifdef WIN32
	KinectConnector *getConnector() const { return m_pConnector; }
#endif

	// Mode
	enum SERVER_MODE {
		SERVE,
		CALIBRATE
	};
	bool isMode(SERVER_MODE mode) const { return m_mode == mode; }

	// Data
	Skeleton getLatestSkeleton() const;

	// Lock the current thread untill data available
	// Also automatically locks the associated mutex
	bool waitForNewSkeletonData(int ms = -1);
	DataMutex &getSkeletonMutex();
	void signalNewSkeletonDataAvailable();

#ifdef WIN32
	void waitForNewVideoData();
	DataMutex &getVideoMutex();
	void signalNewVideoDataAvailable();

	void waitForNewDepthData();
	DataMutex &getDepthMutex();
	void signalNewDepthDataAvailable();
#endif

#ifdef WIN32
	static BOOL WINAPI ConsoleHandler(DWORD CEvent);
#else
	static void ConsoleHandler(int signal);
#endif

private:
	static Server *s_pInstance;

	osg::ArgumentParser *m_pArguments;
	
	bool m_bIsRunning;

#ifdef WIN32
	KinectConnector *m_pConnector;
#endif

	RequestListener *m_pListener;

	Recorder *m_pRecorder;
	Player *m_pPlayer;

	std::string m_sCalibrationFile;
	osg::Matrix m_mxCalibration;
	
	SERVER_MODE m_mode;

	FXApp *m_pFOXApp;
	CalibrationWindow * m_pCalibrationWindow;

	OpenThreads::Condition 
		*m_pServerStopCondition,
		*m_pNewVideoDataCondition, *m_pNewDepthDataCondition, 
		*m_pNewSkeletonDataCondition;

private:
	Server();

	void checkFile(std::string const &sFileName, char const *mode);

	void startServerThreads();
	void startCalibration();
};

}
