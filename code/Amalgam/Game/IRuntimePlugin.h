/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_IRuntimePlugin_H
#define traktor_amalgam_IRuntimePlugin_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;
class IState;

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS IRuntimePlugin : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getDependencies(TypeInfoSet& outDependencies) const = 0;

	virtual bool startup(IEnvironment* environment) = 0;

	virtual bool shutdown(IEnvironment* environment) = 0;

	virtual Ref< IState > createInitialState(IEnvironment* environment) const = 0;
};

	}
}

#endif	// traktor_amalgam_IRuntimePlugin_H
