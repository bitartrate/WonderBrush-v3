/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Text.h"

#include "CharacterStyle.h"
#include "FontCache.h"
#include "RenderEngine.h"
#include "StyleRun.h"
#include "StyleRunList.h"
#include "TextSnapshot.h"
#include "ui_defines.h"

// constructor
Text::Text(const rgb_color& color)
	: Styleable(color)
	, fText()
	, fCharCount(0)
	, fTextLayout(FontCache::getInstance())
	, fStyleRuns()
{
	InitBounds();
}

// constructor
Text::Text(const Text& other, CloneContext& context)
	: Styleable(other, context)
	, fText(other.fText)
	, fCharCount(other.fCharCount)
	, fTextLayout(FontCache::getInstance())
	, fStyleRuns(other.fStyleRuns, context)
{
	InitBounds();
	
	fTextLayout = other.fTextLayout;
	
	UpdateLayout();
}

// destructor
Text::~Text()
{
}

// #pragma mark -

// Clone
BaseObject*
Text::Clone(CloneContext& context) const
{
	return new(std::nothrow) Text(*this, context);
}

// DefaultName
const char*
Text::DefaultName() const
{
	return "Text";
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Text::Snapshot() const
{
	return new TextSnapshot(this);
}

// HitTest
bool
Text::HitTest(const BPoint& canvasPoint)
{
	RenderEngine engine(Transformation());
	return engine.HitTest(Bounds(), canvasPoint);
}

// Bounds
BRect
Text::Bounds()
{
	BRect bounds(0.0f, 0.0f, fTextLayout.getActualWidth(),
		fTextLayout.getHeight());
	Style()->ExtendBounds(bounds);
	return bounds;
}

// #pragma mark -

// SetWidth
void
Text::SetWidth(double width)
{
	if (width == fTextLayout.getWidth())
		return;

	fTextLayout.setWidth(width);

	NotifyAndUpdate();
}

// Width
double
Text::Width() const
{
	return fTextLayout.getWidth();
}

// ActualWidth
double
Text::ActualWidth()
{
	return fTextLayout.getActualWidth();
}

// SetAlignment
void
Text::SetAlignment(uint32 alignment)
{
	if (alignment == Alignment())
		return;

	switch (alignment) {
		default:
		case TEXT_ALIGNMENT_LEFT:
			fTextLayout.setAlignment(ALIGNMENT_LEFT);
			fTextLayout.setJustify(false);
			break;
		case TEXT_ALIGNMENT_CENTER:
			fTextLayout.setAlignment(ALIGNMENT_CENTER);
			fTextLayout.setJustify(false);
			break;
		case TEXT_ALIGNMENT_RIGHT:
			fTextLayout.setAlignment(ALIGNMENT_RIGHT);
			fTextLayout.setJustify(false);
			break;
		case TEXT_ALIGNMENT_JUSTIFY:
			fTextLayout.setAlignment(ALIGNMENT_LEFT);
			fTextLayout.setJustify(true);
			break;
	}

	NotifyAndUpdate();
}

// Alignment
uint32
Text::Alignment() const
{
	uint32 alignment = fTextLayout.getAlignment();
	bool justify = fTextLayout.getJustify();

	if (alignment == ALIGNMENT_LEFT && !justify)
		return TEXT_ALIGNMENT_LEFT;

	if (alignment == ALIGNMENT_CENTER && !justify)
		return TEXT_ALIGNMENT_CENTER;

	if (alignment == ALIGNMENT_RIGHT && !justify)
		return TEXT_ALIGNMENT_RIGHT;

	if (alignment == ALIGNMENT_LEFT && justify)
		return TEXT_ALIGNMENT_JUSTIFY;

	return TEXT_ALIGNMENT_LEFT;
}

// SetGlyphSpacing
void
Text::SetGlyphSpacing(double spacing)
{
	if (spacing == fTextLayout.getGlyphSpacing())
		return;

	fTextLayout.setGlyphSpacing(spacing);

	NotifyAndUpdate();
}

// GlyphSpacing
double
Text::GlyphSpacing() const
{
	return fTextLayout.getGlyphSpacing();
}

// SetText
void
Text::SetText(const char* utf8String, const Font& font, rgb_color color)
{
	::Style* style = new(std::nothrow) ::Style();
	if (style == NULL)
		return;

	style->SetFillPaint(PaintRef(new(std::nothrow) Paint(color), true));

	StyleRef styleRef(style, true);
	SetText(utf8String, font, styleRef);
}

// SetText
void
Text::SetText(const char* utf8String, const Font& font, const StyleRef& style)
{
	fText = "";
	fStyleRuns.MakeEmpty();

	Insert(0, utf8String, font, style);
}

// GetText
const BString&
Text::GetText() const
{
	return fText;
}

// GetCharCount
int32
Text::GetCharCount() const
{
	return fCharCount;
}

// GetStyleRuns
const StyleRunList&
Text::GetStyleRuns() const
{
	return fStyleRuns;
}

// Insert
void
Text::Insert(int32 textOffset, const char* utf8String, const Font& font,
	rgb_color color)
{
	::Style* style = new(std::nothrow) ::Style();
	if (style == NULL)
		return;

	style->SetFillPaint(PaintRef(new(std::nothrow) Paint(color), true));

	StyleRef styleRef(style, true);

	Insert(textOffset, utf8String, font, 0.0, 0.0, 0.0, styleRef);
}

// Insert
void
Text::Insert(int32 textOffset, const char* utf8String, const Font& font,
	const StyleRef& style)
{
	Insert(textOffset, utf8String, font, 0.0, 0.0, 0.0, style);
}

// Insert
void
Text::Insert(int32 textOffset, const char* utf8String, const Font& font,
	double glyphSpacing, double fauxWeight, double fauxItalic,
	const StyleRef& style)
{
	if (textOffset < 0 || textOffset > fCharCount)
		return;

	CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
		font, glyphSpacing, fauxWeight, fauxItalic, style);

	if (characterStyle == NULL)
		return;

	CharacterStyleRef styleRef(characterStyle);

	BString text(utf8String);
	int32 charCount = text.CountChars();

	StyleRun styleRun(styleRef);
	styleRun.SetLength(charCount);

	if (!fStyleRuns.Insert(textOffset, styleRun))
		return;

	fText.InsertChars(text, textOffset);
	fCharCount += charCount;

	styleRef.Detach();

	UpdateLayout();
}

