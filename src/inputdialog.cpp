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

#include "inputdialog.h"
#include "messagebox.h"
#include "debug.h"

using namespace std;
using namespace fastdelegate;

InputDialog::InputDialog(GMenu2X *gmenu2x, const string &text, const string &startvalue, const string &title, const string &icon):
gmenu2x(gmenu2x) {
	gmenu2x->input.dropEvents(); // prevent passing input away

	if (title == "") {
		this->title = text;
		this->text = "";
	} else {
		this->title = title;
		this->text = text;
	}
	this->icon = "";
	if (icon != "" && gmenu2x->sc[icon] != NULL)
		this->icon = icon;

	input = startvalue;
	selCol = 0;
	selRow = 0;
	keyboard.resize(MAX_KB + 1);

	kb11 = "qwertyuiop-789";
	kb12 = "asdfghjkl\\/456";
	kb13 = "zxcvbnm_,.0123";
	kb21 = "QWERTYUIOP_-+=";
	kb22 = "ASDFGHJKL@'\"`";
	kb23 = "ZXCVBNM#:;/?";
	kb31 = "¡¿*+-/\\&<=>|";
	kb32 = "()[]{}@#$%^~";
	kb33 = "_\"'`.,:;!?";

	string lang = gmenu2x->tr.lang();

	kbc11 = gmenu2x->tr["_keyboard_t1_l1_"];
	kbc12 = gmenu2x->tr["_keyboard_t1_l2_"];
	kbc13 = gmenu2x->tr["_keyboard_t1_l3_"];
	kbc21 = gmenu2x->tr["_keyboard_t2_l1_"];
	kbc22 = gmenu2x->tr["_keyboard_t2_l2_"];
	kbc23 = gmenu2x->tr["_keyboard_t2_l3_"];
	kbc31 = gmenu2x->tr["_keyboard_t3_l1_"];
	kbc32 = gmenu2x->tr["_keyboard_t3_l2_"];
	kbc33 = gmenu2x->tr["_keyboard_t3_l3_"];

	if (kbc11 == "_keyboard_t1_l1_") kbc11 = kb11;
	if (kbc12 == "_keyboard_t1_l2_") kbc12 = kb12;
	if (kbc13 == "_keyboard_t1_l3_") kbc13 = kb13;
	if (kbc21 == "_keyboard_t2_l1_") kbc21 = kb21;
	if (kbc22 == "_keyboard_t2_l2_") kbc22 = kb22;
	if (kbc23 == "_keyboard_t2_l3_") kbc23 = kb23;
	if (kbc31 == "_keyboard_t3_l1_") kbc31 = kb31;
	if (kbc32 == "_keyboard_t3_l2_") kbc32 = kb32;
	if (kbc33 == "_keyboard_t3_l3_") kbc33 = kb33;

	keyboard[0].push_back(kb11);
	keyboard[0].push_back(kb12);
	keyboard[0].push_back(kb13);

	keyboard[1].push_back(kb21);
	keyboard[1].push_back(kb22);
	keyboard[1].push_back(kb23);

	keyboard[2].push_back(kb31);
	keyboard[2].push_back(kb32);
	keyboard[2].push_back(kb33);

	keyboard[3].push_back(kbc11);
	keyboard[3].push_back(kbc12);
	keyboard[3].push_back(kbc13);

	keyboard[4].push_back(kbc21);
	keyboard[4].push_back(kbc22);
	keyboard[4].push_back(kbc23);

	keyboard[MAX_KB].push_back(kbc31);
	keyboard[MAX_KB].push_back(kbc32);
	keyboard[MAX_KB].push_back(kbc33);

	setKeyboard(0);
}

