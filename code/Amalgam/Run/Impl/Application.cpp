/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/TargetManagerConnection.h"
#include "Amalgam/TargetPerformance.h"
#include "Amalgam/Run/Impl/Application.h"
#include "Amalgam/Run/Impl/Environment.h"
#include "Amalgam/Run/Impl/ScriptServer.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Application", Application, IApplication)

Application::Application()
{
}

bool Application::create(
	const PropertyGroup* defaultSettings,
	PropertyGroup* settings
)
{
	// Establish target manager connection is launched from the Editor.
	std::wstring targetManagerHost = settings->getProperty< std::wstring >(L"Amalgam.TargetManager/Host");
	int32_t targetManagerPort = settings->getProperty< int32_t >(L"Amalgam.TargetManager/Port");
	Guid targetManagerId = Guid(settings->getProperty< std::wstring >(L"Amalgam.TargetManager/Id"));
	if (!targetManagerHost.empty() && targetManagerPort && targetManagerId.isValid())
	{
		m_targetManagerConnection = new TargetManagerConnection();
		if (!m_targetManagerConnection->connect(targetManagerHost, targetManagerPort, targetManagerId))
		{
			log::warning << L"Unable to connect to target manager at \"" << targetManagerHost << L"\"; unable to debug" << Endl;
			m_targetManagerConnection = 0;
		}
	}

	// Load dependent modules.
#if !defined(T_STATIC)
	std::set< std::wstring > modules = defaultSettings->getProperty< std::set< std::wstring > >(L"Amalgam.Modules");
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		Ref< Library > library = new Library();
		if (library->open(*i))
			m_libraries.push_back(library);
		else
			log::warning << L"Unable to load module \"" << *i << L"\"" << Endl;
	}
#endif

	// Database
	T_DEBUG(L"Creating database...");
	m_database = new db::Database();
	std::wstring connectionString = settings->getProperty< std::wstring >(L"Amalgam.Database");
	if (!m_database->open(connectionString))
	{
		log::error << L"Application failed; unable to open database \"" << connectionString << L"\"" << Endl;
		return false;
	}

	// Script
	if (settings->getProperty(L"Script.Type"))
	{
		T_DEBUG(L"Creating script server...");
		m_scriptServer = new ScriptServer();

		bool attachDebugger = settings->getProperty< bool >(L"Script.AttachDebugger", false);
		bool attachProfiler = settings->getProperty< bool >(L"Script.AttachProfiler", false);

		if ((attachDebugger || attachProfiler) && m_targetManagerConnection)
		{
			if (!m_scriptServer->create(defaultSettings, settings, attachDebugger, attachProfiler, m_targetManagerConnection->getTransport()))
				return false;
		}
		else
		{
			if (!m_scriptServer->create(defaultSettings, settings, false, false, 0))
				return false;
		}
	}

	// Environment
	T_DEBUG(L"Creating environment...");
	m_environment = new Environment(
		settings,
		m_database,
		m_scriptServer
	);

	m_settings = settings;
	return true;
}

void Application::destroy()
{
	if (m_environment)
		m_environment->m_alive = false;

	safeDestroy(m_scriptServer);

	m_environment = 0;

	if (m_database)
	{
		m_database->close();
		m_database = 0;
	}

	for (RefArray< Library >::iterator i = m_libraries.begin(); i != m_libraries.end(); ++i)
		(*i)->detach();
}

bool Application::execute()
{
	if (!m_scriptServer->execute(m_environment))
		return false;

	for (;;)
	{
		// Update target manager connection.
		if (m_targetManagerConnection && !m_targetManagerConnection->update())
		{
			log::warning << L"Connection to target manager lost; terminating application..." << Endl;
			return false;
		}

		// Update script execution.
		if (!m_scriptServer->update())
			break;
	}

	return true;
}

Ref< IEnvironment > Application::getEnvironment()
{
	return m_environment;
}

	}
}
