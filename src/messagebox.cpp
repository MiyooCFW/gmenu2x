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
#include "messagebox.h"
#include "powermanager.h"
#include "debug.h"

using namespace std;

MessageBox::MessageBox(GMenu2X *gmenu2x, vector<MenuOption> options):
gmenu2x(gmenu2x) {
	bool inputAction = false;
	int32_t selected = 0;
	uint32_t i, fadeAlpha = 0, h = gmenu2x->font->getHeight(), h2 = gmenu2x->font->getHalfHeight();
	SDL_Rect box;

	Surface bg(gmenu2x->s);

	gmenu2x->input.dropEvents(); // prevent passing input away
	gmenu2x->powerManager->clearTimer();

	box.h = h * options.size() + 8;
	box.w = 0;
	for (i = 0; i < options.size(); i++) {
		box.w = max(gmenu2x->font->getTextWidth(options[i].text), box.w);
	};
	box.w += 23;
	box.x = (gmenu2x->w - box.w) / 2;
	box.y = (gmenu2x->h - box.h) / 2;

	uint32_t tickStart = SDL_GetTicks();
	while (true) {
		if (selected < 0) selected = options.size() - 1;
		if (selected >= options.size()) selected = 0;

		bg.blit(gmenu2x->s, 0, 0);

		gmenu2x->s->box(0, 0, gmenu2x->w, gmenu2x->h, 0,0,0, fadeAlpha);
		gmenu2x->s->box(box.x, box.y, box.w, box.h, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BG]);
		gmenu2x->s->rectangle(box.x + 2, box.y + 2, box.w - 4, box.h - 4, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BORDER]);

		// draw selection rect
		gmenu2x->s->box(box.x + 4, box.y + 4 + h * selected, box.w - 8, h, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_SELECTION]);
		for (i = 0; i < options.size(); i++)
			gmenu2x->s->write(gmenu2x->font, options[i].text, box.x + 12, box.y + h2 + 3 + h * i, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

		gmenu2x->s->flip();

		if (fadeAlpha < 200) {
			fadeAlpha = intTransition(0, 200, tickStart, 200);
			continue;
		}

		do {
			inputAction = gmenu2x->input.update();

			if (gmenu2x->inputCommonActions(inputAction)) continue;

			if (gmenu2x->input[MENU] || gmenu2x->input[CANCEL]) return;
			else if (gmenu2x->input[UP]) selected--;
			else if (gmenu2x->input[DOWN]) selected++;
			else if (gmenu2x->input[LEFT] || gmenu2x->input[PAGEUP]) selected = 0;
			else if (gmenu2x->input[RIGHT] || gmenu2x->input[PAGEDOWN]) selected = (int)options.size() - 1;
			else if (gmenu2x->input[SETTINGS] || gmenu2x->input[CONFIRM]) {
				options[selected].action();
				return;
			}
		} while (!inputAction);
	}
}

MessageBox::MessageBox(GMenu2X *gmenu2x, const string &text, const string &icon):
gmenu2x(gmenu2x), text(text), icon(icon) {
	buttonText.resize(19);
	button.resize(19);
	buttonPosition.resize(19);

	bool anyaction = false;
	for (uint32_t x = 0; x < buttonText.size(); x++) {
		buttonText[x] = "";
		button[x] = "";
		buttonPosition[x].h = gmenu2x->font->getHeight();
		anyaction = true;
	}

	if (!anyaction)
		// Default enabled button
		buttonText[CONFIRM] = "OK";

	// Default labels
	button[UP] = "up";
	button[DOWN] = "down";
	button[LEFT] = "left";
	button[RIGHT] = "right";
	button[MODIFIER] = "x";
	button[CONFIRM] = "a";
	button[CANCEL] = "b";
	button[MANUAL] = "y";
	button[DEC] = "x";
	button[INC] = "y";
	button[SECTION_PREV] = "l";
	button[SECTION_NEXT] = "r";
	button[PAGEUP] = "l";
	button[PAGEDOWN] = "r";
	button[SETTINGS] = "start";
	button[MENU] = "select";
	button[VOLUP] = "vol+";
	button[VOLDOWN] = "vol-";
}

MessageBox::~MessageBox() {
	gmenu2x->input.dropEvents(); // prevent passing input away
	gmenu2x->powerManager->resetSuspendTimer();
	clearTimer();
}

void MessageBox::setButton(int action, const string &btn) {
	buttonText[action] = btn;
}

void MessageBox::setAutoHide(uint32_t autohide) {
	this->autohide = autohide;
}

void MessageBox::setBgAlpha(uint32_t bgalpha) {
	this->bgalpha = bgalpha;
}

