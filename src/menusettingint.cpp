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
#include "menusettingint.h"
#include "gmenu2x.h"
#include "utilities.h"

#include <sstream>

using std::string;
using std::stringstream;
using fastdelegate::MakeDelegate;

MenuSettingInt::MenuSettingInt(GMenu2X *gmenu2x, const string &title, const string &description, int *value, int def, int min, int max, int delta):
MenuSetting(gmenu2x, title, description), _value(value), def(def), min(min), max(max), delta(delta), off(false) {
	originalValue = *value;
	setValue(evalIntConf(value, def, min, max));

	//Delegates
	// ButtonAction actionInc = MakeDelegate(this, &MenuSettingInt::inc);
	// ButtonAction actionDec = MakeDelegate(this, &MenuSettingInt::dec);

	btn = new IconButton(gmenu2x, "select", gmenu2x->tr["Reset"]);
	// btn->setAction(MakeDelegate(this, &MenuSettingInt::setDefault));
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "dpad", gmenu2x->tr["Change"]);
	// btn->setAction(actionInc);
	buttonBox.add(btn);
}

void MenuSettingInt::draw(int y) {
	MenuSetting::draw(y);
	
	int w = 0;
	if (off && *_value <= offValue) {
		strvalue = "OFF";
		w = gmenu2x->font->getHeight() / 2.5;
		RGBAColor color = (RGBAColor){255, 0, 0, 255};
		gmenu2x->s->box(155, y + 1, w, gmenu2x->font->getHeight() - 2, color);
		gmenu2x->s->rectangle(155, y + 1, w, gmenu2x->font->getHeight() - 2, 0, 0, 0, 255);
		w += 2;
	}
	gmenu2x->s->write(gmenu2x->font, strvalue, 155 + w, y+gmenu2x->font->getHeight() / 2, VAlignMiddle);
}

uint32_t MenuSettingInt::manageInput() {
	if (gmenu2x->input[LEFT])		dec();
	else if (gmenu2x->input[RIGHT])		inc();
	else if (gmenu2x->input[DEC])		setValue(value() - 10 * delta);
	else if (gmenu2x->input[INC])		setValue(value() + 10 * delta);
	else if (gmenu2x->input[MENU])		setDefault();
	else if (gmenu2x->input[CONFIRM])	current();

	return 0; // SD_NO_ACTION
}

void MenuSettingInt::inc() {
	setValue(value() + delta);
}

void MenuSettingInt::dec() {
	setValue(value() - delta);
}

void MenuSettingInt::current() {
	setValue(*_value);
}

void MenuSettingInt::setValue(int value) {
	if (off && *_value < value && value <= offValue)
		*_value = offValue + 1;
	else if (off && *_value > value && value <= offValue)
		*_value = min;
	else {
		*_value = constrain(value,min,max);
		if (off && *_value <= offValue)
			*_value = min;
	}

	stringstream ss;
	ss << *_value;
	strvalue = "";
	ss >> strvalue;
	gmenu2x->allyTTS(strvalue.c_str(), SLOW_GAP_TTS, SLOW_SPEED_TTS, 0);
}

void MenuSettingInt::setDefault() {
	setValue(def);
}

int MenuSettingInt::value() {
	return *_value;
}

bool MenuSettingInt::edited() {
	return originalValue != value();
}

MenuSettingInt *MenuSettingInt::setOff(int value) {
	off = true;
	offValue = constrain(value,min,max);
	return this;
}
