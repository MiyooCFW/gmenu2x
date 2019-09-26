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

#ifndef IMAGEVIEWERDIALOG_H_
#define IMAGEVIEWERDIALOG_H_

#include <string>
#include <fstream>

#include "gmenu2x.h"
#include "dialog.h"

using namespace std;
using std::string;
using std::vector;
using std::ifstream;
using std::ios_base;

class ImageViewerDialog : protected Dialog {
protected:
	string title, description, icon, path;

public:
	ImageViewerDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon = "icons/ebook.png", const string &path = "");
	void exec();
};

#endif /*IMAGEVIEWERDIALOG_H_*/
