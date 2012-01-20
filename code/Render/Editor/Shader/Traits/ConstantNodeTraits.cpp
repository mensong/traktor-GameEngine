#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConstantNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConstantNodeTraits", 0, ConstantNodeTraits, INodeTraits)

TypeInfoSet ConstantNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Color >());
	typeSet.insert(&type_of< FragmentPosition >());
	typeSet.insert(&type_of< Scalar >());
	typeSet.insert(&type_of< TargetSize >());
	typeSet.insert(&type_of< Texture >());
	typeSet.insert(&type_of< Vector >());
	return typeSet;
}

PinType ConstantNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< Color >(node) || is_a< Vector >(node))
		return PntScalar4;
	else if (is_a< Scalar >(node))
		return PntScalar1;
	else if (is_a< FragmentPosition >(node) || is_a< TargetSize >(node))
		return PntScalar2;
	else if (is_a< Texture >(node))
		return PntTexture;
	else
		return PntVoid;
}

PinType ConstantNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return PntVoid;
}

int32_t ConstantNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool ConstantNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (const Color* color = dynamic_type_cast< const Color* >(node))
	{
		outputConstant[0] = color->getColor().r / 255.0f;
		outputConstant[1] = color->getColor().g / 255.0f;
		outputConstant[2] = color->getColor().b / 255.0f;
		outputConstant[3] = color->getColor().a / 255.0f;
	}
	else if (const Vector* vectr = dynamic_type_cast< const Vector* >(node))
	{
		outputConstant[0] = vectr->get().x();
		outputConstant[1] = vectr->get().y();
		outputConstant[2] = vectr->get().z();
		outputConstant[3] = vectr->get().w();
	}
	else if (const Scalar* scalar = dynamic_type_cast< const Scalar* >(node))
	{
		outputConstant[0] = scalar->get();
	}
	else
		return false;

	return true;
}

bool ConstantNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool ConstantNodeTraits::evaluatePartial(
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
