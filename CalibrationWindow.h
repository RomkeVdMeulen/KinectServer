#pragma once

#ifdef WIN32

#include <fx.h>
#include <string>
#include <vector>
#include <utility>
#include "Vector3.h"
#include <osg/Matrix>
#include "Skeleton.h"

namespace RuGKinectInterfaceServer
{

class CalibrationWindow : public FXMainWindow
{
	// Macro for class hierarchy declarations
	FXDECLARE(CalibrationWindow)

public:
	CalibrationWindow(FXApp* a, std::string const &filename);
	virtual ~CalibrationWindow();

	virtual void create();

	void redrawVideo();

	// Messages for our class
	enum {
		FIRST=FXMainWindow::ID_LAST,
		ID_CANVAS,
		ID_JOINTSELECT,
		ID_CAMERA_INPUT,
		ID_WORLD_INPUT,
		ID_USE_CURRENT_COORDINATES,
		ID_RESET_BUTTON,
		ID_ADD_POINT_BUTTON,
		ID_CALC_BUTTON,
		ID_SAVE_BUTTON,
		ID_LAST
    } MESSAGES;

	long onPaint(FXObject*,FXSelector,void*);
	long onUseCoordinatesClick(FXObject*,FXSelector,void*ptr);
	long onJointSelect(FXObject*,FXSelector,void*ptr);
	long onAddPointClick(FXObject*,FXSelector,void*ptr);
	long onResetClick(FXObject*,FXSelector,void*);
	long onCalcClick(FXObject*,FXSelector,void*);
	long onSaveClick(FXObject*,FXSelector,void*ptr);
	long onClose(FXObject*,FXSelector,void*);

protected:
	CalibrationWindow();

	FXCanvas			* m_pVideoCanvas;
	FXCanvas			* m_pDepthCanvas;

	FXLabel				* m_pCoordinates;
	FXLabel				* m_pCameraPoints;
	FXLabel				* m_pWorldPoints;
	FXLabel				* m_pCalculatedWorldPoints;
	FXLabel				* m_pTransformationDescription;
	FXLabel				* m_pSaveFeedback;

	FXButton			* m_pCalcButton;

	FXTextField			* m_pCameraX;
	FXTextField			* m_pCameraY;
	FXTextField			* m_pCameraZ;
	FXTextField			* m_pWorldX;
	FXTextField			* m_pWorldY;
	FXTextField			* m_pWorldZ;

	FXListBox			* m_pJointSelect;

	Skeleton::SKELETON_JOINT_INDEX m_uTrackedJoint;

	std::vector< std::pair<Vector3,Vector3> > m_vPoints;
	osg::Matrix m_mxTransformation;

	std::string m_sFilename;

	bool m_bOpen;

	void repaintVideoCavas();
	void repaintSkeletonCavas();

	void updateTrackingInfo();

	void updateStatusInfo();
	void recalcTransformation();

	float fxStringToFloat(FXString const &string) const;
};

}

#endif //ifdef WIN32
