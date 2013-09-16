#include "Ai/MoveQuery.h"
#include "Ai/NavMesh.h"
#include "Ai/NavMeshEntity.h"
#include "Amalgam/Classes/AiClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Ref< script::BoxedVector4 > MoveQuery_update(ai::MoveQuery* self, const Vector4& currentPosition)
{
	Vector4 moveToPosition;
	if (self->update(currentPosition, moveToPosition))
		return new script::BoxedVector4(moveToPosition);
	else
		return 0;
}

Vector4 NavMesh_findClosestPoint(ai::NavMesh* self, const Vector4& searchFrom)
{
	Vector4 point;
	if (self->findClosestPoint(searchFrom, point))
		return point;
	else
		return searchFrom;
}

const ai::NavMesh* NavMeshEntity_get(ai::NavMeshEntity* self)
{
	return self->get();
}

		}

void registerAiClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< ai::MoveQuery > > classMoveQuery = new script::AutoScriptClass< ai::MoveQuery >();
	classMoveQuery->addMethod("update", &MoveQuery_update);
	scriptManager->registerClass(classMoveQuery);

	Ref< script::AutoScriptClass< ai::NavMesh > > classNavMesh = new script::AutoScriptClass< ai::NavMesh >();
	classNavMesh->addMethod("createMoveQuery", &ai::NavMesh::createMoveQuery);
	classNavMesh->addMethod("findClosestPoint", &NavMesh_findClosestPoint);
	scriptManager->registerClass(classNavMesh);

	Ref< script::AutoScriptClass< ai::NavMeshEntity > > classNavMeshEntity = new script::AutoScriptClass< ai::NavMeshEntity >();
	classNavMeshEntity->addMethod("get", &NavMeshEntity_get);
	scriptManager->registerClass(classNavMeshEntity);
}

	}
}
