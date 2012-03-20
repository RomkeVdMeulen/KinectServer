#include "stdafx.h"
#include "StructuredStream.h"

// Blatantly stolen from the osgrc codebase

using namespace RuGKinectInterfaceServer;

StructuredStream::StructuredStream()
: 
	m_bLocked( false ), 
	m_nReadPosition( 0 )
{
	m_vBuffer.clear();
	m_pAppendToBuffer = &m_vBuffer;
}

StructuredStream::StructuredStream(const StructuredStream& other)
:
	m_vBuffer(other.m_vBuffer),
	m_nReadPosition(other.m_nReadPosition),
	m_bLocked(other.m_bLocked)
{	
	m_pAppendToBuffer = &m_vBuffer;
}
