#ifndef STRUCTUREDSTREAM_H_INCLUDED
#define STRUCTUREDSTREAM_H_INCLUDED

// Blatantly stolen from the osgrc codebase

#ifdef __sgi
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#else
#include <cassert>
#include <cstdlib>
#include <cstdio>
#endif

// for htons() and friends
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <string>
#include <string.h> //need memcpy
#include <vector>
#include "BasicTypes.h"
//#include "Logger.h"

namespace RuGKinectInterfaceServer {

/*
 * Byte-order independent data stream. Values of the different basic types
 * can be placed in the stream and later extracted.
 */

class StructuredStream
{
public:

	StructuredStream();
	StructuredStream(const StructuredStream& other);
	
	inline void 		clear();

	inline bool			empty() const;
	inline bool			dataLeft() const;

	inline bool 		bytesLeft(Uint16 numbytes) const;
	inline bool 		reserveSpace(Uint16 numbytes);

	inline void 		rewind();
	inline void 		setReadPosition(Uint32 p);
	inline void 		skipAhead(Uint32 o);

	inline Byte*		getBuffer();	
	inline const Byte*	getBuffer() const;

	inline char*		getBufferAsChar();
	inline const char*	getBufferAsChar() const;

	inline Uint32		size() const;
	inline void			setSize(Uint32 size);
	
	inline void			appendByte(Byte b);	
	inline void			appendBool(bool b);	
	inline void			appendUint16(Uint16 u);
	inline void			appendUint32(Uint32 u);
	inline void			appendFloat(float f);
	inline void 		appendDouble(double d);
	inline void			appendString(const std::string& s);
	inline void			appendData(const void* d, Uint16 len);

	inline Byte			getByte();
	inline bool			getBool();
	inline Uint16		getUint16();
	inline Uint32		getUint32();
	inline float 		getFloat();
	inline double		getDouble();
	inline void			getString(std::string& s);
	inline void			getData(void *d, Uint16 datasize);

	inline Uint16		peekUint16();
	
protected:
	std::vector<Byte>	m_vBuffer;
	Uint32				m_nReadPosition;

	std::vector<Byte> * m_pAppendToBuffer;
	
