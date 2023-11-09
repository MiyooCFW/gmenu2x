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
#include "gmenu2x.h"
#include "dialog.h"
#include "filelister.h"
#include "menu.h"

class FileLister;

using std::string;

class BrowseDialog : protected Dialog, public FileLister {
protected:
	virtual void onChangeDir() {};

private:
	bool ts_pressed;
	virtual const std::string getPreview(uint32_t i = 0);
	vector<int> browse_history;

	void contextMenu();
	void deleteFile();
	void umountDir();
	void exploreHome();
	void exploreMedia();
	void setWallpaper();

public:
	BrowseDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon = "icons/explorer.png");
	virtual ~BrowseDialog() {};
	bool allowSelectDirectory = false, allowEnterDirectory = true;
	int32_t selected = 0;
	bool exec();
	void directoryEnter(const string &path);

	virtual const std::string getFileName(uint32_t i = 0);
	virtual const std::string getParams(uint32_t i = 0);
	virtual void customOptions(vector<MenuOption> &options) { return; };
};

#endif /*BROWSEDIALOG_H_*/
