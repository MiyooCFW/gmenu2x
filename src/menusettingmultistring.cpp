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
#include "menusettingmultistring.h"
#include "gmenu2x.h"
#include "iconbutton.h"
#include <algorithm>
using std::find;

MenuSettingMultiString::MenuSettingMultiString(GMenu2X *gmenu2x, const string &title, const string &description, string *value, const vector<string> *choices, msms_onchange_t onChange, msms_onselect_t onSelect):
MenuSettingStringBase(gmenu2x, title, description, value), choices(choices), onChange(onChange), onSelect(onSelect) {
	setSel(find(choices->begin(), choices->end(), *value) - choices->begin());

	if (choices->size() > 1) {
		btn = new IconButton(gmenu2x, "dpad", gmenu2x->tr["Change"]);
		// btn->setAction(MakeDelegate(this, &MenuSettingMultiString::incSel));
		buttonBox.add(btn);
	}

	if (this->onSelect) {
		btn = new IconButton(gmenu2x, "a", gmenu2x->tr["Open"]);
		// btn->setAction(MakeDelegate(this, &MenuSettingMultiString::incSel));
		buttonBox.add(btn);
	}
}

uint32_t MenuSettingMultiString::manageInput() {
	if (gmenu2x->input[LEFT]) {
		decSel();
		return this->onChange && this->onChange();
	}
	else if (gmenu2x->input[RIGHT]) {
		incSel();
		return this->onChange && this->onChange();
	}
	else if (gmenu2x->input[CONFIRM] && this->onSelect) {
		this->onSelect();
		return this->onChange && this->onChange();
	}
	else if (gmenu2x->input[MENU]) {
		setSel(0);
		return this->onChange && this->onChange();
	}
	return 0; // SD_NO_ACTION
}

void MenuSettingMultiString::incSel() {
	setSel(selected + 1);
}

void MenuSettingMultiString::decSel() {
	setSel(selected - 1);
}

void MenuSettingMultiString::setSel(int sel) {
	if (sel < 0) {
		sel = choices->size()-1;
	} else if (sel >= (int)choices->size()) {
		sel = 0;
	}
	selected = sel;

	setValue((*choices)[sel]);
}

void MenuSettingMultiString::draw(int y) {
	MenuSetting::draw(y);

	int w = 0;
	if (value() == "ON" || value() == "AUTO" || value() == "OFF") {
		w = gmenu2x->font->getHeight()/2.5;
		RGBAColor color = (RGBAColor){255, 0, 0, 255};
		if (value() == "ON" || value() == "AUTO") color = (RGBAColor) {0, 255, 0, 255};
		gmenu2x->s->box(155, y + 1, w, gmenu2x->font->getHeight() - 2, color);
		gmenu2x->s->rectangle(155, y + 1, w, gmenu2x->font->getHeight() - 2, 0, 0, 0, 255);
		w += 2;
	}
	gmenu2x->s->write(gmenu2x->font, value(), 155 + w, y + gmenu2x->font->getHalfHeight(), VAlignMiddle);
}
