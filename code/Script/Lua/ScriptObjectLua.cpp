#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptObjectLua", ScriptObjectLua, Object)

ScriptObjectLua::ScriptObjectLua(lua_State*& luaState, int32_t tableRef, const ScriptClassLua* scriptClass)
:	m_luaState(luaState)
,	m_tableRef(tableRef)
,	m_scriptClass(scriptClass)
{
}

ScriptObjectLua::~ScriptObjectLua()
{
	luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
}

void ScriptObjectLua::push()
{
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
	T_ASSERT (lua_istable(m_luaState, -1));
}

	}
}
