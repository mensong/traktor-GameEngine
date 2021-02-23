#include "Runtime/Editor/Deploy/BuildTargetAction.h"
#include "Runtime/Editor/Deploy/DeployTargetAction.h"
#include "Runtime/Editor/Deploy/LaunchTargetAction.h"
#include "Runtime/Editor/Deploy/MigrateTargetAction.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Local/LocalDatabase.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

using namespace traktor;

namespace
{

std::wstring mangleName(std::wstring name)
{
	name = replaceAll(name, ' ', '_');
	name = replaceAll(name, ',', '_');
	name = replaceAll(name, ';', '_');
	name = replaceAll(name, '|', '_');
	return name;
}

bool loadSettings(const Path& pathName, Ref< PropertyGroup >& outOriginalSettings/*, Ref< PropertyGroup >* outSettings*/)
{
	Ref< IStream > file;

#if defined(_WIN32)
    std::wstring system = L"win32";
#elif defined(__APPLE__)
    std::wstring system = L"osx";
#elif defined(__LINUX__)
    std::wstring system = L"linux";
#elif defined(__RPI__)
    std::wstring system = L"rpi";
#endif

	std::wstring globalFile = pathName.getPathName();
	std::wstring systemFile = pathName.getPathNameNoExtension() + L"." + system + L"." + pathName.getExtension();

    // Read global properties.
	if ((file = FileSystem::getInstance().open(globalFile, File::FmRead)) != nullptr)
	{
		outOriginalSettings = xml::XmlDeserializer(file, globalFile).readObject< PropertyGroup >();
		file->close();

		if (!outOriginalSettings)
	        log::error << L"Error while parsing properties \"" << globalFile << L"\"." << Endl;
        else
            T_DEBUG(L"Successfully read properties from \"" << globalFile << L"\".");
	}
	else
        log::warning << L"Unable to read global properties \"" << globalFile << L"\"." << Endl;

    // Read system properties.
    if ((file = FileSystem::getInstance().open(systemFile, File::FmRead)) != nullptr)
    {
        Ref< PropertyGroup > systemSettings = xml::XmlDeserializer(file, systemFile).readObject< PropertyGroup >();
        file->close();

        if (systemSettings)
        {
            if (outOriginalSettings)
            {
                outOriginalSettings = outOriginalSettings->merge(systemSettings, PropertyGroup::MmJoin);
                T_ASSERT(outOriginalSettings);
            }
            else
                outOriginalSettings = systemSettings;

            T_DEBUG(L"Successfully read properties from \"" << systemFile << L"\".");
        }
		else
		{
            log::error << L"Error while parsing properties \"" << systemFile << L"\"." << Endl;
			return false;
		}
    }

	if (!outOriginalSettings)
		return false;

	//if (outSettings)
	//{
	//	std::wstring userFile = OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/" + pathName.getFileName();

	//	*outSettings = DeepClone(outOriginalSettings).create< PropertyGroup >();
	//	T_FATAL_ASSERT (*outSettings);

	//	// Read user properties.
	//	if ((file = FileSystem::getInstance().open(userFile, File::FmRead)) != nullptr)
	//	{
	//		Ref< PropertyGroup > userSettings = xml::XmlDeserializer(file, userFile).readObject< PropertyGroup >();
	//		file->close();

	//		if (!userSettings)
	//		{
	//			log::error << L"Error while parsing properties \"" << userFile << L"\"" << Endl;
	//			return false;
	//		}

	//		*outSettings = (*outSettings)->merge(userSettings, PropertyGroup::MmJoin);
	//		T_FATAL_ASSERT (*outSettings);
	//	}
	//}

	return true;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Traktor.Editor.Build.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << Endl;
		log::info << L"Usage: Traktor.Editor.Build.App (option(s)) [workspace]" << Endl;
		log::info << Endl;
		log::info << L"  Options:" << Endl;
		log::info << L"    -s,-settings                   Settings file (default \"$(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Editor.config\")" << Endl;
		log::info << L"    -v,-verbose                    Verbose building." << Endl;
		log::info << L"    -standalone                    Build using a standalone pipeline." << Endl;
		log::info << L"    -f,-force                      Force build." << Endl;
		log::info << L"    -file-cache=path               Specify pipeline file cache directory." << Endl;
		log::info << L"    -file-cache-access=r|w|rw      File cache access." << Endl;
		log::info << L"    -memcached-cache=host:port     Specify pipeline memcached host." << Endl;
		log::info << L"    -memcached-cache-access=r|w|rw Memcached cache access." << Endl;
		log::info << L"    -sequential-depends            Disable multithreaded pipeline dependency scanner." << Endl;
		log::info << L"    -sequential-build              Disable multithreaded pipeline build." << Endl;
		return 1;
	}

	std::wstring settingsFile = L"$(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Editor.config";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	std::wstring workspaceFile = cmdLine.getString(0);

	Ref< PropertyGroup > settings;
	if (!loadSettings(settingsFile, settings))
	{
		traktor::log::error << L"Unable to load settings \"" << settingsFile << L"\"." << Endl;
		return 1;
	}

