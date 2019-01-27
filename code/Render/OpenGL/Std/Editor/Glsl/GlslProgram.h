#pragma once

#include <string>
#include <vector>
#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class GlslProgram
{
public:
	GlslProgram();

	GlslProgram(
		const std::wstring& vertexShader,
		const std::wstring& fragmentShader,
		const std::wstring& computeShader,
		const std::vector< std::wstring >& textures,
		const std::vector< NamedUniformType >& uniforms,
		const std::vector< SamplerBindingOpenGL >& samplers,
		const RenderStateOpenGL& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getFragmentShader() const;

	const std::wstring& getComputeShader() const;

	const std::vector< std::wstring >& getTextures() const;

	const std::vector< NamedUniformType >& getUniforms() const;

	const std::vector< SamplerBindingOpenGL >& getSamplers() const;

	const RenderStateOpenGL& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
	std::wstring m_computeShader;
	std::vector< std::wstring > m_textures;
	std::vector< NamedUniformType > m_uniforms;
	std::vector< SamplerBindingOpenGL > m_samplers;
	RenderStateOpenGL m_renderState;
};

	}
}
