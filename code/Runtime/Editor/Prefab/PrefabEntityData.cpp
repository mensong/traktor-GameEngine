#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Runtime/Editor/Prefab/PrefabEntityData.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.runtime.PrefabEntityData", 0, PrefabEntityData, world::EntityData)

PrefabEntityData::PrefabEntityData()
:	m_partitionMesh(false)
{
	m_outputGuid[0] = Guid::create();
	m_outputGuid[1] = Guid::create();
}

void PrefabEntityData::addEntityData(world::EntityData* entityData)
{
	T_ASSERT(std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void PrefabEntityData::removeEntityData(world::EntityData* entityData)
{
	auto it = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (it != m_entityData.end())
		m_entityData.erase(it);
}

void PrefabEntityData::removeAllEntityData()
{
	m_entityData.resize(0);
}

void PrefabEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * getTransform().inverse();
	for (auto entityData : m_entityData)
	{
		Transform currentTransform = entityData->getTransform();
		entityData->setTransform(deltaTransform * currentTransform);
	}
	world::EntityData::setTransform(transform);
}

void PrefabEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> MemberStaticArray< Guid, 2 >(L"outputGuid", m_outputGuid, AttributePrivate());
	s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
	s >> Member< bool >(L"partitionMesh", m_partitionMesh);
}

	}
}
