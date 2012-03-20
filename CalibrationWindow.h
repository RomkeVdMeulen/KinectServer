#pragma once

#ifdef WIN32

#include <fx.h>
#include <string>
#include <vector>
#include <utility>
#include "Vector3.h"
#include <osg/Matrix>

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
		ID_SLIDER,
		ID_CAMERA_INPUT,
		ID_WORLD_INPUT,
		ID_RESET_BUTTON,
		ID_ADD_POINT_BUTTON,
		ID_SAVE_BUTTON,
		ID_LAST
    } MESSAGES;

	long onPaint(FXObject*,FXSelector,void*);
	long onMouseDown(FXObject*,FXSelector,void* ptr);
	long onSliderChange(FXObject*,FXSelector,void*ptr);
	long onAddPointClick(FXObject*,FXSelector,void*ptr);
	long onResetClick(FXObject*,FXSelector,void*);
	long onSaveClick(FXObject*,FXSelector,void*ptr);
	long onClose(FXObject*,FXSelector,void*);

protected:
	CalibrationWindow();

	FXuint m_uSelectedX, m_uSelectedY;
	Vector3 m_vSelectedPosition;

	FXCanvas			* m_pVideoCanvas;
	FXCanvas			* m_pDepthCanvas;

	FXLabel				* m_pCoordinates;
	FXLabel				* m_pCameraPoints;
	FXLabel				* m_pWorldPoints;
	FXLabel				* m_pTransformationDescription;
	FXLabel				* m_pSaveFeedback;

	FXSlider			* m_pFieldX;
	FXSlider			* m_pFieldY;

	FXTextField			* m_pCameraX;
	FXTextField			* m_pCameraY;
	FXTextField			* m_pCameraZ;
	FXTextField			* m_pWorldX;
	FXTextField			* m_pWorldY;
	FXTextField			* m_pWorldZ;

	std::vector< std::pair<Vector3,Vector3> > m_vPoints;
	osg::Matrix m_mxTransformation;

	std::string m_sFilename;

	bool m_bOpen;

	void repaintVideoCavas();
	void repaintDepthCavas();

	void updatePixelInfo();
	void resetPixelInfo();

	void updateStatusInfo();
	void recalcTransformation();

	float fxStringToFloat(FXString const &string) const;
};

}

#endif //ifdef WIN32
