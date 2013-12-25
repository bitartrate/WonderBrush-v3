/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <List.h>
#include <Rect.h>

#include "BaseObject.h"
#include "NotifyingList.h"
#include "RWLocker.h"

class BaseObject;
class EditManager;
class Layer;
class Path;
class Style;

typedef NotifyingList<BaseObject> ResourceList;

class Document : public BaseObject, public RWLocker {
public:
	class Listener {
	 public:
								Listener();
		virtual					~Listener();
	};


								Document(const BRect& bounds);
	virtual						~Document();

	virtual	BaseObject*			Clone(ResourceResolver& resolver) const;

	// BaseObject interface
	virtual	const char*			DefaultName() const;

	// Document
	inline	::EditManager*		EditManager() const
									{ return fEditManager; }

			status_t			InitCheck() const;

			BRect				Bounds() const;

			bool				AddListener(Listener* listener);
			void				RemoveListener(Listener* listener);

	inline	Layer*				RootLayer() const
									{ return fRootLayer; }
			bool				HasLayer(Layer* layer) const;

	inline	ResourceList&		GlobalResources()
									{ return fGlobalResources; }
	inline	const ResourceList&	GlobalResources() const
									{ return fGlobalResources; }

			void				PrintToStream();

private:
			bool				_HasLayer(Layer* parent, Layer* child) const;

private:
			::EditManager*		fEditManager;
			Layer*				fRootLayer;

			ResourceList		fGlobalResources;

			BList				fListeners;
};

typedef Reference<Document> DocumentRef;

#endif // DOCUMENT_H
