#include "Amalgam/Editor/DeployTargetAction.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/PlatformInstance.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Editor/IEditor.h"
#include "Net/SocketAddressIPv4.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_remoteDatabasePort = 35000;
const uint16_t c_targetConnectionPort = 36000;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.DeployTargetAction", DeployTargetAction, ITargetAction)

DeployTargetAction::DeployTargetAction(const editor::IEditor* editor, PlatformInstance* platformInstance, TargetInstance* targetInstance, const Guid& activeGuid)
:	m_editor(editor)
,	m_platformInstance(platformInstance)
,	m_targetInstance(targetInstance)
,	m_activeGuid(activeGuid)
{
}

bool DeployTargetAction::execute()
{
	const Platform* platform = m_platformInstance->getPlatform();
	T_ASSERT (platform);

	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsDeploying);

	// Read platform default application configuration.
	Ref< Settings > applicationConfiguration = Settings::read< xml::XmlDeserializer >(FileSystem::getInstance().open(
		platform->getApplicationConfiguration(),
		File::FmRead
	));
	if (!applicationConfiguration)
	{
		log::error << L"Unable to open platform application configuration \"" << platform->getApplicationConfiguration() << L"\"" << Endl;
		return false;
	}

	// Merge with target application configuration.
	if (!target->getApplicationConfiguration().empty())
	{
		Ref< Settings > targetApplicationConfiguration = Settings::read< xml::XmlDeserializer >(FileSystem::getInstance().open(
			target->getApplicationConfiguration(),
			File::FmRead
		));
		if (!targetApplicationConfiguration)
		{
			log::error << L"Unable to open target application configuration \"" << target->getApplicationConfiguration() << L"\"" << Endl;
			return false;
		}

		applicationConfiguration->merge(targetApplicationConfiguration, true);
	}

	int32_t remoteDatabasePort = m_editor->getSettings()->getProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", c_remoteDatabasePort);
	int32_t targetManagerPort = m_editor->getSettings()->getProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", c_targetConnectionPort);

	// Determine our interface address; we let applications know where to find data.
	RefArray< net::SocketAddressIPv4 > interfaces = net::SocketAddressIPv4::getInterfaces();
	std::wstring host = !interfaces.empty() ? interfaces[0]->getHostName() : L"localhost";

	// Modify configuration to connect to embedded database server.
	db::ConnectionString remoteCs;
	remoteCs.set(L"provider", L"traktor.db.RemoteDatabase");
	remoteCs.set(L"host", host + L":" + toString(remoteDatabasePort));
	remoteCs.set(L"database", m_targetInstance->getName() + L"|" + m_platformInstance->getName());
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Database", remoteCs.format());

	// Modify configuration to connect to embedded target manager.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.TargetManager/Host", host);
	applicationConfiguration->setProperty< PropertyInteger >(L"Amalgam.TargetManager/Port", targetManagerPort);
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.TargetManager/Id", m_targetInstance->getId().format());

	// Append optional "active guid" to application configuration.
	if (m_activeGuid.isValid() && !m_activeGuid.isNull())
		applicationConfiguration->setProperty< PropertyString >(L"Amalgam.ActiveGuid", m_activeGuid.format());

	// Append target guids to application configuration.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.RootGuid", target->getRootAsset().format());
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.StartupGuid", target->getStartupInstance().format());

	// Write generated application configuration in output directory.
	std::wstring outputPath = L"output/" + m_targetInstance->getName() + L"/" + m_platformInstance->getName();
	applicationConfiguration->write< xml::XmlSerializer >(FileSystem::getInstance().open(
		outputPath + L"/Application.config",
		File::FmWrite
	));

	// Launch deploy tool to ensure platform is ready for launch.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
	envmap[L"DEPLOY_PROJECTNAME"] = m_targetInstance->getName();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathNameNoVolume();
#endif

	Ref< IProcess > process = OS::getInstance().execute(
		platform->getDeployTool(),
		L"deploy",
		outputPath,
		&envmap,
#if defined(_DEBUG)
		false, false, false
#else
		true, true, false
#endif
	);
	if (!process || !process->wait())
	{
		log::error << L"Failed to launch process \"" << platform->getDeployTool() << L"\"" << Endl;
		m_targetInstance->setState(TsIdle);
		return false;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process failed with exit code " << exitCode << Endl;

	return exitCode == 0;
}

	}
}
