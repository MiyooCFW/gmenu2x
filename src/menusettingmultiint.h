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
#ifndef MENUSETTINGMULTIINT_H
#define MENUSETTINGMULTIINT_H

#include "menusetting.h"

class MenuSettingMultiInt : public MenuSetting {
private:
	int originalValue;
	int *_value;
	std::string strvalue;
	int def, min, max;
	int selection_max;
	int selection;
	void inc();
	void dec();
	int *choices;
	int reverseLookup(int value);
  
public:
  MenuSettingMultiInt(GMenu2X *gmenu2x, const std::string &title, const std::string &description, int *value, int *choice_pointer, int choice_size, int def, int min, int max);
	// MenuSettingMultiInt(GMenu2X *gmenu2x, const std::string &title, const std::string &description, int *value, int def, int min, int max);
	virtual ~MenuSettingMultiInt() {};

	virtual uint32_t manageInput();
	// virtual void adjustInput();
	virtual void draw(int);
	virtual bool edited();

	virtual void setValue(int value);
	virtual void setDefault();
	
	int value();
};

#endif
