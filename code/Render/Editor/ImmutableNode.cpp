#include "Render/Editor/ImmutableNode.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const static Guid c_null;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImmutableNode", ImmutableNode, Node)

ImmutableNode::ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins)
{
	if (inputPins)
	{
		int32_t pinCount = 0;
		for (auto pin = inputPins; pin->name; ++pin)
			++pinCount;

		if (pinCount > 0)
		{
			m_inputPins = new InputPin [pinCount];
			m_inputPinCount = pinCount;

			InputPin* ptr = m_inputPins;
			for (auto pin = inputPins; pin->name; ++pin)
				*ptr++ = InputPin(this, c_null, pin->name, pin->optional);
		}
	}

	if (outputPins)
	{
		int32_t pinCount = 0;
		for (auto pin = outputPins; pin->name; ++pin)
			++pinCount;

		if (pinCount > 0)
		{
			m_outputPins = new OutputPin [pinCount];
			m_outputPinCount = pinCount;

			OutputPin* ptr = m_outputPins;
			for (auto pin = outputPins; pin->name; ++pin)
				*ptr++ = OutputPin(this, c_null, pin->name); 
		}
	}
}

ImmutableNode::~ImmutableNode()
{
	delete[] m_inputPins;
	delete[] m_outputPins;
}

int ImmutableNode::getInputPinCount() const
{
	return m_inputPinCount;
}

const InputPin* ImmutableNode::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < m_inputPinCount);
	return &m_inputPins[index];
}

int ImmutableNode::getOutputPinCount() const
{
	return m_outputPinCount;
}

const OutputPin* ImmutableNode::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < m_outputPinCount);
	return &m_outputPins[index];
}

	}
}
