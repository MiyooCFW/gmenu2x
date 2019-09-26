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
#ifndef LINK_H
#define LINK_H

#include <string>
#include <iostream>

#include "button.h"
#include "surface.h"

// linkaction
#include "FastDelegate.h"
using namespace fastdelegate;
typedef FastDelegate0<> LinkAction;

using std::string;

class GMenu2X;

/**
Base class that represents a link on screen.

	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class Link : public Button {
private:
	uint32_t padding;
	LinkAction action;

protected:
	GMenu2X *gmenu2x;
	bool edited;
	string title, description, icon, iconPath;

	Surface *iconSurface;

public:
	// linkaction
	Link(GMenu2X *gmenu2x, LinkAction action);

	virtual ~Link() {};
	void updateSurfaces();

	const string &getTitle();
	void setTitle(const string &title);
	const string &getDescription();
	void setDescription(const string &description);
	const string &getIcon();
	void setIcon(const string &icon);
	virtual const string &searchIcon();
	const string &getIconPath();
	void setIconPath(const string &icon);

	virtual void run();
};

#endif
