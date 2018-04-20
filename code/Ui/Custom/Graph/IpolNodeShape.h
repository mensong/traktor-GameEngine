/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_IpolNodeShape_H
#define traktor_ui_custom_IpolNodeShape_H

#include "Ui/Custom/Graph/NodeShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IBitmap;

		namespace custom
		{

/*! \brief Interpolator node shape.
 * \ingroup UIC
 */
class T_DLLCLASS IpolNodeShape : public NodeShape
{
	T_RTTI_CLASS;

public:
	IpolNodeShape();

	virtual Point getPinPosition(const Node* node, const Pin* pin) const T_OVERRIDE T_FINAL;

	virtual Pin* getPinAt(const Node* node, const Point& pt) const T_OVERRIDE T_FINAL;

	virtual void paint(const Node* node, GraphCanvas* canvas, const Size& offset) const T_OVERRIDE T_FINAL;

	virtual Size calculateSize(const Node* node) const T_OVERRIDE T_FINAL;

private:
	Ref< IBitmap > m_imageNode[4];
	Ref< IBitmap > m_imagePin;
};

		}
	}
}

#endif	// traktor_ui_custom_IpolNodeShape_H
