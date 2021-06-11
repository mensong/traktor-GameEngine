#pragma once

#include "Runtime/IApplication.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Target/TargetPerformance.h"
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Core/Library/Library.h"
#include "Core/Math/Color4f.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;
class Thread;

	namespace db
	{

class Database;

	}

	namespace runtime
	{

// \fixme Move to configuration file.
#define T_MEASURE_PERFORMANCE 1

class AudioServer;
class InputServer;
class OnlineServer;
class PhysicsServer;
class RenderServer;
class ResourceServer;
class ScriptServer;
class WorldServer;

class Environment;
class IRuntimePlugin;
class IState;
class StateManager;
class TargetManagerConnection;

/*! Runtime application implementation.
 * \ingroup Runtime
 */
class T_DLLCLASS Application : public IApplication
{
	T_RTTI_CLASS;

public:
	Application();

	bool create(
		const PropertyGroup* defaultSettings,
		PropertyGroup* settings,
		const SystemApplication& sysapp,
		const SystemWindow* syswin
	);

	void destroy();

	bool update();

	void suspend();

	void resume();

	virtual IEnvironment* getEnvironment() override final;

	virtual IStateManager* getStateManager() override final;

private:
	Ref< PropertyGroup > m_settings;
	RefArray< Library > m_libraries;
	RefArray< IRuntimePlugin > m_plugins;
	Ref< TargetManagerConnection > m_targetManagerConnection;
	Ref< db::Database > m_database;
	Ref< AudioServer > m_audioServer;
	Ref< InputServer > m_inputServer;
	Ref< OnlineServer > m_onlineServer;
	Ref< PhysicsServer > m_physicsServer;
	Ref< RenderServer > m_renderServer;
	Ref< ResourceServer > m_resourceServer;
	Ref< ScriptServer > m_scriptServer;
	Ref< WorldServer > m_worldServer;
	Ref< Environment > m_environment;
	Ref< StateManager > m_stateManager;
	Semaphore m_lockUpdate;
#if !defined(__EMSCRIPTEN__)
	Thread* m_threadDatabase;
	Thread* m_threadRender;
#endif
	Timer m_timer;
	int32_t m_maxSimulationUpdates;
	int32_t m_deltaTimeError;
	UpdateControl m_updateControl;
	UpdateInfo m_updateInfo;
	bool m_renderViewActive;
	Color4f m_backgroundColor;
	float m_updateDuration;
	float m_buildDuration;
	float m_renderCpuDuration;
	float m_renderGpuDuration;
	int32_t m_renderGpuDurationQuery;
	uint32_t m_renderCollisions;
	Semaphore m_lockRender;
	Signal m_signalRenderBegin;
	Signal m_signalRenderFinish;
	uint32_t m_frameBuild;
	uint32_t m_frameRender;
	Ref< IState > m_stateRender;
	UpdateInfo m_updateInfoRender;
	render::RenderViewStatistics m_renderViewStats;
	TargetPerformance m_targetPerformance;

	bool updateInputDevices();

	void pollDatabase();

#if !defined(__EMSCRIPTEN__)

	void threadDatabase();

	void threadRender();

#endif
};

	}
}

