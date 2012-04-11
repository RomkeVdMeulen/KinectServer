#include "stdafx.h"

#ifdef WIN32

#include "KinectConnector.h"
#include "Server.h"
#include <time.h>

using namespace RuGKinectInterfaceServer;

KinectConnector::KinectConnector()
: m_bConnected( false ), m_bActive( false ),
  m_hNextSkeletonEvent( NULL ), m_hNextDepthFrameEvent( NULL ), m_hNextColorFrameEvent( NULL ),
  m_pDepthStreamHandle( NULL ), m_pVideoStreamHandle( NULL ),
  m_pNuiSensor( NULL ), m_instanceId( NULL ), m_pLatestSkeletonFrame( NULL ),
  m_nActiveSkeleton( 0 )
{
	m_bArrSkeletonsTracked = new bool[NUI_SKELETON_COUNT];
	for ( int i = 0; i < NUI_SKELETON_COUNT; ++i )
	{
		m_bArrSkeletonsTracked[i] = false;
	}

	m_pLatestSkeletonFrame = new NUI_SKELETON_FRAME;
	m_pLatestSkeleton = new Skeleton;

	m_pLatestDepthData = new NUI_IMAGE_FRAME;
	m_pLatestVideoData = new NUI_IMAGE_FRAME;
	m_bDepthDataInitialized = false;
	m_bVideoDataInitialized = false;
}

KinectConnector::~KinectConnector()
{
	if ( m_bConnected )
		disconnect();

	delete m_bArrSkeletonsTracked, delete m_pLatestSkeletonFrame, delete m_pLatestSkeleton,
	delete m_pLatestDepthData, delete m_pLatestVideoData;
}

void KinectConnector::connect()
{
	HRESULT hr;

	int nSensorCount = 0;
	hr = NuiGetSensorCount(&nSensorCount);
	if ( FAILED(hr) ) {
		cerr << "\nCould not retrieve number of available Kinect sensors.\n";
		throw KINECT_CONNECTION_ERROR;
	}
	if ( nSensorCount < 1 )
	{
		cerr << "\nNo Kinect sensors found.\n";
		throw KINECT_CONNECTION_ERROR;
	}
	
	cout << " * Connecting to Kinect sensor...   ";

	hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);
	if ( FAILED(hr) ) {
		cerr << "could not connect\n";
		throw KINECT_CONNECTION_ERROR;
	}

	m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();

	cout << "connected to sensor " << *m_instanceId << "\n";

	cout << " * Initializing Kinect depth, skeleton and video streams...   ";


	DWORD nuiFlags;
	if ( Server::instance()->isMode( Server::CALIBRATE ) )
		nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR;
	else
		nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON;

    hr = m_pNuiSensor->NuiInitialize( nuiFlags );
    if ( E_NUI_SKELETAL_ENGINE_BUSY == hr ) {
		cerr << "could not initialize: skeletal engine is busy (is another Kinect app running?)\n";
		throw KINECT_CONNECTION_ERROR;
	}
	else if ( E_NUI_DEVICE_IN_USE == hr ) {
		cerr << "could not initialize: device is busy (is another Kinect app running?)\n";
		throw KINECT_CONNECTION_ERROR;
	}
	else if ( FAILED( hr ) ) {
		cerr << "could not initialize: unknown error " << hr << "\n";
		throw KINECT_CONNECTION_ERROR;
	}

	if ( !HasSkeletalEngine(m_pNuiSensor) )
		throw "Error: sensor has no skeletal engine.\n";

	m_hNextDepthFrameEvent = CreateEvent( NULL, true, false, NULL );
    m_hNextSkeletonEvent = CreateEvent( NULL, true, false, NULL );
	m_hNextColorFrameEvent = CreateEvent( NULL, true, false, NULL );

    hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
    if( FAILED( hr ) ) {
		cerr << "could not initialize skeletal tracking: unknown error " << hr << "\n";
		throw KINECT_CONNECTION_ERROR;
	}

	if ( Server::instance()->isMode( Server::CALIBRATE ) )
	{
		hr = m_pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			m_hNextColorFrameEvent,
			&m_pVideoStreamHandle );

		if ( FAILED( hr ) )
		{
			cerr << "unknown error while initializing video stream\n";
			throw KINECT_CONNECTION_ERROR;
		}
	}

    hr = m_pNuiSensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );

    if ( FAILED( hr ) )
    {
        cerr << "unknown error while initializing depth stream\n";
        throw KINECT_CONNECTION_ERROR;
    }

	cout << "initialized\n";

	m_bConnected = true;
}

void KinectConnector::disconnect()
{
	cout << " * Disconnecting from Kinect sensor...  ";

	if ( m_pNuiSensor )
    {
        m_pNuiSensor->NuiShutdown( );
    }
    if ( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }

	cout << "done.\n";

	m_bConnected = false;
}

bool KinectConnector::isConnected() const
{
	return m_bConnected;
}

bool KinectConnector::setCameraAnlge(long degrees)
{
	cout << " * Setting camera angle to " << degrees << "\n";
	return ( NuiCameraElevationSetAngle(degrees) == S_OK );
}

