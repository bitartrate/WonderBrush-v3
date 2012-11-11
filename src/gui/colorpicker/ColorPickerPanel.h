/*
 * Copyright 2002-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#ifndef COLOR_PICKER_PANEL_H
#define COLOR_PICKER_PANEL_H

#include "Panel.h"

#include "SelectedColorMode.h"

class ColorPickerView;

class ColorPickerPanel : public Panel {
private:
	enum {
		MSG_CANCEL					= 'cncl',
		MSG_DONE					= 'done'
	};

public:
								ColorPickerPanel(BRect frame,
												 rgb_color color,
												 SelectedColorMode mode
												 	= H_SELECTED,
												 BWindow* window = NULL,
												 BMessage* message = NULL,
												 BHandler* target = NULL);
	virtual						~ColorPickerPanel();

	// Panel interface
	virtual	void				Cancel();

	virtual	void				MessageReceived(BMessage* message);

	// ColorPickerPanel
			void				SetColor(rgb_color color);

			void				SetMessage(BMessage* message);
			void				SetTarget(BHandler* target);
			const BHandler*		Target() const
									{ return fTarget; }

	static	ColorPickerPanel*	DefaultPanel();

private:
			class PlatformDelegate;
			friend class PlatformDelegate;

private:
	static	ColorPickerPanel*	sDefaultPanel;

			PlatformDelegate*	fPlatformDelegate;

			ColorPickerView*	fColorPickerView;
			BWindow*			fWindow;
			BMessage*			fMessage;
			BHandler*			fTarget;
};

#endif // COLOR_PICKER_PANEL_H
