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
#ifndef LINKAPP_H
#define LINKAPP_H

#include <string>
#include "link.h"

using std::string;

/**
	Parses links files.
	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class LinkApp : public Link {
private:
	// InputManager &input;
	int		clock = 0,
			layout = 1,
			selectorelement = 0,
			scalemode,
			_scalemode = 0; //, ivolume = 0;

	bool	selectorbrowser = true,
			terminal = false;

	string	params = "",
			homedir = "",
			manual = "",
			manualPath = "",
			selectordir = "",
			selectorfilter = "",
			selectorscreens = "",
			aliasfile = "",
			file = "",
			icon_opk = "";

public:
	LinkApp(GMenu2X *gmenu2x, const char* file);
	const string &getExec() { return exec; }
	void setExec(const string &exec);
	const string &getParams() { return params; }
	void setParams(const string &params);
	const string &getHomeDir() { return homedir; }
	void setHomeDir(const string &homedir);
	const string &getManual() { return manual; }
	const string &getManualPath() { return manualPath; }
	void setManual(const string &manual);
	const string &getSelectorDir() { return selectordir; }
	void setSelectorDir(const string &selectordir);
	bool getSelectorBrowser() { return selectorbrowser; }
	void setSelectorBrowser(bool value);
	bool getTerminal() { return terminal; }
	void setTerminal(bool value);
	int getScaleMode() { return scalemode; }
	void setScaleMode(int value);
	const string &getSelectorScreens() { return selectorscreens; }
	void setSelectorScreens(const string &selectorscreens);
	const string &getSelectorFilter() { return selectorfilter; }
	void setSelectorFilter(const string &selectorfilter);
	int getSelectorElement() { return selectorelement; }
	void setSelectorElement(int i);
	const string &getAliasFile() { return aliasfile; }
	void setAliasFile(const string &aliasfile);
	int getCPU() { return clock; }
	void setCPU(int mhz = 0);
	int getKbdLayout() { return layout; }
	void setKbdLayout(int val = 1);
	bool save();
	void run();
	void selector(int startSelection = 0, const string &selectorDir = "");
	void launch(const string &selectedFile = "", string selectedDir = "");
	bool targetExists();
	void renameFile(const string &name);
	const string &getFile() { return file; }

#if defined(TARGET_GP2X)
	// int volume();
	// const string &volumeStr();
	// void setVolume(int vol);
	// bool getUseRamTimings() { return useRamTimings; }
	// void setUseRamTimings(bool value);
	// bool getUseGinge() { return useGinge; }
	// void setUseGinge(bool value);
	// const string &clockStr(int maxClock);
	// string sgamma;
#endif

#if defined(HW_GAMMA)
	int gamma;
	int getGamma() { return gamma; }
	void setGamma(int gamma);
	// const string &gammaStr();
	// bool &needsWrapperRef() { return wrapper; }
	// bool &runsInBackgroundRef() { return dontleave; }
#endif

	const string searchIcon();
	const string searchBackdrop();
	const string searchManual();
};

#endif
