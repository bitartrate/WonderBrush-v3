/*
 * Copyright 2007-2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <Window.h>

#include "ListenerAdapter.h"
#include "Selection.h"

class BCardLayout;
class BMenu;
class BSplitView;
class CanvasView;
class ColumnTreeModel;
class Document;
class IconButton;
class IconOptionsControl;
class InspectorView;
class Layer;
class ObjectColumnTreeItem;
class ObjectTreeView;
class RenderManager;
class ResourceTreeView;
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

			status_t			StoreSettings(BMessage& settings) const;
			void				RestoreSettings(const BMessage& settings);

private:
			void				_InitTools();

			void				_ObjectChanged(const Notifier* object);

private:
			CanvasView*			fView;
			Document*			fDocument;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;
			Selection			fSelection;

			BMenu*				fFileMenu;
			BMenu*				fEditMenu;
			BMenu*				fObjectMenu;
			BMenu*				fResourceMenu;
			BMenu*				fPropertyMenu;

			BMenuItem*			fUndoMI;
			BMenuItem*			fRedoMI;

			IconOptionsControl*	fToolIconControl;
			BCardLayout*		fToolConfigLayout;
			ObjectTreeView*		fLayerTreeView;
//			ColumnTreeModel*	fLayerTreeModel;
			ResourceTreeView*	fResourceTreeView;
			InspectorView*		fInspectorView;

			BSplitView*			fHorizontalSplit;
			BSplitView*			fVerticalSplit;

			IconButton*			fUndoIcon;
			IconButton*			fRedoIcon;
			IconButton*			fConfirmIcon;
			IconButton*			fCancelIcon;

			BList				fTools;
};

#endif // WINDOW_H