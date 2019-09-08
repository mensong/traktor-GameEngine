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

class MiniButton;

/*! \brief Object property item.
 * \ingroup UI
 */
class T_DLLCLASS ObjectPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ObjectPropertyItem(const std::wstring& text, const TypeInfo* objectType, Object* object);

	void setObjectType(const TypeInfo* objectType);

	const TypeInfo* getObjectType() const;

	void setObject(Object* object);

	Ref< Object > getObject() const;

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	Ref< MiniButton > m_buttonEdit;
	const TypeInfo* m_objectType;
	Ref< Object > m_object;

	void eventClick(ButtonClickEvent* event);
};

	}
}

