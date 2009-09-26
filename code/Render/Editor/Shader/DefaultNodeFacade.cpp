#include "Render/Editor/Shader/DefaultNodeFacade.h"
#include "Render/Nodes.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/InputNodeShape.h"
#include "Ui/Custom/Graph/OutputNodeShape.h"
#include "I18N/Text.h"
#include "Core/Misc/StringUtilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DefaultNodeFacade", DefaultNodeFacade, NodeFacade)

DefaultNodeFacade::DefaultNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShapes[0] = gc_new< ui::custom::DefaultNodeShape >(graphControl);
	m_nodeShapes[1] = gc_new< ui::custom::InputNodeShape >(graphControl);
	m_nodeShapes[2] = gc_new< ui::custom::OutputNodeShape >(graphControl);
}

Node* DefaultNodeFacade::createShaderNode(
	const Type* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->newInstance());
}

ui::custom::Node* DefaultNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	std::wstring title = type_name(shaderNode);
	size_t p = title.find_last_of(L'.');
	if (p > 0)
		title = i18n::Text(L"SHADERGRAPH_NODE_" + toUpper(title.substr(p + 1)));

	Ref< ui::custom::NodeShape > shape;
	if (shaderNode->getInputPinCount() == 1 && shaderNode->getOutputPinCount() == 0)
		shape = m_nodeShapes[2];
	else if (shaderNode->getInputPinCount() == 0 && shaderNode->getOutputPinCount() == 1)
		shape = m_nodeShapes[1];
	else
		shape = m_nodeShapes[0];

	Ref< ui::custom::Node > editorNode = gc_new< ui::custom::Node >(
		title,
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		shape
	);

	if (is_a< InputPort >(shaderNode) || is_a< OutputPort >(shaderNode))
		editorNode->setColor(traktor::Color(200, 255, 200));
	else if (is_a< Uniform >(shaderNode) || is_a< IndexedUniform >(shaderNode))
		editorNode->setColor(traktor::Color(255, 255, 200));

	return editorNode;
}

void DefaultNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
}

void DefaultNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	traktor::Color color(255, 255, 255);

	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");

	if (is_a< InputPort >(shaderNode) || is_a< OutputPort >(shaderNode))
		color = traktor::Color(200, 255, 200);
	else if (is_a< Uniform >(shaderNode) || is_a< IndexedUniform >(shaderNode))
		color = traktor::Color(255, 255, 200);

	editorNode->setColor(validationSucceeded ? color : traktor::Color(255, 60, 60));
}

	}
}