void InputDialog::setKeyboard(int kb) {
	kb = constrain(kb,0,keyboard.size()-1);
	curKeyboard = kb;
	this->kb = &(keyboard[kb]);
	kbLength = this->kb->at(0).length();

	for (int x = 0, l = kbLength; x < l; x++) {
		if (gmenu2x->font->utf8Code(this->kb->at(0)[x])) {
			kbLength--;
			x++;
		}
	}

	kbLeft = (gmenu2x->w - kbLength * KEY_WIDTH) / 2;
	kbWidth = kbLength * KEY_WIDTH + 3;
	kbHeight = (this->kb->size()) * KEY_HEIGHT + 3;

	kbRect.x = kbLeft - 3;
	kbRect.y = gmenu2x->bottomBarRect.y - kbHeight;
	kbRect.w = kbWidth;
	kbRect.h = kbHeight;
}

bool InputDialog::exec() {
	Surface *bg = new Surface(gmenu2x->s);

	SDL_Rect box = {gmenu2x->listRect.x + 2, 0, gmenu2x->listRect.w - 4, gmenu2x->font->getHeight() + 4};
	box.y = kbRect.y - box.h;

	uint32_t caretTick = 0, curTick;
	bool caretOn = true;

	bg->box(gmenu2x->bottomBarRect, (RGBAColor){0,0,0,255});

	gmenu2x->s->box(gmenu2x->bottomBarRect, gmenu2x->skinConfColors[COLOR_BOTTOM_BAR_BG]);

	gmenu2x->drawButton(bg, "r", gmenu2x->tr["⎵"],
	gmenu2x->drawButton(bg, "l", gmenu2x->tr["←"],
	gmenu2x->drawButton(bg, "y", gmenu2x->tr["Shift"],
	gmenu2x->drawButton(bg, "x", gmenu2x->tr["Alt"],
	gmenu2x->drawButton(bg, "start", gmenu2x->tr["Save"],
	gmenu2x->drawButton(bg, "b", gmenu2x->tr["Exit"]
	))))));

	while (true) {
		SDL_RemoveTimer(wakeUpTimer);
		wakeUpTimer = SDL_AddTimer(500, gmenu2x->input.wakeUp, (void*)false);

		bg->blit(gmenu2x->s,0,0);

		gmenu2x->s->box(gmenu2x->listRect.x, box.y - 2, gmenu2x->listRect.w, box.h + 2, (RGBAColor){0,0,0,220});
		gmenu2x->s->box(box, (RGBAColor){0x33,0x33,0x33,220});
		gmenu2x->s->setClipRect(box);

		gmenu2x->s->write(gmenu2x->font, input, box.x + box.w / 2, box.y + box.h / 2, HAlignCenter | VAlignMiddle, (RGBAColor){0xff,0xff,0xff,0xff}, (RGBAColor){0,0,0,200});

		curTick = SDL_GetTicks();
		if (curTick - caretTick >= 600) {
			caretOn = !caretOn;
			caretTick = curTick;
		}

		if (caretOn) gmenu2x->s->box(box.x + (box.w + gmenu2x->font->getTextWidth(input)) / 2, box.y + 3, 8, box.h - 6, (RGBAColor){0xff,0xff,0xff,220});
		gmenu2x->s->clearClipRect();

		// if (gmenu2x->f200) ts.poll();
		// action =
		drawVirtualKeyboard();
		gmenu2x->s->flip();

		bool inputAction = gmenu2x->input.update();
		if (gmenu2x->inputCommonActions(inputAction)) continue;

		if (gmenu2x->input[CANCEL] || gmenu2x->input[MENU]) return false;
		else if (gmenu2x->input[SETTINGS])		return true;
		else if (gmenu2x->input[UP] || gmenu2x->input.hatEvent(DUP) == DUP)			selRow--;
		else if (gmenu2x->input[DOWN] || gmenu2x->input.hatEvent(DDOWN) == DDOWN)			selRow++;
		else if (gmenu2x->input[LEFT] || gmenu2x->input.hatEvent(DLEFT) == DLEFT)			selCol--;
		else if (gmenu2x->input[RIGHT] || gmenu2x->input.hatEvent(DRIGHT) == DRIGHT)			selCol++;
		else if (gmenu2x->input[CONFIRM])		confirm();
		else if (gmenu2x->input[MANUAL])		changeKeys();
		else if (gmenu2x->input[MODIFIER])		changeKeysCustom();
		else if (gmenu2x->input[SECTION_PREV])	backspace();
		else if (gmenu2x->input[SECTION_NEXT])	space();
	}
}

