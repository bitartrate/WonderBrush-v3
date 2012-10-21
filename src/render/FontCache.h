/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include <String.h>

#include "RWLocker.h"
#include "TextRenderer.h"

class FontCache : public RWLocker {
public:
	FontCache(int dpiX, int dpiY);
	virtual ~FontCache();

	static FontCache* getInstance();

	inline FontEngine& getFontEngine()
	{
		return fFontEngine;
	}

	inline FontManager& getFontManager()
	{
		return fFontManager;
	}

	void setFontFolder(const char* path);
	
	const BString& getFontFolder() const;

	BString resolveFont(const char* fontFilePath) const;

private:
	static FontCache	sDefaultInstance;

	FontEngine			fFontEngine;
	FontManager			fFontManager;

	BString				fFontFolder;
};

#endif // FONT_CACHE_H
