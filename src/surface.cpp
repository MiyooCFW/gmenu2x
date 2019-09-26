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

#include "surface.h"
#include "fonthelper.h"
#include "utilities.h"
#include "debug.h"
#include <cassert>

RGBAColor strtorgba(const string &strColor) {
	const int s = (strColor.at(0) == '#') ? 1 : 0;
	RGBAColor c = {0,0,0,255};
	c.r = constrain( strtol( strColor.substr(0 + s, 2).c_str(), NULL, 16 ), 0, 255 );
	c.g = constrain( strtol( strColor.substr(2 + s, 2).c_str(), NULL, 16 ), 0, 255 );
	c.b = constrain( strtol( strColor.substr(4 + s, 2).c_str(), NULL, 16 ), 0, 255 );
	c.a = constrain( strtol( strColor.substr(6 + s, 2).c_str(), NULL, 16 ), 0, 255 );
	return c;
}

string rgbatostr(RGBAColor color) {
	char hexcolor[10];
	snprintf(hexcolor, sizeof(hexcolor), "#%02x%02x%02x%02x", color.r, color.g, color.b, color.a);
	return (string)hexcolor;
}

SDL_Color rgbatosdl(RGBAColor color) {
	return (SDL_Color){color.r, color.g, color.b, color.a};
}

Surface::Surface() {
	raw = NULL;
	dblbuffer = NULL;
}

Surface::Surface(const string &img, bool alpha, const string &skin) {
	raw = NULL;
	dblbuffer = NULL;
	load(img, alpha, skin);
	halfW = raw->w/2;
	halfH = raw->h/2;
}

Surface::Surface(const string &img, const string &skin, bool alpha) {
	raw = NULL;
	dblbuffer = NULL;
	load(img, alpha, skin);
	halfW = raw->w/2;
	halfH = raw->h/2;
}

Surface::Surface(SDL_Surface *s, SDL_PixelFormat *fmt, uint32_t flags) {
	dblbuffer = NULL;
	this->operator =(s);
	if (fmt != NULL || flags != 0) {
		if (fmt == NULL) fmt = s->format;
		if (flags == 0) flags = s->flags;
		raw = SDL_ConvertSurface(s, fmt, flags);
	}
}

Surface::Surface(Surface *s) {
	dblbuffer = NULL;
	this->operator =(s->raw);
}

Surface::Surface(int w, int h, uint32_t flags) {
	dblbuffer = NULL;
	uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	raw = SDL_DisplayFormat( SDL_CreateRGBSurface( flags, w, h, 16, rmask, gmask, bmask, amask ) );
	//SDL_SetAlpha(raw, SDL_SRCALPHA|SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
	halfW = w/2;
	halfH = h/2;
}

Surface::~Surface() {
	free();
}

void Surface::enableVirtualDoubleBuffer(SDL_Surface *surface, bool alpha) {
	dblbuffer = surface;
	if (alpha)
		raw = SDL_DisplayFormatAlpha(dblbuffer);
	else
		raw = SDL_DisplayFormat(dblbuffer);
}

void Surface::enableAlpha() {
	SDL_Surface *alpha_surface = SDL_DisplayFormatAlpha(raw);
	SDL_FreeSurface(raw);
	raw = alpha_surface;
}

void Surface::free() {
	SDL_FreeSurface( raw );
	SDL_FreeSurface( dblbuffer );
	raw = NULL;
	dblbuffer = NULL;
}

SDL_PixelFormat *Surface::format() {
	if (raw == NULL)
		return NULL;
	else
		return raw->format;
}

void Surface::load(const string &img, bool alpha, const string &skin) {
	free();

	string skinpath;
	if (!skin.empty() && !img.empty() && img[0]!='/') {
		skinpath = "skins/"+skin+"/"+img;
		if (!fileExists(skinpath))
			skinpath = "skins/Default/"+img;
	} else {
		skinpath = img;
	}

	raw = IMG_Load(skinpath.c_str());
	if (raw != NULL) {
		if (alpha)
			enableAlpha();
	} else {
		ERROR("Couldn't load surface '%s'", img.c_str());
	}
}