// Insert
void
Text::Insert(int32 textOffset, const BString& utf8String,
	const StyleRunList& styleRuns)
{
	if (textOffset < 0 || textOffset > fCharCount)
		return;

	int32 charCount = utf8String.CountChars();

	if (!fStyleRuns.Insert(textOffset, styleRuns))
		return;

	fText.InsertChars(utf8String, textOffset);
	fCharCount += charCount;

	UpdateLayout();
}

// Append
void
Text::Append(const char* utf8String, const Font& font,
	rgb_color color)
{
	Insert(fCharCount, utf8String, font, color);
}

// Append
void
Text::Append(const char* utf8String, const Font& font,
	const StyleRef& style)
{
	Insert(fCharCount, utf8String, font, style);
}

// Append
void
Text::Append(const char* utf8String, const Font& font,
	double glyphSpacing, double fauxWeight, double fauxItalic,
	const StyleRef& style)
{
	Insert(fCharCount, utf8String, font, glyphSpacing, fauxWeight, fauxItalic,
		style);
}

// Append
void
Text::Append(const BString& utf8String, const StyleRunList& styleRuns)
{
	Insert(fCharCount, utf8String, styleRuns);
}

// ReplaceStyles
void
Text::ReplaceStyles(int32 textOffset, int32 length,
	const StyleRunList& styleRuns)
{
	if (textOffset < 0 || textOffset > fCharCount)
		return;

	fStyleRuns.Remove(textOffset, length);
	fStyleRuns.Insert(textOffset, styleRuns);

	UpdateLayout();
}

// Remove
void
Text::Remove(int32 textOffset, int32 length)
{
	if (textOffset < 0 || textOffset + length > fCharCount)
		return;

	fText.RemoveChars(textOffset, length);
	fStyleRuns.Remove(textOffset, length);
	fCharCount -= length;

	UpdateLayout();
}

// GetSubString
BString
Text::GetSubString(int32 textOffset, int32 length) const
{
	BString subString;
	if (textOffset < 0 || textOffset + length > fCharCount)
		return subString;
	subString = fText;
	subString.RemoveChars(0, textOffset);
	subString.TruncateChars(length);
	return subString;
}

// GetStyleRuns
StyleRunList*
Text::GetStyleRuns(int32 textOffset, int32 length) const
{
	return fStyleRuns.GetSubList(textOffset, length);
}

// SetStyle
void
Text::SetStyle(int32 textOffset, int32 length, const Font& font,
	const StyleRef& style)
{
	if (textOffset < 0 || textOffset + length > fCharCount || length == 0)
		return;

	CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
		font, 0.0, 0.0, 0.0, style);

	if (characterStyle == NULL)
		return;

	CharacterStyleRef styleRef(characterStyle, true);

	StyleRun styleRun(styleRef);
	styleRun.SetLength(length);

	if (!fStyleRuns.Insert(textOffset, styleRun))
		return;

	fStyleRuns.Remove(textOffset + length, length);

	styleRef.Detach();

	UpdateLayout();
}

