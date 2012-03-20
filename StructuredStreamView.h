#ifndef STRUCTUREDSTREAMVIEW_H_INCLUDED
#define STRUCTUREDSTREAMVIEW_H_INCLUDED

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
#pragma comment(lib, "wsock32.lib")
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <string>
#include <string.h> //need memcpy
#include <vector>
#include "BasicTypes.h"

namespace RuGKinectInterfaceServer {

/*
 * Byte-order independent data stream view (a read-only version) of a buffer. 
 * Values of the different basic types can be extracted from the stream.
 */

class StructuredStreamView
{
public:

	StructuredStreamView(const Byte* buffer, Uint32 buffersize);
	StructuredStreamView(const StructuredStreamView& other);
	
	~StructuredStreamView();

	void				setBuffer(const Byte* buffer, Uint32 buffersize);
	void				setBufferSize(Uint32 buffersize);
		
	inline bool			empty() const;
	inline bool			dataLeft() const;
	inline bool			bytesLeft(Uint16 numbytes) const;

	inline void			rewind();
	inline void			setReadPosition(Uint32 p);
	inline void			skipAhead(Uint32 o);

	inline const Byte*	getBuffer() const;
	inline Uint32		size() const;
	
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

	const Byte*			_buffer;
	Uint32				_bufferSize;
	
	Uint32				_readPosition;
	
	//bool				_locked;			
};

inline const Byte*
StructuredStreamView::getBuffer() const
{
	if (_bufferSize > 0)
		return _buffer;
	else
		return NULL;
}

inline void 
StructuredStreamView::rewind()
{
	_readPosition = 0;
}

inline void 
StructuredStreamView::setReadPosition(Uint32 p)
{	
	assert (p <= _bufferSize);

	_readPosition = p;
}

inline void 
StructuredStreamView::skipAhead(Uint32 o)
{
	_readPosition += o;

	assert (_readPosition <= _bufferSize);
}

inline Uint32		
StructuredStreamView::size() const
{
	return _bufferSize;
}

inline bool			
StructuredStreamView::empty() const
{
	return _bufferSize == 0;
}

inline bool			
StructuredStreamView::dataLeft() const
{
	return _readPosition < _bufferSize;
}

inline bool 
StructuredStreamView::bytesLeft(Uint16 numbytes) const
{
	return _readPosition+numbytes <= _bufferSize;
}

/* 
 * Byte 
 */

inline Byte
StructuredStreamView::getByte()
{
	assert (bytesLeft(1));

	Byte 	b;

	b = _buffer[_readPosition++];
	
	return b;
}

/*
 * Bool 
 */

inline bool
StructuredStreamView::getBool()
{
	assert (bytesLeft(1));

	// FIXME: perhaps pad with a zero byte?

	bool 	b;

	b = (_buffer[_readPosition++] == 0 ? false : true);
	
	return b;
}

/* 
 * Uint16 
 */

inline Uint16
StructuredStreamView::getUint16()
{
	assert (bytesLeft(2));

	Uint16	n;

	memcpy(&n, &(_buffer[_readPosition]), sizeof(n));	
	_readPosition += sizeof(n);
	
	return ntohs(n);
}

inline Uint16
StructuredStreamView::peekUint16()
{
	assert (bytesLeft(2));

	Uint16	n;

	memcpy(&n, &(_buffer[_readPosition]), sizeof(n));		
	
	return ntohs(n);
}

/* 
 * Uint32 
 */

inline Uint32
StructuredStreamView::getUint32()
{
	assert (bytesLeft(4));

	Uint32	n;

	memcpy(&n, &(_buffer[_readPosition]), sizeof(n));	
	_readPosition += sizeof(n);
	
	return ntohl(n);
}

/*
 * float 
 */

inline float
StructuredStreamView::getFloat()
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

inline double
StructuredStreamView::getDouble()
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

inline void
StructuredStreamView::getString(std::string& s)
{
	assert (bytesLeft(2));

	Uint16 size;

	if (!s.empty())
		s.erase();
	size = getUint16();

	assert (bytesLeft(size));

	if (size > 0)
	{
		s.insert(s.begin(), 
			reinterpret_cast<const char*>(&_buffer[_readPosition]), 
			reinterpret_cast<const char*>(&_buffer[_readPosition+size]));
		_readPosition += size;
	}
}

/* arbitrary data */

inline void
StructuredStreamView::getData(void* d, Uint16 datasize)
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
			memcpy (d, &_buffer[_readPosition], datasize);
			_readPosition += datasize;
		}
	}
	else
	{
		fprintf(stderr, "StructuredStreamView::getData(): Attempt to read %d bytes, but %d bytes of 'data' in the stream!\n",
			datasize, sizeinstream);
		exit(1);
	}
}

}

#endif