void Surface::lock() {
	if ( SDL_MUSTLOCK(raw) && !locked ) {
		if ( SDL_LockSurface(raw) < 0 ) {
			ERROR("Can't lock surface: '%s'", SDL_GetError());
			SDL_Quit();
		}
		locked = true;
	}
}

void Surface::unlock() {
	if ( SDL_MUSTLOCK(raw) && locked ) {
		SDL_UnlockSurface(raw);
		locked = false;
	}
}

void Surface::flip() {
	// if (dblbuffer != NULL) {
		// this->blit(dblbuffer,0,0);
		// SDL_Flip(dblbuffer);
	// } else 
	{
#if defined(TARGET_RS97) || defined(TARGET_MIYOO)
	SDL_SoftStretch(raw, NULL, ScreenSurface, NULL);
	SDL_Flip(ScreenSurface);
#else
	SDL_Flip(raw);
#endif
	}
}

void Surface::putPixel(int x, int y, RGBAColor color) {
	putPixel(x,y, SDL_MapRGBA( raw->format , color.r , color.g , color.b, color.a ));
}

void Surface::putPixel(int x, int y, uint32_t color) {
	//determine position
	char* pPosition = ( char* ) raw->pixels ;
	//offset by y
	pPosition += ( raw->pitch * y ) ;
	//offset by x
	pPosition += ( raw->format->BytesPerPixel * x ) ;
	//copy pixel data
	memcpy ( pPosition , &color , raw->format->BytesPerPixel ) ;
}

RGBAColor Surface::pixelColor(int x, int y) {
	RGBAColor color;
	uint32_t col = pixel(x,y);
	SDL_GetRGBA( col, raw->format, &color.r, &color.g, &color.b, &color.a );
	return color;
}

uint32_t Surface::pixel(int x, int y) {
	//determine position
	char* pPosition = ( char* ) raw->pixels ;
	//offset by y
	pPosition += ( raw->pitch * y ) ;
	//offset by x
	pPosition += ( raw->format->BytesPerPixel * x ) ;
	//copy pixel data
	uint32_t col = 0;
	memcpy ( &col , pPosition , raw->format->BytesPerPixel ) ;
	return col;
}

void Surface::blendAdd(Surface *target, int x, int y) {
	RGBAColor targetcol, blendcol;
	for (int iy = 0; iy < raw->h; iy++)
		if (iy + y >= 0 && iy + y < target->raw->h)
			for (int ix = 0; ix < raw->w; ix++) {
				if (ix + x >= 0 && ix + x < target->raw->w) {
					blendcol = pixelColor(ix,iy);
					targetcol = target->pixelColor(ix + x, iy + y);
					targetcol.r = min(targetcol.r + blendcol.r, 255);
					targetcol.g = min(targetcol.g + blendcol.g, 255);
					targetcol.b = min(targetcol.b + blendcol.b, 255);
					target->putPixel(ix + x, iy + y, targetcol);
				}
			}

/*
	uint32_t bcol, tcol;
	char *pPos, *tpPos;
	for (int iy=0; iy<raw->h; iy++)
		if (iy+y >= 0 && iy+y < target->raw->h) {
			pPos = (char*)raw->pixels + raw->pitch*iy;
			tpPos = (char*)target->raw->pixels + target->raw->pitch*(iy+y);

			for (int ix=0; ix<raw->w; ix++) {
				memcpy(&bcol, pPos, raw->format->BytesPerPixel);
				memcpy(&tcol, tpPos, target->raw->format->BytesPerPixel);
				//memcpy(tpPos, &bcol, target->raw->format->BytesPerPixel);
				pPos += raw->format->BytesPerPixel;
				tpPos += target->raw->format->BytesPerPixel;
				target->putPixel(ix+x,iy+y,bcol);
			}
		}
*/
}

void Surface::write(FontHelper *font, const string &text, int x, int y, const uint8_t align) {
	font->write(this, text, x, y, align);
}
void Surface::write(FontHelper *font, const string &text, int x, int y, const uint8_t align, RGBAColor fgColor, RGBAColor bgColor) {
	font->write(this, text, x, y, align, fgColor, bgColor);
}

void Surface::operator = (SDL_Surface *s) {
	raw = SDL_DisplayFormat(s);
	halfW = raw->w/2;
	halfH = raw->h/2;
}

