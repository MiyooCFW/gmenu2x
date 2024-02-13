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

#ifndef INPUTDIALOG_H_
#define INPUTDIALOG_H_

#define KEY_WIDTH 20
#define KEY_HEIGHT 20
#define KB_TOP 90

#define MAX_KB 5

#include <string>
#include "gmenu2x.h"

using std::string;
using std::vector;

typedef vector<string> stringlist;

// class InputManager;
// class Touchscreen;

class InputDialog {
protected:
	GMenu2X *gmenu2x;
	SDL_TimerID wakeUpTimer = NULL;

private:
	enum id_actions {
		ID_NO_ACTION,
		ID_ACTION_SAVE,
		ID_ACTION_CLOSE,
		ID_ACTION_UP,
		ID_ACTION_DOWN,
		ID_ACTION_LEFT,
		ID_ACTION_RIGHT,
		ID_ACTION_BACKSPACE,
		ID_ACTION_SPACE,
		ID_ACTION_GOUP,
		ID_ACTION_SELECT,
		ID_ACTION_KB_CHANGE
	};

	int selRow, selCol;
	string title, text, icon;
	string kb11, kb12, kb13, kb21, kb22, kb23, kb31, kb32, kb33;
	string kbc11, kbc12, kbc13, kbc21, kbc22, kbc23, kbc31, kbc32, kbc33;
	int16_t curKeyboard;
	vector<stringlist> keyboard;
	stringlist *kb;
	int kbLength, kbWidth, kbHeight, kbLeft;
	SDL_Rect kbRect;
	IconButton *btnBackspaceX, *btnBackspaceL, *btnSpace, *btnConfirm, *btnChangeKeys;
	string input;

	void backspace();
	void space();
	void confirm();
	void changeKeys();

	int drawVirtualKeyboard();
	void setKeyboard(int);

public:
	InputDialog(GMenu2X *gmenu2x, /*Touchscreen &ts,*/ const string &text, const string &startvalue = "", const string &title = "", const string &icon = "");
	~InputDialog();

	bool exec();
	const string &getInput() { return input; }
};

#endif /*INPUTDIALOG_H_*/
