#pragma once

#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*!
 * \ingroup Model
 */
class T_DLLCLASS Triangulate : public IModelOperation
{
	T_RTTI_CLASS;

public:
	virtual bool apply(Model& model) const override final;

	virtual void serialize(ISerializer& s) override final;
};

	}
}

