#include "CanvasView.h"

#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#ifdef __HAIKU__
#	include <LayoutUtils.h>
#endif
#include <Message.h>
#include <MessageRunner.h>
#include <Messenger.h>
#include <Region.h>
#include <Window.h>

#include "cursors.h"
#include "ui_defines.h"
#include "support.h"

#include "Document.h"
#include "RenderManager.h"


static const rgb_color kStripeLight = (rgb_color){ 112, 112, 112, 255 };
static const rgb_color kStripeDark = (rgb_color){ 104, 104, 104, 255 };

#define AUTO_SCROLL_DELAY		40000 // 40 ms

enum {
	MSG_AUTO_SCROLL	= 'ascr'
};

// constructor
CanvasView::CanvasView(BRect frame, Document* document, RenderManager* manager)
	:
	BackBufferedStateView(frame, "canvas view", B_FOLLOW_NONE,
		B_WILL_DRAW | B_FRAME_EVENTS),
	fDocument(document),
	fRenderManager(manager),

	fZoomLevel(1.0),

	fSpaceHeldDown(false),
	fScrollTracking(false),
	fInScrollTo(false),
	fScrollTrackingStart(0.0, 0.0),
	fScrollOffsetStart(0.0, 0.0),

	fAutoScroller(NULL)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	SetHighColor(kStripeLight);
	SetLowColor(kStripeDark);
		// used for drawing the stripes pattern
	SetSyncToRetrace(true);
}

#ifdef __HAIKU__

// constructor
CanvasView::CanvasView(Document* document, RenderManager* manager)
	:
	BackBufferedStateView("canvas view", B_WILL_DRAW | B_FRAME_EVENTS),
	fDocument(document),
	fRenderManager(manager),

	fZoomLevel(1.0),

	fSpaceHeldDown(false),
	fScrollTracking(false),
	fInScrollTo(false),
	fScrollTrackingStart(0.0, 0.0),
	fScrollOffsetStart(0.0, 0.0),

	fAutoScroller(NULL)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	SetHighColor(kStripeLight);
	SetLowColor(kStripeDark);
		// used for drawing the stripes pattern
	SetSyncToRetrace(true);
}

#endif // __HAIKU__

// destructor
CanvasView::~CanvasView()
{
	delete fAutoScroller;
}

// MessageReceived
void
CanvasView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_AUTO_SCROLL:
			if (fAutoScroller) {
				BPoint scrollOffset(0.0, 0.0);
				BRect bounds(Bounds());
				BPoint mousePos = MouseInfo()->position;
				mousePos.ConstrainTo(bounds);
				float inset = min_c(min_c(40.0, bounds.Width() / 10),
					min_c(40.0, bounds.Height() / 10));
				bounds.InsetBy(inset, inset);
				if (!bounds.Contains(mousePos)) {
					// mouse is close to the border
					if (mousePos.x <= bounds.left)
						scrollOffset.x = mousePos.x - bounds.left;
					else if (mousePos.x >= bounds.right)
						scrollOffset.x = mousePos.x - bounds.right;
					if (mousePos.y <= bounds.top)
						scrollOffset.y = mousePos.y - bounds.top;
					else if (mousePos.y >= bounds.bottom)
						scrollOffset.y = mousePos.y - bounds.bottom;

					scrollOffset.x = roundf(scrollOffset.x * 0.8);
					scrollOffset.y = roundf(scrollOffset.y * 0.8);
				}
				if (scrollOffset != B_ORIGIN) {
					SetScrollOffset(ScrollOffset() + scrollOffset);
				}
			}
			break;
		case MSG_BITMAP_CLEAN: {
			BRect area;
			if (message->FindRect("area", &area) == B_OK) {
				ConvertFromCanvas(&area);
				area.left = floorf(area.left);
				area.top = floorf(area.top);
				area.right = ceilf(area.right);
				area.bottom = ceilf(area.bottom);
				Invalidate(area);
			}
			break;
		}
		default:
			BackBufferedStateView::MessageReceived(message);
			break;
	}
}

