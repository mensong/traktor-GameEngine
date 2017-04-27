/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptDebuggerLua_H
#define traktor_script_ScriptDebuggerLua_H

#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Thread/Semaphore.h"
#include "Script/IScriptDebugger.h"

struct lua_Debug;
struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptManagerLua;

/*! \brief LUA script debugger
 * \ingroup Script
 */
class ScriptDebuggerLua : public IScriptDebugger
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState);

	virtual ~ScriptDebuggerLua();

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual bool captureStackFrame(uint32_t depth, Ref< StackFrame >& outStackFrame) T_OVERRIDE T_FINAL;

	virtual bool captureLocals(uint32_t depth, RefArray< Variable >& outLocals) T_OVERRIDE T_FINAL;

	virtual bool captureObject(uint32_t object, RefArray< Variable >& outMembers) T_OVERRIDE T_FINAL;

	virtual bool isRunning() const T_OVERRIDE T_FINAL;

	virtual bool actionBreak() T_OVERRIDE T_FINAL;

	virtual bool actionContinue() T_OVERRIDE T_FINAL;

	virtual bool actionStepInto() T_OVERRIDE T_FINAL;

	virtual bool actionStepOver() T_OVERRIDE T_FINAL;

	virtual void addListener(IListener* listener) T_OVERRIDE T_FINAL;

	virtual void removeListener(IListener* listener) T_OVERRIDE T_FINAL;

private:
	friend class ScriptManagerLua;

	enum State
	{
		StRunning,
		StHalted,
		StBreak,
		StStepInto,
		StStepOver
	};

	ScriptManagerLua* m_scriptManager;
	lua_State* m_luaState;
	Semaphore m_lock;
	SmallMap< int32_t, SmallSet< Guid > > m_breakpoints;
	SmallSet< IListener* > m_listeners;
	Guid m_lastId;
	State m_state;

	void analyzeState(lua_State* L, lua_Debug* ar);

	void hookCallback(lua_State* L, lua_Debug* ar);
};

	}
}

#endif	// traktor_script_ScriptDebuggerLua_H
