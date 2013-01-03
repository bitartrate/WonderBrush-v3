/*
 * Copyright 2012-2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "TextToolConfigView.h"

#include <stdio.h>

#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <Slider.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

#include "FontPopup.h"
#include "FontRegistry.h"
#include "IconButton.h"
#include "IconOptionsControl.h"
#include "Text.h"
#include "TextTool.h"
#include "TextToolState.h"

enum {
	MSG_FONT_SELECTED		= 'fnsl',
	MSG_SIZE_SLIDER			= 'szsl',
	MSG_SIZE_TEXT			= 'sztx',
	MSG_SUBPIXELS			= 'sbpx',

	MSG_INSERT				= 'isrt',
	MSG_REMOVE				= 'rmov',

	MSG_SELECTION_CHANGED	= 'slch',
	MSG_SHOW_TEXT_OFFSET	= 'shwo',

	MSG_SET_TEXT_ALIGNMENT	= 'stta',
};

// NotifyingTextView
class NotifyingTextView : public BTextView {
public:
	NotifyingTextView(const char* name)
		: BTextView(name)
		, fNotificationsEnabled(true)
	{
	}

	void SetTarget(BHandler* target)
	{
		fMessenger = BMessenger(target);
	}

	virtual void Select(int32 startOffset, int32 endOffset)
	{
		BTextView::Select(startOffset, endOffset);
		_SelectionChanged(startOffset, endOffset);
	}

	virtual void ScrollToOffset(int32 inOffset)
	{
		BTextView::ScrollToOffset(inOffset);
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_SHOW_TEXT_OFFSET);
		message.AddInt32("offset", inOffset);
		fMessenger.SendMessage(&message);
	}

	void SetNotificationsEnabled(bool enabled)
	{
		fNotificationsEnabled = enabled;
	}

protected:
	void _UpdateSelection()
	{
		int32 startOffset;
		int32 endOffset;
		GetSelection(&startOffset, &endOffset);
		_SelectionChanged(startOffset, endOffset);
	}

	void _SelectionChanged(int startOffset, int endOffset)
	{
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_SELECTION_CHANGED);
		message.AddInt32("start offset", startOffset);
		message.AddInt32("end offset", endOffset);
		fMessenger.SendMessage(&message);
	}

	virtual void InsertText(const char* inText, int32 inLength, int32 inOffset,
		const text_run_array* inRuns)
	{
		BTextView::InsertText(inText, inLength, inOffset, inRuns);
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_INSERT);
		message.AddString("text", inText);
		message.AddInt32("offset", inOffset);

		fMessenger.SendMessage(&message);

		_UpdateSelection();
	}

	virtual void DeleteText(int32 fromOffset, int32 toOffset)
	{
		BTextView::DeleteText(fromOffset, toOffset);
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_REMOVE);
		message.AddInt32("offset", fromOffset);
		message.AddInt32("length", toOffset - fromOffset);

		fMessenger.SendMessage(&message);

		_UpdateSelection();
	}

private:
	BMessenger	fMessenger;
	bool		fNotificationsEnabled;
};

// #pragma mark - TextToolConfigView

// constructor
TextToolConfigView::TextToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fFontPopup = new FontPopup("Font", true);

	fSizeLabel = new BStringView("size label", "Size");

	fSizeSlider = new BSlider("size slider", NULL,
		new BMessage(MSG_SIZE_SLIDER),
		1, 1024, B_HORIZONTAL, B_TRIANGLE_THUMB);
	fSizeSlider->SetExplicitMinSize(BSize(80, B_SIZE_UNSET));
	fSizeSlider->SetModificationMessage(new BMessage(MSG_SIZE_SLIDER));

	fSizeTextControl = new BTextControl("size text y", "", "",
		new BMessage(MSG_SIZE_TEXT));

	fTextAlignmentControl = new IconOptionsControl("text alignment");

	// text align left
	IconButton* iconButton = new IconButton("text align left", 0);
	iconButton->SetIcon(601, 16);
	BMessage* alignmentMessage = new BMessage(MSG_SET_TEXT_ALIGNMENT);
	alignmentMessage->AddInt32("alignment", TEXT_ALIGNMENT_LEFT);
	iconButton->SetMessage(alignmentMessage);
	fTextAlignmentControl->AddOption(iconButton);

	// text align center
	iconButton = new IconButton("text align center", 1);
	iconButton->SetIcon(602, 16);
	alignmentMessage = new BMessage(MSG_SET_TEXT_ALIGNMENT);
	alignmentMessage->AddInt32("alignment", TEXT_ALIGNMENT_CENTER);
	iconButton->SetMessage(alignmentMessage);
	fTextAlignmentControl->AddOption(iconButton);

	// text align right
	iconButton = new IconButton("text align right", 2);
	iconButton->SetIcon(603, 16);
	alignmentMessage = new BMessage(MSG_SET_TEXT_ALIGNMENT);
	alignmentMessage->AddInt32("alignment", TEXT_ALIGNMENT_RIGHT);
	iconButton->SetMessage(alignmentMessage);
	fTextAlignmentControl->AddOption(iconButton);

	// text align justify
	iconButton = new IconButton("text align justify", 3);
	iconButton->SetIcon(604, 16);
	alignmentMessage = new BMessage(MSG_SET_TEXT_ALIGNMENT);
	alignmentMessage->AddInt32("alignment", TEXT_ALIGNMENT_JUSTIFY);
	iconButton->SetMessage(alignmentMessage);
	fTextAlignmentControl->AddOption(iconButton);

	fSubpixels = new BCheckBox("subpixels", "Subpixels",
		new BMessage(MSG_SUBPIXELS));

	fTextView = new NotifyingTextView("text view");

	BScrollView* scrollView = new BScrollView("text scroll view",
		fTextView, 0, false, true);

	BGroupLayoutBuilder(layout)
		.AddGroup(B_VERTICAL, 3.0f)
			.AddGroup(B_HORIZONTAL, 0.0f)
				.Add(fFontPopup->CreateLabelLayoutItem())
				.Add(fFontPopup->CreateMenuBarLayoutItem())
			.End()
			.AddGroup(B_HORIZONTAL, 0.0f)
				.Add(fSizeLabel)
				.Add(fSizeSlider, 0.8f)
				.Add(fSizeTextControl, 0.2f)
			.End()
		.End()
		.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
		.AddGroup(B_VERTICAL, 3.0f)
			.Add(fSubpixels)
			.Add(fTextAlignmentControl)
		.End()
		.Add(scrollView)
//		.AddGlue()
		.SetInsets(5, 5, 5, 5)
	;
}

// destructor
TextToolConfigView::~TextToolConfigView()
{
}

// #pragma mark -

// AttachedToWindow
void
TextToolConfigView::AttachedToWindow()
{
	fSizeSlider->SetTarget(this);
	fSizeTextControl->SetTarget(this);
	fSubpixels->SetTarget(this);
	fTextView->SetTarget(this);
	fTextAlignmentControl->SetTarget(this);

	FontRegistry* fontRegistry = FontRegistry::Default();
	if (fontRegistry->LockWithTimeout(3000) == B_OK) {
		_PopulateFontMenu(fFontPopup->Menu(), this, NULL, NULL);
		fontRegistry->Unlock();
	}
	fontRegistry->StartWatchingAll(this);
}

// DetachedFromWindow
void
TextToolConfigView::DetachedFromWindow()
{
	FontRegistry* fontRegistry = FontRegistry::Default();
	fontRegistry->StopWatchingAll(this);
}

// MessageReceived
void
TextToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
			_PopulateFontMenu(fFontPopup->Menu(), this, NULL, NULL);
			break;

		case MSG_FONT_SELECTED:
		{
			fFontPopup->RefreshItemLabel();

			const char* family;
			if (message->FindString("font family", &family) != B_OK)
				break;

			const char* style;
			if (message->FindString("font style", &style) != B_OK)
				break;

			((TextTool*)fTool)->SetFont(family, style);
			break;
		}

		case MSG_SIZE_SLIDER:
			fTool->SetOption(TextTool::SIZE,
				(float)_FromLinearSize(fSizeSlider->Value()));
			break;

		case MSG_SIZE_TEXT:
			fTool->SetOption(TextTool::SIZE, _Value(fSizeTextControl));
			break;

		case MSG_SUBPIXELS:
			fTool->SetOption(TextTool::SUBPIXELS,
				fSubpixels->Value() == B_CONTROL_ON);
			break;

		case MSG_INSERT:
		{
			const char* text;
			if (message->FindString("text", &text) != B_OK)
				break;

			int32 textOffset;
			if (message->FindInt32("offset", &textOffset) != B_OK)
				break;

			((TextTool*)fTool)->Insert(textOffset, text);
			break;
		}

		case MSG_REMOVE:
		{
			int32 textOffset;
			if (message->FindInt32("offset", &textOffset) != B_OK)
				break;

			int32 length;
			if (message->FindInt32("length", &length) != B_OK)
				break;

			((TextTool*)fTool)->Remove(textOffset, length);
			break;
		}

		case MSG_SELECTION_CHANGED:
		{
			int32 startOffset;
			int32 endOffset;
			if (message->FindInt32("start offset", &startOffset) == B_OK
				&& message->FindInt32("end offset", &endOffset) == B_OK) {
				dynamic_cast<TextToolState*>(fTool->ToolViewState())
					->SelectionChanged(startOffset, endOffset);
			}
			break;
		}

		case MSG_SHOW_TEXT_OFFSET:
		{
			// TODO: Make the ViewState scroll the canvas to show the
			// text offset.
			break;
		}

		case MSG_LAYOUT_CHANGED:
		{
			float size;
			if (message->FindFloat("size", &size) == B_OK) {
				fSizeSlider->SetValue(_ToLinearSize(size));
				_SetValue(fSizeTextControl, size);
			}

			const char* text;
			if (message->FindString("text", &text) == B_OK) {
				fTextView->SetNotificationsEnabled(false);
				fTextView->SetText(text);
				fTextView->SetNotificationsEnabled(true);
			}

			const char* family;
			const char* style;
			if (message->FindString("family", &family) == B_OK
				&& message->FindString("style", &style) == B_OK) {
				fFontPopup->SetFamilyAndStyle(family, style);
			}

			int32 alignment;
			if (message->FindInt32("alignment", &alignment) == B_OK) {
				switch (alignment) {
					default:
					case TEXT_ALIGNMENT_LEFT:
						fTextAlignmentControl->SetValue(0);
						break;
					case TEXT_ALIGNMENT_CENTER:
						fTextAlignmentControl->SetValue(1);
						break;
					case TEXT_ALIGNMENT_RIGHT:
						fTextAlignmentControl->SetValue(2);
						break;
					case TEXT_ALIGNMENT_JUSTIFY:
						fTextAlignmentControl->SetValue(3);
						break;
				}
			}
			break;
		}

		case MSG_SET_SELECTION:
		{
			int32 selectionStart;
			int32 selectionEnd;
			if (message->FindInt32("selection start", &selectionStart) == B_OK
				&& message->FindInt32("selection end", &selectionEnd)
					== B_OK) {
				fTextView->SetNotificationsEnabled(false);
				fTextView->Select(selectionStart, selectionEnd);
				fTextView->SetNotificationsEnabled(true);
			}
			break;
		}

		case MSG_SET_TEXT_ALIGNMENT:
		{
			int32 alignment;
			if (message->FindInt32("alignment", &alignment) == B_OK)
				((TextTool*)fTool)->SetAlignment(alignment);
			break;
		}

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// UpdateStrings
void
TextToolConfigView::UpdateStrings()
{
}

// SetActive
void
TextToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
TextToolConfigView::SetEnabled(bool enable)
{
	fSizeSlider->SetEnabled(enable);
	fSizeTextControl->SetEnabled(enable);
}

// #pragma mark - private

// _SetText
void
TextToolConfigView::_SetValue(BTextControl* control, float value) const
{
	char text[64];
	snprintf(text, sizeof(text), "%.2f", value);
	int32 selectionStart;
	int32 selectionEnd;
	control->TextView()->GetSelection(&selectionStart, &selectionEnd);
	bool selectionEndIsTextEnd
		= selectionEnd == control->TextView()->TextLength();

	control->SetText(text);

	if (selectionEndIsTextEnd)
		selectionEnd = control->TextView()->TextLength();
	control->TextView()->Select(selectionStart, selectionEnd);
}

// _Value
float
TextToolConfigView::_Value(BTextControl* control) const
{
	return atof(control->Text());
}

// _PopulateFontMenu
void
TextToolConfigView::_PopulateFontMenu(BMenu* menu, BHandler* target,
	const char* markedFamily, const char* markedStyle)
{
	if (menu == NULL)
		return;

	font_family defaultFamily;
	font_style defaultStyle;
	if (markedFamily == NULL || markedStyle == NULL) {
		be_plain_font->GetFamilyAndStyle(&defaultFamily, &defaultStyle);
		markedFamily = defaultFamily;
		markedStyle = defaultStyle;
	}

	FontRegistry* manager = FontRegistry::Default();

	if (!manager->Lock())
		return;

	while (menu->CountItems() > 0) {
		delete menu->RemoveItem(0L);
	}

	BMenu* fontMenu = NULL;

	font_family family;
	font_style style;

	int32 count = manager->CountFontFiles();
	for (int32 i = 0; i < count; i++) {
		if (!manager->GetFontAt(i, family, style))
			break;

		BMessage* message = new BMessage(MSG_FONT_SELECTED);
		message->AddString("font family", family);
		message->AddString("font style", style);

		FontMenuItem* item = new FontMenuItem(style, family, style, message);
		item->SetTarget(target);

		bool markStyle = false;
		if (fontMenu == NULL
			|| (fontMenu->Name()
				&& strcmp(fontMenu->Name(), family) != 0)) {
			// create new entry
			fontMenu = new BMenu(family);
			fontMenu->AddItem(item);
			menu->AddItem(fontMenu);
			// mark the menu if necessary
			if (markedFamily != NULL && strcmp(markedFamily, family) == 0) {
				if (BMenuItem* superItem = fontMenu->Superitem())
					superItem->SetMarked(true);
				markStyle = true;
			}
		} else {
			// reuse old entry
			fontMenu->AddItem(item);
		}
		// mark the item if necessary
		if (markStyle
			&& markedStyle != NULL && strcmp(markedStyle, style) == 0) {
			item->SetMarked(true);
		}
	}

	fFontPopup->SetFamilyAndStyle(markedFamily, markedStyle);

	manager->Unlock();
}


// _FromLinearSize
double
TextToolConfigView::_FromLinearSize(double value) const
{
	return 1.0 + 1023.0 * pow((value - 1) / 1023.0, 2.0);
}

// _ToLinearSize
double
TextToolConfigView::_ToLinearSize(double value) const
{
	return 1.0 + 1023.0 * sqrt((value - 1) / 1023.0);
}
