#pragma once

#include "World/IWorldRenderPass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ISimpleTexture;
class StructBuffer;

	}

	namespace world
	{

/*! World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassSimple : public IWorldRenderPass
{
	T_RTTI_CLASS;

public:
	WorldRenderPassSimple(
		render::handle_t technique,
		render::ProgramParameters* globalProgramParams,
		const Matrix44& view
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual void setShaderTechnique(render::Shader* shader) const override final;

	virtual void setShaderCombination(render::Shader* shader) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const override final;

private:
	render::handle_t m_technique;
	render::ProgramParameters* m_globalProgramParams;
	Matrix44 m_view;
	Matrix44 m_viewInverse;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& world) const;

	void setLightProgramParameters(render::ProgramParameters* programParams) const;

	void setFogProgramParameters(render::ProgramParameters* programParams) const;

	void setColorMapProgramParameters(render::ProgramParameters* programParams) const;

	void setShadowMapProgramParameters(render::ProgramParameters* programParams) const;

	void setDepthMapProgramParameters(render::ProgramParameters* programParams) const;

	void setOcclusionMapProgramParameters(render::ProgramParameters* programParams) const;
};

	}
}
