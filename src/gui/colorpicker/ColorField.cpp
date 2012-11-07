/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */

#include "ColorField.h"

#include <stdio.h>

#include <Bitmap.h>
#include <LayoutUtils.h>
#include <OS.h>
#include <Region.h>
#include <Window.h>

#include "support_ui.h"

#include "rgb_hsv.h"

#define round(x) (int)(x +.5)

enum {
	MSG_UPDATE			= 'Updt',
};

#define MAX_X 255
#define MAX_Y 255

// constructor
ColorField::ColorField(BPoint offsetPoint, SelectedColorMode mode,
	float fixedValue, orientation orient)
	: BControl(BRect(0.0, 0.0, MAX_X + 4.0, MAX_Y + 4.0)
			.OffsetToCopy(offsetPoint),
		"ColorField", "", new BMessage(MSG_COLOR_FIELD),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS)
{
	_Init(mode, fixedValue, orient);
}

// constructor
ColorField::ColorField(SelectedColorMode mode, float fixedValue,
	orientation orient)
	: BControl("ColorField", "", new BMessage(MSG_COLOR_FIELD),
		B_WILL_DRAW | B_FRAME_EVENTS)
{
	_Init(mode, fixedValue, orient);
}

// destructor
ColorField::~ColorField()
{
	if (fUpdatePort >= B_OK)
		delete_port(fUpdatePort);

	if (fUpdateThread >= B_OK) {
//		status_t exitValue;
//		wait_for_thread(fUpdateThread, &exitValue);
		kill_thread(fUpdateThread);
	}

	delete fBgBitmap[0];
	delete fBgBitmap[1];
}

// MinSize
BSize
ColorField::MinSize()
{
	BSize minSize;
	if (fOrientation == B_VERTICAL)
		minSize = BSize(4 + MAX_X / 17, 4 + MAX_Y / 5);
	else
		minSize = BSize(4 + MAX_X / 5, 4 + MAX_Y / 17);

	return BLayoutUtils::ComposeSize(ExplicitMinSize(), minSize);
}

// PreferredSize
BSize
ColorField::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), MinSize());
}

// MaxSize
BSize
ColorField::MaxSize()
{
	BSize maxSize;
	if (fOrientation == B_VERTICAL)
		maxSize = BSize(4 + MAX_X, 4 + MAX_Y);
	else
		maxSize = BSize(4 + MAX_X, 4 + MAX_Y);
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), maxSize);
//	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
//		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}

// Invoke
status_t
ColorField::Invoke(BMessage* message)
{
	if (message == NULL)
		message = Message();

	if (message == NULL)
		return BControl::Invoke(message);

	message->RemoveName("value");

	float v1 = 0;
	float v2 = 0;

	switch (fMode) {
		case R_SELECTED:
		case G_SELECTED:
		case B_SELECTED:
			v1 = fMarkerPosition.x / Width();
			v2 = 1.0 - fMarkerPosition.y / Height();
			break;

		case H_SELECTED:
			if (fOrientation == B_VERTICAL) {
				v1 = fMarkerPosition.x / Width();
				v2 = 1.0 - fMarkerPosition.y / Height();
			} else {
				v1 = fMarkerPosition.y / Height();
				v2 = 1.0 - fMarkerPosition.x / Width();
			}
			break;

		case S_SELECTED:
		case V_SELECTED:
			v1 = fMarkerPosition.x / Width() * 6.0;
			v2 = 1.0 - fMarkerPosition.y / Height();
			break;

	}

	message->AddFloat("value", v1);
	message->AddFloat("value", v2);

	return BControl::Invoke(message);
}

// AttachedToWindow
void
ColorField::AttachedToWindow()
{
	Update(3);
}

// Draw
void
ColorField::Draw(BRect updateRect)
{
	Update(0);
}

// MouseDown
void
ColorField::MouseDown(BPoint where)
{
	fMouseDown = true;
	SetMouseEventMask(B_POINTER_EVENTS,
		B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
	PositionMarkerAt(where);

	if (Message() != NULL) {
		BMessage message(*Message());
		message.AddBool("begin", true);
		Invoke(&message);
	} else
		Invoke();
}

// MouseUp
void
ColorField::MouseUp(BPoint where)
{
	fMouseDown = false;
}

// MouseMoved
void
ColorField::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	if (dragMessage != NULL || !fMouseDown ) {
		BView::MouseMoved(where, transit, dragMessage);
		return;
	}

	PositionMarkerAt(where);
	Invoke();
}

