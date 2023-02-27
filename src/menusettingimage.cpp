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
#include "menusettingimage.h"
#include "gmenu2x.h"
#include "utilities.h"

using std::string;

MenuSettingImage::MenuSettingImage(GMenu2X *gmenu2x, const string &title, const string &description, string *value, const string &filter, const string &startPath, const string &dialogTitle, const string &dialogIcon):
MenuSettingFile(gmenu2x, title, description, value, filter, startPath, dialogTitle, dialogIcon) {}

void MenuSettingImage::setValue(const string &value) {
	string skinpath(exe_path() + "/skins/" + gmenu2x->confStr["skin"]);
	bool inSkinDir = value.substr(0, skinpath.length()) == skinpath;

	if (!inSkinDir && gmenu2x->confStr["skin"] != "Default") {
		skinpath = exe_path() + "/skins/Default";
		inSkinDir = value.substr(0, skinpath.length()) == skinpath;
	}

	*_value = value;
	if (inSkinDir) {
		string tempIcon = value.substr(skinpath.length(), value.length());
		string::size_type pos = tempIcon.find("/");
		if (pos != string::npos) {
			*_value = "skin:" + tempIcon.substr(pos + 1, value.length());
		}
	}
}
