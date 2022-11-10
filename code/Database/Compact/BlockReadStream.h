/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/StreamStream.h"

namespace traktor
{
	namespace db
	{

class BlockFile;

class BlockReadStream : public StreamStream
{
public:
	BlockReadStream(BlockFile* blockFile, IStream* stream, int64_t endOffset);

	virtual ~BlockReadStream();

	virtual void close() override final;

private:
	Ref< BlockFile > m_blockFile;
};

	}
}

