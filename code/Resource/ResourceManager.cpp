#include "Resource/ResourceManager.h"
#include "Resource/ResourceLoader.h"
#include "Resource/ResourceCache.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Heap/New.h"
#include "Core/Thread/Acquire.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, Singleton)

ResourceManager& ResourceManager::getInstance()
{
	static ResourceManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new ResourceManager();
		SingletonManager::getInstance().addBefore(s_instance, &Heap::getInstance());
	}
	return *s_instance;
}

void ResourceManager::addLoader(ResourceLoader* loader)
{
	Acquire< Semaphore > scope(m_lock);
	m_loaders.push_back(loader);
}

void ResourceManager::removeLoader(ResourceLoader* loader)
{
	Acquire< Semaphore > scope(m_lock);
	m_loaders.remove(loader);
}

void ResourceManager::setCache(IResourceCache* cache)
{
	Acquire< Semaphore > scope(m_lock);
	m_cache = cache;
}

IResourceCache* ResourceManager::getCache() const
{
	return m_cache;
}

void ResourceManager::setResource(const Type& type, const Guid& guid, Object* resource)
{
	Acquire< Semaphore > scope(m_lock);
	m_cache->put(guid, resource);
}

Object* ResourceManager::getResource(const Type& type, const Guid& guid)
{
	Acquire< Semaphore > scope(m_lock);

	// If no cache is available we cannot load it.
	if (!m_cache)
		return 0;

	Ref< Object > resource;
	if (!m_cache->get(guid, resource))
	{
		bool cacheable = true;
		for (RefList< ResourceLoader >::iterator i = m_loaders.begin(); i != m_loaders.end(); ++i)
		{
			if ((resource = (*i)->load(type, guid, cacheable)) != 0)
				break;
		}
		if (cacheable)
			m_cache->put(guid, resource);
	}

	return resource;
}

void ResourceManager::beginPrepareResources()
{
	m_acceptResourceRequests = true;
}

void ResourceManager::requestResource(const Type& type, const Guid& guid)
{
	if (m_acceptResourceRequests)
		m_requestedResources.push_back(std::make_pair(&type, guid));
}

void ResourceManager::endPrepareResources(bool cancel)
{
	T_ASSERT_M (m_acceptResourceRequests, L"endPrepareResources called without beginPrepareResources");

	if (!cancel)
	{
		while (!m_requestedResources.empty())
		{
			std::pair< const Type*, Guid > requestedResource = m_requestedResources.front();
			m_requestedResources.pop_front();

			getResource(*requestedResource.first, requestedResource.second);
		}
	}
	else
		m_requestedResources.resize(0);

	m_acceptResourceRequests = false;
}

void ResourceManager::destroy()
{
	delete this;
}

ResourceManager::ResourceManager()
:	m_cache(gc_new< ResourceCache >())
,	m_requestedResources(false)
{
}

	}
}
