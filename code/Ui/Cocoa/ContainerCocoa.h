/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ContainerCocoa_H
#define traktor_ui_ContainerCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Cocoa/NSCustomControl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerCocoa
:	public WidgetCocoaImpl< IContainer, NSCustomControl >
,	public INSControlEventsCallback
{
public:
	ContainerCocoa(EventSubject* owner);
	
	// IWidget
	
	virtual void destroy() T_OVERRIDE T_FINAL;

	// IContainer

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;
	
	// INSControlEventsCallback
	
	virtual bool event_drawRect(const NSRect& rect) T_OVERRIDE T_FINAL;
	
	virtual bool event_viewDidEndLiveResize() T_OVERRIDE T_FINAL;
	
	virtual bool event_mouseDown(NSEvent* theEvent, int button) T_OVERRIDE T_FINAL;
	
	virtual bool event_mouseUp(NSEvent* theEvent, int button) T_OVERRIDE T_FINAL;
		
	virtual bool event_mouseMoved(NSEvent* theEvent, int button) T_OVERRIDE T_FINAL;

	virtual bool event_keyDown(NSEvent* theEvent) T_OVERRIDE T_FINAL;
	
	virtual bool event_keyUp(NSEvent* theEvent) T_OVERRIDE T_FINAL;

	virtual bool event_performKeyEquivalent(NSEvent* theEvent) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_ContainerCocoa_H
