#ifndef traktor_script_ScriptManagerJs_H
#define traktor_script_ScriptManagerJs_H

#include "Core/RefArray.h"
#include "Script/IScriptManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_JS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief JavaScript script manager.
 * \ingroup JS Script
 */
class T_DLLCLASS ScriptManagerJs : public IScriptManager
{
	T_RTTI_CLASS;

public:
	ScriptManagerJs();

	virtual void destroy();
	
	virtual void registerClass(IScriptClass* scriptClass);

	virtual Ref< IScriptResource > compile(const std::wstring& fileName, const std::wstring& script, const source_map_t* map, IErrorCallback* errorCallback) const;

	virtual Ref< IScriptContext > createContext(const IScriptResource* scriptResource);

	virtual Ref< IScriptDebugger > createDebugger();

	virtual void collectGarbage();

private:
	RefArray< IScriptClass > m_registeredClasses;
};

	}
}

#endif	// traktor_script_ScriptManagerJs_H
