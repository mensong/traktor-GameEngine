#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/UniformNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.UniformNodeTraits", 0, UniformNodeTraits, INodeTraits)

TypeInfoSet UniformNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IndexedUniform >());
	typeSet.insert(&type_of< Uniform >());
	return typeSet;
}

PinType UniformNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	ParameterType parameterType;

	if (const IndexedUniform* indexedUniformNode = dynamic_type_cast< const IndexedUniform* >(node))
		parameterType = indexedUniformNode->getParameterType();
	else if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(node))
		parameterType = uniformNode->getParameterType();
	else
		return PntVoid;

	switch (parameterType)
	{
	case PtScalar:
		return PntScalar1;
	case PtVector:
		return PntScalar4;
	case PtMatrix:
		return PntMatrix;
	case PtTexture:
		return PntTexture;
	default:
		return PntVoid;
	}
}

PinType UniformNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (is_a< IndexedUniform >(node))
		return PntScalar1;
	else
		return PntVoid;
}

int32_t UniformNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool UniformNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool UniformNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool UniformNodeTraits::evaluatePartial(
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
