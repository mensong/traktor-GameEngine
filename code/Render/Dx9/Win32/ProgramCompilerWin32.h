#ifndef traktor_render_ProgramCompilerWin32_H
#define traktor_render_ProgramCompilerWin32_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX9 program compiler.
 * \ingroup Render
 */
class ProgramCompilerWin32 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		uint32_t* outCostEstimate
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerWin32_H