// AttachedToWindow
void
CanvasView::AttachedToWindow()
{
	BackBufferedStateView::AttachedToWindow();

	fRenderManager->SetBitmapListener(new BMessenger(this));

	// init data rect for scrolling and center bitmap in the view
	BRect dataRect = _LayoutCanvas();
	SetDataRect(dataRect);
	BRect bounds(Bounds());
	BPoint dataRectCenter((dataRect.left + dataRect.right) / 2,
		(dataRect.top + dataRect.bottom) / 2);
	BPoint boundsCenter((bounds.left + bounds.right) / 2,
		(bounds.top + bounds.bottom) / 2);
	BPoint offset = ScrollOffset();
	offset.x = roundf(offset.x + dataRectCenter.x - boundsCenter.x);
	offset.y = roundf(offset.y + dataRectCenter.y - boundsCenter.y);
	SetScrollOffset(offset);
}

// FrameResized
void
CanvasView::FrameResized(float width, float height)
{
	BackBufferedStateView::FrameResized(width, height);
}

void
CanvasView::GetPreferredSize(float* _width, float* _height)
{
	if (_width != NULL)
		*_width = 100;
	if (_height != NULL)
		*_height = 100;
}

#ifdef __HAIKU__

BSize
CanvasView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}

#endif // __HAIKU__


// DrawInto
void
CanvasView::DrawInto(BView* view, BRect updateRect)
{
	BRect canvas(_CanvasRect());

	// draw document bitmap
	if (fRenderManager->LockDisplay()) {

		const BBitmap* bitmap = fRenderManager->DisplayBitmap();
		view->DrawBitmap(bitmap, bitmap->Bounds(), canvas);

		fRenderManager->UnlockDisplay();
	} else {
		view->FillRect(canvas);
	}

	// outside canvas
	BRegion outside(Bounds() & updateRect);
	outside.Exclude(canvas);
	view->FillRegion(&outside, kStripes);

	BackBufferedStateView::DrawInto(view, updateRect);
}

// #pragma mark -

// MouseDown
void
CanvasView::MouseDown(BPoint where)
{
	if (!IsFocus())
		MakeFocus(true);

	uint32 buttons;
	if (Window()->CurrentMessage()->FindInt32("buttons",
		(int32*)&buttons) < B_OK)
		buttons = 0;

	// handle clicks of the third mouse button ourselves (panning),
	// otherwise have BackBufferedStateView handle it (normal clicks)
	if (fSpaceHeldDown || buttons & B_TERTIARY_MOUSE_BUTTON) {
		// switch into scrolling mode and update cursor
		fScrollTracking = true;
		where.x = roundf(where.x);
		where.y = roundf(where.y);
		fScrollOffsetStart = ScrollOffset();
		fScrollTrackingStart = where - fScrollOffsetStart;
		_UpdateToolCursor();
		SetMouseEventMask(B_POINTER_EVENTS,
						  B_LOCK_WINDOW_FOCUS | B_SUSPEND_VIEW_FOCUS);
	} else {
		SetAutoScrolling(true);
		BackBufferedStateView::MouseDown(where);
	}
}

// MouseUp
void
CanvasView::MouseUp(BPoint where)
{
	if (fScrollTracking) {
		// stop scroll tracking and update cursor
		fScrollTracking = false;
		_UpdateToolCursor();
		// update BackBufferedStateView mouse position
		uint32 transit = Bounds().Contains(where) ?
			B_INSIDE_VIEW : B_OUTSIDE_VIEW;
		BackBufferedStateView::MouseMoved(where, transit, NULL);
	} else {
		BackBufferedStateView::MouseUp(where);
	}
	SetAutoScrolling(false);
}

// MouseMoved
void
CanvasView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (fScrollTracking) {
		uint32 buttons;
		GetMouse(&where, &buttons, false);
		if (!buttons) {
			MouseUp(where);
			return;
		}
		where.x = roundf(where.x);
		where.y = roundf(where.y);
		where -= ScrollOffset();
		BPoint offset = where - fScrollTrackingStart;
		SetScrollOffset(fScrollOffsetStart - offset);
	} else {
		// normal mouse movement handled by BackBufferedStateView
//		if (!fSpaceHeldDown)
			BackBufferedStateView::MouseMoved(where, transit, dragMessage);
	}
	_UpdateToolCursor();
}

// #pragma mark -

// MouseWheelChanged
bool
CanvasView::MouseWheelChanged(BPoint where, float x, float y)
{
	if (!Bounds().Contains(where))
		return false;

	if (y > 0.0) {
		SetZoomLevel(NextZoomOutLevel(fZoomLevel), true);
		return true;
	} else if (y < 0.0) {
		SetZoomLevel(NextZoomInLevel(fZoomLevel), true);
		return true;
	}
	return false;
}

