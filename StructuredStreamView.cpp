#include "stdafx.h"
#include "StructuredStreamView.h"

using namespace RuGKinectInterfaceServer;

StructuredStreamView::StructuredStreamView(const Byte* buffer, Uint32 buffersize)
{
	setBuffer(buffer, buffersize);
}

StructuredStreamView::StructuredStreamView(const StructuredStreamView& other):
	_buffer(other._buffer),
	_bufferSize(other._bufferSize),
	_readPosition(other._readPosition)
	//_locked(other._locked)
{	
}

StructuredStreamView::~StructuredStreamView()
{	
}

void
StructuredStreamView::setBuffer(const Byte* buffer, Uint32 buffersize)
{	
	_buffer = buffer;
	_bufferSize = buffersize;
	_readPosition = 0;
}

void		
StructuredStreamView::setBufferSize(Uint32 buffersize)
{
	_bufferSize = buffersize;
	_readPosition = 0;
}
