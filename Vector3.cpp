#include "stdafx.h"
#include "Vector3.h"
#include <sstream>
#include <string>

using namespace RuGKinectInterfaceServer;

Vector3::Vector3()
: m_x(0), m_y(0), m_z(0)
{}

Vector3::Vector3(float x, float y, float z)
: m_x(x), m_y(y), m_z(z)
{}

Vector3::Vector3(Vector3 const &other)
: m_x(other.m_x), m_y(other.m_y), m_z(other.m_z)
{}

Vector3::Vector3(Vector4 const &other)
: m_x(other.x), m_y(other.y), m_z(other.z)
{}

Vector3::Vector3(osg::Vec3f const &vec)
: m_x(vec.x()), m_y(vec.y()), m_z(vec.z())
{}

std::string Vector3::x_string() const
{
	ostringstream stream;
	stream << x();
	return stream.str();
}
std::string Vector3::y_string() const
{
	ostringstream stream;
	stream << y();
	return stream.str();
}
std::string Vector3::z_string() const
{
	ostringstream stream;
	stream << z();
	return stream.str();
}

bool Vector3::isZero() const
{
	return m_x != 0 || m_y != 0 || m_z != 0;
}

// Kinect camera space runs between certain values
// This scales a camera space point so values lie between -1 and 1
void Vector3::kinectNormalize()
{ 
	m_x /= 2.2f;
	m_y /= 1.6f;
	m_z /= 4.0f;
}

Vector3 Vector3::kinectNormalized() const
{ 
	 Vector3 returnValue = Vector3(*this);
	 returnValue.kinectNormalize();
	 return returnValue;
}

float Vector3::magnitude() const
{
	return sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
}

void Vector3::makeMagnitudeEqualTo(float magnitude)
{
	float scale = magnitude / this->magnitude();
	m_x *= scale;
	m_y *= scale;
	m_z *= scale;
}

void Vector3::makeMagnitudeEqualTo(Vector3 const &other)
{
	float scale = other.magnitude() / magnitude();
	m_x *= scale;
	m_y *= scale;
	m_z *= scale;
}

osg::Vec3f Vector3::toVec3f() const
{
	return osg::Vec3f(m_x,m_y,m_z);
}

Vector3 Vector3::operator-(Vector3 const &other) const
{
	return Vector3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
}

Vector3 Vector3::operator-(Vector3 const *other) const
{
	return Vector3(m_x - other->m_x, m_y - other->m_y, m_z - other->m_z);
}

Vector3 Vector3::operator*(osg::Matrix const &other) const
{
	osg::Matrix thisPos;
	thisPos.makeTranslate(x(),y(),z());
	osg::Matrix transformed = thisPos * other;
	return Vector3(transformed.getTrans());
}

Vector3 &Vector3::operator*=(osg::Matrix const &other)
{
	osg::Matrix thisPos;
	thisPos.makeTranslate(x(),y(),z());
	osg::Vec3f transformed = (thisPos * other).getTrans();
	m_x = transformed.x();
	m_y = transformed.y();
	m_z = transformed.z();

	return *this;
}

float Vector3::dotProduct(Vector3 const &other) const
{
	return (m_x * other.m_x) + (m_y * other.m_y) + (m_z * other.m_z);
}

bool Vector3::operator!= (Vector3 const &other) const
{
	return m_x != other.m_x && m_y != other.m_y && m_z != other.m_z;
}
