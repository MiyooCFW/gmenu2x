/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SURFACE_H
#define SURFACE_H

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

using std::string;

const int	HAlignLeft		= 1,
			HAlignRight		= 2,
			HAlignCenter	= 4,
			VAlignTop		= 8,
			VAlignBottom	= 16,
			VAlignMiddle	= 32;

class FontHelper;

struct RGBAColor {
	uint8_t r,g,b,a;
	// static RGBAColor fromString(std::string const& strColor);
	// static string toString(RGBAColor &color);
	RGBAColor() : r(0), g(0), b(0), a(0) {}
	RGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
		: r(r), g(g), b(b), a(a) {}
	Uint32 pixelValue(SDL_PixelFormat *fmt) const {
		return SDL_MapRGBA(fmt, r, g, b, a);
	}

};

RGBAColor strtorgba(const string &strColor);
string rgbatostr(RGBAColor color);
SDL_Color rgbatosdl(RGBAColor color);

/**
	Wrapper around SDL_Surface
	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class Surface {
private:
	bool locked;
	int halfW, halfH;
	SDL_Surface *dblbuffer;

public:
	Surface();
	Surface(const string &img, const string &skin="", bool alpha=true);
	Surface(const string &img, bool alpha, const string &skin="");
	Surface(SDL_Surface *s, SDL_PixelFormat *fmt = NULL, uint32_t flags = 0);
	Surface(Surface *s);
	Surface(int w, int h, uint32_t flags = SDL_HWSURFACE|SDL_SRCALPHA);
	~Surface();

	void enableVirtualDoubleBuffer(SDL_Surface *surface, bool alpha=true);
	void enableAlpha();

	SDL_Surface *raw;
#if defined(TARGET_RS97) || defined(TARGET_MIYOO)
	SDL_Surface *ScreenSurface;
#endif

	void free();
	void load(const string &img, bool alpha=true, const string &skin="");
	void lock();
	void unlock();
	void flip();
	SDL_PixelFormat *format();

	void putPixel(int,int,RGBAColor);
	void putPixel(int,int,uint32_t);
	RGBAColor pixelColor(int,int);
	uint32_t pixel(int,int);

	void blendAdd(Surface*, int,int);

	void clearClipRect();
	void setClipRect(SDL_Rect rect);

	bool blit(Surface *destination, int x, int y, const uint8_t align = HAlignLeft | VAlignTop, uint8_t alpha=-1);
	bool blit(Surface *destination, SDL_Rect destrect, const uint8_t align = HAlignLeft | VAlignTop, uint8_t alpha=-1);

	void write(FontHelper *font, const string &text, int x, int y, const uint8_t align = HAlignLeft | VAlignTop);
	void write(FontHelper *font, const string &text, int x, int y, const uint8_t align, RGBAColor fgColor, RGBAColor bgColor);

	void box(SDL_Rect re, RGBAColor c);
	void box(Sint16 x, Sint16 y, Uint16 w, Uint16 h, RGBAColor c) {
		box((SDL_Rect){ x, y, w, h }, c);
	}
	void box(Sint16 x, Sint16 y, Uint16 w, Uint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
		box((SDL_Rect){ x, y, w, h }, RGBAColor(r, g, b, a));
	}
	/** Draws the given rectangle on this surface in the given color, blended
	  * according to the alpha value of the color argument.
	  */
	void fillRectAlpha(SDL_Rect rect, RGBAColor c);

	/** Clips the given rectangle against this surface's active clipping
	  * rectangle.
	  */
	void applyClipRect(SDL_Rect& rect);

	void rectangle(SDL_Rect re, RGBAColor c);
	void rectangle(Sint16 x, Sint16 y, Uint16 w, Uint16 h, RGBAColor c) {
		rectangle((SDL_Rect){ x, y, w, h }, c);
	}
	void rectangle(Sint16 x, Sint16 y, Uint16 w, Uint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
		rectangle((SDL_Rect){ x, y, w, h }, RGBAColor(r, g, b, a));
	}

	void operator = (SDL_Surface*);
	void operator = (Surface*);

	void softStretch(uint16_t x, uint16_t y, bool keep_aspect = false, bool maximize = true);
	void setAlpha(uint8_t alpha);
};

#endif