void InputDialog::backspace() {
	// check for utf8 characters
	input = input.substr(0, input.length() - (gmenu2x->font->utf8Code(input[input.length() - 2]) ? 2 : 1));
}

void InputDialog::space() {
	input += " ";
}

void InputDialog::confirm() {
	bool utf8;
	int xc=0;
	for (uint32_t x = 0; x < kb->at(selRow).length(); x++) {
		utf8 = gmenu2x->font->utf8Code(kb->at(selRow)[x]);
		if (xc == selCol) input += kb->at(selRow).substr(x, utf8 ? 2 : 1);
		if (utf8) x++;
		xc++;
	}
}

void InputDialog::changeKeys() {
	if (curKeyboard >= MAX_KB / 2) setKeyboard(0);
	else setKeyboard(curKeyboard + 1);
}

void InputDialog::changeKeysCustom() {
	if (curKeyboard == 0) setKeyboard(0 + 3);
	else if (curKeyboard == 1) setKeyboard(1 + 3);
	else if (curKeyboard == 2) setKeyboard(2 + 3);
	else if (curKeyboard == 3) setKeyboard(3 - 3);
	else if (curKeyboard == 4) setKeyboard(4 - 3);
	else if (curKeyboard == MAX_KB) setKeyboard(5 - 3);
}

int InputDialog::drawVirtualKeyboard() {
	// int action = ID_NO_ACTION;
	gmenu2x->s->box(gmenu2x->listRect.x, kbRect.y, gmenu2x->listRect.w, kbRect.h, (RGBAColor){0,0,0,220});

	if (selCol < 0) selCol = selRow == (int)kb->size() ? 1 : kbLength - 1;
	if (selCol >= (int)kbLength) selCol = 0;
	if (selRow < 0) selRow = kb->size() - 1;
	if (selRow >= (int)kb->size()) selRow = 0;

	// selection
	// if (selRow < (int)kb->size())
	gmenu2x->s->box(kbLeft + selCol * KEY_WIDTH, kbRect.y + 2 + selRow * KEY_HEIGHT, KEY_WIDTH - 1, KEY_HEIGHT - 2, (RGBAColor){0xff,0xff,0xff,220});

	// keys
	for (uint32_t l = 0; l < kb->size(); l++) {
		string line = kb->at(l);
		for (uint32_t x = 0, xc = 0; x < line.length(); x++) {
			string charX;
			// utf8 characters
			if (gmenu2x->font->utf8Code(line[x])) {
				charX = line.substr(x,2);
				x++;
			} else {
				charX = line[x];
			}

			SDL_Rect re = {kbLeft + xc * KEY_WIDTH, kbRect.y + 2 + l * KEY_HEIGHT, KEY_WIDTH - 1, KEY_HEIGHT - 2};

			// if ts on rect, change selection
			// if (gmenu2x->f200 && ts.pressed() && ts.inRect(re)) {
			// 	selCol = xc;
			// 	selRow = l;
			// }

			gmenu2x->s->rectangle(re, (RGBAColor){0xff,0xff,0xff,220});
			gmenu2x->s->write(gmenu2x->font, charX, kbLeft + xc * KEY_WIDTH + KEY_WIDTH / 2, kbRect.y + 2 + l * KEY_HEIGHT + KEY_HEIGHT / 2 - 2, HAlignCenter | VAlignMiddle, (RGBAColor){0xff,0xff,0xff,0xff}, (RGBAColor){0,0,0,200});
			xc++;
		}
	}

	// if ts released
	// if (gmenu2x->f200 && ts.released() && ts.inRect(kbRect)) {
	// 	action = ID_ACTION_SELECT;
	// }

	return 0; //action;
}

InputDialog::~InputDialog() {
	SDL_RemoveTimer(wakeUpTimer); wakeUpTimer = NULL;
	gmenu2x->input.dropEvents(); // prevent passing input away
}