// ViewStateBoundsChanged
void
CanvasView::ViewStateBoundsChanged()
{
	if (!Window())
		return;
//	if (fScrollTracking)
//		return;

//	fScrollTracking = true;
	SetDataRect(_LayoutCanvas());
//	fScrollTracking = false;
}

// #pragma mark -

// ConvertFromCanvas
void
CanvasView::ConvertFromCanvas(BPoint* point) const
{
	point->x *= fZoomLevel;
	point->y *= fZoomLevel;
}

// ConvertToCanvas
void
CanvasView::ConvertToCanvas(BPoint* point) const
{
	point->x /= fZoomLevel;
	point->y /= fZoomLevel;
}

// ConvertFromCanvas
void
CanvasView::ConvertFromCanvas(BRect* r) const
{
	r->left *= fZoomLevel;
	r->top *= fZoomLevel;
	r->right++;
	r->bottom++;
	r->right *= fZoomLevel;
	r->bottom *= fZoomLevel;
	r->right--;
	r->bottom--;
}

// ConvertToCanvas
void
CanvasView::ConvertToCanvas(BRect* r) const
{
	r->left /= fZoomLevel;
	r->right /= fZoomLevel;
	r->top /= fZoomLevel;
	r->bottom /= fZoomLevel;
}

// ZoomLevel
float
CanvasView::ZoomLevel() const
{
	return fZoomLevel;
}

// #pragma mark -

// SetScrollOffset
void
CanvasView::SetScrollOffset(BPoint newOffset)
{
	if (fInScrollTo)
		return;

	fInScrollTo = true;

	newOffset = ValidScrollOffsetFor(newOffset);
	if (!fScrollTracking) {
		BPoint mouseOffset = newOffset - ScrollOffset();
		MouseMoved(fMouseInfo.position + mouseOffset, fMouseInfo.transit, NULL);
	}

	Scrollable::SetScrollOffset(newOffset);

	fInScrollTo = false;
}

// ScrollOffsetChanged
void
CanvasView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	BPoint offset = newOffset - oldOffset;

	if (offset == B_ORIGIN)
		// prevent circular code (MouseMoved might call ScrollBy...)
		return;

	ScrollBy(offset.x, offset.y);
}

// VisibleSizeChanged
void
CanvasView::VisibleSizeChanged(float oldWidth, float oldHeight,
							   float newWidth, float newHeight)
{
	SetDataRect(_LayoutCanvas());
}

// #pragma mark -

// NextZoomInLevel
double
CanvasView::NextZoomInLevel(double zoom) const
{
	if (zoom < 0.25)
		return 0.25;
	if (zoom < 0.33)
		return 0.33;
	if (zoom < 0.5)
		return 0.5;
	if (zoom < 0.66)
		return 0.66;
	if (zoom < 1)
		return 1;
	if (zoom < 1.5)
		return 1.5;
	if (zoom < 2)
		return 2;
	if (zoom < 3)
		return 3;
	if (zoom < 4)
		return 4;
	if (zoom < 6)
		return 6;
	if (zoom < 8)
		return 8;
	if (zoom < 16)
		return 16;
	if (zoom < 32)
		return 32;
	return 64;
}

// NextZoomOutLevel
double
CanvasView::NextZoomOutLevel(double zoom) const
{
	if (zoom > 32)
		return 32;
	if (zoom > 16)
		return 16;
	if (zoom > 8)
		return 8;
	if (zoom > 6)
		return 6;
	if (zoom > 4)
		return 4;
	if (zoom > 3)
		return 3;
	if (zoom > 2)
		return 2;
	if (zoom > 1.5)
		return 1.5;
	if (zoom > 1.0)
		return 1.0;
	if (zoom > 0.66)
		return 0.66;
	if (zoom > 0.5)
		return 0.5;
	if (zoom > 0.33)
		return 0.33;
	return 0.25;
}

