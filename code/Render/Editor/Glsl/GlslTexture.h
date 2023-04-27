/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Editor/Glsl/GlslResource.h"
#include "Render/Editor/Glsl/GlslType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{
	
class T_DLLCLASS GlslTexture : public GlslResource
{
	T_RTTI_CLASS;

public:
	explicit GlslTexture(const std::wstring& name, uint8_t stages, GlslType uniformType);

	GlslType getUniformType() const { return m_uniformType; }

	virtual int32_t getOrdinal() const override final;

	virtual bool isBindless() const override final { return true; }

private:
	GlslType m_uniformType;
};

}
