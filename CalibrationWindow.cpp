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
: FXMainWindow(), m_bOpen( false ), m_uTrackedJoint( Skeleton::JOINT_HAND_RIGHT ), m_mxTransformation( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 )
{}

CalibrationWindow::CalibrationWindow(FXApp* a, string const &filename)
: FXMainWindow(a,WINDOW_TITLE,NULL,NULL,DECOR_ALL,0,0,1280,800), m_bOpen( false ), m_uTrackedJoint( Skeleton::JOINT_HAND_RIGHT ), m_mxTransformation( osg::Matrix::identity() ), m_sFilename( filename )
{
	FXVerticalFrame *pContents		= new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);
	FXHorizontalFrame *pTopHalf		= new FXHorizontalFrame(pContents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_HEIGHT,0,0,0,480, 0,0,0,0);
	FXHorizontalFrame *pBottomHalf	= new FXHorizontalFrame(pContents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	FXHorizontalFrame *pColorCanvasFrame = new FXHorizontalFrame(pTopHalf,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
	FXHorizontalFrame *pDepthCanvasFrame = new FXHorizontalFrame(pTopHalf,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
	
	m_pVideoCanvas = new FXCanvas(pColorCanvasFrame,this,ID_CANVAS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
	m_pDepthCanvas = new FXCanvas(pDepthCanvasFrame,this,ID_CANVAS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);

	FXVerticalFrame *pOptionsFrame = new FXVerticalFrame(pBottomHalf,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);

	FXHorizontalFrame *pOptionsSub  = new FXHorizontalFrame(pOptionsFrame,LAYOUT_SIDE_TOP|LAYOUT_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	FXVerticalFrame   *pInputFrame	= new FXVerticalFrame(pOptionsSub,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0);

	m_pJointSelect = new FXListBox(pInputFrame, this, ID_JOINTSELECT);

	m_pCoordinates = new FXLabel(pInputFrame,"Current coordinates: ",0,LABEL_NORMAL|LAYOUT_FIX_HEIGHT,0,0,0,20);
	new FXButton(pInputFrame,"Use these",0,this,ID_USE_CURRENT_COORDINATES);

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

	new FXButton(pButtonFrame,"Gebruik als referentie punt",0,this,ID_ADD_POINT_BUTTON);

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

	m_pJointSelect->appendItem("Elbow left");
	m_pJointSelect->appendItem("Wrist left");
	m_pJointSelect->appendItem("Hand left");
	m_pJointSelect->appendItem("Elbow right");
	m_pJointSelect->appendItem("Wrist right");
	m_pJointSelect->appendItem("Hand right");
	m_pJointSelect->appendItem("Knee left");
	m_pJointSelect->appendItem("Ankle left");
	m_pJointSelect->appendItem("Foot left");
	m_pJointSelect->appendItem("Knee right");
	m_pJointSelect->appendItem("Ankle right");
	m_pJointSelect->appendItem("Foot right");
	m_pJointSelect->setCurrentItem(5);
}

CalibrationWindow::~CalibrationWindow()
{}

void CalibrationWindow::create()
{
	FXMainWindow::create();

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
	repaintSkeletonCavas();
	repaintVideoCavas();

	return 0;
}

long CalibrationWindow::onJointSelect(FXObject*,FXSelector,void*ptr)
{
	cout << " * Tracking ";

	switch ( m_pJointSelect->getCurrentItem() )
	{
		case 0:
			m_uTrackedJoint = Skeleton::JOINT_ELBOW_LEFT;
			cout << "left elbow";
		break;

		case 1:
			m_uTrackedJoint = Skeleton::JOINT_WRIST_LEFT;
			cout << "left wrist";
		break;

		case 2:
			m_uTrackedJoint = Skeleton::JOINT_HAND_LEFT;
			cout << "left hand";
		break;

		case 3:
			m_uTrackedJoint = Skeleton::JOINT_ELBOW_RIGHT;
			cout << "right elbow";
		break;

		case 4:
			m_uTrackedJoint = Skeleton::JOINT_WRIST_RIGHT;
			cout << "right wrist";
		break;

		case 5:
			m_uTrackedJoint = Skeleton::JOINT_HAND_RIGHT;
			cout << "right hand";
		break;

		case 6:
			m_uTrackedJoint = Skeleton::JOINT_KNEE_LEFT;
			cout << "left knee";
		break;

		case 7:
			m_uTrackedJoint = Skeleton::JOINT_ANKLE_LEFT;
			cout << "left ankle";
		break;

		case 8:
			m_uTrackedJoint = Skeleton::JOINT_FOOT_LEFT;
			cout << "left foot";
		break;

		case 9:
			m_uTrackedJoint = Skeleton::JOINT_KNEE_RIGHT;
			cout << "right knee";
		break;

		case 10:
			m_uTrackedJoint = Skeleton::JOINT_ANKLE_RIGHT;
			cout << "right ankle";
		break;

		case 11:
			m_uTrackedJoint = Skeleton::JOINT_FOOT_RIGHT;
			cout << "right foot";
		break;
	}

	cout << " for mapping data";
	
	return 0;
}

void CalibrationWindow::updateTrackingInfo()
{
	Skeleton skeleton = Server::instance()->getConnector()->getLatestSkeleton();

	string coordinate_text = string("Current coordinates: ")
		+ skeleton.getJointPosition(m_uTrackedJoint).x_string() + ' '
		+ skeleton.getJointPosition(m_uTrackedJoint).y_string() + ' '
		+ skeleton.getJointPosition(m_uTrackedJoint).z_string() + ' ';
	m_pCoordinates->setText(coordinate_text.c_str());
}

long CalibrationWindow::onUseCoordinatesClick(FXObject*,FXSelector,void*ptr)
{
	Skeleton skeleton = Server::instance()->getConnector()->getLatestSkeleton();

	m_pCameraX->setText(skeleton.getJointPosition(m_uTrackedJoint).x_string().c_str());
	m_pCameraY->setText(skeleton.getJointPosition(m_uTrackedJoint).y_string().c_str());
	m_pCameraZ->setText(skeleton.getJointPosition(m_uTrackedJoint).z_string().c_str());

	return 0;
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

void CalibrationWindow::repaintSkeletonCavas()
{
	KinectConnector *pConnector = Server::instance()->getConnector();

	if ( !pConnector->hasDepthData() )
		return;

	Skeleton skeleton = pConnector->getLatestSkeleton();
	if ( !skeleton )
		return;

	int height = m_pDepthCanvas->getHeight();
	int width  = m_pDepthCanvas->getWidth();

	FXDCWindow dc(m_pDepthCanvas);
	dc.setForeground(m_pDepthCanvas->getBackColor());
	dc.fillRectangle( 0,0,width,height );

	dc.setForeground(FXRGB(0,0,255));
	for ( unsigned i = 0; i < Skeleton::BONE_COUNT; ++i )
	{
		Vector3 normalizedFirstPosition  = skeleton.getJointPosition(Skeleton::BONES[i][0]).kinectNormalized();
		Vector3 normalizedSecondPosition = skeleton.getJointPosition(Skeleton::BONES[i][1]).kinectNormalized();
		
		dc.drawLine((normalizedFirstPosition.x() + 0.5) * width + 5, (-normalizedFirstPosition.y() + 0.5) * height + 5, (normalizedSecondPosition.x() + 0.5) * width + 5, (-normalizedSecondPosition.y() + 0.5) * height + 5);
	}

	dc.setForeground( FXRGB(0,255,0) );
	Vector3 normalizedPosition = skeleton.getJointPosition(m_uTrackedJoint).kinectNormalized();
	dc.fillEllipse((normalizedPosition.x() + 0.5) * width, (-normalizedPosition.y() + 0.5) * height, 10, 10);
	updateTrackingInfo();
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
	//_________Message_Type___________________________ID___________________________________________Message_Handler________________
	FXMAPFUNC(SEL_UPDATE,			 CalibrationWindow::ID_CANVAS,						CalibrationWindow::onPaint),
	FXMAPFUNC(SEL_PAINT,			 CalibrationWindow::ID_CANVAS,						CalibrationWindow::onPaint),
	FXMAPFUNC(SEL_CLOSE,			 CalibrationWindow::ID_CANVAS,						CalibrationWindow::onClose),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_JOINTSELECT,					CalibrationWindow::onJointSelect),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_USE_CURRENT_COORDINATES,		CalibrationWindow::onUseCoordinatesClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_ADD_POINT_BUTTON,			CalibrationWindow::onAddPointClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_RESET_BUTTON,				CalibrationWindow::onResetClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_CALC_BUTTON,					CalibrationWindow::onCalcClick),
	FXMAPFUNC(SEL_COMMAND,			 CalibrationWindow::ID_SAVE_BUTTON,					CalibrationWindow::onSaveClick)
};

// Macro for the Kinect Client class hierarchy implementation
FXIMPLEMENT(CalibrationWindow,FXMainWindow,CalibrationWindowMap,ARRAYNUMBER(CalibrationWindowMap))

#endif //ifdef WIN32