#include "stdafx.h"

#ifdef WIN32

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>

//#include <pcl/point_cloud.h>
//#include <pcl/impl/point_types.hpp>
//#include <pcl/registration/transformation_estimation_svd.h>

#include "CalibrationWindow.h"
#include "Server.h"
#include "KinectConnector.h"
#include "Skeleton.h"

#include <sstream>
#include <fstream>
#include <iomanip>

using namespace RuGKinectInterfaceServer;

CalibrationWindow::CalibrationWindow()
: FXMainWindow(), m_bOpen( false ), m_uSelectedX( 0 ), m_uSelectedY( 0 ), m_mxTransformation( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 )
{}

CalibrationWindow::CalibrationWindow(FXApp* a, string const &filename)
: FXMainWindow(a,WINDOW_TITLE,NULL,NULL,DECOR_ALL,0,0,1280,800), m_bOpen( false ), m_uSelectedX( 0 ), m_uSelectedY( 0 ), m_mxTransformation( osg::Matrix::identity() ), m_sFilename( filename )
{
	FXVerticalFrame *pContents		= new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);
	FXHorizontalFrame *pTopHalf		= new FXHorizontalFrame(pContents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_HEIGHT,0,0,0,480, 0,0,0,0);
	FXHorizontalFrame *pBottomHalf	= new FXHorizontalFrame(pContents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	FXHorizontalFrame *pColorCanvasFrame = new FXHorizontalFrame(pTopHalf,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	m_pFieldY = new FXSlider(pTopHalf,this,ID_SLIDER,SLIDER_VERTICAL|LAYOUT_FIX_HEIGHT,0,0,0,480);
	m_pFieldY->setValue(0);
	m_pFieldY->setRange(0,480);

	FXHorizontalFrame *pDepthCanvasFrame = new FXHorizontalFrame(pTopHalf,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
	
	m_pVideoCanvas = new FXCanvas(pColorCanvasFrame,this,ID_CANVAS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
	m_pDepthCanvas = new FXCanvas(pDepthCanvasFrame,this,ID_CANVAS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);

	FXVerticalFrame *pOptionsFrame = new FXVerticalFrame(pBottomHalf,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);

	m_pFieldX = new FXSlider(pOptionsFrame,this,ID_SLIDER,SLIDER_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_X,640,0,640);
	m_pFieldX->setValue(0);
	m_pFieldX->setRange(0,640);

	FXHorizontalFrame *pOptionsSub  = new FXHorizontalFrame(pOptionsFrame,LAYOUT_SIDE_TOP|LAYOUT_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	FXVerticalFrame   *pInputFrame	= new FXVerticalFrame(pOptionsSub,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);

	m_pCoordinates = new FXLabel(pInputFrame,"",0,LABEL_NORMAL|LAYOUT_FIX_HEIGHT,0,0,0,20);

	FXHorizontalFrame *pInputFields	= new FXHorizontalFrame(pInputFrame,LAYOUT_SIDE_TOP|LAYOUT_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
	FXVerticalFrame   *pCameraInput	= new FXVerticalFrame(pInputFields,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);
	FXVerticalFrame   *pWorldInput	= new FXVerticalFrame(pInputFields,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);

	new FXLabel(pCameraInput,"Camera coordinaten");

	m_pCameraX = new FXTextField(pCameraInput,8,this,ID_CAMERA_INPUT);
	m_pCameraY = new FXTextField(pCameraInput,8,this,ID_CAMERA_INPUT);
	m_pCameraZ = new FXTextField(pCameraInput,8,this,ID_CAMERA_INPUT);

	new FXLabel(pWorldInput,"Ruimte coordinaten");

	m_pWorldX = new FXTextField(pWorldInput,8,this,ID_WORLD_INPUT);
	m_pWorldY = new FXTextField(pWorldInput,8,this,ID_WORLD_INPUT);
	m_pWorldZ = new FXTextField(pWorldInput,8,this,ID_WORLD_INPUT);

	FXHorizontalFrame *pButtonFrame	= new FXHorizontalFrame(pInputFrame,LAYOUT_SIDE_TOP|LAYOUT_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	new FXButton(pButtonFrame,"Gebruik als referentie punt",0,this,ID_ADD_POINT_BUTTON,BUTTON_NORMAL,0,0);

	FXVerticalFrame   *pStorageFrame	= new FXVerticalFrame(pOptionsSub,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);
	FXHorizontalFrame *pStatusFrame 	= new FXHorizontalFrame(pStorageFrame,LAYOUT_SIDE_TOP|LAYOUT_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
	FXVerticalFrame   *pPointsFrame		= new FXVerticalFrame(pStatusFrame,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,300,0);
	FXVerticalFrame   *pMatrixFrame		= new FXVerticalFrame(pStatusFrame,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,20);

	new FXLabel(pPointsFrame,"Referentie punten");
	FXHorizontalFrame *pPointsLists 	= new FXHorizontalFrame(pPointsFrame,LAYOUT_SIDE_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
	m_pCameraPoints = new FXLabel(pPointsLists,"",0,LABEL_NORMAL,0,0,0,0,2,15);
	m_pWorldPoints  = new FXLabel(pPointsLists,"");

	new FXButton(pPointsFrame,"Reset",0,this,ID_RESET_BUTTON,BUTTON_NORMAL,0,0);
	m_pCalcButton = new FXButton(pPointsFrame,"Bereken",0,this,ID_CALC_BUTTON,BUTTON_NORMAL,0,0);
	m_pCalcButton->disable();

	new FXLabel(pMatrixFrame,"Transformatie");
	m_pTransformationDescription = new FXLabel(pMatrixFrame,"");

	new FXButton(pStorageFrame,"Opslaan",0,this,ID_SAVE_BUTTON,BUTTON_NORMAL,0,0);

	m_pSaveFeedback = new FXLabel(pStorageFrame,"");
}

CalibrationWindow::~CalibrationWindow()
{}

void CalibrationWindow::create()
{
	FXMainWindow::create();
	m_pFieldX->create();
	m_pFieldY->create();

	std::cout << " * Opening calibration window\n";

#ifdef USE_OPENCV
	std::cout << " * Using openCV calibration algorythm\n";
#else
	std::cout << " * Using homebaked calibration algorythm\n";
#endif

	show(PLACEMENT_SCREEN);

	m_bOpen = true;
}

long CalibrationWindow::onClose(FXObject*,FXSelector,void*)
{
	m_bOpen = false;

	return 0;
}

long CalibrationWindow::onPaint(FXObject*,FXSelector,void*)
{
	repaintDepthCavas();
	repaintVideoCavas();

	return 0;
}

long CalibrationWindow::onMouseDown(FXObject*,FXSelector,void* ptr)
{
	FXEvent *ev=(FXEvent*) ptr;
	m_uSelectedX = ev->win_x;
	m_uSelectedY = ev->win_y;
	resetPixelInfo();

	return 0;
}

long CalibrationWindow::onSliderChange(FXObject*,FXSelector,void*ptr)
{
	m_uSelectedX = m_pFieldX->getValue();
	m_uSelectedY = 480 - m_pFieldY->getValue();
	resetPixelInfo();

	return 0;
}

void CalibrationWindow::updatePixelInfo()
{
	m_pFieldX->setValue(m_uSelectedX);
	m_pFieldY->setValue(480 - m_uSelectedY);

	string coordinate_text = string("Current coordinates: ") + 
		m_vSelectedPosition.x_string() + ", " + 
		m_vSelectedPosition.y_string() + ", " + 
		m_vSelectedPosition.z_string();
	m_pCoordinates->setText(coordinate_text.c_str());

	if ( !m_pCameraX->getText() )
		m_pCameraX->setText(m_vSelectedPosition.x_string().c_str());
	if ( !m_pCameraY->getText() )
		m_pCameraY->setText(m_vSelectedPosition.y_string().c_str());
	if ( !m_pCameraZ->getText() )
		m_pCameraZ->setText(m_vSelectedPosition.z_string().c_str());
}

void CalibrationWindow::resetPixelInfo()
{
	m_vSelectedPosition = Vector3();
	
	m_pCoordinates->setText("");
	m_pCameraX->setText("");
	m_pCameraY->setText("");
	m_pCameraZ->setText("");

	m_pSaveFeedback->setText("");
}

long CalibrationWindow::onAddPointClick(FXObject*,FXSelector,void*ptr)
{
	Vector3 cameraPosition(
		fxStringToFloat(m_pCameraX->getText()),
		fxStringToFloat(m_pCameraY->getText()),
		fxStringToFloat(m_pCameraZ->getText())
	);
	Vector3 worldPosition(
		fxStringToFloat(m_pWorldX->getText()),
		fxStringToFloat(m_pWorldY->getText()),
		fxStringToFloat(m_pWorldZ->getText())
	);
	m_vPoints.push_back(pair<Vector3,Vector3>(cameraPosition,worldPosition));

	if ( m_vPoints.size() > 3)
		m_pCalcButton->enable();

	updateStatusInfo();
	
	return 0;
}

void CalibrationWindow::recalcTransformation()
{
	if ( m_vPoints.size() < 4 )
		return;

	ostringstream feedback;
	feedback << "Matrix berekenen met " << m_vPoints.size() << " punten...";
	m_pSaveFeedback->setText(feedback.str().c_str());

#ifdef USE_OPENCV
	cv::Mat camera(1, m_vPoints.size(), CV_32FC3);
    cv::Mat  world(1, m_vPoints.size(), CV_32FC3);

	for ( unsigned i = 0; i < m_vPoints.size(); ++i )
	{
		Vector3 cameraVector = m_vPoints[i].first;
		Vector3 worldVector  = m_vPoints[i].second;

		camera.at<cv::Point3f>(i) = cv::Point3f(cameraVector.x(), cameraVector.y(), cameraVector.z());
		world.at<cv::Point3f>(i)  = cv::Point3f(worldVector.x(), worldVector.y(), worldVector.z());
	}
   
    cv::Mat estimate;
    std::vector<uchar> outliers;
	try {
		cout << " * Estimating transform from " << m_vPoints.size() << " points...\n";

		int res = estimateAffine3D(camera, world, estimate, outliers);

		cout << " * Estimated new transform with " << outliers.size() << " outliers.\n";

		m_mxTransformation.makeIdentity();
		for ( unsigned x = 0; x < 4; ++x )
			for ( unsigned y = 0; y < 3; ++y )
				m_mxTransformation(x,y) = estimate.at<double>(y,x);

	} catch ( cv::Exception &exception ) {
		cerr << "\n### Error while trying to estimate transform: " << exception.msg << " ###\n";
	}
#else
	float camera[4][4];
	float world[4][4];
	for ( unsigned i = 0; i < 4; ++i )
	{
		Vector3 cameraVector = m_vPoints[i].first;
		Vector3 worldVector  = m_vPoints[i].second;

		camera[i][0] = cameraVector.x();
		camera[i][1] = cameraVector.y();
		camera[i][2] = cameraVector.z();

		world[i][0] = worldVector.x();
		world[i][1] = worldVector.y();
		world[i][2] = worldVector.z();
	}

	osg::Matrix C((float *) camera);
	osg::Matrix W((float *) world);
	
	m_mxTransformation = C.inverse(C) * W;
	m_mxTransformation(0,3) = m_mxTransformation(1,3) = m_mxTransformation(2,3) = 0;
#endif

	// Uses the PCL library
	/*
	pcl::PointCloud<pcl::PointXYZ> camera;
	pcl::PointCloud<pcl::PointXYZ> world;
	...
	pcl::registration::TransformationEstimationSVD<pcl::PointXYZ,pcl::PointXYZ>;
	*/

	return;
}

void CalibrationWindow::updateStatusInfo()
{
	m_pSaveFeedback->setText("");
	
	string cameraPointsInfo;
	string worldPointsInfo;
	for ( vector< pair<Vector3,Vector3> >::iterator i = m_vPoints.begin(); i != m_vPoints.end(); ++i )
	{
		cameraPointsInfo +=
			i->first.x_string() + ", " + i->first.y_string() + ", " + i->first.z_string() + "\n";
		worldPointsInfo +=
			i->second.x_string() + ", " + i->second.y_string() + ", " + i->second.z_string() + "\n";
	}
	m_pCameraPoints->setText(cameraPointsInfo.c_str());
	m_pWorldPoints->setText(worldPointsInfo.c_str());

	
	osg::Vec3d translation;
    osg::Quat rotation;
    osg::Vec3d scale;
    osg::Quat so;
	m_mxTransformation.decompose(translation, rotation, scale, so);
	ostringstream transformationInfo;
	transformationInfo.unsetf(ios::floatfield);
	transformationInfo
		<< setw(8) << setprecision(4) << m_mxTransformation(0,0) << setw(8) << setprecision(4) << m_mxTransformation(0,1) << setw(8) << setprecision(4) << m_mxTransformation(0,2) << setw(8) << setprecision(4) << m_mxTransformation(0,3) << "\n"
		<< setw(8) << setprecision(4) << m_mxTransformation(1,0) << setw(8) << setprecision(4) << m_mxTransformation(1,1) << setw(8) << setprecision(4) << m_mxTransformation(1,2) << setw(8) << setprecision(4) << m_mxTransformation(1,3) << "\n"
		<< setw(8) << setprecision(4) << m_mxTransformation(2,0) << setw(8) << setprecision(4) << m_mxTransformation(2,1) << setw(8) << setprecision(4) << m_mxTransformation(2,2) << setw(8) << setprecision(4) << m_mxTransformation(2,3) << "\n"
		<< setw(8) << setprecision(4) << m_mxTransformation(3,0) << setw(8) << setprecision(4) << m_mxTransformation(3,1) << setw(8) << setprecision(4) << m_mxTransformation(3,2) << setw(8) << setprecision(4) << m_mxTransformation(3,3) << "\n"
		<< "\n\n"
		<< "Translate: " << setw(8) << setprecision(4) << translation.x() << setw(8) << setprecision(4) << translation.y() << setw(8) << setprecision(4) << translation.z() << "\n"
		<< "Rotate: " << setw(8) << setprecision(4) << rotation.x() << setw(8) << setprecision(4) << rotation.y() << setw(8) << setprecision(4) << rotation.z() << setw(8) << setprecision(4) << rotation.w() << "\n"
		<< "Scale: " << setw(8) << setprecision(4) << scale.x() << setw(8) << setprecision(4) << scale.y() << setw(8) << setprecision(4) << scale.z() << "\n"
		<< "Scale orient: " << setw(8) << setprecision(4) << so.x() << setw(8) << setprecision(4) << so.y() << setw(8) << setprecision(4) << so.z() << setw(8) << setprecision(4) << so.w() << "\n";
	m_pTransformationDescription->setText(transformationInfo.str().c_str());
}

long CalibrationWindow::onResetClick(FXObject*,FXSelector,void*)
{
	m_mxTransformation.makeIdentity();
	m_vPoints.clear();

	updateStatusInfo();

	return 0;
}

long CalibrationWindow::onCalcClick(FXObject*,FXSelector,void*)
{
	recalcTransformation();

	updateStatusInfo();

	return 0;
}

long CalibrationWindow::onSaveClick(FXObject*,FXSelector,void*ptr)
{
	ofstream fileStore( m_sFilename.c_str() );
	fileStore
		<< m_mxTransformation(0,0) << ' ' << m_mxTransformation(0,1) << ' ' << m_mxTransformation(0,2) << ' ' << m_mxTransformation(0,3) << "\n"
		<< m_mxTransformation(1,0) << ' ' << m_mxTransformation(1,1) << ' ' << m_mxTransformation(1,2) << ' ' << m_mxTransformation(1,3) << "\n"
		<< m_mxTransformation(2,0) << ' ' << m_mxTransformation(2,1) << ' ' << m_mxTransformation(2,2) << ' ' << m_mxTransformation(2,3) << "\n"
		<< m_mxTransformation(3,0) << ' ' << m_mxTransformation(3,1) << ' ' << m_mxTransformation(3,2) << ' ' << m_mxTransformation(3,3) << "\n"
		<< "\n\n\nUsed reference points:\n\n";

	for ( unsigned i = 0; i < m_vPoints.size(); ++i )
	{
		Vector3 cameraVector = m_vPoints[i].first;
		Vector3 worldVector  = m_vPoints[i].second;

		fileStore
			 << cameraVector.x() << ' ' << cameraVector.y() << ' ' << cameraVector.z()
			 << " => "
			 << worldVector.x() << ' ' << worldVector.y() << ' ' << worldVector.z()
			 << "\n";
	}

#ifdef USE_OPENCV
	fileStore << "Algorythm: openCV RANSAC\n";
#else
	fileStore << "Algorythm: homebrew transpose matrix\n";
#endif

	fileStore << "\n";
	fileStore.close();

	m_pSaveFeedback->setText((string("Matrix geschreven naar ") + m_sFilename).c_str());

	return 0;
}

void CalibrationWindow::repaintDepthCavas()
{
	KinectConnector *pConnector = Server::instance()->getConnector();

	if ( !pConnector->hasDepthData() )
		return;

	pConnector->getDepthMutex().lock();
	
	NUI_IMAGE_FRAME const *pData = pConnector->getLatestDepthData();
	INuiFrameTexture *pTexture = pData->pFrameTexture;
	if ( !pTexture )
	{
		pConnector->getDepthMutex().unlock();
		return;
	}
	FXDCWindow dc(m_pDepthCanvas);

	NUI_LOCKED_RECT LockedRect;
	if ( pTexture->LockRect( 0, &LockedRect, NULL, 0 ) != S_OK)
	{
		pConnector->getDepthMutex().unlock();
		return;
	}

	if ( LockedRect.Pitch != 0 )
	{
		FXColor *colormap = new FXColor[640*480];
		for ( unsigned y = 0; y < 480; ++y )
			for ( unsigned x = 0; x < 640; ++x )
			{
				unsigned i = ((y * 640) + x);
				// Depth pixel map: 2 bytes to the pixel
				byte grayvalue = KinectConnector::depthPixelToGrayscale(reinterpret_cast<USHORT *>(LockedRect.pBits + (2 * i)));
				grayvalue = (grayvalue * 2) % 256;
				if ( grayvalue < 255 )
				{
					grayvalue = min(255,grayvalue + 10);
					colormap[i] = FXRGB(grayvalue,grayvalue,grayvalue);
				}
				else
					colormap[i] = FXRGB(255,240,240);
			}
		FXImage *pDepthImage = new FXImage(getApp(), colormap, 0, 640, 480);
		pDepthImage->create();
		dc.drawImage(pDepthImage,0,0);
		delete[] colormap, delete pDepthImage;

		if ( m_uSelectedX != 0 || m_uSelectedY != 0 )
		{
			dc.setForeground(FXRGB(255,0,0));
			dc.drawLine(m_uSelectedX - 5,m_uSelectedY,m_uSelectedX + 5,m_uSelectedY);
			dc.setForeground(FXRGB(0,255,0));
			dc.drawLine(m_uSelectedX,m_uSelectedY - 5,m_uSelectedX,m_uSelectedY + 5);

			unsigned i = ((m_uSelectedY * 640) + m_uSelectedX);
			m_vSelectedPosition = KinectConnector::depthPixelToSkeletonCoordinates(m_uSelectedX,m_uSelectedY,*reinterpret_cast<USHORT *>(LockedRect.pBits + (2 * i)));
			updatePixelInfo();
			
		}
	}

	pConnector->getDepthMutex().unlock();

	Skeleton skeleton = pConnector->getLatestSkeleton();
	if ( skeleton )
	{
		dc.setForeground(FXRGB(0,0,255));
		for ( unsigned i = 0; i < Skeleton::BONE_COUNT; ++i )
		{
			Vector3 normalizedFirstPosition  = skeleton.getJointPosition(Skeleton::BONES[i][0]).kinectNormalized();
			Vector3 normalizedSecondPosition = skeleton.getJointPosition(Skeleton::BONES[i][1]).kinectNormalized();
		
			int height = m_pDepthCanvas->getHeight();
			int width  = m_pDepthCanvas->getWidth();

			dc.drawLine((normalizedFirstPosition.x() + 0.5) * width + 5, (-normalizedFirstPosition.y() + 0.5) * height + 5, (normalizedSecondPosition.x() + 0.5) * width + 5, (-normalizedSecondPosition.y() + 0.5) * height + 5);
		}
	}
}

void CalibrationWindow::repaintVideoCavas()
{
	KinectConnector *pConnector = Server::instance()->getConnector();
	if ( !pConnector->hasVideoData() )
		return;

	NUI_IMAGE_FRAME const *pData = pConnector->getLatestVideoData();

	pConnector->getVideoMutex().lock();

	INuiFrameTexture *pTexture = pData->pFrameTexture;
	if ( !pTexture )
	{
		pConnector->getVideoMutex().unlock();
		return;
	}
	FXDCWindow dc(m_pVideoCanvas);
	
	NUI_LOCKED_RECT LockedRect;
	if (pTexture->LockRect( 0, &LockedRect, NULL, 0 ) != S_OK)
	{
		pConnector->getVideoMutex().unlock();
		return;
	}

	if ( LockedRect.Pitch != 0 )
	{
		FXColor *colormap = new FXColor[640*480];
		FXColor red = FXRGB(255,0,0);
		for ( unsigned y = 0; y < 480; ++y )
			for ( unsigned x = 0; x < 640; ++x )
			{
				unsigned i = ((y * 640) + x);
				// Color pixel map: 4 bytes to the pixel (BGRA)
				colormap[i] = FXRGB(LockedRect.pBits[4 * i + 2], LockedRect.pBits[4 * i + 1], LockedRect.pBits[4 * i]);
			}

		FXImage *pVideoImage = new FXImage(getApp(), colormap, 0, 640, 480);
		pVideoImage->create();
		dc.drawImage(pVideoImage,0,0);
		delete[] colormap, delete pVideoImage;

		if ( m_uSelectedX != 0 || m_uSelectedY != 0 )
		{
			dc.setForeground(FXRGB(255,0,0));
			dc.drawLine(m_uSelectedX - 5,m_uSelectedY,m_uSelectedX + 5,m_uSelectedY);
			dc.setForeground(FXRGB(0,255,0));
			dc.drawLine(m_uSelectedX,m_uSelectedY - 5,m_uSelectedX,m_uSelectedY + 5);
		}
	}

	pConnector->getVideoMutex().unlock();
}

float CalibrationWindow::fxStringToFloat(FXString const &string) const
{
	if ( !string )
		return 0.00;

	istringstream stream(string.text());
	float retValue;
	stream >> retValue;
	return retValue;
}

// Message Map for the Kinect Client Window class
FXDEFMAP(CalibrationWindow) CalibrationWindowMap[]={
	//_________Message_Type___________________________ID__________________________________Message_Handler__________
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,   CalibrationWindow::ID_CANVAS,				CalibrationWindow::onMouseDown),
	FXMAPFUNC(SEL_UPDATE,			 CalibrationWindow::ID_CANVAS,				CalibrationWindow::onPaint),
	FXMAPFUNC(SEL_PAINT,			 CalibrationWindow::ID_CANVAS,				CalibrationWindow::onPaint),
	FXMAPFUNC(SEL_CLOSE,			 CalibrationWindow::ID_CANVAS,				CalibrationWindow::onClose),
	FXMAPFUNC(SEL_CHANGED,			 CalibrationWindow::ID_SLIDER,				CalibrationWindow::onSliderChange),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_ADD_POINT_BUTTON,	CalibrationWindow::onAddPointClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_RESET_BUTTON,		CalibrationWindow::onResetClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_CALC_BUTTON,			CalibrationWindow::onCalcClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_SAVE_BUTTON,			CalibrationWindow::onSaveClick)
};

// Macro for the Kinect Client class hierarchy implementation
FXIMPLEMENT(CalibrationWindow,FXMainWindow,CalibrationWindowMap,ARRAYNUMBER(CalibrationWindowMap))

#endif //ifdef WIN32