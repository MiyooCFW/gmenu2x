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
#include "menusettingstringbase.h"
#include "gmenu2x.h"
#include "debug.h"

using std::string;

MenuSettingStringBase::MenuSettingStringBase(GMenu2X *gmenu2x, const string &title, const string &description, string *value):
MenuSetting(gmenu2x, title, description) , originalValue(*value) , _value(value) {}

MenuSettingStringBase::~MenuSettingStringBase() {}

void MenuSettingStringBase::draw(int y) {
	MenuSetting::draw(y);
	gmenu2x->s->write(gmenu2x->font, value(), 155, y + gmenu2x->font->getHalfHeight(), VAlignMiddle);
}

uint32_t MenuSettingStringBase::manageInput() {
	if (gmenu2x->input[MENU]) clear();
	if (gmenu2x->input[CONFIRM]) edit();
	return 0; // SD_NO_ACTION
}

void MenuSettingStringBase::clear() {
	setValue("");
}

bool MenuSettingStringBase::edited() {
	return originalValue != value();
}
