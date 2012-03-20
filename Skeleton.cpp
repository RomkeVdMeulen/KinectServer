#include "stdafx.h"
#include "Skeleton.h"

using namespace RuGKinectInterfaceServer;

Skeleton::SKELETON_JOINT_INDEX Skeleton::BONES[19][2] = 
{
	{JOINT_FOOT_LEFT,		JOINT_ANKLE_LEFT},		{JOINT_FOOT_RIGHT,			JOINT_ANKLE_RIGHT},
	{JOINT_ANKLE_LEFT,		JOINT_KNEE_LEFT},		{JOINT_ANKLE_RIGHT,			JOINT_KNEE_RIGHT},
	{JOINT_KNEE_LEFT,		JOINT_HIP_LEFT},		{JOINT_KNEE_RIGHT,			JOINT_HIP_RIGHT},
	{JOINT_HIP_LEFT,		JOINT_HIP_CENTER},		{JOINT_HIP_RIGHT,			JOINT_HIP_CENTER},

	{JOINT_HAND_LEFT,		JOINT_WRIST_LEFT},		{JOINT_HAND_RIGHT,			JOINT_WRIST_RIGHT},
	{JOINT_WRIST_LEFT,		JOINT_ELBOW_LEFT},		{JOINT_WRIST_RIGHT,			JOINT_ELBOW_RIGHT},
	{JOINT_ELBOW_LEFT,		JOINT_SHOULDER_LEFT},	{JOINT_ELBOW_RIGHT,			JOINT_SHOULDER_RIGHT},
	{JOINT_SHOULDER_LEFT,	JOINT_SHOULDER_CENTER},	{JOINT_SHOULDER_RIGHT,		JOINT_SHOULDER_CENTER},

	{JOINT_HIP_CENTER,		JOINT_SPINE},			{JOINT_SPINE,				JOINT_SHOULDER_CENTER},
	{JOINT_SHOULDER_CENTER,	JOINT_HEAD}
};

// Default (zero) constructor
Skeleton::Skeleton()
: m_dTime( 0 )
{
	for ( unsigned i = 0; i < JOINT_COUNT; ++i )
		m_vSkeletonPositions[i] = Vector3();
	for ( unsigned i = 0; i < 4; ++i )
		m_bvClipping[i] = false;
}

Skeleton::Skeleton(NUI_SKELETON_DATA const *data, long unsigned int time)
: m_vCentralPosition( data->Position ), m_vSkeletonPositions(), m_dTime( time )
{
	for ( unsigned i = 0; i < JOINT_COUNT; ++i )
		m_vSkeletonPositions[i] = Vector3(data->SkeletonPositions[i]);

#ifdef WIN32
	m_bvClipping[TOP] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_TOP) == NUI_SKELETON_QUALITY_CLIPPED_TOP);
	m_bvClipping[RIGHT] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_RIGHT) == NUI_SKELETON_QUALITY_CLIPPED_RIGHT);
	m_bvClipping[BOTTOM] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_BOTTOM) == NUI_SKELETON_QUALITY_CLIPPED_BOTTOM);
	m_bvClipping[LEFT] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_LEFT) == NUI_SKELETON_QUALITY_CLIPPED_LEFT);
#else
	m_bvClipping[TOP] = m_bvClipping[RIGHT] = m_bvClipping[BOTTOM] = m_bvClipping[LEFT] = false;
#endif
}

Skeleton::Skeleton(NUI_SKELETON_DATA const *data, long unsigned int time, Vector4 const *vGravityNormal)
: m_vCentralPosition( data->Position ), m_vGravityNormal( *vGravityNormal ), m_vSkeletonPositions(), m_dTime( time )
{
	for ( unsigned i = 0; i < 20; ++i )
		m_vSkeletonPositions[i] = Vector3(data->SkeletonPositions[i]);

#ifdef WIN32
	m_bvClipping[TOP] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_TOP) == NUI_SKELETON_QUALITY_CLIPPED_TOP);
	m_bvClipping[RIGHT] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_RIGHT) == NUI_SKELETON_QUALITY_CLIPPED_RIGHT);
	m_bvClipping[BOTTOM] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_BOTTOM) == NUI_SKELETON_QUALITY_CLIPPED_BOTTOM);
	m_bvClipping[LEFT] = ((data->dwQualityFlags & NUI_SKELETON_QUALITY_CLIPPED_LEFT) == NUI_SKELETON_QUALITY_CLIPPED_LEFT);
#else
	m_bvClipping[TOP] = m_bvClipping[RIGHT] = m_bvClipping[BOTTOM] = m_bvClipping[LEFT] = false;
#endif
}

Skeleton::operator bool() const
{
	return  ( m_vCentralPosition.x() != 0 && m_vCentralPosition.y() != 0 );
}

void Skeleton::transform(osg::Matrix const &transform)
{
	m_vCentralPosition *= transform;
	
	if ( m_vGravityNormal.magnitude() > 0 )
		m_vGravityNormal *= transform;

	for ( unsigned i = 0; i < JOINT_COUNT; ++i )
		m_vSkeletonPositions[i] *= transform;
}