// Update
void
ColorField::Update(int depth)
{
	// depth:
	// 0 = only onscreen redraw,
	// 1 = only cursor 1,
	// 2 = full update part 2,
	// 3 = full

	if (depth == 3) {
		write_port(fUpdatePort, MSG_UPDATE, NULL, 0);
		return;
	}

	if (depth >= 1) {
		fBgBitmap[1]->Lock();

		fBgView[1]->DrawBitmap( fBgBitmap[0], BPoint(-2.0, -2.0) );

		fBgView[1]->SetHighColor( 0, 0, 0 );
		fBgView[1]->StrokeEllipse( fMarkerPosition, 5.0, 5.0 );
		fBgView[1]->SetHighColor( 255.0, 255.0, 255.0 );
		fBgView[1]->StrokeEllipse( fMarkerPosition, 4.0, 4.0 );

		fBgView[1]->Sync();

		fBgBitmap[1]->Unlock();
	}

	if (depth != 0 && depth != 2 && fMarkerPosition != fLastMarkerPosition) {

		fBgBitmap[1]->Lock();

		DrawBitmap( fBgBitmap[1],
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fMarkerPosition),
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fMarkerPosition));
		DrawBitmap( fBgBitmap[1],
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fLastMarkerPosition),
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fLastMarkerPosition));

		fLastMarkerPosition = fMarkerPosition;

		fBgBitmap[1]->Unlock();

	} else
		DrawBitmap(fBgBitmap[1]);

}

// SetModeAndValue
void
ColorField::SetModeAndValue(SelectedColorMode mode, float fixedValue)
{
	float R(0), G(0), B(0);
	float H(0), S(0), V(0);

	fBgBitmap[0]->Lock();

	float width = Width();
	float height = Height();

	switch (fMode) {

		case R_SELECTED: {
			R = fFixedValue * 255;
			G = round(fMarkerPosition.x / width * 255.0);
			B = round(255.0 - fMarkerPosition.y / height * 255.0);
		}; break;

		case G_SELECTED: {
			R = round(fMarkerPosition.x / width * 255.0);
			G = fFixedValue * 255;
			B = round(255.0 - fMarkerPosition.y / height * 255.0);
		}; break;

		case B_SELECTED: {
			R = round(fMarkerPosition.x / width * 255.0);
			G = round(255.0 - fMarkerPosition.y / height * 255.0);
			B = fFixedValue * 255;
		}; break;

		case H_SELECTED: {
			H = fFixedValue;
			S = fMarkerPosition.x / width;
			V = 1.0 - fMarkerPosition.y / height;
		}; break;

		case S_SELECTED: {
			H = fMarkerPosition.x / width * 6.0;
			S = fFixedValue;
			V = 1.0 - fMarkerPosition.y / height;
		}; break;

		case V_SELECTED: {
			H = fMarkerPosition.x / width * 6.0;
			S = 1.0 - fMarkerPosition.y / height;
			V = fFixedValue;
		}; break;
	}

	if (fMode & (H_SELECTED | S_SELECTED | V_SELECTED)) {
		HSV_to_RGB(H, S, V, R, G, B);
		R *= 255.0; G *= 255.0; B *= 255.0;
	}

	rgb_color color = { round(R), round(G), round(B), 255 };

	fBgBitmap[0]->Unlock();

	if (fFixedValue != fixedValue || fMode != mode) {
		fFixedValue = fixedValue;
		fMode = mode;

		Update(3);
	}

	SetMarkerToColor(color);
}

// SetFixedValue
void
ColorField::SetFixedValue(float fixedValue)
{
	if (fFixedValue != fixedValue) {
		fFixedValue = fixedValue;
		Update(3);
	}
}