void KinectConnector::run()
{
	cout << " * Kinect Processing thread started, waiting for input\n";

	const int numEvents = 3;
	HANDLE hEvents[numEvents] = { m_hNextColorFrameEvent, m_hNextDepthFrameEvent, m_hNextSkeletonEvent};
    int    nEventIdx;

	// Main thread loop
	while ( m_bContinueThread )
    {
        // Wait for any of the events to be signalled
        nEventIdx = WaitForMultipleObjects( numEvents, hEvents, false, LISTEN_FOR_KINECT_SIGNALS_EVERY_MS );

		// Process signal events
        switch ( nEventIdx )
        {
			case WAIT_OBJECT_0:
				updateVideoData();
				break;

            case WAIT_OBJECT_0 + 1:
				updateDepthData();
                break;

            case WAIT_OBJECT_0 + 2:
				updateSkeletonData();
                break;

			case WAIT_TIMEOUT:
			default:
				continue;
        }
    }

	cout << " * No longer processing Kinect input\n";
}

void KinectConnector::updateDepthData()
{
	m_ghDepthMutex.lock();
	
	if ( m_bDepthDataInitialized )
		m_pNuiSensor->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, m_pLatestDepthData );

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        m_pLatestDepthData );

	m_ghDepthMutex.unlock();

    if ( FAILED( hr ) )
        // Never mind, we'll get it next cycle
		return;
	else
		Server::instance()->signalNewDepthDataAvailable();

	m_bDepthDataInitialized = true;
}

void KinectConnector::updateVideoData()
{
	m_ghVideoMutex.lock();

	if ( m_bVideoDataInitialized )
		m_pNuiSensor->NuiImageStreamReleaseFrame( m_pVideoStreamHandle, m_pLatestVideoData );

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(
        m_pVideoStreamHandle,
        0,
        m_pLatestVideoData );

	m_ghVideoMutex.unlock();

    if ( FAILED( hr ) )
        // Never mind, we'll get it next cycle
		return;
	else
		Server::instance()->signalNewVideoDataAvailable();

	m_bVideoDataInitialized = true;
}

unsigned KinectConnector::getClockInMs()
{
	return  ((float) clock() / CLOCKS_PER_SEC) * 1000;
}

void KinectConnector::updateSkeletonData()
{
    bool bFoundSkeleton = false;
	float fClosestDistance = 0;
    if ( SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, m_pLatestSkeletonFrame )) )
    {
        for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {
            if( m_pLatestSkeletonFrame->SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
            {
                if ( !m_bArrSkeletonsTracked[i] )
				{
					cout << " * Skeleton " << i << " found at " << m_pLatestSkeletonFrame->SkeletonData[i].Position.z << " meters from camera, " << (int) (clock() / CLOCKS_PER_SEC) << " seconds after startup\n\n";
					
					if ( fClosestDistance == 0 || fClosestDistance > m_pLatestSkeletonFrame->SkeletonData[i].Position.z )
					{
						m_nActiveSkeleton = i;
						fClosestDistance =  m_pLatestSkeletonFrame->SkeletonData[i].Position.z;
					}

					m_bArrSkeletonsTracked[i] = true;
				}
				
				bFoundSkeleton = true;

            } else if ( m_bArrSkeletonsTracked[i] ) {
				cout << " * Skeleton " << i << " lost\n\n";
				m_bArrSkeletonsTracked[i] = false;
			}
        }
    } else {
		return;
	}

    // no skeletons!
    if( !bFoundSkeleton )
    {
		return;
    }

#ifdef APPLY_SMOOTHING
	// smooth out the skeleton data
    HRESULT hr = m_pNuiSensor->NuiTransformSmooth(m_pLatestSkeletonFrame,NULL);
#endif

	// m_pLatestSkeletonFrame->vNormalToGravity is never set
	// but the A, B, and C coefficients of the plane equation are also a normal vector

	m_ghSkeletonMutex.lock();

	delete m_pLatestSkeleton;
	m_pLatestSkeleton = new Skeleton(&m_pLatestSkeletonFrame->SkeletonData[m_nActiveSkeleton], getClockInMs(), &m_pLatestSkeletonFrame->vFloorClipPlane);

	m_ghSkeletonMutex.unlock();

	Server::instance()->signalNewSkeletonDataAvailable();
}

Skeleton KinectConnector::getLatestSkeleton()
{ 
	m_ghSkeletonMutex.lock();

	Skeleton returnValue(*m_pLatestSkeleton);

	m_ghSkeletonMutex.unlock();

	return returnValue;
}

byte KinectConnector::depthPixelToGrayscale(USHORT *pixel)
{
	USHORT RealDepth = NuiDepthPixelToDepth(*pixel);
	return (byte)~(RealDepth >> 4);
}

Vector3 KinectConnector::depthPixelToSkeletonCoordinates(unsigned x, unsigned y, USHORT pixel)
{
	return NuiTransformDepthImageToSkeleton(x,y,pixel);
}

#endif // ifdef WIN32
