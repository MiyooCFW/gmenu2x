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
#ifndef SURFACECOLLECTION_H
#define SURFACECOLLECTION_H

#include <string>
#include <tr1/unordered_map>

class Surface;

typedef std::tr1::unordered_map<std::string, Surface *> SurfaceHash;

/**
Hash Map of surfaces that loads surfaces not already loaded and reuses already loaded ones.

	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class SurfaceCollection {
private:
	SurfaceHash surfaces;
	std::string skin;

public:
	SurfaceCollection();
	void setSkin(const std::string &skin);
	std::string getSkinFilePath(const std::string &file, bool falback = true);
	void debug();
	Surface *add(Surface *s, const std::string &path);
	Surface *add(std::string path, std::string key="");
	bool del(const std::string &key);
	void clear();
	void move(const std::string &from, const std::string &to);
	bool exists(const std::string &path);
	Surface *operator[](const std::string &);
};

#endif