// SetMarkerToColor
void
ColorField::SetMarkerToColor(rgb_color color)
{
	float h, s, v;
	RGB_to_HSV(color.red / 255.0, color.green / 255.0, color.blue / 255.0,
		h, s, v );

	fLastMarkerPosition = fMarkerPosition;

	float width = Width();
	float height = Height();

	switch (fMode) {
		case R_SELECTED:
			fMarkerPosition = BPoint(color.green / 255.0 * width,
				(255.0 - color.blue) / 255.0 * height);
			break;

		case G_SELECTED:
			fMarkerPosition = BPoint(color.red / 255.0 * width,
				(255.0 - color.blue) / 255.0 * height);
			break;

		case B_SELECTED:
			fMarkerPosition = BPoint(color.red / 255.0 * width,
				(255.0 - color.green) / 255.0 * height);
			break;

		case H_SELECTED:
			if (fOrientation == B_VERTICAL)
				fMarkerPosition = BPoint(s * width, height - v * height);
			else
				fMarkerPosition = BPoint(width - v * width, s * height);
			break;

		case S_SELECTED:
			fMarkerPosition = BPoint(h / 6.0 * width, height - v * height);
			break;

		case V_SELECTED:
			fMarkerPosition = BPoint( h / 6.0 * width, height - s * height);
			break;
	}

	Update(1);
}

// PositionMarkerAt
void
ColorField::PositionMarkerAt( BPoint where )
{
	BRect rect = Bounds().InsetByCopy(2.0, 2.0).OffsetToCopy(0.0, 0.0);
	where = BPoint(max_c(min_c(where.x - 2.0, rect.right), 0.0),
		max_c(min_c(where.y - 2.0, rect.bottom), 0.0));

	fLastMarkerPosition = fMarkerPosition;
	fMarkerPosition = where;
	Update(1);

}

// Width
float
ColorField::Width() const
{
	return Bounds().IntegerWidth() + 1 - 4;
}

// Height
float
ColorField::Height() const
{
	return Bounds().IntegerHeight() + 1 - 4;
}

// set_bits
static inline void
set_bits(uint8* bits, uint8 r, uint8 g, uint8 b)
{
	bits[0] = b;
	bits[1] = g;
	bits[2] = r;
	bits[3] = 255;
}

// _Init
void
ColorField::_Init(SelectedColorMode mode, float fixedValue,
	orientation orient)
{
	fMode = mode;
	fFixedValue = fixedValue;
	fOrientation = orient;

	fMarkerPosition = BPoint(0.0, 0.0);
	fLastMarkerPosition = BPoint(-1.0, -1.0);
	fMouseDown = false;
	fUpdateThread = B_ERROR;
	fUpdatePort = B_ERROR;

	SetViewColor(B_TRANSPARENT_COLOR);

	BRect bounds = BRect(0.0, 0.0, MAX_X + 4.0, MAX_Y + 4.0);
	for (int32 i = 0; i < 2; i++) {
		fBgBitmap[i] = new BBitmap(bounds, B_RGB32, true);

		fBgBitmap[i]->Lock();
		fBgView[i] = new BView(bounds, "", B_FOLLOW_NONE, B_WILL_DRAW);
		fBgBitmap[i]->AddChild(fBgView[i]);
		fBgView[i]->SetOrigin(2.0, 2.0);
		fBgBitmap[i]->Unlock();
	}

	_DrawBorder();

	fUpdatePort = create_port(100, "color field update port");

	fUpdateThread = spawn_thread(ColorField::_UpdateThread,
		"color field update thread", 10, this);
	resume_thread(fUpdateThread);
}

