/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/PropertyList/PropertyItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! Check property item.
 * \ingroup UI
 */
class T_DLLCLASS CheckPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	CheckPropertyItem(const std::wstring& text, bool checked);

	void setChecked(bool checked);

	bool isChecked() const;

protected:
	virtual void mouseButtonDown(MouseButtonDownEvent* event) override;

	virtual void paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	Ref< IBitmap > m_imageUnchecked;
	Ref< IBitmap > m_imageChecked;
	bool m_checked;
};

	}
}

