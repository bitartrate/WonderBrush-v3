/*
 * Font.h
 *
 *  Created on: 22.08.2012
 *      Author: stippi
 */

#ifndef FONT_H_
#define FONT_H_

class Font {
public:
	static const unsigned kNormal		= 0x00;
	static const unsigned kBold			= 0x01;
	static const unsigned kItalic		= 0x02;

	enum ScriptLevel {
		NORMAL = 0, SUBSCRIPT, SUPERSCRIPT, LOW_SUPERSCRIPT, HIGH_SUBSCRIPT
	};

public:
	Font(const char* name, double size, unsigned style);
	Font(const char* name, double size, unsigned style,
			ScriptLevel scriptLevel);
	Font(const Font& other);
	virtual ~Font();

	bool operator==(const Font& other) const;
	bool operator!=(const Font& other) const;

	Font& operator=(const Font& other);

	inline const char* getName() const
	{
		return fName;
	}

	inline double getSize() const
	{
		return fSize;
	}

	inline unsigned getStyle() const
	{
		return fStyle;
	}

	inline ScriptLevel getScriptLevel() const
	{
		return fScriptLevel;
	}

private:
	char			fName[256];
	double			fSize;
	unsigned		fStyle;
	ScriptLevel		fScriptLevel;
};

#endif /* FONT_H_ */
