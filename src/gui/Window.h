/*
 * Copyright 2007-2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <Window.h>

#include "LayerObserver.h"
#include "ListenerAdapter.h"
#include "Selection.h"

class BMenu;
class CanvasView;
class ColumnTreeModel;
class Document;
class IconOptionsControl;
class InspectorView;
class Layer;
class ObjectColumnTreeItem;
class ObjectTreeView;
class RenderManager;
class Tool;

class Window : public BWindow {
public:
								Window(BRect frame, const char* title,
									Document* document, Layer* layer);
	virtual						~Window();

	// BWindow interface
	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);

	// Window
			void				SetDocument(Document* document);

			void				AddTool(Tool* tool);

private:
			void				_InitTools();

			void				_ObjectChanged(const Notifier* object);

			CanvasView*			fView;
			Document*			fDocument;
			LayerObserver		fLayerObserver;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;
			Selection			fSelection;

			BMenu*				fFileMenu;
			BMenu*				fEditMenu;
			BMenu*				fObjectMenu;
			BMenu*				fPropertyMenu;

			BMenuItem*			fUndoMI;
			BMenuItem*			fRedoMI;

			IconOptionsControl*	fToolIconControl;
			ObjectTreeView*		fLayerTreeView;
//			ColumnTreeModel*	fLayerTreeModel;
			InspectorView*		fInspectorView;

			BList				fTools;
};

#endif // WINDOW_H
