#pragma once

#include "NuiData.h"
#include <iosfwd>
#include <osg/Vec3f>
#include <osg/Matrix>

namespace RuGKinectInterfaceServer
{

class Vector3
{
	float m_x;
	float m_y;
	float m_z;

public:
	Vector3();
	Vector3(float x, float y, float z);
	Vector3(Vector3 const &other);
	Vector3(Vector4 const &other);
	Vector3(osg::Vec3f const &vec);

	float x() const { return m_x; }
	float y() const { return m_y; }
	float z() const { return m_z; }

	std::string x_string() const;
	std::string y_string() const;
	std::string z_string() const;

	bool isZero() const;

	void kinectNormalize();
	Vector3 kinectNormalized() const;

	float magnitude() const;
	float length() const { return magnitude(); }

	void makeMagnitudeEqualTo(float magnitude);
	void makeMagnitudeEqualTo(Vector3 const &other);

	void normalize() { makeMagnitudeEqualTo(1.00); }

	osg::Vec3f toVec3f() const;

	Vector3 operator-(Vector3 const &other) const;
	Vector3 operator-(Vector3 const *other) const;

	Vector3 operator*(osg::Matrix const &other) const;
	Vector3 &operator*=(osg::Matrix const &other);

	bool operator!= (Vector3 const &other) const;

	float dotProduct(Vector3 const &other) const;
};

}
