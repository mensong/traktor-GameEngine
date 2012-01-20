#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/MetaNodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		if (node->getInputPin(i) == inputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MetaNodeTraits", 0, MetaNodeTraits, INodeTraits)

TypeInfoSet MetaNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Branch >());
	typeSet.insert(&type_of< InputPort >());
	typeSet.insert(&type_of< OutputPort >());
	typeSet.insert(&type_of< Platform >());
	typeSet.insert(&type_of< Type >());
	return typeSet;
}

PinType MetaNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	PinType outputPinType = PntVoid;
	uint32_t inputPinCount = node->getInputPinCount();
	for (uint32_t i = 0; i < inputPinCount; ++i)
		outputPinType = std::max< PinType >(
			outputPinType,
			inputPinTypes[i]
		);
	return outputPinType;
}

PinType MetaNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (is_a< OutputPort >(node))
		return PntVoid;
	else
		return outputPinTypes[0];
}

int32_t MetaNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool MetaNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool MetaNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool MetaNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	return false;
}

	}
}
