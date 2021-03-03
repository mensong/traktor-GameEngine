#pragma once

#include <spawn.h>
#include <sys/wait.h>
#include "Core/System/IProcess.h"

namespace traktor
{

class ProcessLinux : public IProcess
{
	T_RTTI_CLASS;

public:
	virtual ~ProcessLinux();

	virtual bool setPriority(Priority priority) override final;

	virtual IStream* getPipeStream(StdPipe pipe) override final;

	virtual IStream* waitPipeStream(int32_t timeout) override final;

	virtual bool signal(SignalType signalType) override final;

	virtual bool terminate(int32_t exitCode) override final;

	virtual int32_t exitCode() const override final;

	virtual bool wait(int32_t timeout) override final;

private:
	friend class OS;

	pid_t m_pid;
	posix_spawn_file_actions_t* m_fileActions;
	int m_childStdOut;
	int m_childStdErr;
	Ref< IStream > m_streamStdOut;
	Ref< IStream > m_streamStdErr;
	int32_t m_exitCode;

	ProcessLinux(pid_t pid, posix_spawn_file_actions_t* fileActions, int childStdOut, int childStdErr);
};

}