// _UpdateThread
status_t
ColorField::_UpdateThread(void* data)
{
	// initializing
	ColorField* colorField = (ColorField *)data;

	bool looperLocked = colorField->LockLooper();

	BBitmap* bitmap = colorField->fBgBitmap[0];
	port_id	port = colorField->fUpdatePort;
	orientation orient = colorField->fOrientation;

	if (looperLocked)
		colorField->UnlockLooper();

	float h = 0;
	float s = 0;
	float v = 0;
	float r = 0;
	float g = 0;
	float b = 0;
	int R = 0;
	int G = 0;
	int B = 0;

	// drawing

    int32 msg_code;
    char msg_buffer;

	while (true) {
		port_info info;
		do {
			read_port(port, &msg_code, &msg_buffer, sizeof(msg_buffer));
			get_port_info(port, &info);
		} while (info.queue_count);

		if (colorField->LockLooper()) {

			uint 	colormode = colorField->fMode;
			float	fixedvalue = colorField->fFixedValue;

			int width = (int)colorField->Width();
			int height = (int)colorField->Height();

			colorField->UnlockLooper();

			bitmap->Lock();
	//bigtime_t now = system_time();
			uint8* bits = (uint8*)bitmap->Bits();
			uint32 bpr = bitmap->BytesPerRow();
			// offset 2 pixels from top and left
			bits += 2 * 4 + 2 * bpr;

			switch (colormode) {

				case R_SELECTED: {
					R = round(fixedvalue * 255);
					for (int y = height; y >= 0; y--) {
						uint8* bitsHandle = bits;
						int B = y / height * 255;
						for (int x = 0; x <= width; ++x) {
							int G = x / width * 255;
							set_bits(bitsHandle, R, G, B);
							bitsHandle += 4;
						}
						bits += bpr;
					}
				}; break;

				case G_SELECTED: {
					G = round(fixedvalue * 255);
					for (int y = height; y >= 0; y--) {
						uint8* bitsHandle = bits;
						int B = y / height * 255;
						for (int x = 0; x <= width; ++x) {
							int R = x / width * 255;
							set_bits(bitsHandle, R, G, B);
							bitsHandle += 4;
						}
						bits += bpr;
					}
				}; break;

				case B_SELECTED: {
					B = round(fixedvalue * 255);
					for (int y = height; y >= 0; y--) {
						uint8* bitsHandle = bits;
						int G = y / height * 255;
						for (int x = 0; x <= width; ++x) {
							int R = x / width * 255;
							set_bits(bitsHandle, R, G, B);
							bitsHandle += 4;
						}
						bits += bpr;
					}
				}; break;

				case H_SELECTED: {
					h = fixedvalue;
					if (orient == B_VERTICAL) {
						for (int y = 0; y <= height; ++y) {
							v = (float)(height - y) / height;
							uint8* bitsHandle = bits;
							for (int x = 0; x <= width; ++x) {
								s = (float)x / width;
								HSV_to_RGB( h, s, v, r, g, b );
								set_bits(bitsHandle, round(r * 255.0), round(g * 255.0), round(b * 255.0));
								bitsHandle += 4;
							}
							bits += bpr;
						}
					} else {
						for (int y = 0; y <= height; ++y) {
							s = (float)y / height;
							uint8* bitsHandle = bits;
							for (int x = 0; x <= width; ++x) {
								v = (float)(width - x) / width;
								HSV_to_RGB( h, s, v, r, g, b );
								set_bits(bitsHandle, round(r * 255.0), round(g * 255.0), round(b * 255.0));
								bitsHandle += 4;
							}
							bits += bpr;
						}
					}
				}; break;

				case S_SELECTED: {
					s = fixedvalue;
					for (int y = 0; y <= height; ++y) {
						v = (float)(height - y) / height;
						uint8* bitsHandle = bits;
						for (int x = 0; x <= width; ++x) {
							h = 6.0 / width * x;
							HSV_to_RGB( h, s, v, r, g, b );
							set_bits(bitsHandle, round(r * 255.0), round(g * 255.0), round(b * 255.0));
							bitsHandle += 4;
						}
						bits += bpr;
					}
				}; break;

				case V_SELECTED: {
					v = fixedvalue;
					for (int y = 0; y <= height; ++y) {
						s = (float)(height - y) / height;
						uint8* bitsHandle = bits;
						for (int x = 0; x <= width; ++x) {
							h = 6.0 / width * x;
							HSV_to_RGB( h, s, v, r, g, b );
							set_bits(bitsHandle, round(r * 255.0), round(g * 255.0), round(b * 255.0));
							bitsHandle += 4;
						}
						bits += bpr;
					}
				}; break;
			}

	//printf("color field update: %lld\n", system_time() - now);
			bitmap->Unlock();

			if (colorField->LockLooper()) {
				colorField->Update(2);
				colorField->UnlockLooper();
			}
		}
	}
	return B_OK;
}

// _DrawBorder
void
ColorField::_DrawBorder()
{
	bool looperLocked = LockLooper();

	fBgBitmap[1]->Lock();

	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color shadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_3_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);

	BRect r(fBgView[1]->Bounds());
	r.OffsetBy(-2.0, -2.0);
	BRegion region(r);
	fBgView[1]->ConstrainClippingRegion(&region);

	r = Bounds();
	r.OffsetBy(-2.0, -2.0);

	stroke_frame(fBgView[1], r, shadow, shadow, light, light);
	r.InsetBy(1.0, 1.0);
	stroke_frame(fBgView[1], r, darkShadow, darkShadow, background, background);
	r.InsetBy(1.0, 1.0);

	region.Set(r);
	fBgView[1]->ConstrainClippingRegion(&region);

	fBgBitmap[1]->Unlock();

	if (looperLocked)
		UnlockLooper();
}
