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
#ifndef MENUSETTINGBOOL_H
#define MENUSETTINGBOOL_H

#include "menusetting.h"

// class GMenu2X;

class MenuSettingBool : public MenuSetting {
private:
	void initButton();
	void toggle();

	bool originalValue;
	bool *_value;
	int *_ivalue;
	std::string strvalue;

public:
	MenuSettingBool(GMenu2X *gmenu2x, const std::string &title, const std::string &description, bool *value);
	MenuSettingBool(GMenu2X *gmenu2x, const std::string &title, const std::string &description, int *value);
	virtual ~MenuSettingBool() {};

	virtual void draw(int y);
	virtual uint32_t manageInput();
	virtual bool edited();

	void setValue(int value);
	void setValue(bool value);
	bool value();
};

#endif
