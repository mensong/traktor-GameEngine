#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/Lua/ScriptResourceLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptResourceLua", 0, ScriptResourceLua, IScriptResource)

ScriptResourceLua::ScriptResourceLua()
{
}

ScriptResourceLua::ScriptResourceLua(const std::string& fileName, const std::string& script, const source_map_t& map)
:	m_fileName(fileName)
,	m_script(script)
,	m_map(map)
{
}

const std::string& ScriptResourceLua::getFileName() const
{
	return m_fileName;
}

const std::string& ScriptResourceLua::getScript() const
{
	return m_script;
}

const source_map_t& ScriptResourceLua::getMap() const
{
	return m_map;
}

bool ScriptResourceLua::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"fileName", m_fileName);
	s >> Member< std::string >(L"script", m_script);
	s >> MemberStlList< source_map_t::value_type, MemberStlPair< source_map_t::value_type::first_type, source_map_t::value_type::second_type > >(L"map", m_map);
	return true;
}

	}
}
