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
#include "link.h"
#include "gmenu2x.h"

Link::Link(GMenu2X *gmenu2x, LinkAction action):
Button(gmenu2x->ts, true), gmenu2x(gmenu2x), action(action) {}

void Link::run() {
	this->action();
}

void Link::setTitle(const string &title) {
	this->title = title;
	edited = true;
}

void Link::setDescription(const string &description) {
	this->description = description;
	edited = true;
}

const string &Link::getIcon() {
	int pos = iconPath.find('#'); // search for "opkfile.opk#icon.png"
	if (pos != string::npos) icon = "";
	return icon;
}

void Link::setIcon(const string &icon) {
	int pos = icon.find('#'); // search for "opkfile.opk#icon.png"

	this->icon = icon;
	if (icon.compare(0, 5, "skin:") == 0) {
		this->iconPath = gmenu2x->sc.getSkinFilePath(icon.substr(5, string::npos));
	} else if (file_exists(icon)) {
		this->iconPath = icon;
	} else if (!(icon.empty() && pos != string::npos)) { // keep the opk icon
		this->iconPath = "";
	}

	edited = true;
}

void Link::setBackdrop(const string &backdrop) {
	this->backdrop = backdrop;

	if (backdrop.compare(0, 5, "skin:") == 0)
		this->backdropPath = gmenu2x->sc.getSkinFilePath(backdrop.substr(5, string::npos));
	else if (file_exists(backdrop))
		this->backdropPath = backdrop;
	else
		this->backdropPath = "";

	edited = true;
}

const string Link::searchIcon() {
	if (!gmenu2x->sc.getSkinFilePath(iconPath).empty())
		return gmenu2x->sc.getSkinFilePath(iconPath);
	return gmenu2x->sc.getSkinFilePath("icons/generic.png");
}

const string &Link::getIconPath() {
	if (iconPath.empty()) iconPath = searchIcon();
	return iconPath;
}

void Link::setIconPath(const string &icon) {
	if (file_exists(icon))
		iconPath = icon;
	else
		iconPath = gmenu2x->sc.getSkinFilePath("icons/generic.png");
}
