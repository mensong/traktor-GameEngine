#ifndef traktor_script_ScriptClass_H
#define traktor_script_ScriptClass_H

#include "Script/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Script class definition.
 * \ingroup Script
 *
 * This class is used to describe native
 * classes which are intended to be useable
 * from scripts.
 */
class T_DLLCLASS ScriptClass : public Object
{
	T_RTTI_CLASS(ScriptClass)

public:
	/*! \brief Get exported native type. */
	virtual const Type& getExportType() const = 0;

	/*! \brief Get exported method count. */
	virtual uint32_t getMethodCount() const = 0;

	/*! \brief Get name of exported method. */
	virtual std::wstring getMethodName(uint32_t methodId) const = 0;

	/*! \brief Invoke exported method. */
	virtual Any invoke(Object* object, uint32_t methodId, const std::vector< Any >& args) const = 0;
};

	}
}

#endif	// traktor_script_ScriptClass_H