// SetFont
void
Text::SetFont(int32 textOffset, int32 length, const char* family,
	const char* style)
{
	if (textOffset < 0 || textOffset + length > fCharCount || length == 0)
		return;

	while (length > 0) {
		// TODO: Make more efficient
		const StyleRun* run = fStyleRuns.FindStyleRun(textOffset);
		const CharacterStyle* previousCharacterStyle = run->GetStyle().Get();
		Font font = previousCharacterStyle->GetFont()
			.setFamilyAndStyle(family, style);

		CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
			previousCharacterStyle->SetFont(font));

		if (characterStyle == NULL)
			return;

		CharacterStyleRef styleRef(characterStyle, true);

		StyleRun replaceRun(styleRef);
		replaceRun.SetLength(1);

		if (!fStyleRuns.Insert(textOffset, replaceRun))
			return;

		fStyleRuns.Remove(textOffset + 1, 1);
		styleRef.Detach();

		textOffset++;
		length--;
	}

	UpdateLayout();
}

// SetSize
void
Text::SetSize(int32 textOffset, int32 length, double size)
{
	if (textOffset < 0 || textOffset + length > fCharCount || length == 0)
		return;

	while (length > 0) {
		// TODO: Make more efficient
		const StyleRun* run = fStyleRuns.FindStyleRun(textOffset);
		const CharacterStyle* previousCharacterStyle = run->GetStyle().Get();
		Font font = previousCharacterStyle->GetFont().setSize(size);

		CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
			previousCharacterStyle->SetFont(font));

		if (characterStyle == NULL)
			return;

		CharacterStyleRef styleRef(characterStyle, true);

		StyleRun replaceRun(styleRef);
		replaceRun.SetLength(1);

		if (!fStyleRuns.Insert(textOffset, replaceRun))
			return;

		fStyleRuns.Remove(textOffset + 1, 1);
		styleRef.Detach();

		textOffset++;
		length--;
	}

	UpdateLayout();
}

// SetColor
void
Text::SetColor(int32 textOffset, int32 length, const rgb_color& color)
{
	if (textOffset < 0 || textOffset + length > fCharCount || length == 0)
		return;

	::Style* style = new(std::nothrow) ::Style();
	if (style == NULL)
		return;

	style->SetFillPaint(PaintRef(new(std::nothrow) Paint(color), true));

	StyleRef styleRef(style, true);

	while (length > 0) {
		// TODO: Make more efficient
		const StyleRun* run = fStyleRuns.FindStyleRun(textOffset);

		CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
			run->GetStyle()->SetStyle(styleRef));

		if (characterStyle == NULL)
			return;

		CharacterStyleRef styleRef(characterStyle, true);

		StyleRun replaceRun(styleRef);
		replaceRun.SetLength(1);

		if (!fStyleRuns.Insert(textOffset, replaceRun))
			return;

		fStyleRuns.Remove(textOffset + 1, 1);
		styleRef.Detach();

		textOffset++;
		length--;
	}

	UpdateLayout();
}

// getTextLayout
TextLayout&
Text::getTextLayout()
{
	return fTextLayout;
}

// getTextLayout
const TextLayout&
Text::getTextLayout() const
{
	return fTextLayout;
}

// #pragma mark -

// UpdateLayout
void
Text::UpdateLayout()
{
//	printf("UpdateLayout() (%p)\n", &fTextLayout);

	fTextLayout.clearStyleRuns();

	int32 start = 0;
	for (int32 i = 0; i < fStyleRuns.CountRuns(); i++) {
		const StyleRun* run = fStyleRuns.StyleRunAt(i);

		const CharacterStyleRef& characterStyle = run->GetStyle();

		const Font& font = characterStyle.Get()->GetFont();
		const StyleRef& style = characterStyle.Get()->GetStyle();
		Paint* paint = style.Get()->FillPaint();
		rgb_color color = paint->Color();

//		printf("  run (%d, %d, %d), length: %ld\n",
//			color.red, color.green, color.blue,
//			run->GetLength());

		fTextLayout.addStyleRun(
			start, font,
			0.0, 0.0, 0.0,
			characterStyle->GetGlyphSpacing(),
			characterStyle->GetFauxWeight(),
			characterStyle->GetFauxItalic(),
			TextRenderer::Color(
				RenderEngine::GammaToLinear(color.red),
				RenderEngine::GammaToLinear(color.green),
				RenderEngine::GammaToLinear(color.blue),
				(color.alpha << 8) | 255
			).premultiply(),
			TextRenderer::Color(
				RenderEngine::GammaToLinear(255),
				RenderEngine::GammaToLinear(255),
				RenderEngine::GammaToLinear(255),
				(255 << 8) | 255
			).premultiply(),
			false, TextRenderer::Color(0, 0, 0, 0),
			false, 0, TextRenderer::Color(0, 0, 0, 0)
		);

		start += run->GetLength();
	}

//	printf("  chars: %ld, total run length: %ld\n",
//		fText.CountChars(), start);
	if (fText.CountChars() != start)
		debugger("Text::UpdateLayout() - StyleRunList invalid!");

	fTextLayout.setText(fText.String());

	NotifyAndUpdate();
}

