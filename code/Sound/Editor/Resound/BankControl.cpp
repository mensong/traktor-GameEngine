#include "Ui/Application.h"
#include "Sound/Editor/Resound/BankControl.h"
#include "Sound/Editor/Resound/BankControlGrain.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankControl", BankControl, ui::custom::AutoWidget)

bool BankControl::create(ui::Widget* parent)
{
	if (!ui::custom::AutoWidget::create(parent, ui::WsDoubleBuffer))
		return false;

	setBackgroundColor(ui::getSystemColor(ui::ScButtonShadow));
	return true;
}

void BankControl::add(BankControlGrain* item)
{
	m_cells.push_back(item);
	requestUpdate();
}

void BankControl::removeAll()
{
	m_cells.clear();
	requestUpdate();
}

BankControlGrain* BankControl::getSelected() const
{
	return checked_type_cast< BankControlGrain* >(getFocusCell());
}

const RefArray< BankControlGrain >& BankControl::getGrains() const
{
	return m_cells;
}

void BankControl::layoutCells(const ui::Rect& rc)
{
	ui::Rect rowRect = rc;
	rowRect.top += 16;
	rowRect.bottom = rowRect.top + 32;

	for (RefArray< BankControlGrain >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		uint32_t depth = 0;
		for (const BankControlGrain* item = *i; item; item = item->getParent())
			++depth;

		ui::Rect cellRect = rowRect;
		cellRect.left += depth * 16;
		cellRect.right = cellRect.left + 128;
		placeCell(*i, cellRect);

		rowRect = rowRect.offset(0, 32 + 8);
	}
}

	}
}
