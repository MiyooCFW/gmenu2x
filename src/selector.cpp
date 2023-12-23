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

#include <fstream>
#include "messagebox.h"
#include "linkapp.h"
#include "selector.h"
#include "debug.h"

#if defined(OPK_SUPPORT)
	#include <libopk.h>
#endif

Selector::Selector(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon, LinkApp *link):
BrowseDialog(gmenu2x, title, description, icon), link(link) {
	loadAliases();
	setFilter(link->getSelectorFilter());
	setPath(gmenu2x->confStr["homePath"]);
}

const std::string Selector::getPreview(uint32_t i) {
	string fname = getFile(i);
	string fpath = getFilePath(i);

	if (previews[fpath].empty()) {
		string screendir = link->getSelectorScreens();
		string noext, realdir;
		previews[fpath] = "#"; // dummy path

		int d1 = fname.rfind(".");
		if (d1 != string::npos && d1 > 0) noext = fname.substr(0, d1);

		if (noext.empty() || noext == ".") return previews[fpath];

		if (screendir.empty()) screendir = "./.images";

		if (screendir[0] == '.') realdir = real_path(path + "/" + screendir) + "/"; // allow "." as "current directory", therefore, relative paths
		else realdir = real_path(screendir) + "/";

		INFO("Searching preview '%s%s.(png|jpg)'", realdir.c_str(), noext.c_str());

		if (dir_exists(realdir)) {
			if (file_exists(realdir + noext + ".png"))
				previews[fpath] = realdir + noext + ".png";
			else if (file_exists(realdir + noext + ".jpg"))
				previews[fpath] = realdir + noext + ".jpg";
		}

		if (previews[fpath] == "#") { // fallback - always search for ./filename.png
			if (file_exists(path + "/" + noext + ".png"))
				previews[fpath] = path + "/" + noext + ".png";
			else if (file_exists(path + "/" + noext + ".jpg"))
				previews[fpath] = path + "/" + noext + ".jpg";
		}
	}

	return previews[fpath];
}

void Selector::parseAliases(istream &infile) {
	aliases.clear();
	params.clear();
	string line;

	while (getline(infile, line, '\n')) {
		string name, value;
		int d1 = line.find("=");
		int d2 = line.find(";");
		if (d2 > d1) d2 -= d1 + 1;

		name = lowercase(trim(line.substr(0, d1)));
		aliases[name] = trim(line.substr(d1 + 1, d2));

		if (d2 > 0)
			params[name] = trim(line.substr(d1 + d2 + 2));
	}
}

void Selector::loadAliases() {
	string linkExec = link->getExec();
	string linkAlias = link->getAliasFile();

	if (file_exists(linkAlias)) {
		ifstream infile;
		infile.open(linkAlias.c_str(), ios_base::in);
		parseAliases(infile);
		infile.close();

#if defined(OPK_SUPPORT)
	} else if (file_ext(linkExec, true) == ".opk") {
		void *buf; size_t len;
		struct OPK *opk = opk_open(linkExec.c_str());
		if (!opk) {
			ERROR("Unable to open OPK");
			return;
		}
		if (opk_extract_file(opk, linkAlias.c_str(), &buf, &len) < 0) {
			ERROR("Unable to extract file: %s\n", linkAlias.c_str());
			return;
		}
		opk_close(opk);

		istringstream infile((char *)buf);

		parseAliases(infile);
#endif // OPK_SUPPORT
	}
}

const std::string Selector::getFileName(uint32_t i) {
	string fname = getFile(i);
	string noext = lowercase(fname);
	int d1 = fname.rfind(".");
	if (d1 != string::npos && d1 > 0)
		noext = lowercase(fname.substr(0, d1));

	unordered_map<string, string>::iterator it = aliases.find(noext);
	if (it == aliases.end() || it->second.empty()) return fname;
	return it->second;
}

const std::string Selector::getParams(uint32_t i) {
	string fname = getFile(i);
	string noext = fname;
	int d1 = fname.rfind(".");
	if (d1 != string::npos && d1 > 0)
		noext = lowercase(fname.substr(0, d1));

	unordered_map<string, string>::iterator it = params.find(noext);
	if (it == params.end()) return "";
	return it->second;
}

void Selector::customOptions(vector<MenuOption> &options) {
	if (isFile(selected)) {
		options.push_back((MenuOption){gmenu2x->tr["Add to Favorites"], MakeDelegate(this, &Selector::addFavorite)});
	}
}

void Selector::addFavorite() {
	string favicon = getPreview(selected);
	if (favicon.empty() || favicon == "#") favicon = this->icon;

	gmenu2x->menu->addSection("favorites");
	string title = base_name(getFileName(selected), true);
	string linkpath = "sections/favorites/" + title + "." + base_name(link->getExec(), true) + ".lnk";

	LinkApp *fav = new LinkApp(gmenu2x, linkpath.c_str());

	fav->setExec(link->getExec());
	fav->setTitle(title);
	fav->setDescription(link->getDescription());
	fav->setIcon(favicon);

	string selFullPath = cmdclean(getFilePath(selected));
	string params = link->getParams();
	if (params.find("\%f") != std::string::npos)
		params = strreplace(params, "\%f", selFullPath);
	else
		params = link->getParams() + " " + selFullPath;

	fav->setParams(params);

	fav->save();

	gmenu2x->initMenu();

	MessageBox mb(gmenu2x, gmenu2x->tr["Link created"], favicon);
	mb.setAutoHide(1000);
	mb.exec();
}
