#include "fonthelper.h"
#include "utilities.h"
#include "debug.h"
#include <sstream>

const uint8_t outline = 1;

FontHelper::FontHelper(const string &fontName, int fontSize, RGBAColor textColor, RGBAColor outlineColor):
fontName(fontName), fontSize(fontSize), textColor(textColor), outlineColor(outlineColor) {
	loadFont(fontName, fontSize);
}

FontHelper::~FontHelper() {
	free();
}

void FontHelper::free() {
	TTF_CloseFont(font);
	TTF_CloseFont(fontOutline);
}

void FontHelper::loadFont(const string &fontName, int fontSize) {
	if (!TTF_WasInit()) {
		DEBUG("Initializing font");
		if (TTF_Init() == -1) {
			ERROR("TTF_Init: %s", TTF_GetError());
			exit(2);
		}
	}
	this->font = TTF_OpenFont(fontName.c_str(), fontSize);
	this->fontOutline = TTF_OpenFont(fontName.c_str(), fontSize);
	if (!this->font || !this->fontOutline) {
		ERROR("TTF_OpenFont %s: %s", fontName.c_str(), TTF_GetError());
		return;
	}
	fontFamilyName = TTF_FontFaceFamilyName(font);
	if (fontFamilyName == "") {
		WARNING("Unable to read TTF font family name.\n");
	}
	DEBUG("Font Name: %s", fontFamilyName.c_str());
	TTF_SetFontHinting(this->font, TTF_HINTING_LIGHT);
	TTF_SetFontHinting(this->fontOutline, TTF_HINTING_LIGHT);
	TTF_SetFontOutline(this->fontOutline, outline);
	height = 0;
	// Get maximum line height with a sample text
	TTF_SizeUTF8(this->font, "AZ0987654321", NULL, &height);
	halfHeight = height / 2;
}

bool FontHelper::utf8Code(uint8_t c) {
	return (c >= 194 && c <= 198) || c == 208 || c == 209;
}

FontHelper *FontHelper::setSize(const int size) {
	if (this->fontSize == size) return this;
	TTF_CloseFont(font);
	TTF_CloseFont(fontOutline);
	fontSize = size;
	loadFont(this->fontName, this->fontSize);
	return this;
}

FontHelper *FontHelper::setColor(RGBAColor color) {
	textColor = color;
	return this;
}

FontHelper *FontHelper::setOutlineColor(RGBAColor color) {
	outlineColor = color;
	return this;
}

uint32_t FontHelper::getLineWidth(const string &text) {
	int width = 0;
	TTF_SizeUTF8(fontOutline, text.c_str(), &width, NULL);
	return width;
}

uint32_t FontHelper::getTextWidth(const string &text) {
	if (text.find("\n",0) != string::npos) {
		vector<string> textArr;
		split(textArr,text,"\n");
		return getTextWidth(&textArr);
	}
	return getLineWidth(text);
}

uint32_t FontHelper::getTextWidth(vector<string> *text) {
	int w = 0;
	for (uint32_t i = 0; i < text->size(); i++) {
		w = max(getLineWidth(text->at(i)), w);
	};
	return w;
}

int FontHelper::getTextHeight(const string &text) {
	vector<string> textArr;
	split(textArr, text, "\n");
	return textArr.size();
}

void FontHelper::write(Surface *surface, vector<string> *text, int x, int y, const uint8_t align) {
	write(surface, text, x, y, align, textColor, outlineColor);
}

void FontHelper::write(Surface *surface, vector<string> *text, int x, int y, const uint8_t align, RGBAColor fgColor, RGBAColor bgColor) {
	if (align & VAlignMiddle) {
		y -= getHalfHeight() * text->size();
	} else if (align & VAlignBottom) {
		y -= getHeight() * text->size();
	}

	for (uint32_t i = 0; i < text->size(); i++) {
		int ix = x;
		if (align & HAlignCenter) {
			ix -= getTextWidth(text->at(i)) / 2;
		} else if (align & HAlignRight) {
			ix -= getTextWidth(text->at(i));
		}
		write(surface, text->at(i), ix, y + i * getHeight(), fgColor, bgColor);
	}
}