int MessageBox::exec() {
	int fadeAlpha = 0, ix = 0;
	SDL_Rect box;

	Surface bg(gmenu2x->s);

	gmenu2x->input.dropEvents(); // prevent passing input away
	gmenu2x->powerManager->clearTimer();

	Surface *icn = gmenu2x->sc.add(icon, icon + "mb");

	box.h = gmenu2x->font->getTextHeight(text) * gmenu2x->font->getHeight() + gmenu2x->font->getHeight();
	if (icn != NULL && box.h < 40) box.h = 48;

	box.w = gmenu2x->font->getTextWidth(text) + 24;

	for (uint32_t i = 0; i < buttonText.size(); i++) {
		if (!buttonText[i].empty())
			ix += gmenu2x->font->getTextWidth(buttonText[i]) + 24;
	}
	ix += 6;

	if (ix > box.w) box.w = ix;

	ix = (icn != NULL ? 42 : 0);

	box.w += ix;
	box.x = (gmenu2x->w - box.w) / 2 - 2;
	box.y = (gmenu2x->h - box.h) / 2 - 2;

	uint32_t tickStart = SDL_GetTicks();
	do {
		bg.blit(gmenu2x->s, 0, 0);

		// Darken background
		gmenu2x->s->box(0, 0, gmenu2x->w, gmenu2x->h, 0,0,0, fadeAlpha);

		fadeAlpha = intTransition(0, bgalpha, tickStart, 200);

		// outer box
		gmenu2x->s->box(box, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BG]);

		// draw inner rectangle
		gmenu2x->s->rectangle(box.x + 2, box.y + 2, box.w - 4, box.h - 4, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BORDER]);

		// icon+text
		if (icn != NULL) {
			gmenu2x->s->setClipRect({box.x + 8, box.y + 8, 32, 32});

			if (icn->width() > 32 || icn->height() > 32)
				icn->softStretch(32, 32, SScaleFit);

			icn->blit(gmenu2x->s, box.x + 24, box.y + 24, HAlignCenter | VAlignMiddle);
			gmenu2x->s->clearClipRect();
		}

		// gmenu2x->s->box(ix + box.x, box.y, (box.w - ix), box.h, strtorgba("ffff00ff"));
		gmenu2x->s->write(gmenu2x->font, text, ix + box.x + (box.w - ix) / 2, box.y + box.h / 2, HAlignCenter | VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

		if (!this->autohide) {
			// draw buttons rectangle
			gmenu2x->s->box(box.x, box.y+box.h, box.w, gmenu2x->font->getHeight(), gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BG]);

			int btnX = (gmenu2x->w + box.w) / 2 - 6;
			for (uint32_t i = 0; i < buttonText.size(); i++) {
				if (!buttonText[i].empty()) {
					buttonPosition[i].y = box.y+box.h+gmenu2x->font->getHalfHeight();
					buttonPosition[i].w = btnX;

					btnX = gmenu2x->drawButtonRight(gmenu2x->s, button[i], buttonText[i], btnX, buttonPosition[i].y);

					buttonPosition[i].x = btnX;
					buttonPosition[i].w = buttonPosition[i].x-btnX-6;
				}
			}
		}

		gmenu2x->s->flip();
	} while (fadeAlpha < bgalpha);

	if (this->autohide) {
		SDL_Delay(this->autohide);
		// gmenu2x->powerManager->resetSuspendTimer(); // prevent immediate suspend
		return -1;
	}

	while (true) {
		// touchscreen
		// if (gmenu2x->f200 && gmenu2x->ts.poll()) {
		// 	for (uint32_t i = 0; i < buttonText.size(); i++) {
		// 		if (buttonText[i] != "" && gmenu2x->ts.inRect(buttonPosition[i])) {
		// 			result = i;
		// 			break;
		// 		}
		// 	}
		// }

		bool inputAction = gmenu2x->input.update();
		if (inputAction) {
			// if (gmenu2x->inputCommonActions(inputAction)) continue; // causes power button bounce
			for (uint32_t i = 0; i < buttonText.size(); i++) {
				if (buttonText[i] != "" && gmenu2x->input[i]) {
					return i;
					break;
				}
			}
		}
	}

	return -1;
}

void MessageBox::exec(uint32_t timeOut) {
	clearTimer();
	popupTimer = SDL_AddTimer(timeOut, execTimer, this);
}

void MessageBox::clearTimer() {
	SDL_RemoveTimer(popupTimer); popupTimer = NULL;
}

uint32_t MessageBox::execTimer(uint32_t interval, void *param) {
	MessageBox *mb = reinterpret_cast<MessageBox *>(param);
	mb->clearTimer();
	mb->exec();
	return 0;
}
