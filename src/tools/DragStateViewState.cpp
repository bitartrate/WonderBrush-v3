/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "DragStateViewState.h"

#include <Cursor.h>

#include "UndoableEdit.h"

// constructor
DragStateViewState::DragState::DragState(DragStateViewState* parent)
	:
	fOrigin(0.0, 0.0),
	fParent(parent)
{
}

// destructor
DragStateViewState::DragState::~DragState()
{
}

// SetOrigin
void
DragStateViewState::DragState::SetOrigin(BPoint origin)
{
	fOrigin = origin;
}

// #pragma mark -

// constructor
DragStateViewState::DragStateViewState(StateView* view)
	:
	TransformViewState(view),
	fCurrentState(NULL),
	fDragging(false),
	fCurrentCommand(NULL)
{
}

// destructor
DragStateViewState::~DragStateViewState()
{
}

// MouseDown
void
DragStateViewState::MouseDown(const MouseInfo& info)
{
	BPoint where = info.position;
	TransformViewToCanvas(&where);

	fDragging = true;
	if (fCurrentState != NULL) {
		fCurrentState->SetOrigin(where);

		delete fCurrentCommand;
		fCurrentCommand = StartTransaction(fCurrentState->CommandName());
	}

	fView->SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
}

// MouseMoved
void
DragStateViewState::MouseMoved(const MouseInfo& info)
{
	BPoint where = info.position;
	TransformViewToCanvas(&where);

	if (!fDragging) {
		SetDragState(DragStateFor(where, ZoomLevel()));
	} else {
		if (fCurrentState != NULL) {
			fCurrentState->DragTo(where, Modifiers());
			UpdateCursor();
		}
	}
}

// MouseUp
UndoableEdit*
DragStateViewState::MouseUp()
{
	fDragging = false;
	UndoableEdit* edit = FinishTransaction(fCurrentCommand);
	UpdateDragState();
	return edit;
}

// UpdateCursor
bool
DragStateViewState::UpdateCursor()
{
	if (fCurrentState == NULL)
		return false;

	BPoint where = MousePos();
	TransformViewToCanvas(&where);

	BCursor cursor = fCurrentState->ViewCursor(where);
	fView->SetViewCursor(&cursor);
	return true;
}

// #pragma mark -

// StartTransaction
UndoableEdit*
DragStateViewState::StartTransaction(const char* commandName)
{
	return NULL;
}

// FinishTransaction
UndoableEdit*
DragStateViewState::FinishTransaction(UndoableEdit* edit)
{
	edit = fCurrentCommand;
	fCurrentCommand = NULL;
	return edit;
}

// SetDragState
void
DragStateViewState::SetDragState(DragState* state)
{
	if (fCurrentState != state) {
		fCurrentState = state;
		UpdateCursor();
	}
}

// UpdateDragState
void
DragStateViewState::UpdateDragState()
{
	if (fMouseInfo != NULL)
		MouseMoved(*fMouseInfo);
}

