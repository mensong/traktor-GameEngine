/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IPanel_H
#define traktor_ui_IPanel_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Panel interface.
 * \ingroup UI
 */
class IPanel : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text) = 0;
};

	}
}

#endif	// traktor_ui_IPanel_H