void Surface::operator = (Surface *s) {
	this->operator =(s->raw);
}

void Surface::box(SDL_Rect re, RGBAColor c) {
	if (c.a == 255) {
		SDL_FillRect(raw, &re, c.pixelValue(raw->format));
	} else if (c.a != 0) {
		fillRectAlpha(re, c);
	}
}
void Surface::applyClipRect(SDL_Rect& rect) {
	SDL_Rect clip;
	SDL_GetClipRect(raw, &clip);

	// Clip along X-axis.
	if (rect.x < clip.x) {
		rect.w = max(rect.x + rect.w - clip.x, 0);
		rect.x = clip.x;
	}
	if (rect.x + rect.w > clip.x + clip.w) {
		rect.w = max(clip.x + clip.w - rect.x, 0);
	}

	// Clip along Y-axis.
	if (rect.y < clip.y) {
		rect.h = max(rect.y + rect.h - clip.y, 0);
		rect.y = clip.y;
	}
	if (rect.y + rect.h > clip.y + clip.h) {
		rect.h = max(clip.y + clip.h - rect.y, 0);
	}
}
static inline uint32_t mult8x4(uint32_t c, uint8_t a) {
	return ((((c >> 8) & 0x00FF00FF) * a) & 0xFF00FF00) | ((((c & 0x00FF00FF) * a) & 0xFF00FF00) >> 8);
}

void Surface::fillRectAlpha(SDL_Rect rect, RGBAColor c) {
	applyClipRect(rect);
	if (rect.w == 0 || rect.h == 0) {
		// Entire rectangle is outside clipping area.
		return;
	}

	if (SDL_MUSTLOCK(raw)) {
		if (SDL_LockSurface(raw) < 0) {
			return;
		}
	}

	SDL_PixelFormat *format = raw->format;
	uint32_t color = c.pixelValue(format);
	uint8_t alpha = c.a;

	uint8_t* edge = static_cast<uint8_t*>(raw->pixels)
				   + rect.y * raw->pitch
				   + rect.x * format->BytesPerPixel;

	// Blending: surf' = surf * (1 - alpha) + fill * alpha

	if (format->BytesPerPixel == 2) {
		uint32_t Rmask = format->Rmask;
		uint32_t Gmask = format->Gmask;
		uint32_t Bmask = format->Bmask;

		// Pre-multiply the fill color. We're hardcoding alpha to 1: 15/16bpp
		// modes are unlikely to have an alpha channel and even if they do,
		// the written alpha isn't used by gmenu2x.
		uint16_t f = (((color & Rmask) * alpha >> 8) & Rmask)
				   | (((color & Gmask) * alpha >> 8) & Gmask)
				   | (((color & Bmask) * alpha >> 8) & Bmask)
				   | format->Amask;
		alpha = 255 - alpha;

		for (auto y = 0; y < rect.h; y++) {
			for (auto x = 0; x < rect.w; x++) {
				uint16_t& pixel = reinterpret_cast<uint16_t*>(edge)[x];
				uint32_t R = ((pixel & Rmask) * alpha >> 8) & Rmask;
				uint32_t G = ((pixel & Gmask) * alpha >> 8) & Gmask;
				uint32_t B = ((pixel & Bmask) * alpha >> 8) & Bmask;
				pixel = uint16_t(R | G | B) + f;
			}
			edge += raw->pitch;
		}
	} else if (format->BytesPerPixel == 4) {
		// Assume the pixel format uses 8 bits per component; we don't care
		// which component is where since they all blend the same.
		uint32_t f = mult8x4(color, alpha); // pre-multiply the fill color
		alpha = 255 - alpha;

		for (auto y = 0; y < rect.h; y++) {
			for (auto x = 0; x < rect.w; x++) {
				uint32_t& pixel = reinterpret_cast<uint32_t*>(edge)[x];
				pixel = mult8x4(pixel, alpha) + f;
			}
			edge += raw->pitch;
		}
	} else {
		assert(false);
	}

	if (SDL_MUSTLOCK(raw)) {
		SDL_UnlockSurface(raw);
	}
}

