#pragma once

#include "NuiData.h"
#include "stdafx.h"
#include "Vector3.h"
#include <osg/Matrix>

namespace RuGKinectInterfaceServer
{

enum CLIPPING_BORDER {
	TOP,
	RIGHT,
	BOTTOM,
	LEFT
};

class Skeleton
{
public:
	static const unsigned JOINT_COUNT = 20;
	typedef enum
	{	
		JOINT_HIP_CENTER = 0,
		JOINT_SPINE,
		JOINT_SHOULDER_CENTER,
		JOINT_HEAD,
		JOINT_SHOULDER_LEFT,
		JOINT_ELBOW_LEFT,
		JOINT_WRIST_LEFT,
		JOINT_HAND_LEFT,
		JOINT_SHOULDER_RIGHT,
		JOINT_ELBOW_RIGHT,
		JOINT_WRIST_RIGHT,
		JOINT_HAND_RIGHT,
		JOINT_HIP_LEFT,
		JOINT_KNEE_LEFT,
		JOINT_ANKLE_LEFT,
		JOINT_FOOT_LEFT,
		JOINT_HIP_RIGHT,
		JOINT_KNEE_RIGHT,
		JOINT_ANKLE_RIGHT,
		JOINT_FOOT_RIGHT
	} SKELETON_JOINT_INDEX;

	static const unsigned BONE_COUNT = 19;
	static SKELETON_JOINT_INDEX BONES[19][2];

	Skeleton();
	Skeleton(NUI_SKELETON_DATA const *data, long unsigned int time);
	Skeleton(NUI_SKELETON_DATA const *data, long unsigned int time, Vector4 const *vGravityNormal);

	operator bool() const;

	Vector3 getPosition() const { return m_vCentralPosition; }
	Vector3 getJointPosition(SKELETON_JOINT_INDEX i) const { return m_vSkeletonPositions[i]; }

	unsigned int getTime() const { return m_dTime; }

	void transform(osg::Matrix const &transform);

private:
	Vector3 m_vCentralPosition;
	Vector3 m_vGravityNormal;
	Vector3 m_vSkeletonPositions[JOINT_COUNT];
	bool m_bvClipping[4];
	// Number of milliseconds since server started that the skeleton was captured
	unsigned int m_dTime;
};

}
