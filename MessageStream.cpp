#include "stdafx.h"
#include "MessageStream.h"

using namespace RuGKinectInterfaceServer;

void MessageStream::beginMessage(Byte type)
{
	m_bWritingMessage = true;
	m_pAppendToBuffer = &m_vMessageBuffer;
	appendByte(type);
}

void MessageStream::writeMessage()
{
	m_bWritingMessage = false;
	m_pAppendToBuffer = &m_vBuffer;

	appendByte(m_vMessageBuffer[0]);
	Uint16 nMessageSize = m_vMessageBuffer.size() - 1; // -1 because first byte is message type
	appendData(&(m_vMessageBuffer[1]), nMessageSize);
	m_vMessageBuffer.clear();
}
