#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"
#include "Render/Vulkan/Editor/Glsl/GlslVariable.h"

namespace traktor
{
	namespace render
	{

class GlslLayout;
class OutputPin;

/*!
 * \ingroup Vulkan
 */
class GlslShader
{
public:
	enum ShaderType
	{
		StVertex,
		StFragment,
		StCompute
	};

	enum BlockType
	{
		BtInput,
		BtOutput,
		BtScript,
		BtBody,
		BtLast
	};

	GlslShader(ShaderType shaderType);

	virtual ~GlslShader();

	void addInputVariable(const std::wstring& variableName, GlslVariable* variable);

	GlslVariable* getInputVariable(const std::wstring& variableName);

	GlslVariable* createTemporaryVariable(const OutputPin* outputPin, GlslType type);

	GlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	//void associateVariable(const OutputPin* outputPin, GlslVariable* variable);

	GlslVariable* getVariable(const OutputPin* outputPin);

	void pushScope();

	void popScope();

	bool defineScript(const std::wstring& signature);


	/*! \name Output streams */
	/*! \{ */

	StringOutputStream& pushOutputStream(BlockType blockType, const wchar_t* const tag);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	const StringOutputStream& getOutputStream(BlockType blockType) const;

	/*! \} */


	std::wstring getGeneratedShader(const GlslLayout& layout, const GlslRequirements& requirements) const;

private:
	struct OutputPinVariable
	{
		const OutputPin* outputPin;
		Ref< GlslVariable > variable;
	};

	struct OutputStreamTuple
	{
		Ref< StringOutputStream > outputStream;
		const wchar_t* const tag;
	};

	ShaderType m_shaderType;
	std::map< std::wstring, Ref< GlslVariable > > m_inputVariables;
	AlignedVector< OutputPinVariable > m_variables;
	AlignedVector< uint32_t > m_variableScopes;
	AlignedVector< OutputPinVariable > m_outerVariables;
	std::set< std::wstring > m_scriptSignatures;
	int32_t m_nextTemporaryVariable;
	AlignedVector< OutputStreamTuple > m_outputStreams[BtLast];
};

	}
}

