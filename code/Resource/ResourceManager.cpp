#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Resource/ResidentResourceHandle.h"
#include "Resource/IResourceFactory.h"
#include "Resource/ResourceManager.h"
#include "Resource/ExclusiveResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::~ResourceManager()
{
	destroy();
}

void ResourceManager::destroy()
{
	m_factories.clear();
	m_residentHandles.clear();
	m_exclusiveHandles.clear();
	m_times.clear();
}

void ResourceManager::addFactory(IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.push_back(factory);
}

void ResourceManager::removeFactory(IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.remove(factory);
}

void ResourceManager::removeAllFactories()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.clear();
}

Ref< IResourceHandle > ResourceManager::bind(const TypeInfo& type, const Guid& guid)
{
	Ref< IResourceHandle > handle;

	if (guid.isNull() || !guid.isValid())
		return 0;

	Ref< IResourceFactory > factory = findFactory(type);
	if (!factory)
		return 0;

	bool cacheable = factory->isCacheable();
	if (cacheable)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.find(guid);
		if (i != m_residentHandles.end())
			handle = i->second;
		else
		{
			Ref< ResidentResourceHandle > residentHandle = new ResidentResourceHandle(type);
			m_residentHandles[guid] = residentHandle;
			handle = residentHandle;
		}
	}
	else
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		RefArray< ExclusiveResourceHandle >& handles = m_exclusiveHandles[guid];

		// First try to reuse handles which are no longer in use.
		for (RefArray< ExclusiveResourceHandle >::iterator i = handles.begin(); i != handles.end(); ++i)
		{
			if (!(*i)->get())
			{
				handle = *i;
				break;
			}
		}

		if (!handle)
		{
			Ref< ExclusiveResourceHandle > exclusiveHandle = new ExclusiveResourceHandle(type);
			handles.push_back(exclusiveHandle);
			handle = exclusiveHandle;
		}
	}
	
	T_ASSERT (handle);

	if (!handle->get())
	{
		load(guid, factory, type, handle);
		if (!handle->get())
			return 0;
	}

	return handle;
}

void ResourceManager::reload(const Guid& guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i0 = m_exclusiveHandles.find(guid);
	if (i0 != m_exclusiveHandles.end())
	{
		const RefArray< ExclusiveResourceHandle >& handles = i0->second;
		for (RefArray< ExclusiveResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
		{
			const TypeInfo& resourceType = (*i)->getResourceType();
			Ref< IResourceFactory > factory = findFactory(resourceType);
			if (factory)
				load(guid, factory, resourceType, *i);
		}
		return;
	}

	std::map< Guid, Ref< ResidentResourceHandle > >::iterator i1 = m_residentHandles.find(guid);
	if (i1 != m_residentHandles.end())
	{
		const TypeInfo& resourceType = i1->second->getResourceType();
		Ref< IResourceFactory > factory = findFactory(resourceType);
		if (factory)
			load(guid, factory, resourceType, i1->second);
		return;
	}
}

void ResourceManager::unloadUnusedResident()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		T_ASSERT (i->second);
		if (i->second->getReferenceCount() <= 1 && i->second->get() != 0)
		{
			log::info << L"Unload resource \"" << i->first.format() << L"\" (" << type_name(i->second->get()) << L")" << Endl;
			i->second->replace(0);
		}
	}
}

void ResourceManager::dumpStatistics()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	log::info << L"Resource manager statistics:" << Endl;
	log::info << IncreaseIndent;
	
	log::info << uint32_t(m_residentHandles.size()) << L" resident entries" << Endl;
	log::info << uint32_t(m_exclusiveHandles.size()) << L" exclusive entries" << Endl;
	
	double totalTime = 0.0;
	for (std::map< const TypeInfo*, TimeCount >::const_iterator i = m_times.begin(); i != m_times.end(); ++i)
		totalTime += i->second.time;
	for (std::map< const TypeInfo*, TimeCount >::const_iterator i = m_times.begin(); i != m_times.end(); ++i)
		log::info << i->first->getName() << L" " << (i->second.time * 1000.0) << L" ms (" << i->second.count << L" resource(s), " << (i->second.time * 100.0f / totalTime) << L"%)" << Endl;
	
	log::info << DecreaseIndent;
}

Ref< IResourceFactory > ResourceManager::findFactory(const TypeInfo& type)
{
	for (RefArray< IResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
	{
		TypeInfoSet typeSet = (*i)->getResourceTypes();
		if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
			return *i;
	}
	return 0;
}

void ResourceManager::load(const Guid& guid, IResourceFactory* factory, const TypeInfo& resourceType, IResourceHandle* handle)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	// Do not attempt to load resource if thread has been stopped; application probably terminating
	// so we try to leave early.
	if (currentThread->stopped())
	{
		log::info << L"Resource loader thread stopped; skipped loading resource" << Endl;
		return;
	}

	m_timeStack.push(0.0);

	Timer timer;
	timer.start();

	//const TypeInfo& resourceType = handle->getResourceType();
	Ref< Object > object = factory->create(this, resourceType, guid);
	if (object)
	{
		T_ASSERT_M (is_type_of(resourceType, type_of(object)), L"Incorrect type of created resource");
		
		log::info << L"Resource \"" << guid.format() << L"\" (" << type_name(object) << L") created" << Endl;
		handle->replace(object);

		// Yield current thread; we want other threads to get some periodic CPU time to
		// render loading screens etc. Use sleep as we want lower priority threads also
		// to be able to run.
		currentThread->sleep(0);
	}
	else
		log::error << L"Unable to create resource \"" << guid.format() << L"\" (" << resourceType.getName() << L")" << Endl;

	// Accumulate time spend on creating resources.
	double time = timer.getElapsedTime();

	m_times[&resourceType].count++;
	m_times[&resourceType].time += time + m_timeStack.top();
	m_timeStack.pop();

	if (!m_timeStack.empty())
		m_timeStack.top() -= time;
}

	}
}
