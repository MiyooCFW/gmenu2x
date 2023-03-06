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

#ifndef SELECTOR_H_
#define SELECTOR_H_

#include "browsedialog.h"
#include <sstream>

using namespace std;

class LinkApp;

class Selector : public BrowseDialog {
private:
	LinkApp *link;

	unordered_map<string, string> aliases;
	unordered_map<string, string> params;
	unordered_map<string, string> previews;
	void loadAliases();
	void parseAliases(istream &infile);
	const std::string getPreview(uint32_t i = 0);

public:
	Selector(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon, LinkApp *link);
	const std::string getFileName(uint32_t i = 0);
	const std::string getParams(uint32_t i = 0);
	void addFavourite();
	void customOptions(vector<MenuOption> &options);
};

#endif /*SELECTOR_H_*/
