#include <stack>
#include "Core/Misc/ImmutableCheck.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct CopyVisitor
{
	Ref< ShaderGraph > m_shaderGraph;

	bool operator () (Node* node)
	{
		m_shaderGraph->addNode(node);
		return true;
	}

	bool operator () (Edge* edge)
	{
		m_shaderGraph->addEdge(edge);
		return true;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTechniques", ShaderGraphTechniques, Object)

ShaderGraphTechniques::ShaderGraphTechniques(const ShaderGraph* shaderGraph)
{
	T_IMMUTABLE_CHECK(shaderGraph);

	Ref< ShaderGraph > shaderGraphOpt = new ShaderGraph(
		shaderGraph->getNodes(),
		shaderGraph->getEdges()
	);

	// Constant fold entire graph so disabled outputs can be efficiently evaluated.
	if (shaderGraphOpt)
		shaderGraphOpt = ShaderGraphStatic(shaderGraphOpt).getConstantFolded();
	if (shaderGraphOpt)
		shaderGraphOpt = ShaderGraphStatic(shaderGraphOpt).removeDisabledOutputs();

	// Get all technique names.
	if (shaderGraphOpt)
	{
		std::set< std::wstring > names;

		const RefArray< Node >& nodes = shaderGraphOpt->getNodes();
		for (const auto node : nodes)
		{
			if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(node))
				names.insert(pixelOutput->getTechnique());
			else if (ComputeOutput* computeOutput = dynamic_type_cast< ComputeOutput* >(node))
				names.insert(computeOutput->getTechnique());
		}

		// Generate each technique.
		for (const auto& name : names)
		{
			RefArray< Node > roots;

			bool foundNamedVertexOutput = false;

			// Find named output nodes.
			for (auto node : nodes)
			{
				if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(node))
				{
					if (vertexOutput->getTechnique() == name)
					{
						roots.push_back(vertexOutput);
						foundNamedVertexOutput = true;
					}
				}
				else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(node))
				{
					if (pixelOutput->getTechnique() == name)
						roots.push_back(pixelOutput);
				}
				else if (ComputeOutput* computeOutput = dynamic_type_cast< ComputeOutput* >(node))
				{
					if (computeOutput->getTechnique() == name)
						roots.push_back(computeOutput);
				}
				else
				{
					const INodeTraits* traits = INodeTraits::find(node);
					T_FATAL_ASSERT (traits);

					if (traits->isRoot(shaderGraphOpt, node))
						roots.push_back(node);
				}
			}

			// If no explicit named vertex output we'll try to find an unnamed vertex output.
			if (!foundNamedVertexOutput)
			{
				for (auto node : nodes)
				{
					VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(node);
					if (vertexOutput && vertexOutput->getTechnique().empty())
						roots.push_back(vertexOutput);
				}
			}

			CopyVisitor visitor;
			visitor.m_shaderGraph = new ShaderGraph();
			GraphTraverse(shaderGraphOpt, roots).preorder(visitor);
			m_techniques[name] = visitor.m_shaderGraph;
		}
	}
}

std::set< std::wstring > ShaderGraphTechniques::getNames() const
{
	std::set< std::wstring > names;
	for (auto technique : m_techniques)
		names.insert(technique.first);
	return names;
}

ShaderGraph* ShaderGraphTechniques::generate(const std::wstring& name) const
{
	auto it = m_techniques.find(name);
	if (it != m_techniques.end())
		return it->second;
	else
		return nullptr;
}

	}
}
