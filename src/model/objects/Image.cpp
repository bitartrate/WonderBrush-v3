/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Image.h"

#include "ImageSnapshot.h"
#include "RenderBuffer.h"
#include "RenderEngine.h"

// constructor
ImageListener::ImageListener()
{
}

// destructor
ImageListener::~ImageListener()
{
}

// Deleted
void
ImageListener::Deleted(Image* rect)
{
}

// #pragma mark -

// constructor
Image::Image(RenderBuffer* buffer)
	: BoundedObject()
	, fBuffer(buffer)
	, fListeners(4)
{
	if (fBuffer != NULL)
		fBuffer->AddReference();
}

// destructor
Image::~Image()
{
	_NotifyDeleted();
	if (fBuffer != NULL)
		fBuffer->RemoveReference();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Image::Snapshot() const
{
	return new ImageSnapshot(this);
}

// DefaultName
const char*
Image::DefaultName() const
{
	return "Image";
}

// HitTest
bool
Image::HitTest(const BPoint& canvasPoint) const
{
	RenderEngine engine(Transformation());
	return engine.HitTest(fBuffer->Bounds(), canvasPoint);
}

// #pragma mark -

// Bounds
BRect
Image::Bounds()
{
	if (fBuffer != NULL)
		return fBuffer->Bounds();
	return BRect();
}

// #pragma mark -

// AddListener
bool
Image::AddListener(ImageListener* listener)
{
	if (!listener || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Image::RemoveListener(ImageListener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// _NotifyDeleted
void
Image::_NotifyDeleted()
{
	int32 count = fListeners.CountItems();
	if (count == 0)
		return;

	BList listeners(fListeners);
	for (int32 i = 0; i < count; i++) {
		ImageListener* listener
			= (ImageListener*)listeners.ItemAtFast(i);
		listener->Deleted(this);
	}
}
