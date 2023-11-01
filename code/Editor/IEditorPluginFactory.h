/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Ui/Command.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;
class IEditorPlugin;

/*! Editor plugin factory.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorPluginFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const = 0;

	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;

	virtual Ref< IEditorPlugin > createEditorPlugin(IEditor* editor) const = 0;
};

}
