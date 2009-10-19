#ifndef traktor_ui_NSOutlineViewDelegateProxy_H
#define traktor_ui_NSOutlineViewDelegateProxy_H

#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct INSOutlineViewEventsCallback
{
	virtual void event_selectionDidChange() = 0;
	
	virtual void event_rightMouseDown(NSEvent* event) = 0;
};
	
	}
}

@interface NSOutlineViewDelegateProxy : NSObject
{
	traktor::ui::INSOutlineViewEventsCallback* m_eventsCallback;
}

- (void) setCallback: (traktor::ui::INSOutlineViewEventsCallback*)eventsCallback;

- (void) outlineViewSelectionDidChange: (NSNotification*)notification;

- (void) outlineViewRightMouseDown: (NSEvent*)event;

@end

#endif	// traktor_ui_NSOutlineViewDelegateProxy_H