void Surface::rectangle(SDL_Rect re, RGBAColor c) {
	if (re.h >= 1) {
		// Top.
		box(SDL_Rect { re.x, re.y, re.w, 1 }, c);
	}
	if (re.h >= 2) {
		Sint16 ey = re.y + re.h - 1;
		// Bottom.
		box(SDL_Rect { re.x, ey, re.w, 1 }, c);

		Sint16 ex = re.x + re.w - 1;
		Sint16 sy = re.y + 1;
		Uint16 sh = re.h - 2;
		// Left.
		if (re.w >= 1) {
			box(SDL_Rect { re.x, sy, 1, sh }, c);
		}
		// Right.
		if (re.w >= 2) {
			box(SDL_Rect { ex, sy, 1, sh }, c);
		}
	}
}

void Surface::clearClipRect() {
	SDL_SetClipRect(raw, NULL);
}

void Surface::setClipRect(SDL_Rect rect) {
	SDL_SetClipRect(raw, &rect);
}

bool Surface::blit(Surface *destination, int x, int y, const uint8_t align, uint8_t alpha) {
	if (align & HAlignCenter) {
		x -= raw->w / 2;
	} else if (align & HAlignRight) {
		x = raw->w;
	}

	if (align & VAlignMiddle) {
		y -= raw->h / 2;
	} else if (align & VAlignBottom) {
		y = raw->h;
	}

	return blit(destination, {x, y, raw->w, raw->h}, HAlignLeft | VAlignTop, alpha);
}

bool Surface::blit(Surface *destination, SDL_Rect destrect, const uint8_t align, uint8_t alpha) {
	if (destination->raw == NULL || alpha == 0) return false;

	SDL_Rect srcrect = {0, 0, destrect.w, destrect.h};

	if (align & HAlignCenter) {
		srcrect.x = (raw->w - srcrect.w) / 2;
	} else if (align & HAlignRight) {
		srcrect.x = raw->w - srcrect.w;
	}

	if (align & VAlignMiddle) {
		srcrect.y = (raw->h - srcrect.h) / 2;
	} else if (align & VAlignBottom) {
		srcrect.y = raw->h - srcrect.h;
	}

	if (alpha > 0 && alpha != raw->format->alpha)
		SDL_SetAlpha(raw, SDL_SRCALPHA | SDL_RLEACCEL, alpha);

	return SDL_BlitSurface(raw, &srcrect, destination->raw, &destrect);
}

void Surface::softStretch(uint16_t x, uint16_t y, bool keep_aspect, bool maximize) {
	if (!maximize && raw->w <= x && raw->h <= y) return;
	if (keep_aspect) {
		if (x < y) {
			y = x * raw->h / raw->w;
		} else {
			x = y * raw->h / raw->w;
		}
	}

	Surface *thisSurface = new Surface(this);
	Surface *outSurface = new Surface(x, y);
	SDL_BlitSurface(raw, NULL, thisSurface->raw, NULL);
	SDL_SoftStretch(thisSurface->raw, NULL, outSurface->raw, NULL);
	raw = outSurface->raw;
}

// Changes a surface's alpha value, by altering per-pixel alpha if necessary.
void Surface::setAlpha(uint8_t alpha) {
	SDL_PixelFormat* fmt = raw->format;

	if( fmt->Amask == 0 ) { // If surface has no alpha channel, just set the surface alpha.
		SDL_SetAlpha(raw, SDL_SRCALPHA, alpha);
	} else { // Else change the alpha of each pixel.
		unsigned bpp = fmt->BytesPerPixel;
		float scale = alpha / 255.0f; // Scaling factor to clamp alpha to [0, alpha].

		SDL_LockSurface(raw);

		for (int y = 0; y < raw->h; ++y) {
			for (int x = 0; x < raw->w; ++x) {
				// Get a pointer to the current pixel.
				Uint32* pixel_ptr = (Uint32 *)( 
						(Uint8 *)raw->pixels
						+ y * raw->pitch
						+ x * bpp
						);
	
				Uint8 r, g, b, a;
				SDL_GetRGBA( *pixel_ptr, fmt, &r, &g, &b, &a ); // Get the old pixel components.
				*pixel_ptr = SDL_MapRGBA( fmt, r, g, b, scale * a ); // Set the pixel with the new alpha.
			}
		}
		SDL_UnlockSurface(raw);
	}
}