void FontHelper::write(Surface* surface, const string &text, int x, int y, const uint8_t align, RGBAColor fgColor, RGBAColor bgColor) {
	if (text.find("\n", 0) != string::npos) {
		vector<string> textArr;
		split(textArr,text, "\n");
		write(surface, &textArr, x, y, align, fgColor, bgColor);
		return;
	}

	if (align & HAlignCenter) {
		x -= getTextWidth(text) / 2;
	} else if (align & HAlignRight) {
		x -= getTextWidth(text);
	}

	if (align & VAlignMiddle) {
		y -= getHalfHeight();
	} else if (align & VAlignBottom) {
		y -= getHeight();
	}

	write(surface, text, x, y, fgColor, bgColor);
}

void FontHelper::write(Surface *surface, const string &text, int x, int y, const uint8_t align) {
	write(surface, text, x, y, align, textColor, outlineColor);
}

void FontHelper::write(Surface *surface, const string &text, SDL_Rect &wrapRect, const uint8_t align) {
	string textwrap = "";
	uint32_t maxWidth = wrapRect.w;
	//uint32_t maxString = 0;
	bool firstline = true;
	if (getTextWidth(text) > maxWidth) {
		string line = "";
		string prvsub = "";
		std::istringstream iss(text);
		do {
			string subs;
			iss >> subs;
			if (getTextWidth(subs) > maxWidth) maxWidth = getTextWidth(subs);
			//if (getTextWidth(subs) > maxString) maxString = getTextWidth(subs);
		} while (iss);
		//if (maxWidth == wrapRect.w) firstline = false;
		std::istringstream iss2(text);
		do {
			string subs;
			iss2 >> subs;
			line += subs + " ";
			if (getTextWidth(line) > maxWidth || getTextWidth(prvsub) == maxWidth /*||
			    (maxWidth == wrapRect.w && getTextWidth(textwrap) > maxWidth &&
				 getTextWidth(prvsub + " " + line) > maxWidth && getTextWidth(prvsub) < maxString)*/) {
				textwrap += "\n";
				if (!firstline) {
					line = "";
					firstline = true;
				}
			} else {
				textwrap += " ";
			}
			if (line != "") firstline = false;
			textwrap += subs;
			prvsub = subs;
		} while (iss2);
		textwrap = trim(textwrap);
	} else {
		textwrap = trim(text);
	}
	write(surface, textwrap, wrapRect.x, wrapRect.y, align, textColor, outlineColor);
}

void FontHelper::write(Surface *surface, const string &text, int x, int y, RGBAColor fgColor, RGBAColor bgColor) {
	if (text.empty()) return;

	if (bgColor.a > 0) {
		Surface bg;
		if (fontFamilyName.find("Unifont") == string::npos) {
			bg.raw = TTF_RenderUTF8_Blended(fontOutline, text.c_str(), rgbatosdl(bgColor));
		} else {
			bg.raw = TTF_RenderUTF8_Solid(fontOutline, text.c_str(), rgbatosdl(bgColor));
		}
		bg.setAlpha(bgColor.a);
		bg.blit(surface, x - outline, y - outline);
	}

	if (fgColor.a > 0) {
		Surface fg;
		if (fontFamilyName.find("Unifont") == string::npos) {
			fg.raw = TTF_RenderUTF8_Blended(font, text.c_str(), rgbatosdl(fgColor));
		} else {
			fg.raw = TTF_RenderUTF8_Solid(font, text.c_str(), rgbatosdl(fgColor));
		}
		fg.setAlpha(fgColor.a);
		fg.blit(surface, x, y);
	}
}
