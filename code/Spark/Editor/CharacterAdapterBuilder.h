#ifndef traktor_spark_CharacterAdapterBuilder_H
#define traktor_spark_CharacterAdapterBuilder_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Spark/ICharacterBuilder.h"

namespace traktor
{
	namespace spark
	{

class CharacterAdapter;
class ICharacterFactory;

/*! \brief
 * \ingroup Spark
 */
class CharacterAdapterBuilder : public ICharacterBuilder
{
	T_RTTI_CLASS;

public:
	CharacterAdapterBuilder(Ref< CharacterAdapter >& outRoot, RefArray< CharacterAdapter >& outAdapters);

	void addFactory(ICharacterFactory* factory);

	virtual Ref< CharacterInstance > create(const Context* context, const Character* character, const CharacterInstance* parent, const std::wstring& name) const T_OVERRIDE T_FINAL;

private:
	SmallMap< const TypeInfo*, Ref< ICharacterFactory > > m_factories;
	Ref< CharacterAdapter >& m_outRoot;
	RefArray< CharacterAdapter >& m_outAdapters;
	mutable RefArray< CharacterAdapter > m_adapterStack;
};

	}
}

#endif	// traktor_spark_CharacterAdapterBuilder_H
