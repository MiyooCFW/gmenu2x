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
#include "menusetting.h"
#include "gmenu2x.h"

MenuSetting::MenuSetting(GMenu2X *gmenu2x, const string &title, const string &description):
gmenu2x(gmenu2x), buttonBox(gmenu2x), title(title), description(description) {
	btn = new IconButton(gmenu2x, "start", gmenu2x->tr["Save"]);
	buttonBox.add(btn);
	btn = new IconButton(gmenu2x, "b", gmenu2x->tr["Exit"]);
	buttonBox.add(btn);
}

MenuSetting::~MenuSetting() {}

void MenuSetting::draw(int y) {
	gmenu2x->s->write(gmenu2x->font, title, 5, y + gmenu2x->font->getHalfHeight(), VAlignMiddle);
}

void MenuSetting::handleTS() {
	buttonBox.handleTS();
}

void MenuSetting::adjustInput() {}

void MenuSetting::drawSelected(int /*y*/) {
	buttonBox.paint(5);
}
