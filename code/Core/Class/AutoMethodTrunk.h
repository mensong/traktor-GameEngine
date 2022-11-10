/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Io/OutputStream.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

template < typename ClassType, typename ReturnType, typename ... ArgumentTypes >
class AutoMethodTrunk final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoMethodTrunk);

public:
	typedef ReturnType (*method_t)(ClassType*, ArgumentTypes ...);

	method_t m_method;

	explicit AutoMethodTrunk(method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final 
	{
		os << CastAny< ReturnType >::typeName();
		int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = { (os << L"," << CastAny< ArgumentTypes >::typeName(), 0) ... };
        (void)__dummy__;
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		T_VERIFY_ARGUMENT_TYPES;
		return invokeI(self, argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
	}

private:
	template < std::size_t... Is >
	inline Any invokeI(ITypedObject* self, const Any* argv, std::index_sequence< Is... >) const
	{
		return CastAny< ReturnType >::set((*m_method)(
			T_VERIFY_CAST_SELF(ClassType, self),
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		));
	}
};

template < typename ClassType, typename ... ArgumentTypes >
class AutoMethodTrunk < ClassType, void, ArgumentTypes ... > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoMethodTrunk);

public:
	typedef void (*method_t)(ClassType*, ArgumentTypes ...);

	method_t m_method;

	explicit AutoMethodTrunk(method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final 
	{
		os << L"void";
		int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = { (os << L"," << CastAny< ArgumentTypes >::typeName(), 0) ... };
        (void)__dummy__;
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		T_VERIFY_ARGUMENT_TYPES;
		invokeI(self, argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
		return Any();
	}

private:
	template < std::size_t... Is >
	inline void invokeI(ITypedObject* self, const Any* argv, std::index_sequence< Is... >) const
	{
		(*m_method)(
			T_VERIFY_CAST_SELF(ClassType, self),
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		);
	}
};

/*! \} */

}
