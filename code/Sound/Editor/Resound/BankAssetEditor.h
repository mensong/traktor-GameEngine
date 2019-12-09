#pragma once

#include <map>
#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace ui
	{

class Command;
class Container;
class Menu;
class Panel;
class Slider;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

	}

	namespace sound
	{

class AudioChannel;
class AudioSystem;
class BankAsset;
class BankBuffer;
class BankControl;
class BankControlGrain;
class IGrain;
class IGrainData;
class IGrainFacade;
class GrainProperties;

class BankAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	BankAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< BankAsset > m_asset;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarButton > m_toolBarItemPlay;
	Ref< ui::ToolBarButton > m_toolBarItemRepeat;
	Ref< BankControl > m_bankControl;
	Ref< ui::Panel > m_containerParameters;
	RefArray< ui::Slider > m_sliderParameters;
	Ref< ui::Panel > m_containerGrainProperties;
	Ref< GrainProperties > m_grainProperties;
	Ref< ui::Panel > m_containerGrainView;
	Ref< ui::Menu > m_menuGrains;
	std::map< const TypeInfo*, Ref< IGrainFacade > > m_grainFacades;
	Ref< ui::Widget > m_currentGrainView;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< AudioSystem > m_audioSystem;
	Ref< AudioChannel > m_audioChannel;
	Ref< BankBuffer > m_bankBuffer;
	std::map< const IGrainData*, const IGrain* > m_grainInstances;

	void updateBankControl(BankControlGrain* parent, const RefArray< IGrainData >& grains);

	void updateBankControl();

	void updateProperties();

	void eventParameterChange(ui::ContentChangeEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventGrainSelect(ui::SelectionChangeEvent* event);

	void eventGrainButtonUp(ui::MouseButtonUpEvent* event);

	void eventGrainPropertiesChange(ui::ContentChangeEvent* event);

	void eventGrainViewChange(ui::ContentChangeEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

