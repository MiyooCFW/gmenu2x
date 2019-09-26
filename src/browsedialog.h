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

#ifndef BROWSEDIALOG_H_
#define BROWSEDIALOG_H_

#include <string>
#include "filelister.h"
#include "gmenu2x.h"
// #include "buttonbox.h"
#include "dialog.h"

class FileLister;

using std::string;

class BrowseDialog : protected Dialog {
protected:
	virtual void onChangeDir() {};

	FileLister *fl;
	int32_t selected;

private:
	enum bd_action_t {
		BD_NO_ACTION,
		BD_ACTION_SELECT,
		BD_ACTION_CLOSE,
		BD_ACTION_UP,
		BD_ACTION_DOWN,
		BD_ACTION_PAGEUP,
		BD_ACTION_PAGEDOWN,
		BD_ACTION_GOUP,
		BD_ACTION_CONFIRM,
		BD_ACTION_CANCEL,
	};

	bool close, result, ts_pressed;
	string title, description, icon;

	uint32_t getAction();
	void directoryUp();
	void directoryEnter();
	void confirm();
	void cancel();

public:
	BrowseDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon = "icons/explorer.png");
	virtual ~BrowseDialog();

	bool exec();

	const std::string &getPath();
	std::string getFile();

	const std::string getExt();

	void setFilter(const string &filter);

	bool showDirectories, showFiles, allowSelectDirectory = false;

	void setPath(const string &path);
};

#endif /*INPUTDIALOG_H_*/
