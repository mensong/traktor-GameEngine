/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Editor/DataAccessCache.h"
#include "Editor/IPipelineCache.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DataAccessCache", DataAccessCache, Object)

DataAccessCache::DataAccessCache(IPipelineCache* cache)
:	m_cache(cache)
{
}

Ref< Object > DataAccessCache::readObject(
	const Key& key,
	const std::function< Ref< Object > (IStream* stream) >& read,
	const std::function< bool (const Object* object, IStream* stream) >& write,
	const std::function< Ref< Object > () >& create
)
{
	Ref< IStream > s;

	// Try to read from cache first.
	if (m_cache != nullptr)
	{
		if ((s = m_cache->get(key)) != nullptr)
		{
			Ref< Object > object = read(s); s->close();
			return object;
		}
	}

	// No cached entry; need to fabricate object.
	Ref< Object > object = create();
	if (!object)
		return nullptr;

	// Upload to cache and then return object.
	if (m_cache != nullptr)
	{
		if ((s = m_cache->put(key)) != nullptr)
		{
			if (write(object, s))
				s->close();
			else
				log::error << L"Unable to upload memento object to cache." << Endl;
		}
	}
	
	return object;
}

}
