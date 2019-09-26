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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "messagebox.h"
#include "powermanager.h"
// #include "debug.h"

using namespace std;

MessageBox::MessageBox(GMenu2X *gmenu2x, const string &text, const string &icon) {
	this->gmenu2x = gmenu2x;
	this->text = text;
	this->icon = icon;
	this->autohide = 0;
	this->bgalpha = 200;

	buttons.resize(19);
	buttonLabels.resize(19);
	buttonPositions.resize(19);
	for (uint32_t x = 0; x < buttons.size(); x++) {
		buttons[x] = "";
		buttonLabels[x] = "";
		buttonPositions[x].h = gmenu2x->font->getHeight();
	}

	//Default enabled button
	buttons[CONFIRM] = "OK";

	//Default labels
	buttonLabels[UP] = "up";
	buttonLabels[DOWN] = "down";
	buttonLabels[LEFT] = "left";
	buttonLabels[RIGHT] = "right";
	buttonLabels[MODIFIER] = "a";
#if defined(TARGET_RS97) || defined(TARGET_MIYOO)
	buttonLabels[CONFIRM] = "a";
	buttonLabels[CANCEL] = "b";
#else
	buttonLabels[CONFIRM] = "b";
	buttonLabels[CANCEL] = "x";
#endif
	buttonLabels[MANUAL] = "y";
	buttonLabels[DEC] = "x";
	buttonLabels[INC] = "y";
	buttonLabels[SECTION_PREV] = "l";
	buttonLabels[SECTION_NEXT] = "r";
	buttonLabels[PAGEUP] = "l";
	buttonLabels[PAGEDOWN] = "r";
	buttonLabels[SETTINGS] = "start";
	buttonLabels[MENU] = "select";
	buttonLabels[VOLUP] = "vol+";
	buttonLabels[VOLDOWN] = "vol-";
}

void MessageBox::setButton(int action, const string &btn) {
	buttons[action] = btn;
}

void MessageBox::setAutoHide(int autohide) {
	this->autohide = autohide;
}

void MessageBox::setBgAlpha(bool bgalpha) {
	this->bgalpha = bgalpha;
}

int MessageBox::exec() {
	int result = -1;

	gmenu2x->powerManager->clearTimer();

	// Surface bg(gmenu2x->s);
	//Darken background
	gmenu2x->s->box((SDL_Rect){0, 0, gmenu2x->resX, gmenu2x->resY}, (RGBAColor){0,0,0,bgalpha});

	SDL_Rect box;
	box.h = gmenu2x->font->getTextHeight(text) * gmenu2x->font->getHeight() + gmenu2x->font->getHeight();
	if (gmenu2x->sc[icon] != NULL && box.h < 40) box.h = 48;
	box.w = gmenu2x->font->getTextWidth(text) + 24 + (gmenu2x->sc[icon] != NULL ? 37 : 0);
	box.x = gmenu2x->halfX - box.w/2 - 2;
	box.y = gmenu2x->halfY - box.h/2 - 2;

	//outer box
	gmenu2x->s->box(box, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BG]);
	
	//draw inner rectangle
	gmenu2x->s->rectangle(box.x+2, box.y+2, box.w-4, box.h-4, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BORDER]);

	//icon+text
	if (gmenu2x->sc[icon] != NULL)
		gmenu2x->sc[icon]->blit( gmenu2x->s, box.x + 24, box.y + 24 , HAlignCenter | VAlignMiddle);

	gmenu2x->s->write(gmenu2x->font, text, box.x+(gmenu2x->sc[icon] != NULL ? 47 : 11), gmenu2x->halfY - gmenu2x->font->getHeight()/5, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

	if (this->autohide) {
		gmenu2x->s->flip();
		SDL_Delay(this->autohide);
		gmenu2x->powerManager->resetSuspendTimer(); // = SDL_GetTicks(); // prevent immediate suspend
		return -1;
	}
	//draw buttons rectangle
	gmenu2x->s->box(box.x, box.y+box.h, box.w, gmenu2x->font->getHeight(), gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BG]);

	int btnX = gmenu2x->halfX+box.w/2-6;
	for (uint32_t i = 0; i < buttons.size(); i++) {
		if (buttons[i] != "") {
			buttonPositions[i].y = box.y+box.h+gmenu2x->font->getHalfHeight();
			buttonPositions[i].w = btnX;

			btnX = gmenu2x->drawButtonRight(gmenu2x->s, buttonLabels[i], buttons[i], btnX, buttonPositions[i].y);

			buttonPositions[i].x = btnX;
			buttonPositions[i].w = buttonPositions[i].x-btnX-6;
		}
	}
	gmenu2x->s->flip();

	while (result < 0) {
		//touchscreen
		if (gmenu2x->f200 && gmenu2x->ts.poll()) {
			for (uint32_t i = 0; i < buttons.size(); i++) {
				if (buttons[i] != "" && gmenu2x->ts.inRect(buttonPositions[i])) {
					result = i;
					break;
				}
			}
		}

		bool inputAction = gmenu2x->input.update();
		if (inputAction) {
			// if (gmenu2x->inputCommonActions(inputAction)) continue; // causes power button bounce
			for (uint32_t i = 0; i < buttons.size(); i++) {
				if (buttons[i] != "" && gmenu2x->input[i]) {
					result = i;
					break;
				}
			}
		}
	}

	gmenu2x->input.dropEvents(); // prevent passing input away
	gmenu2x->powerManager->resetSuspendTimer();
	return result;
}
