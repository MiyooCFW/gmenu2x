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
#include "menu.h"
#include "gmenu2x.h"

using namespace std;

SettingsDialog::SettingsDialog(GMenu2X *gmenu2x, Touchscreen &ts, const string &title, const string &icon):
Dialog(gmenu2x, title, "", icon), ts(ts) {}

SettingsDialog::~SettingsDialog() {
	for (uint32_t i = 0; i < voices.size(); i++)
		delete voices[i];
}

bool SettingsDialog::exec() {
	bool ts_pressed = false, inputAction = false;
	uint32_t i, iY, firstElement = 0, action = SD_NO_ACTION, rowHeight, numRows;
	string readSetting = title + " " + voices[selected]->getTitle() + " " + voices[selected]->getDescription();
	gmenu2x->allyTTS(readSetting.c_str(), MEDIUM_GAP_TTS, MEDIUM_SPEED_TTS, 0);

	while (loop) {
		bool ally = false;
		gmenu2x->menu->initLayout();
		gmenu2x->font->setSize(gmenu2x->skinConfInt["fontSize"])->setColor(gmenu2x->skinConfColors[COLOR_FONT])->setOutlineColor(gmenu2x->skinConfColors[COLOR_FONT_OUTLINE]);
		gmenu2x->titlefont->setSize(gmenu2x->skinConfInt["fontSizeTitle"])->setColor(gmenu2x->skinConfColors[COLOR_FONT_ALT])->setOutlineColor(gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);
		rowHeight = gmenu2x->font->getHeight() + 1;
		numRows = (gmenu2x->listRect.h - 2)/rowHeight - 1;

		gmenu2x->setInputSpeed();
		voices[selected]->adjustInput();

		this->description = voices[selected]->getDescription();
		drawDialog(gmenu2x->s);

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
			if (!(action = voices[selected]->manageInput())) {
				if (gmenu2x->input[UP] || gmenu2x->input.hatEvent(DUP) == DUP)	action = SD_ACTION_UP;
				else if (gmenu2x->input[DOWN] || gmenu2x->input.hatEvent(DDOWN) == DDOWN)	action = SD_ACTION_DOWN;
				else if (gmenu2x->input[PAGEUP])	action = SD_ACTION_PAGEUP;
				else if (gmenu2x->input[PAGEDOWN])	action = SD_ACTION_PAGEDOWN;
				else if (gmenu2x->input[SETTINGS])	action = SD_ACTION_SAVE;
				else if (gmenu2x->input[CANCEL] && (allowCancel))	action = SD_ACTION_CLOSE;
				else if (gmenu2x->input[CANCEL] && (allowCancel_nomb))	action = SD_ACTION_CLOSE_NOMB;
				else if (gmenu2x->input[CANCEL] && (allowCancel_link))	action = SD_ACTION_CLOSE_LINK;
				else if (gmenu2x->input[CANCEL] && (allowCancel_link_nomb))	action = SD_ACTION_CLOSE_LINK_NOMB;
			}

			switch (action) {
				case SD_ACTION_SAVE:
					save = true;
					loop = false;
					break;
				case SD_ACTION_CLOSE:
					loop = false;
					if (allowCancel && edited()) {
						MessageBox mb(gmenu2x, gmenu2x->tr["Save changes?"], this->icon);
						mb.setButton(CONFIRM, gmenu2x->tr["Yes"]);
						mb.setButton(CANCEL,  gmenu2x->tr["No"]);
						int res = mb.exec();
						switch (res) {
							case CONFIRM: {
								save = true;
								break;
							}
							case CANCEL: {
								gmenu2x->reinit();
								break;
							}
						}
					}
					break;
				case SD_ACTION_CLOSE_NOMB:
					loop = false;
					if (allowCancel_nomb) {
							if (gmenu2x->input[CONFIRM]) {
								gmenu2x->writeConfig();
								gmenu2x->writeSkinConfig();
								break;
						}
							else if (gmenu2x->input[CANCEL]) {
								gmenu2x->reinit();
								break;
							}
					}
				case SD_ACTION_CLOSE_LINK:
					loop = false;
					if (allowCancel_link && edited()) {
						MessageBox mb(gmenu2x, gmenu2x->tr["Save changes?"], this->icon);
						mb.setButton(CONFIRM, gmenu2x->tr["Yes"]);
						mb.setButton(CANCEL,  gmenu2x->tr["No"]);
						save = (mb.exec() == CONFIRM);
					}
					break;
				case SD_ACTION_CLOSE_LINK_NOMB:
					loop = false;
					if (allowCancel_link_nomb && edited()) {
						save = CONFIRM;
					}
					break;
				case SD_ACTION_UP:
					ally = true;
					selected--;
					break;
				case SD_ACTION_DOWN:
					ally = true;
					selected++;
					break;
				case SD_ACTION_PAGEUP:
					ally = true;
					selected -= numRows;
					if (selected < 0) selected = 0;
					break;
				case SD_ACTION_PAGEDOWN:
					ally = true;
					selected += numRows;
					if (selected >= voices.size()) selected = voices.size() - 1;
					break;
			}
		} while (!inputAction);
		if (selected < 0) selected = voices.size() - 1;
		if (selected >= voices.size()) selected = 0;
		if (ally) {
			readSetting = voices[selected]->getTitle() + " " + voices[selected]->getDescription(); // read whole text for more clarity
			gmenu2x->allyTTS(readSetting.c_str(), MEDIUM_GAP_TTS, MEDIUM_SPEED_TTS, 0);
		}
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