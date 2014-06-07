#ifndef traktor_sound_BankControl_H
#define traktor_sound_BankControl_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace sound
	{

class BankControlGrain;

class BankControl : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(BankControlGrain* item);

	void removeAll();

	BankControlGrain* getSelected() const;

	const RefArray< BankControlGrain >& getGrains() const;

protected:
	virtual void layoutCells(const ui::Rect& rc);

private:
	RefArray< BankControlGrain > m_cells;
};

	}
}

#endif	// traktor_sound_BankControl_H
