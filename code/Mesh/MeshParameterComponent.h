/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;

}

namespace traktor::mesh
{

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS MeshParameterComponent
:	public world::IEntityComponent
,	public IMeshParameterCallback
{
	T_RTTI_CLASS;

public:
	MeshParameterComponent() = default;

	explicit MeshParameterComponent(const SmallMap< render::handle_t, resource::Proxy< render::ITexture > >& textures);

	virtual void destroy() override;

	virtual void setOwner(world::Entity* owner) override;

	virtual void setTransform(const Transform& transform) override;

	virtual Aabb3 getBoundingBox() const override;

	virtual void update(const world::UpdateParams& update) override;

	// IMeshParameterCallback

	virtual void setParameters(render::ProgramParameters* programParameters) const override final;

private:
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > m_textures;
};

}
