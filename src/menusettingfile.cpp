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
#include "menusettingfile.h"
#include "iconbutton.h"
#include "browsedialog.h"
#include "utilities.h"
#include "debug.h"

using std::string;
using fastdelegate::MakeDelegate;

MenuSettingFile::MenuSettingFile(GMenu2X *gmenu2x, const string &title, const string &description, string *value, const string &filter, const string &startPath, const string &dialogTitle, const string &dialogIcon)
	: MenuSettingStringBase(gmenu2x, title, description, value),
	filter(filter), startPath(startPath),
	dialogTitle(dialogTitle), dialogIcon(dialogIcon)
{
	btn = new IconButton(gmenu2x, "skin:imgs/buttons/select.png", gmenu2x->tr["Clear"]);
	btn->setAction(MakeDelegate(this, &MenuSettingFile::clear));
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/a.png", gmenu2x->tr["Select"]);
	btn->setAction(MakeDelegate(this, &MenuSettingFile::edit));
	buttonBox.add(btn);
}

void MenuSettingFile::edit() {
	string _value = value();
	if (_value.empty())
		_value = startPath + "/";

	_value = dir_name(_value);

	BrowseDialog fd(gmenu2x, dialogTitle, description, dialogIcon);
	fd.showDirectories = true;
	fd.showFiles = true;
	fd.setFilter(filter);
	fd.setPath(_value);

	if (fd.exec())
		setValue(real_path(fd.getPath() + "/" + fd.getFile()));
}
