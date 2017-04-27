/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/CommandEvent.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageState.h"
#include "Amalgam/Game/Events/ActiveEvent.h"
#include "Amalgam/Game/Events/ReconfigureEvent.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.StageState", StageState, IState)

StageState::StageState(
	IEnvironment* environment,
	Stage* stage
)
:	m_environment(environment)
,	m_stage(stage)
{
}

void StageState::enter()
{
	log::info << Endl;
	log::info << L"-----------------------------------------------------------------------------" << Endl;
	log::info << L"Enter \"" << m_stage->getName() << L"\"" << Endl;
	log::info << L"-----------------------------------------------------------------------------" << Endl;
}

void StageState::leave()
{
	if (m_stage)
	{
		m_stage->transition();
		safeDestroy(m_stage);
	}
}

StageState::UpdateResult StageState::update(IStateManager* stateManager, const UpdateInfo& info)
{
	if (m_stage->update(stateManager, info))
		return UrOk;
	else
		return UrExit;
}

StageState::BuildResult StageState::build(uint32_t frame, const UpdateInfo& info)
{
	if (m_stage->build(info, frame))
		return BrOk;
	else
		return BrFailed;
}

bool StageState::render(uint32_t frame, render::EyeType eye, const UpdateInfo& info)
{
	m_stage->render(eye, frame);
	return true;
}

void StageState::flush()
{
	m_stage->flush();
}

bool StageState::take(const Object* event)
{
	if (const ReconfigureEvent* reconfigureEvent = dynamic_type_cast< const ReconfigureEvent* >(event))
	{
		if (!reconfigureEvent->isFinished())
			m_stage->preReconfigured();
		else
			m_stage->postReconfigured();
	}
	else if (const ActiveEvent* activeEvent = dynamic_type_cast< const ActiveEvent* >(event))
	{
		if (activeEvent->becameActivated())
			m_stage->resume();
		else
			m_stage->suspend();
	}
	else if (const CommandEvent* commandEvent = dynamic_type_cast< const CommandEvent* >(event))
	{
		m_stage->invokeScript(
			wstombs(commandEvent->getFunction()),
			0,
			0
		);
	}
	return true;
}

	}
}
