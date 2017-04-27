/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_ICharacterFactory_H
#define traktor_spark_ICharacterFactory_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Character;
class CharacterData;
class Context;
class ICharacterBuilder;

/*! \brief Character instance factory interface.
 * \ingroup Spark
 */
class T_DLLCLASS ICharacterFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getCharacterTypes() const = 0;

	virtual Ref< Character > create(const Context* context, const ICharacterBuilder* builder, const CharacterData* characterData, const Character* parent, const std::wstring& name) const = 0;
};

	}
}

#endif	// traktor_spark_ICharacterFactory_H