	bool				m_bLocked;			
};

inline Byte*		
StructuredStream::getBuffer()
{
	if (m_vBuffer.size() > 0)
		return (&m_vBuffer[0]);
	else
		return NULL;
}

inline const Byte*
StructuredStream::getBuffer() const
{
	if (m_vBuffer.size() > 0)
		return (&m_vBuffer[0]);
	else
		return NULL;
}

inline char*		
StructuredStream::getBufferAsChar()
{
	if (m_vBuffer.size() > 0)
		return ((char*)&m_vBuffer[0]);
	else
		return NULL;
}

inline const char*
StructuredStream::getBufferAsChar() const
{
	if (m_vBuffer.size() > 0)
		return ((const char*)&m_vBuffer[0]);
	else
		return NULL;
}

inline void 
StructuredStream::rewind()
{
	m_nReadPosition = 0;
}

inline void 
StructuredStream::setReadPosition(Uint32 p)
{	
	assert (p <= m_vBuffer.size());

	m_nReadPosition = p;
}

inline void 
StructuredStream::skipAhead(Uint32 o)
{
	m_nReadPosition += o;

	assert (m_nReadPosition <= m_vBuffer.size());
}

inline void 
StructuredStream::clear()
{
	//m_vBuffer.clear();
	m_vBuffer.erase(m_vBuffer.begin(), m_vBuffer.end());
	m_nReadPosition = 0;
}

inline Uint32		
StructuredStream::size() const
{
	return static_cast<Uint32>(m_vBuffer.size());
}

inline void			
StructuredStream::setSize(Uint32 size)
{
	m_vBuffer.resize(size);
	rewind();
}

inline bool			
StructuredStream::empty() const
{
	return m_vBuffer.size() == 0;
}

inline bool			
StructuredStream::dataLeft() const
{
	return m_nReadPosition < m_vBuffer.size();
}

inline bool 
StructuredStream::bytesLeft(Uint16 numbytes) const
{
	return m_nReadPosition+numbytes <= m_vBuffer.size();
}

inline bool 
StructuredStream::reserveSpace(Uint16 numbytes)
{
	m_vBuffer.reserve(numbytes);
	return (m_vBuffer.capacity() >= numbytes);
}

/* 
 * Byte 
 */

inline void
StructuredStream::appendByte(Byte b)
{
	//logMessage("appendBool(%d), buffersize now %d",b,size());
	m_pAppendToBuffer->push_back(b);
}

inline Byte
StructuredStream::getByte()
{
	assert (bytesLeft(1));

	Byte 	b;

	b = m_vBuffer[m_nReadPosition++];
	
	return b;
}

/*
 * Bool 
 */

inline void
StructuredStream::appendBool(bool b)
{
	//logMessage("appendBool(%d), buffersize now %d",b,size());
	m_pAppendToBuffer->push_back(b ? 1 : 0);
}

inline bool
StructuredStream::getBool()
{
	assert (bytesLeft(1));

	// FIXME: perhaps pad with a zero byte?

	bool 	b;

	b = (m_vBuffer[m_nReadPosition++] == 0 ? false : true);
	
	return b;
}

/* 
 * Uint16 
 */

inline void
StructuredStream::appendUint16(Uint16 u)
{
	//logMessage("appendUint16(%d), buffersize now %d",u,size());
	Uint16 n = htons(u);

	m_pAppendToBuffer->insert(m_pAppendToBuffer->end(), reinterpret_cast<Byte*>(&n), reinterpret_cast<Byte*>(&n)+sizeof(n));
}

inline Uint16
StructuredStream::getUint16()
{
	assert (bytesLeft(2));

	Uint16	n;

	memcpy(&n, &(m_vBuffer[m_nReadPosition]), sizeof(n));	
	m_nReadPosition += sizeof(n);
	
	return ntohs(n);
}

inline Uint16
StructuredStream::peekUint16()
{
	assert (bytesLeft(2));

	Uint16	n;

	memcpy(&n, &(m_vBuffer[m_nReadPosition]), sizeof(n));		
	
	return ntohs(n);
}

/* 
 * Uint32 
 */

inline void
StructuredStream::appendUint32(Uint32 u)
{
	//logMessage("appendUint32(%d), buffersize now %d",u,size());
	Uint32 n = htonl(u);

	m_pAppendToBuffer->insert(m_pAppendToBuffer->end(), reinterpret_cast<Byte*>(&n), reinterpret_cast<Byte*>(&n)+sizeof(n));
}

inline Uint32
StructuredStream::getUint32()
{
	assert (bytesLeft(4));

	Uint32	n;

	memcpy(&n, &(m_vBuffer[m_nReadPosition]), sizeof(n));	
	m_nReadPosition += sizeof(n);
	
	return ntohl(n);
}

/*
 * float 
 */

inline void
StructuredStream::appendFloat(float f)
{
	//logMessage("appendFloat(%f), buffersize now %d",f,size());
	//appendUint32(*(reinterpret_cast<Uint32*>(&f)));
	union {
		Uint32 asUint;
		float  asFloat;
	}v;
	v.asFloat = f;
	appendUint32(v.asUint);
}

inline float
StructuredStream::getFloat()
{
	assert (bytesLeft(4));

	union {
		Uint32 asUint;
		float  asFloat;
	}v;

	v.asUint = getUint32();

//	return *(reinterpret_cast<float*>(&v));
	return v.asFloat;
}

inline void
StructuredStream::appendDouble(double d)
{

	union {
        Uint64  asUint64;
		double  asDouble;
	}v;
	v.asDouble = d;
	appendUint32(v.asUint64.upper);
	appendUint32(v.asUint64.lower);
}

inline double
StructuredStream::getDouble()
{
	assert (bytesLeft(8));

	union {
        Uint64  asUint64;
		double  asDouble;
	}v;
	v.asUint64.upper = getUint32();
	v.asUint64.lower = getUint32();
	return v.asDouble;
}


/*
 * std::string 
 */

inline void
StructuredStream::appendString(const std::string& s)
{
	//logMessage("appendString(%s), buffersize now %d",s.c_str(),size());
	appendUint16(static_cast<Uint16>(s.size()));
	if (s.size() > 0)
	{
		const Byte *c = reinterpret_cast<const Byte*>(s.c_str());
		m_pAppendToBuffer->insert(m_pAppendToBuffer->end(), c, c+s.size());
	}
}

inline void
StructuredStream::getString(std::string& s)
{
	assert (bytesLeft(2));

	Uint16 size;

	if (!s.empty())
		s.erase();

	size = getUint16();

	assert (bytesLeft(size));

	if (size > 0)
	{
		s.insert(s.begin(), reinterpret_cast<char*>(&m_vBuffer[m_nReadPosition]), reinterpret_cast<char*>(&m_vBuffer[m_nReadPosition+size]));
		m_nReadPosition += size;
	}
}

/* arbitrary data */

inline void
StructuredStream::appendData(const void *d, Uint16 datasize)
{
	//logMessage("appendData(%d bytes), buffersize now %d",datasize,size());
	appendUint16(datasize);
	if (datasize > 0)
	{
		m_pAppendToBuffer->insert(m_pAppendToBuffer->end(), reinterpret_cast<const Byte*>(d), reinterpret_cast<const Byte*>(d)+datasize);
	}
}

inline void
StructuredStream::getData(void* d, Uint16 datasize)
{
	assert (bytesLeft(2));

	// the data in the stream is guarded by a size value, 
	// check it
	Uint16 sizeinstream = getUint16();	

	assert (bytesLeft(sizeinstream));

	if (datasize == sizeinstream)
	{
		if (datasize > 0)
		{
			memcpy (d, &m_vBuffer[m_nReadPosition], datasize);
			m_nReadPosition += datasize;
		}
	}
	else
	{
		fprintf(stderr, "StructuredStream::getData(): Attempt to read %d bytes, but %d bytes of 'data' in the stream!\n",
			datasize, sizeinstream);
		exit(1);
	}
}

}

#endif
