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

#include "settingsdialog.h"
#include "messagebox.h"

using namespace std;

SettingsDialog::SettingsDialog(
		GMenu2X *gmenu2x_, Touchscreen &ts_,
		const string &title, const string &icon)
	: Dialog(gmenu2x_)
	, ts(ts_)
	, title(title)
{
	if (icon != "" && gmenu2x->sc[icon] != NULL)
		this->icon = icon;
	else
		this->icon = "icons/generic.png";
}

SettingsDialog::~SettingsDialog() {
	for (uint32_t i = 0; i < voices.size(); i++)
		delete voices[i];
}

bool SettingsDialog::exec() {
	bool ts_pressed = false, inputAction = false;
	uint32_t i, iY, firstElement = 0, action = SD_NO_ACTION, rowHeight, numRows;
	voices[selected]->adjustInput();

	while (!close) {
		gmenu2x->initLayout();
		gmenu2x->font->setSize(gmenu2x->skinConfInt["fontSize"])->setColor(gmenu2x->skinConfColors[COLOR_FONT])->setOutlineColor(gmenu2x->skinConfColors[COLOR_FONT_OUTLINE]);
		gmenu2x->titlefont->setSize(gmenu2x->skinConfInt["fontSizeTitle"])->setColor(gmenu2x->skinConfColors[COLOR_FONT_ALT])->setOutlineColor(gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

		rowHeight = gmenu2x->font->getHeight() + 1;
		numRows = (gmenu2x->listRect.h - 2)/rowHeight - 1;

		this->bg->blit(gmenu2x->s,0,0);

		// redraw to due to realtime skin
		drawTopBar(gmenu2x->s, title, voices[selected]->getDescription(), icon);
		drawBottomBar(gmenu2x->s);
		gmenu2x->s->box(gmenu2x->listRect, gmenu2x->skinConfColors[COLOR_LIST_BG]);

		//Selection
		if (selected >= firstElement + numRows) firstElement = selected - numRows;
		if (selected < firstElement) firstElement = selected;

		iY = gmenu2x->listRect.y + 1;
		for (i = firstElement; i < voices.size() && i <= firstElement + numRows; i++, iY += rowHeight) {
			if (i == selected) {
				gmenu2x->s->box(gmenu2x->listRect.x, iY, gmenu2x->listRect.w, rowHeight, gmenu2x->skinConfColors[COLOR_SELECTION_BG]);
				voices[selected]->drawSelected(iY);
			}
			voices[i]->draw(iY);
		}

		gmenu2x->drawScrollBar(numRows, voices.size(), firstElement, gmenu2x->listRect);

		gmenu2x->s->flip();
		do {
			inputAction = gmenu2x->input.update();
			if (gmenu2x->inputCommonActions(inputAction)) continue;

			action = SD_NO_ACTION;
			if ( gmenu2x->input[SETTINGS] ) action = SD_ACTION_SAVE;
			else if ( gmenu2x->input[CANCEL] && allowCancel) action = SD_ACTION_CLOSE;
			else if ( gmenu2x->input[UP      ] ) action = SD_ACTION_UP;
			else if ( gmenu2x->input[DOWN    ] ) action = SD_ACTION_DOWN;
			else if ( gmenu2x->input[PAGEUP  ] ) action = SD_ACTION_PAGEUP;
			else if ( gmenu2x->input[PAGEDOWN] ) action = SD_ACTION_PAGEDOWN;
			else action = voices[selected]->manageInput();
			switch (action) {
				case SD_ACTION_SAVE:
					save = true;
					close = true;
					break;
				case SD_ACTION_CLOSE:
					close = true;
					if (allowCancel) {
						if (edited()) {
							MessageBox mb(gmenu2x, gmenu2x->tr["Save changes?"], this->icon);
							mb.setButton(CONFIRM, gmenu2x->tr["Yes"]);
							mb.setButton(CANCEL,  gmenu2x->tr["No"]);
							save = (mb.exec() == CONFIRM);
						}}
					break;
				case SD_ACTION_UP:
					selected -= 1;
					if (selected < 0) selected = voices.size() - 1;
					gmenu2x->setInputSpeed();
					voices[selected]->adjustInput();
					break;
				case SD_ACTION_DOWN:
					selected += 1;
					if (selected >= voices.size()) selected = 0;
					gmenu2x->setInputSpeed();
					voices[selected]->adjustInput();
					break;
				case SD_ACTION_PAGEUP:
					selected -= numRows;
					if (selected < 0) selected = 0;
					break;
				case SD_ACTION_PAGEDOWN:
					selected += numRows;
					if (selected >= voices.size()) selected = voices.size() - 1;
					break;
			}
		} while (!inputAction);
	}

	gmenu2x->setInputSpeed();

	return true;
}

void SettingsDialog::addSetting(MenuSetting* set) {
	voices.push_back(set);
}

bool SettingsDialog::edited() {
	for (uint32_t i = 0; i < voices.size(); i++)
		if (voices[i]->edited()) return true;
	return false;
}
