#ifndef traktor_editor_DeleteEmptyGroupsTool_H
#define traktor_editor_DeleteEmptyGroupsTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class DeleteEmptyGroupsTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, IEditor* editor) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_editor_DeleteEmptyGroupsTool_H