// SetZoomLevel
void
CanvasView::SetZoomLevel(double zoomLevel, bool mouseIsAnchor)
{
	if (fZoomLevel == zoomLevel)
		return;

	BPoint anchor;
	if (mouseIsAnchor) {
		// zoom into mouse position
		anchor = MouseInfo()->position;
	} else {
		// zoom into center of view
		BRect bounds(Bounds());
		anchor.x = (bounds.left + bounds.right + 1) / 2.0;
		anchor.y = (bounds.top + bounds.bottom + 1) / 2.0;
	}

	BPoint canvasAnchor = anchor;
	ConvertToCanvas(&canvasAnchor);

	fZoomLevel = zoomLevel;
	BRect dataRect = _LayoutCanvas();

	ConvertFromCanvas(&canvasAnchor);

	BPoint offset = ScrollOffset();
	offset.x = roundf(offset.x + canvasAnchor.x - anchor.x);
	offset.y = roundf(offset.y + canvasAnchor.y - anchor.y);

	Invalidate();
		// Cause the (Haiku) app_server to skip visual scrolling
	SetDataRectAndScrollOffset(dataRect, offset);
}

// SetAutoScrolling
void
CanvasView::SetAutoScrolling(bool scroll)
{
	if (scroll) {
		if (!fAutoScroller) {
			BMessenger messenger(this, Window());
			BMessage message(MSG_AUTO_SCROLL);
			// this trick avoids the MouseMoved() hook
			// to think that the mouse is not pressed
			// anymore when we call it ourselfs from the
			// autoscrolling code
			message.AddInt32("buttons", 1);
			fAutoScroller = new BMessageRunner(messenger,
											   &message,
											   AUTO_SCROLL_DELAY);
		}
	} else {
		delete fAutoScroller;
		fAutoScroller = NULL;
	}
}

// #pragma mark -

// _HandleKeyDown
bool
CanvasView::_HandleKeyDown(const StateView::KeyEvent& event,
	BHandler* originalHandler)
{
	switch (event.key) {
		case 'z':
		case 'y':
//			if (modifiers & B_SHIFT_KEY)
//				CommandStack()->Redo();
//			else
//				CommandStack()->Undo();
			break;

		case '+':
			SetZoomLevel(NextZoomInLevel(fZoomLevel));
			break;
		case '-':
			SetZoomLevel(NextZoomOutLevel(fZoomLevel));
			break;

		case B_SPACE:
			fSpaceHeldDown = true;
			_UpdateToolCursor();
			break;

		default:
			return BackBufferedStateView::_HandleKeyDown(event,
				originalHandler);
	}

	return true;
}

// _HandleKeyUp
bool
CanvasView::_HandleKeyUp(const StateView::KeyEvent& event,
	BHandler* originalHandler)
{
	switch (event.key) {
		case B_SPACE:
			fSpaceHeldDown = false;
			_UpdateToolCursor();
			break;

		default:
			return BackBufferedStateView::_HandleKeyUp(event,
				originalHandler);
	}

	return true;
}

// #pragma mark -

// _CanvasRect()
BRect
CanvasView::_CanvasRect() const
{
	BRect r(fDocument->Bounds());
	ConvertFromCanvas(&r);
	return r;
}

// _LayoutCanvas
BRect
CanvasView::_LayoutCanvas()
{
	// size of zoomed bitmap
	BRect r(_CanvasRect());
	r.OffsetTo(B_ORIGIN);

	// ask current view state to extend size
	BRect stateBounds = ViewStateBounds();

	// resize for empty area around bitmap
	// (the size we want, but might still be much smaller than view)
	r.InsetBy(-50, -50);

	// center data rect in bounds
	BRect bounds(Bounds());
	if (bounds.Width() > r.Width())
		r.InsetBy(-ceilf((bounds.Width() - r.Width()) / 2), 0);
	if (bounds.Height() > r.Height())
		r.InsetBy(0, -ceilf((bounds.Height() - r.Height()) / 2));

	if (stateBounds.IsValid()) {
		stateBounds.InsetBy(-20, -20);
		r = r | stateBounds;
	}

	return r;
}

// #pragma mark -

// _UpdateToolCursor
void
CanvasView::_UpdateToolCursor()
{
	if (fScrollTracking || fSpaceHeldDown) {
		// indicate scrolling mode
		const uchar* cursorData = fScrollTracking ? kGrabCursor : kHandCursor;
		BCursor cursor(cursorData);
		SetViewCursor(&cursor, true);
	} else {
		// pass on to current state of BackBufferedStateView
		UpdateStateCursor();
	}
}
