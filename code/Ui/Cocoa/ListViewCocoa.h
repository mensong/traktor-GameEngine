#ifndef traktor_ui_ListViewCocoa_H
#define traktor_ui_ListViewCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IListView.h"

namespace traktor
{
	namespace ui
	{

class ListViewCocoa : public WidgetCocoaImpl< IListView, NSTableView >
{
public:
	ListViewCocoa(EventSubject* owner);
	
	// IListView

	virtual bool create(IWidget* parent, int style);
	
	virtual void setStyle(int style);

	virtual int addImage(IBitmap* image, int imageCount, bool smallImage);
	
	virtual void removeAllColumns();

	virtual void removeColumn(int columnIndex);

	virtual int getColumnCount() const;

	virtual int addColumn(const std::wstring& columnHeader, int width);

	virtual void setColumnHeader(int columnIndex, const std::wstring& columnHeader);

	virtual std::wstring getColumnHeader(int columnIndex) const;

	virtual void setColumnWidth(int columnIndex, int width);

	virtual int getColumnWidth(int columnIndex) const;

	virtual void setItems(ListViewItems* items);

	virtual ListViewItems* getItems() const;
	
	virtual int getSelectedItems(std::vector< int >& items) const;
};

	}
}

#endif	// traktor_ui_ListViewCocoa_H
