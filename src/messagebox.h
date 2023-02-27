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

#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

#include <string>
#include "gmenu2x.h"

using std::string;
using std::vector;

enum {
	MB_BTN_B,
	MB_BTN_X,
	MB_BTN_START,
	MB_BTN_SELECT
};

class MessageBox {
private:
	string text, icon;
	uint32_t autohide = 0, bgalpha = 200;
	GMenu2X *gmenu2x;
	vector<string> button;
	vector<string> buttonText;
	vector<SDL_Rect> buttonPosition;

public:
	MessageBox(GMenu2X *gmenu2x, vector<MenuOption> options);
	MessageBox(GMenu2X *gmenu2x, const string &text, const string &icon = "");
	~MessageBox();

	void setButton(int action, const string &btn);
	void setAutoHide(uint32_t delay);
	void setBgAlpha(uint32_t bgalpha);
	int exec();
	void exec(uint32_t timeOut);
	void clearTimer();
	static uint32_t execTimer(uint32_t interval, void *param);
	SDL_TimerID popupTimer;
};

#endif /*MESSAGEBOX_H_*/
