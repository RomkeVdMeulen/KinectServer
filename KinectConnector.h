// This class handles the raw input from the Kinect API

#pragma once

#ifdef WIN32

#include "NuiApi.h"
#include "Skeleton.h"
#include "Vector3.h"
#include "ServerThread.h"
#include "DataMutex.h"

namespace RuGKinectInterfaceServer
{

class KinectConnector : public ServerThread
{
public:
	KinectConnector();
	~KinectConnector();

	virtual void run();

	void connect();
	void disconnect();
	bool isConnected() const;

	Skeleton getLatestSkeleton();

	bool hasDepthData() const { return m_bDepthDataInitialized; }
	bool hasVideoData() const { return m_bVideoDataInitialized; }

	NUI_IMAGE_FRAME const *getLatestDepthData() { return m_pLatestDepthData; }
	NUI_IMAGE_FRAME const *getLatestVideoData() { return m_pLatestVideoData; }

	DataMutex &getVideoMutex() { return m_ghVideoMutex; }
	DataMutex &getDepthMutex() { return m_ghDepthMutex; }
	DataMutex &getSkeletonMutex() { return m_ghSkeletonMutex; }

	static byte depthPixelToGrayscale(USHORT *pixel);
	static Vector3 depthPixelToSkeletonCoordinates(unsigned x, unsigned y, USHORT pixelValue);

private:
	void updateSkeletonData();
	void updateDepthData();
	void updateVideoData();

	bool setCameraAnlge(long degrees);

	static unsigned getClockInMs();

private:
	// status
	bool m_bConnected, m_bActive,
		 m_bDepthDataInitialized,
		 m_bVideoDataInitialized;
	bool *m_bArrSkeletonsTracked;
	unsigned m_nActiveSkeleton;

	// Data
	Skeleton *m_pLatestSkeleton;
	NUI_SKELETON_FRAME *m_pLatestSkeletonFrame;
	NUI_IMAGE_FRAME *m_pLatestDepthData;
	NUI_IMAGE_FRAME *m_pLatestVideoData;
	
	DataMutex m_ghSkeletonMutex, m_ghDepthMutex, m_ghVideoMutex;

	// Kinect thread handling
	HANDLE m_hNextSkeletonEvent, m_hNextDepthFrameEvent, m_hNextColorFrameEvent,
		   m_pDepthStreamHandle, m_pVideoStreamHandle;

	// Kinect sensor
	INuiSensor *m_pNuiSensor;
	BSTR m_instanceId;
};

}

#endif // ifdef WIN32
