/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshShapeDesc", ShapeDesc::Version, MeshShapeDesc, ShapeDesc)

void MeshShapeDesc::setMesh(const resource::Id< Mesh >& mesh)
{
	m_mesh = mesh;
}

const resource::Id< Mesh >& MeshShapeDesc::getMesh() const
{
	return m_mesh;
}

void MeshShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> resource::Member< Mesh >(L"mesh", m_mesh);
}

	}
}