	Ref< PropertyGroup > workspace;
	if (!loadSettings(workspaceFile, workspace))
	{
		traktor::log::error << L"Unable to load workspace \"" << workspaceFile << L"\"." << Endl;
		return 1;
	}

	if (workspace)
		settings = settings->merge(workspace, PropertyGroup::MmReplace);

	// Set flag to indicate we're running editor build standalone, do
	// not confuse this with "Pipeline.TargetEditor".
	settings->setProperty< PropertyBoolean >(L"Pipeline.TargetEditor.Build", true);

	// If cache has is explicitly set then we first clear property to ensure exclusivly enabled.
	if (cmdLine.hasOption(L"file-cache") || cmdLine.hasOption(L"memcached-cache"))
	{
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", false);
		settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached", false);
	}

	if (cmdLine.hasOption(L"file-cache"))
	{
		bool read = true;
		bool write = true;
		if (cmdLine.hasOption(L"file-cache-access"))
		{
			read =
			write = false;

			std::wstring access = cmdLine.getOption(L"file-cache-access").getString();
			for (auto ch : access)
			{
				if (ch == L'r')
					read = true;
				if (ch == L'w')
					write = true;
			}
		}

		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", true);
		settings->setProperty< PropertyString >(L"Pipeline.FileCache.Path", cmdLine.getOption(L"file-cache").getString());
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", read);
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", write);
	}

	if (cmdLine.hasOption(L"memcached-cache"))
	{
		std::wstring host = cmdLine.getOption(L"memcached-cache").getString();
		int32_t port = 11211;

		size_t i = host.find(L':');
		if (i != std::wstring::npos)
		{
			port = parseString< int32_t >(host.substr(i + 1));
			host = host.substr(0, i);
		}

		bool read = true;
		bool write = true;
		if (cmdLine.hasOption(L"memcached-cache-access"))
		{
			read =
			write = false;

			std::wstring access = cmdLine.getOption(L"memcached-cache-access").getString();
			for (auto ch : access)
			{
				if (ch == L'r')
					read = true;
				if (ch == L'w')
					write = true;
			}
		}

		settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached", true);
		settings->setProperty< PropertyString >(L"Pipeline.MemCached.Host", host);
		settings->setProperty< PropertyInteger >(L"Pipeline.MemCached.Port", port);
		settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", read);
		settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", write);
	}

	if (cmdLine.hasOption(L"sequential-depends"))
		settings->setProperty< PropertyBoolean >(L"Pipeline.DependsThreads", false);
	if (cmdLine.hasOption(L"sequential-build"))
		settings->setProperty< PropertyBoolean >(L"Pipeline.BuildThreads", false);

	// Remove filestore option from source database.
	db::ConnectionString sourceDatabaseCS = settings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	sourceDatabaseCS.set(L"fileStore", L"");
	settings->setProperty< PropertyString >(L"Editor.SourceDatabase", sourceDatabaseCS.format());

	// Write generated pipeline configuration in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
		L"Pipeline.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(settings);
		file->close();
	}
	else
	{
		log::error << L"Unable to write pipeline configuration." << Endl;
		return false;
	}

	StringOutputStream ss;
	ss << L"$(TRAKTOR_HOME)/bin/latest/win64/releaseshared/Traktor.Pipeline.App -s=Pipeline";

	if (cmdLine.hasOption(L"standalone"))
		ss << L" -standalone";
	if (cmdLine.hasOption(L'v', L"verbose"))
		ss << L" -v";
	if (cmdLine.hasOption(L'f', L"force"))
		ss << L" -f";

	Ref< IProcess > process = OS::getInstance().execute(
		ss.str(),
		L"",
		nullptr,
		OS::EfRedirectStdIO | OS::EfMute
	);
	if (!process)
	{
		log::error << L"Failed to launch pipeline process." << Endl;
		return false;
	}

	PipeReader stdOutReader(
		process->getPipeStream(IProcess::SpStdOut)
	);
	PipeReader stdErrReader(
		process->getPipeStream(IProcess::SpStdErr)
	);

	std::list< std::wstring > errors;
	std::wstring str;

	for (;;)
	{
		PipeReader::Result result1 = stdOutReader.readLine(str, 10);
		if (result1 == PipeReader::RtOk)
			log::info << str << Endl;

		PipeReader::Result result2 = stdErrReader.readLine(str, 10);
		if (result2 == PipeReader::RtOk)
		{
			str = trim(str);
			if (!str.empty())
			{
				log::error << str << Endl;
				errors.push_back(str);
			}
		}

		if (result1 == PipeReader::RtEnd && result2 == PipeReader::RtEnd)
			break;
	}

	if (!errors.empty())
	{
		log::error << L"Unsuccessful build, error(s):" << Endl;
		for (std::list< std::wstring >::const_iterator i = errors.begin(); i != errors.end(); ++i)
			log::error << L"\t" << *i << Endl;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process failed with exit code " << exitCode << L"." << Endl;

	return 0;
}
