#include <algorithm>
#include "Render/Editor/Shader/Node.h"
#include "Render/Editor/Shader/OutputPin.h"

namespace traktor
{
	namespace render
	{

OutputPin::OutputPin()
{
}

OutputPin::OutputPin(Node* node, const Guid& id, const std::wstring& name)
:	m_node(node)
,	m_id(id)
,	m_name(name)
{
}

Node* OutputPin::getNode() const
{
	return m_node;
}

const Guid& OutputPin::getId() const
{
	return m_id;
}

const std::wstring& OutputPin::getName() const
{
	return m_name;
}

	}
}
