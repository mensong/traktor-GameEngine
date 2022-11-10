/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Volume texture.
 * \ingroup Render
 */
class T_DLLCLASS IVolumeTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	/*! Get width in pixels of texture.
	 *
	 * \return Texture width in pixels.
	 */
	virtual int32_t getWidth() const = 0;

	/*! Get height in pixels of texture.
	 *
	 * \return Texture height in pixels.
	 */
	virtual int32_t getHeight() const = 0;

	/*! Get depth in pixels of texture.
	 *
	 * \return Texture depth in pixels.
	 */
	virtual int32_t getDepth() const = 0;

	/*! Get number of mip maps.
	 *
	 * \return Number of mips.
	 */
	virtual int32_t getMips() const = 0;	
};

}
