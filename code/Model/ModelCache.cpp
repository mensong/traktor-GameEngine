#include "Core/Io/FileSystem.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/HashStream.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Model/IModelOperation.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const Guid c_globalLockId(L"{BF9144AA-FA5C-49D6-8E5B-9593EEE92148}");

uint32_t hash(const std::wstring& text)
{
	Adler32 cs;
	cs.begin();
	cs.feed(text);
	cs.end();
	return cs.get();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelCache", ModelCache, Object)

ModelCache::ModelCache(
	const Path& cachePath,
	const std::function< Ref< File >(const Path&) >& getFile,
	const std::function< Ref< IStream >(const Path&) >& openStream
)
:	m_cachePath(cachePath)
,	m_getFile(getFile)
,	m_openStream(openStream)
,	m_lock(c_globalLockId)
{
}

Ref< Model > ModelCache::get(const Path& fileName, const std::wstring& filter)
{
	return get(fileName, filter, RefArray< const IModelOperation >());
}

Ref< Model > ModelCache::get(const Path& fileName, const std::wstring& filter, const RefArray< const IModelOperation >& operations)
{
	// Get information about source file.
	Ref< File > file = m_getFile(fileName);
	if (!file)
		return nullptr;

	// Calculate hash of resolved file name.
	uint32_t fileNameHash = hash(fileName.getPathName() + L"!" + filter);

	// Calculate hash of all operations.
	Adler32 cs;
	HashStream hs(&cs);
	for (auto operation : operations)
		BinarySerializer(&hs).writeObject(operation);
	uint32_t operationHash = cs.get();

	// Generate file name of cached model.
	Path cachedFileName = m_cachePath.getPathName() + L"/" + toString(fileNameHash) + L"/" + toString(operationHash) + L".tmd";

	// Check if cached file exist and if it's time stamp match source file's.
	bool haveCachedFile = false;
	{
		T_ANONYMOUS_VAR(Acquire< Mutex >)(m_lock);
		Ref< File > cachedFile = FileSystem::getInstance().get(cachedFileName);
		if (cachedFile != nullptr && file->getLastWriteTime() <= cachedFile->getLastWriteTime())
			haveCachedFile = true;
	}
	if (haveCachedFile)
	{
		// Valid cache entry found; read from model from cache,
		// do not use filter as it's written into cache after filter has been applied.
		return ModelFormat::readAny(cachedFileName, L"");
	}

	// No cached file exist; need to read source model and apply all operations.
	Ref< Model > model = ModelFormat::readAny(fileName, filter, m_openStream);
	if (!model)
		return nullptr;

	for (auto operation : operations)
		operation->apply(*model);

	// Write cached copy of post-operation model.
	{
		T_ANONYMOUS_VAR(Acquire< Mutex >)(m_lock);
		FileSystem::getInstance().makeAllDirectories(cachedFileName.getPathOnly());
		ModelFormat::writeAny(cachedFileName, model);
	}

	// Return model; should be same as one we've written to cache.
	return model;
}

Ref< Model > ModelCache::get(const Path& fileName, const std::wstring& filter, uint32_t user)
{
	return nullptr;
}

void ModelCache::put(const Path& fileName, const std::wstring& filter, const Model* model, uint32_t user)
{
}

	}
}