#ifndef traktor_ai_NavMeshEntityEditorFactory_H
#define traktor_ai_NavMeshEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace ai
	{

/*! \brief Navigation mesh entity scene editor factory.
 * \ingroup AI
 */
class NavMeshEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const;
};

	}
}

#endif	// traktor_ai_NavMeshEntityEditorFactory_H
