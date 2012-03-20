#pragma once

#include "StructuredStream.h"

namespace RuGKinectInterfaceServer 
{

class MessageStream : public StructuredStream
{
	bool m_bWritingMessage;
	std::vector<Byte> m_vMessageBuffer;

public:
	MessageStream() : StructuredStream(), m_bWritingMessage(false) {}

	void beginMessage(Byte type);

	void writeMessage();
};



}
