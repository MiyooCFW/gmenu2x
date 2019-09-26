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

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <string>
#include "gmenu2x.h"
#include "menusetting.h"
#include "dialog.h"

using std::string;
using std::vector;

class Touchscreen;

class SettingsDialog : protected Dialog {
private:
	enum sd_action_t {
		SD_NO_ACTION,
		SD_ACTION_CLOSE,
		SD_ACTION_UP,
		SD_ACTION_DOWN,
		SD_ACTION_SAVE,
		SD_ACTION_PAGEUP,
		SD_ACTION_PAGEDOWN,
	};
	Touchscreen &ts;
	vector<MenuSetting *> voices;
	string title, icon;

public:
	SettingsDialog(GMenu2X *gmenu2x, Touchscreen &ts, const string &title, const string &icon = "skin:sections/settings.png");
	~SettingsDialog();
	bool save = false, close = false, allowCancel = true;
	bool edited();
	bool exec();
	void addSetting(MenuSetting* set);
	int32_t selected = 0;
};

#endif /*INPUTDIALOG_H_*/
