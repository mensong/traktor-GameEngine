/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/Shared/WorldRendererShared.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IrradianceGrid;
class Packer;
class WorldEntityRenderers;

/*! World renderer implementation.
 * \ingroup World
 *
 * Simple and naive implementation supporting
 * only a single shadowing directional light
 * and max 2 overlapping lights.
 *
 * Operation
 * 1. Render global shadow map if directional light 0 enabled.
 * 2. Render depth pass if enabled (either explicitly or shadows enabled).
 * 3. Render screen-space shadow mask.
 * 4. Render visuals.
 *
 * Techniques used
 * "Default" - Visual, final, color output.
 * "Depth"   - Depth pass; write view distance into depth buffer.
 * "Shadow"  - Shadow map pass; write distance from light into shadow map.
 */
class T_DLLCLASS WorldRendererForward : public WorldRendererShared
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) override final;

private:
	friend class TilesOverlay;
	friend class SliceOverlay;

#pragma pack(1)
	struct LightShaderData
	{
		float typeRangeRadius[4];
		float position[4];
		float direction[4];
		float color[4];
		float viewToLight0[4];
		float viewToLight1[4];
		float viewToLight2[4];
		float viewToLight3[4];
		float atlasTransform[4];
	};
#pragma pack()

	Ref< render::Buffer > m_lightSBuffer;
	resource::Proxy< IrradianceGrid > m_irradianceGrid;
	Ref< Packer > m_shadowAtlasPacker;
	float m_slicePositions[MaxSliceCount + 1];

	void setupLightPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t& outShadowMapAtlasTargetSetId
	);

	void setupVisualPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t visualWriteTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t ambientOcclusionTargetSetId,
		render::handle_t reflectionsTargetSetId,
		render::handle_t shadowMapAtlasTargetSetId
	);
};

}
