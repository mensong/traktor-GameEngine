#if !defined(__APPLE__) && !defined(__PNACL__) && !defined(__ANDROID__)
#	include <glsl_optimizer.h>
#endif
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ES2/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Semaphore s_lock;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGLES2", 0, ProgramCompilerOpenGLES2, IProgramCompiler)

const wchar_t* ProgramCompilerOpenGLES2::getPlatformSignature() const
{
	return L"OpenGL ES2";
}

Ref< ProgramResource > ProgramCompilerOpenGLES2::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	// Generate GLSL shader.
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, glslProgram))
		return 0;

#if !defined(__APPLE__) && !defined(__PNACL__) && !defined(__ANDROID__)
	// Optimize GLSL shader.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_lock);

		glslopt_ctx* ctx = glslopt_initialize(kGlslTargetOpenGLES20);
		if (ctx)
		{
			std::string vs = wstombs(glslProgram.getVertexShader());
			std::string fs = wstombs(glslProgram.getFragmentShader());

			// Hack to allow optimization of instancing shaders.
			vs = replaceAll< std::string >(vs, "GL_EXT_draw_instanced", "GL_ARB_draw_instanced");
			vs = replaceAll< std::string >(vs, "gl_InstanceIDEXT", "gl_InstanceIDARB");

			glslopt_shader* vso = glslopt_optimize(ctx, kGlslOptShaderVertex, vs.c_str(), 0);
			glslopt_shader* fso = glslopt_optimize(ctx, kGlslOptShaderFragment, fs.c_str(), 0);

			if (
				glslopt_get_status(vso) &&
				glslopt_get_status(fso)
			)
			{
				std::string vss = glslopt_get_output(vso);
				std::string fss = glslopt_get_output(fso);

				vss = replaceAll< std::string >(vss, "GL_ARB_draw_instanced", "GL_EXT_draw_instanced");
				vss = replaceAll< std::string >(vss, "gl_InstanceIDARB", "gl_InstanceIDEXT");

				glslProgram = GlslProgram(
					mbstows(vss),
					mbstows(fss),
					glslProgram.getTextures(),
					glslProgram.getUniforms(),
					glslProgram.getSamplers(),
					glslProgram.getRenderState()
				);
			}
			else
			{
				//errorLog = glslopt_get_log (shader);
				log::warning << L"Unable to optimize GLSL shader" << Endl;
			}

			glslopt_shader_delete(fso);
			glslopt_shader_delete(vso);

			glslopt_cleanup(ctx);
		}
		else
			log::error << L"Unable to initialize GLSL optimizer" << Endl;
	}
#endif

	Ref< ProgramResource > resource = ProgramOpenGLES2::compile(glslProgram, optimize, validate);
	if (!resource)
		return 0;

	return resource;
}

bool ProgramCompilerOpenGLES2::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outShader
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, glslProgram))
		return false;

	outShader =
		std::wstring(L"// Vertex shader\n") +
		std::wstring(L"\n") +
		glslProgram.getVertexShader() +
		std::wstring(L"\n") +
		std::wstring(L"// Fragment shader\n") +
		std::wstring(L"\n") +
		glslProgram.getFragmentShader();

	return true;
}

	}
}
