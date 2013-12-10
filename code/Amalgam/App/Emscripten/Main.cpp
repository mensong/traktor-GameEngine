#include <emscripten.h>
#include "Amalgam/IOnlineServer.h"
#include "Amalgam/Impl/Application.h"
#include "Amalgam/Impl/Environment.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/OS.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

Ref< amalgam::Application > g_application;

Ref< PropertyGroup > loadSettings(const Path& settingsFile)
{
	Ref< PropertyGroup > settings;

	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}

}

void mainLoop()
{
	if (g_application)
	{
		if (!g_application->update())
			safeDestroy(g_application);
	}
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	Path settingsPath = L"Application.config";
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);

	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		return 0;
	}

	Path workingDirectory = FileSystem::getInstance().getAbsolutePath(settingsPath).getPathOnly();
	FileSystem::getInstance().setCurrentVolumeAndDirectory(workingDirectory);

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	g_application = new amalgam::Application();
	if (!g_application->create(
		defaultSettings,
		settings,
		0,
		0
	))
		return 0;

	// Ok, everything seems to be setup fine, give main loop back to Emscripten.
	emscripten_set_main_loop(&mainLoop, 60, 0);
	return 0;
}
