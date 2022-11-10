/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPlugin.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace render
	{

class ShaderDependencyTracker;

/*! Render editor plugin.
 * \ingroup Render
 */
class RenderEditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	explicit RenderEditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

private:
	editor::IEditor* m_editor;
	Ref< ShaderDependencyTracker > m_tracker;

	bool createRenderSystem();
};

	}
}

