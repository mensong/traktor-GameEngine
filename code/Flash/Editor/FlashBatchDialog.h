#ifndef traktor_flash_FlashBatchDialog_H
#define traktor_flash_FlashBatchDialog_H

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class ListBox;

		namespace custom
		{

class AutoPropertyList;
class ToolBarButtonClickEvent;

		}
	}

	namespace flash
	{

class FlashMovieAsset;

class FlashBatchDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	FlashBatchDialog(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	bool showModal(RefArray< FlashMovieAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::ListBox > m_movieList;
	Ref< ui::custom::AutoPropertyList > m_moviePropertyList;

	void addTexture();

	void removeTexture();

	void eventTextureListToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventTextureListSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_flash_FlashBatchDialog_H
