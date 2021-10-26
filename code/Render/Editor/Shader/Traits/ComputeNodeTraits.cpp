#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ComputeNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeNodeTraits", 0, ComputeNodeTraits, INodeTraits)

TypeInfoSet ComputeNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ComputeOutput >();
	return typeSet;
}

bool ComputeNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return is_a< ComputeOutput >(node);
}

PinType ComputeNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PinType::Void;
}

PinType ComputeNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Enable")
		return PinType::Scalar1;
	else if (inputPin->getName() == L"Storage")
		return PinType::Texture2D;
	else if (inputPin->getName() == L"Offset")
		return PinType::Scalar2;
	else
		return PinType::Scalar4;
}

int32_t ComputeNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool ComputeNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool ComputeNodeTraits::evaluatePartial(
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

PinOrderType ComputeNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return PotLinear;
}

	}
}
