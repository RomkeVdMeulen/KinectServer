#pragma once

#ifdef WIN32
	#include "NuiApi.h"
#else
	typedef struct _Vector4
	{
	    FLOAT x;
	    FLOAT y;
	    FLOAT z;
	    FLOAT w;
	} Vector4;

	typedef
	enum _NUI_SKELETON_POSITION_TRACKING_STATE
	    {	NUI_SKELETON_POSITION_NOT_TRACKED	= 0,
		NUI_SKELETON_POSITION_INFERRED	= ( NUI_SKELETON_POSITION_NOT_TRACKED + 1 ) ,
		NUI_SKELETON_POSITION_TRACKED	= ( NUI_SKELETON_POSITION_INFERRED + 1 )
	    } 	NUI_SKELETON_POSITION_TRACKING_STATE;

	typedef
	enum _NUI_SKELETON_TRACKING_STATE
	    {	NUI_SKELETON_NOT_TRACKED	= 0,
		NUI_SKELETON_POSITION_ONLY	= ( NUI_SKELETON_NOT_TRACKED + 1 ) ,
		NUI_SKELETON_TRACKED	= ( NUI_SKELETON_POSITION_ONLY + 1 )
	    } 	NUI_SKELETON_TRACKING_STATE;

	typedef struct _NUI_SKELETON_DATA
	    {
	    NUI_SKELETON_TRACKING_STATE eTrackingState;
	    DWORD dwTrackingID;
	    DWORD dwEnrollmentIndex;
	    DWORD dwUserIndex;
	    Vector4 Position;
	    Vector4 SkeletonPositions[ 20 ];
	    NUI_SKELETON_POSITION_TRACKING_STATE eSkeletonPositionTrackingState[ 20 ];
	    DWORD dwQualityFlags;
	    } NUI_SKELETON_DATA;

#endif
