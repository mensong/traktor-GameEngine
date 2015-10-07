#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineDbFlat.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const uint32_t c_version = 1;
const uint32_t c_flushAfterChanges = 10;	//!< Flush pipeline after N changes.

class MemberDependencyHash : public MemberComplex
{
public:
	MemberDependencyHash(const wchar_t* const name, IPipelineDb::DependencyHash& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< uint32_t >(L"pipelineVersion", m_ref.pipelineVersion);
		s >> Member< uint32_t >(L"hash", m_ref.hash);
	}

private:
	IPipelineDb::DependencyHash& m_ref;
};

class MemberFileHash : public MemberComplex
{
public:
	MemberFileHash(const wchar_t* const name, IPipelineDb::FileHash& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< uint64_t >(L"size", m_ref.size);
		s >> MemberComposite< DateTime >(L"lastWriteTime", m_ref.lastWriteTime);
		s >> Member< uint32_t >(L"hash", m_ref.hash);
	}

private:
	IPipelineDb::FileHash& m_ref;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDbFlat", PipelineDbFlat, IPipelineDb)

PipelineDbFlat::PipelineDbFlat()
:	m_changes(0)
{
}

bool PipelineDbFlat::open(const std::wstring& connectionString)
{
	std::vector< std::wstring > pairs;
	std::map< std::wstring, std::wstring > cs;

	if (Split< std::wstring >::any(connectionString, L";", pairs) == 0)
	{
		log::error << L"Unable to open pipeline db; incorrect connection string." << Endl;
		return false;
	}

	for (std::vector< std::wstring >::const_iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		size_t p = i->find(L'=');
		if (p == 0 || p == i->npos)
		{
			log::error << L"Unable to open pipeline db; incorrect connection string." << Endl;
			return false;
		}

		cs[trim(i->substr(0, p))] = i->substr(p + 1);
	}

	m_file = cs[L"fileName"];

	// If flat database file doesn't exist we assume this is the first run; ie. don't fail.
	if (!FileSystem::getInstance().exist(m_file))
	{
		// But ensure full path is created first.
		return FileSystem::getInstance().makeAllDirectories(Path(m_file).getPathOnly());
	}

	Ref< IStream > f = FileSystem::getInstance().open(m_file, File::FmRead);
	if (!f)
		return false;

	BinarySerializer s(f);

	uint32_t version = 0;
	s >> Member< uint32_t >(L"version", version);

	if (version == c_version)
	{
		s >> MemberStlMap<
			Guid,
			DependencyHash,
			MemberStlPair<
				Guid,
				DependencyHash,
				Member< Guid >,
				MemberDependencyHash
			>
		>(L"dependencies", m_dependencies);

		s >> MemberStlMap<
			std::wstring,
			FileHash,
			MemberStlPair<
				std::wstring,
				FileHash,
				Member< std::wstring >,
				MemberFileHash
			>
		>(L"files", m_files);
	}

	f->close();
	f = 0;

	return true;
}

void PipelineDbFlat::close()
{
	endTransaction();
}

void PipelineDbFlat::beginTransaction()
{
}

void PipelineDbFlat::endTransaction()
{
	if (m_changes > 0)
	{
		Ref< IStream > f = FileSystem::getInstance().open(m_file, File::FmWrite);
		if (f)
		{
			BinarySerializer s(f);

			uint32_t version = c_version;
			s >> Member< uint32_t >(L"version", version);

			s >> MemberStlMap<
				Guid,
				DependencyHash,
				MemberStlPair<
				Guid,
				DependencyHash,
				Member< Guid >,
				MemberDependencyHash
				>
			>(L"dependencies", m_dependencies);

			s >> MemberStlMap<
				std::wstring,
				FileHash,
				MemberStlPair<
				std::wstring,
				FileHash,
				Member< std::wstring >,
				MemberFileHash
				>
			>(L"files", m_files);

			f->close();
			f = 0;

			m_changes = 0;
		}
		else
			log::error << L"Unable to flush pipeline db; failed to write latest changes." << Endl;
	}
}

void PipelineDbFlat::setDependency(const Guid& guid, const DependencyHash& hash)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);
	m_dependencies[guid] = hash;
	if (++m_changes >= c_flushAfterChanges)
		endTransaction();
}

bool PipelineDbFlat::getDependency(const Guid& guid, DependencyHash& outHash) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	std::map< Guid, DependencyHash >::const_iterator it = m_dependencies.find(guid);
	if (it == m_dependencies.end())
		return false;

	outHash = it->second;
	return true;
}

void PipelineDbFlat::setFile(const Path& path, const FileHash& file)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);
	m_files[path.getPathName()] = file;
	if (++m_changes >= c_flushAfterChanges)
		endTransaction();
}

bool PipelineDbFlat::getFile(const Path& path, FileHash& outFile)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	std::map< std::wstring, FileHash >::const_iterator it = m_files.find(path.getPathName());
	if (it == m_files.end())
		return false;

	outFile = it->second;
	return true;
}

Ref< IPipelineReport > PipelineDbFlat::createReport(const std::wstring& name, const Guid& guid)
{
	return 0;
}

	}
}
