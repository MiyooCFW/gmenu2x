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
#include "menusettingmultiint.h"
#include "gmenu2x.h"
#include "utilities.h"

#include <sstream>

using std::string;
using std::stringstream;
using fastdelegate::MakeDelegate;

// MenuSettingMultiInt::MenuSettingMultiInt(GMenu2X *gmenu2x, const string &title, const string &description, int *value, int def, int min, int max)
MenuSettingMultiInt::MenuSettingMultiInt(GMenu2X *gmenu2x, const string &title, const string &description, int *value, int *choice_pointer, int choice_size, int def, int min, int max)
	: MenuSetting(gmenu2x, title, description) {

	_value = value;
	originalValue = *value;
	choices = choice_pointer;
	selection_max = choice_size - 1;

	selection = reverseLookup(*value);
	this->def = choices[reverseLookup(def)];
	this->min = min;
	this->max = max;

	setValue(evalIntConf(choices[selection], def, min, max));
		
	//Delegates
	ButtonAction actionInc = MakeDelegate(this, &MenuSettingMultiInt::inc);
	ButtonAction actionDec = MakeDelegate(this, &MenuSettingMultiInt::dec);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/select.png", gmenu2x->tr["Default"]);
	btn->setAction(MakeDelegate(this, &MenuSettingMultiInt::setDefault));
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/left.png");
	btn->setAction(actionDec);
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/right.png", gmenu2x->tr["Change"]);
	btn->setAction(actionInc);
	buttonBox.add(btn);
}

void MenuSettingMultiInt::draw(int y) {
	MenuSetting::draw(y);
	gmenu2x->s->write( gmenu2x->font, strvalue, 155, y+gmenu2x->font->getHalfHeight(), VAlignMiddle );
}

uint32_t MenuSettingMultiInt::manageInput() {
	if ( gmenu2x->input[LEFT ] ) dec();
	if ( gmenu2x->input[RIGHT] ) inc();
	if ( gmenu2x->input[DEC] ) dec();
	if ( gmenu2x->input[INC] ) inc();
	if ( gmenu2x->input[MENU] ) setDefault();
	return 0; // SD_NO_ACTION
}

int MenuSettingMultiInt::reverseLookup(int value) {
  int output;
  for(int i = 0; i <= selection_max + 1; i++){
	if(choices[i] <= value){
	  output = i;
	} else {
	  break;
	}
  }
  return output;
}

void MenuSettingMultiInt::inc() {
	if(selection < selection_max && choices[selection + 1] <= max){
		selection = selection + 1;
		setValue(choices[selection]);
	}
}

void MenuSettingMultiInt::dec() {
	if(selection > 0 && choices[selection - 1] >= min) {
		selection = selection - 1;
		setValue(choices[selection]);
	}
}

void MenuSettingMultiInt::setValue(int value) {
	*_value = value;
	stringstream ss;
	ss << *_value;
	strvalue = "";
	ss >> strvalue;
}

void MenuSettingMultiInt::setDefault() {
	setValue(def);
}

int MenuSettingMultiInt::value() {
	return *_value;
}

// void MenuSettingMultiInt::adjustInput() {
// 	gmenu2x->input.setInterval(100, LEFT);
// 	gmenu2x->input.setInterval(100, RIGHT);
// 	gmenu2x->input.setInterval(100, DEC);
// 	gmenu2x->input.setInterval(100, INC);
// }

bool MenuSettingMultiInt::edited() {
	return originalValue != value();
}
