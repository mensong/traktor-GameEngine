/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace hf
	{

class Heightfield;

	}

	namespace terrain
	{

class ElevateBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	ElevateBrush(const resource::Proxy< hf::Heightfield >& heightfield, drawing::Image* splatImage);

	virtual uint32_t begin(float x, float y, const State& state) override final;

	virtual void apply(float x, float y) override final;

	virtual void end(float x, float y) override final;

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	Ref< drawing::Image > m_splatImage;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	int32_t m_attribute;
};

	}
}

