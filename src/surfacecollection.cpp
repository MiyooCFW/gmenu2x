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

#include "surfacecollection.h"
#include "surface.h"
#include "utilities.h"
#include "debug.h"

#if defined(OPK_SUPPORT)
#include <libopk.h>
#endif

using std::endl;
using std::string;

SurfaceCollection::SurfaceCollection() {
	setSkin("Default");
}

void SurfaceCollection::debug() {
	SurfaceHash::iterator end = surfaces.end();
	for (SurfaceHash::iterator curr = surfaces.begin(); curr != end; curr++) {
		DEBUG("key: %i", curr->first.c_str());
	}
}

void SurfaceCollection::setSkin(const string &skin) {
	this->skin = skin;
}

string SurfaceCollection::getSkinFilePath(const string &file, bool fallback) {
	string ret = "skins/" + skin + "/" + file;
	if (file_exists(ret))
		return ret;

	if (fallback) {
		ret = "skins/Default/" + file;
		if (file_exists(ret))
			return ret;
	}

	return "";
}

Surface *SurfaceCollection::add(string path, string key) {
	if (path.empty()) return NULL;
	if (key.empty()) key = path;
	if (exists(key)) return surfaces[key]; //del(key);

	Surface *s = NULL;

#if defined(OPK_SUPPORT)
	int pos = path.find('#'); // search for "opkfile.opk#icon.png"
	if (pos != path.npos) {
		string iconpath = "icons/" + path.substr(pos + 1);
		iconpath = getSkinFilePath(iconpath, false);

		if (!iconpath.empty()) {
			DEBUG("Adding OPK skin surface: '%s'", iconpath.c_str());
			s = new Surface(iconpath, true);

		} else {
			DEBUG("Adding OPK surface: %s", path.c_str());
			void *buf; size_t len;
			struct OPK *opk = opk_open(path.substr(0, pos).c_str());
			if (!opk) {
				ERROR("Unable to open OPK");
				// return NULL;
			} else if (opk_extract_file(opk, path.substr(pos + 1).c_str(), &buf, &len) < 0) {
				ERROR("Unable to extract file: %s", path.substr(pos + 1).c_str());
				// return NULL;
			} else {
				opk_close(opk);
				s = new Surface(buf, len);
			}
		}
	} else
#endif // OPK_SUPPORT
	{
		if (path.substr(0, 5) == "skin:") {
			path = getSkinFilePath(path.substr(5));
		}

		if (!path.empty() && file_exists(path)) {
			DEBUG("Adding surface: '%s'", path.c_str());
			s = new Surface(path, true);
		}
	}

	// if (s != NULL)
	surfaces[key] = s;

	return s;
}

Surface *SurfaceCollection::add(Surface *s, const string &key) {
	if (exists(key)) return surfaces[key]; //del(key);
	surfaces[key] = s;
	return s;
}

Surface *SurfaceCollection::operator[](const string &key) {
	if (exists(key)) return surfaces[key];
	return add(key);
}

bool SurfaceCollection::del(const string &path) {
	SurfaceHash::iterator i = surfaces.find(path);
	if (i != surfaces.end()) {
		delete i->second;
		surfaces.erase(i);
		return true;
	}
	return false;
}

void SurfaceCollection::clear() {
	while (surfaces.size() > 0) {
		delete surfaces.begin()->second;
		surfaces.erase(surfaces.begin());
	}
}

bool SurfaceCollection::exists(const string &path) {
	return surfaces.find(path) != surfaces.end();
}

void SurfaceCollection::move(const string &from, const string &to) {
	del(to);
	surfaces[to] = surfaces[from];
	surfaces.erase(from);
}
