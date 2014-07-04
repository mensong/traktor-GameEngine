#include <CoreFoundation/CFBundle.h>#include <Foundation/Foundation.h>#if !TARGET_OS_IPHONE#	include <crt_externs.h>#endif#include <glob.h>#include <pwd.h>#include <spawn.h>#include <stdlib.h>#include <unistd.h>#include <mach/mach_host.h>#include <sys/param.h>#include <sys/types.h>#include <sys/stat.h>#include <sys/syslimits.h>#include "Core/Io/Path.h"#include "Core/Log/Log.h"#include "Core/Misc/String.h"#include "Core/Misc/StringSplit.h"#include "Core/Misc/TString.h"#include "Core/Singleton/SingletonManager.h"#include "Core/System/OS.h"#include "Core/System/ResolveEnv.h"#include "Core/System/OsX/ProcessOsX.h"#include "Core/System/OsX/SharedMemoryOsX.h"namespace traktor{T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)OS& OS::getInstance(){	static OS* s_instance = 0;	if (!s_instance)	{		s_instance = new OS();		s_instance->addRef(0);		SingletonManager::getInstance().add(s_instance);	}	return *s_instance;}uint32_t OS::getCPUCoreCount() const{	host_basic_info_data_t hostInfo;	mach_msg_type_number_t infoCount;		infoCount = HOST_BASIC_INFO_COUNT;	host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount);		return uint32_t(hostInfo.max_cpus);}Path OS::getExecutable() const{	return Path(L"");}std::wstring OS::getCommandLine() const{	return L"";}std::wstring OS::getComputerName() const{	char name[MAXHOSTNAMELEN];	if (gethostname(name, sizeof_array(name)) != -1)		return mbstows(name);	return L"";}std::wstring OS::getCurrentUser() const{	passwd* pwd = getpwuid(geteuid());	if (!pwd)		return L"";	const char* who = pwd->pw_name;	if (!who)		return L"";	return mbstows(who);}std::wstring OS::getUserHomePath() const{	char path[] = { "~" };	glob_t globbuf;		if (glob(path, GLOB_TILDE, NULL, &globbuf) == 0)	{		char* ep = globbuf.gl_pathv[0];		return mbstows(ep);	}		return L"";}std::wstring OS::getUserApplicationDataPath() const{	return getUserHomePath();}std::wstring OS::getWritableFolderPath() const{#if TARGET_OS_IPHONE	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);	NSString* documentsDirectory = [paths objectAtIndex: 0];	return mbstows([documentsDirectory UTF8String]);#else	return getUserHomePath() + L"/Library";#endif}bool OS::openFile(const std::wstring& file) const{#if !TARGET_OS_IPHONE	system(("open " + wstombs(file)).c_str());	return true;#else    return false;#endif}bool OS::editFile(const std::wstring& file) const{	return false;}bool OS::exploreFile(const std::wstring& file) const{	return false;}OS::envmap_t OS::getEnvironment() const{	envmap_t envmap;	#if !TARGET_OS_IPHONE	char** environ = *_NSGetEnviron();	for (char** e = environ; *e; ++e)	{		std::wstring pair(mbstows(*e));		size_t p = pair.find('=');		if (p != pair.npos)		{			std::wstring key = pair.substr(0, p);			std::wstring value = pair.substr(p + 1);			envmap[key] = value;		}	}#endif	return envmap;}bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const{	CFBundleRef mainBundle = CFBundleGetMainBundle();	if (mainBundle)	{		bool gotValue = false;				if (name == L"BUNDLE_PATH")	// /path-to-bundle/MyApplication.app		{			if (mainBundle)			{				CFURLRef appUrl = CFBundleCopyBundleURL(mainBundle);				if (appUrl)				{					char bundlePath[PATH_MAX];					CFURLGetFileSystemRepresentation(appUrl, TRUE, (uint8_t*)bundlePath, PATH_MAX);					CFRelease(appUrl);									outValue = mbstows(bundlePath);					gotValue = true;				}			}		}				if (gotValue)			return true;		// Try to get value from bundle.		CFStringRef keyStr = CFStringCreateWithCString(kCFAllocatorDefault, wstombs(name).c_str(), kCFStringEncodingMacRoman);		if (keyStr)		{			CFStringRef valueRef = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(mainBundle, keyStr);			if (valueRef)			{				const char* valueStr = CFStringGetCStringPtr(valueRef, kCFStringEncodingMacRoman);				if (valueStr)				{					outValue = mbstows(valueStr);										// \hack Replace "\$" sequences with plain "$" as Info.plist preprocessor					// seems to intercept and remove those.					outValue = replaceAll< std::wstring >(outValue, L"\\$", L"$");										gotValue = true;				}			}			CFRelease(keyStr);		}				if (gotValue)			return true;	}		// User home path.	if (name == L"HOME_PATH")	{		outValue = getUserHomePath();		return true;	}	// Ordinary variables; read from process environment.	const char* env = getenv(wstombs(name).c_str());	if (!env)		return false;			outValue = mbstows(env);	return true;}Ref< IProcess > OS::execute(	const std::wstring& commandLine,	const Path& workingDirectory,	const envmap_t* envmap,	bool redirect,	bool mute,	bool detach) const{#if !TARGET_OS_IPHONE	char cwd[512];	char* envv[256];	char* argv[64];	int envc = 0;	int argc = 0;	int err;	std::string executable;	std::string arguments;	// Resolve entire command line.	std::wstring resolvedCommandLine = resolveEnv(commandLine);	// Extract executable file from command line.	if (resolvedCommandLine.empty())		return 0;	if (resolvedCommandLine[0] == L'\"')	{		size_t i = resolvedCommandLine.find(L'\"', 1);		if (i == resolvedCommandLine.npos)			return 0;		executable = wstombs(resolvedCommandLine.substr(1, i - 1));		arguments = wstombs(resolvedCommandLine.substr(i + 1));	}	else	{		size_t i = resolvedCommandLine.find(L' ');		if (i != resolvedCommandLine.npos)		{			executable = wstombs(resolvedCommandLine.substr(0, i));			arguments = wstombs(resolvedCommandLine.substr(i + 1));		}		else			executable = wstombs(resolvedCommandLine);	}	// Convert all arguments; append bash if executing shell script.	if (endsWith< std::string >(executable, ".sh"))		argv[argc++] = strdup("/bin/sh");	else	{		// Ensure file has executable permission.		struct stat st;		if (stat(executable.c_str(), &st) == 0)		{			if (chmod(executable.c_str(), st.st_mode | S_IXUSR) != 0)				log::warning << L"Unable to set mode of executable" << Endl;		}		else			log::warning << L"Unable to get mode of executable" << Endl;	}		argv[argc++] = strdup(executable.c_str());		StringSplit< std::string > s(arguments, " ");	for (StringSplit< std::string >::const_iterator i = s.begin(); i != s.end(); ++i)		argv[argc++] = strdup((*i).c_str());		// Convert environment variables.	if (envmap)	{		for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)			envv[envc++] = strdup(wstombs(i->first + L"=" + i->second).c_str());	}		// Terminate argument and environment vectors.	envv[envc] = 0;	argv[argc] = 0;		// Spawned process inherit working directory from our process; thus	// we need to temporarily change directory.	getcwd(cwd, sizeof(cwd));	chdir(wstombs(workingDirectory.getPathNameNoVolume()).c_str());	// Redirect standard IO.	/*	posix_spawn_file_actions_t fileActions;	posix_spawn_file_actions_init(&fileActions);	posix_spawn_file_actions_adddup2(&fileActions, 1, 2);	*/		// Spawn process.	pid_t pid;	err = posix_spawn(&pid, argv[0], 0, 0, argv, envv);		// Restore our working directory before returning.	chdir(cwd);		if (err != 0)		return 0;	return new ProcessOsX(pid);	#else	return 0;#endif}Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const{	return new SharedMemoryOsX(size);}bool OS::setOwnProcessPriorityBias(int32_t priorityBias){	return false;}OS::OS(){}OS::~OS(){}void OS::destroy(){	T_SAFE_RELEASE(this);}}