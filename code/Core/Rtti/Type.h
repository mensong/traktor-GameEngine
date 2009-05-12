#ifndef traktor_kernel_Type_H
#define traktor_kernel_Type_H

#include <vector>
#include <set>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \ingroup Core */
//@{

extern void T_DLLCLASS __forceLinkReference(const class Type& type);
#define T_FORCE_LINK_REF(CLASS) \
	traktor::__forceLinkReference(traktor::type_of< CLASS >());

class Object;

#define T_RTTI_CLASS(CLASS) \
	public: \
		static const traktor::Type& getClassType(); \
		virtual const traktor::Type& getType() const; \
	private: \
		static const traktor::Type m__type__;

#define T_IMPLEMENT_RTTI_CLASS_NB(CLASSID, CLASS) \
	const traktor::Type CLASS::m__type__(0, CLASSID, sizeof(CLASS), 0); \
	\
	const traktor::Type& CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_CLASS(CLASSID, CLASS, SUPER) \
	const traktor::Type CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(CLASS), 0); \
	\
	const traktor::Type& CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_COMPOSITE_CLASS(CLASSID, OUTER, CLASS, SUPER) \
	const traktor::Type OUTER::CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(OUTER::CLASS), 0); \
	\
	const traktor::Type& OUTER::CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& OUTER::CLASS::getType() const { \
		return m__type__; \
	}

struct T_DLLCLASS ObjectFactory
{
	virtual Object* newInstance() const = 0;
};

/*! \brief RTTI type descriptor. */
class T_DLLCLASS Type
{
public:
	Type(const Type* super, const wchar_t* name, size_t size, const ObjectFactory* factory);

	virtual ~Type();

	/*! \brief Return super type. */
	const Type* getSuper() const;

	/*! \brief Name of type. */
	const wchar_t* getName() const;

	/*! \brief Size of type in bytes. */
	size_t getSize() const;

	/*! \brief Instantiable type. */
	bool isInstantiable() const;

	/*! \brief Create new instance of type. */
	Object* newInstance() const;

	/*! \brief Find type from string representation. */
	static const Type* find(const std::wstring& name);

	/*! \brief Find all types derived from this type. */
	void findAllOf(std::vector< const Type* >& outTypes) const;

private:
	const Type* m_super;
	const wchar_t* m_name;
	size_t m_size;
	const ObjectFactory* m_factory;
};

typedef std::set< const Type* > TypeSet;

/*! \brief Get type of class */
template < typename T >
inline const Type& type_of()
{
	return T::getClassType();
}

/*! \brief Check if type is derived from a base type. */
inline bool is_type_of(const Type& base, const Type& type)
{
	if (&base == &type)
		return true;

	if (!type.getSuper())
		return false;

	return is_type_of(base, *type.getSuper());
}

/*! \brief Check if type is derived from a base type. */
template < typename T >
inline bool is_type_of(const Type& type)
{
	return is_type_of(T::getClassType(), type);
}

//@}

}

#endif	// traktor_kernel_Type_H
