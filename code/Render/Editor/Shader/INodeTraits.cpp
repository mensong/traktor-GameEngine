#include <map>
#include "Core/RefArray.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Shader/Node.h"
#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Semaphore s_lock;
std::map< const TypeInfo*, Ref< INodeTraits > > s_traits;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.INodeTraits", INodeTraits, Object)

const INodeTraits* INodeTraits::find(const Node* node)
{
	// Allow a race to initializing traits; lock at write.
	if (s_traits.empty())
	{
		std::map< const TypeInfo*, Ref< INodeTraits > > traits;

		// Find all concrete INodeTraits classes.
		std::vector< const TypeInfo* > traitsTypes;
		type_of< INodeTraits >().findAllOf(traitsTypes, false);

		// Instantiate traits.
		for (std::vector< const TypeInfo* >::const_iterator i = traitsTypes.begin(); i != traitsTypes.end(); ++i)
		{
			Ref< INodeTraits > tr = checked_type_cast< INodeTraits*, false >((*i)->createInstance());
			T_ASSERT (tr);

			TypeInfoSet nodeTypes = tr->getNodeTypes();
			for (TypeInfoSet::const_iterator j = nodeTypes.begin(); j != nodeTypes.end(); ++j)
				traits[*j] = tr;
		}

		// Update global traits.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_lock);
			s_traits = traits;
		}
	}

	// Find traits from node type.
	std::map< const TypeInfo*, Ref< INodeTraits > >::const_iterator i = s_traits.find(&type_of(node));
	return i != s_traits.end() ? i->second : 0;
}

	}
}
