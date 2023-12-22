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

#ifndef GMENU2X_H
#define GMENU2X_H

class PowerManager;

#include <iostream>
#include <string>
#include <vector>
#include <tr1/unordered_map>

#include "surfacecollection.h"
#include "iconbutton.h"
#include "translator.h"
#include "FastDelegate.h"
#include "utilities.h"
#include "touchscreen.h"
#include "inputmanager.h"
#include "surface.h"
#include "fonthelper.h"
#include "debug.h"

// Note: Keep this in sync with colorNames!
enum color {
	COLOR_TOP_BAR_BG,
	COLOR_LIST_BG,
	COLOR_BOTTOM_BAR_BG,
	COLOR_SELECTION_BG,
	COLOR_PREVIEW_BG,
	COLOR_MESSAGE_BOX_BG,
	COLOR_MESSAGE_BOX_BORDER,
	COLOR_MESSAGE_BOX_SELECTION,
	COLOR_FONT,
	COLOR_FONT_OUTLINE,
	COLOR_FONT_ALT,
	COLOR_FONT_ALT_OUTLINE,

	NUM_COLORS,
};

enum sb {
	SB_OFF,
	SB_LEFT,
	SB_BOTTOM,
	SB_RIGHT,
	SB_TOP,
	SB_CLASSIC,
};

enum bd {
	BD_OFF,
	BD_MENU,
	BD_DIALOG,
};

enum tvout {
	TV_OFF,
	TV_PAL,
	TV_NTSC,
};

using std::string;
using std::vector;
using fastdelegate::FastDelegate0;

extern uint16_t mmcPrev, mmcStatus;
extern uint16_t udcPrev, udcStatus;
extern uint16_t tvOutPrev, tvOutStatus;
extern uint16_t volumeModePrev, volumeMode;
extern uint16_t batteryIcon;
extern uint8_t numJoyPrev, numJoy; // number of connected joysticks

extern int CPU_MENU;
extern int CPU_LINK;
extern int CPU_MAX;
extern int CPU_MIN;
extern int CPU_STEP;
extern int LAYOUT_VERSION;
extern int LAYOUT_VERSION_MAX;
extern int TEFIX;
extern int TEFIX_MAX;

typedef FastDelegate0<> MenuAction;
typedef unordered_map<string, string, hash<string> > ConfStrHash;
typedef unordered_map<string, int, hash<string> > ConfIntHash;

struct MenuOption {
	string text;
	MenuAction action;
};

class Menu;

class GMenu2X {
private:
	string lastSelectorDir;
	int lastSelectorElement;
	void explorer();
	void readConfig();
	bool readTmp();
	void initFont();
	void umountSdDialog();
	void opkInstall(string path);
	void opkScanner();
	string ipkName(string cmd);
	void ipkInstall(string path);

	virtual void udcDialog(int udcStatus = -1) { };
	virtual void tvOutDialog(int16_t mode = -1) { };
	virtual void hwInit() { };
	virtual void hwDeinit() { };

public:
	static GMenu2X *instance;

	/*
	 * Variables needed for elements disposition
	 */
	uint32_t w = 320, h = 240, bpp = 16;
	SDL_Rect listRect, linksRect, sectionBarRect, bottomBarRect;

	//Configuration hashes
	ConfStrHash confStr, skinConfStr;
	ConfIntHash confInt, skinConfInt;

	RGBAColor skinConfColors[NUM_COLORS];
	SurfaceCollection sc;
	Surface *s, *bg, *iconInet = NULL;
	Translator tr;
	FontHelper *font = NULL, *titlefont = NULL;
	PowerManager *powerManager;
	InputManager input;
	Touchscreen ts;
	Menu *menu;
	bool f200 = true; //gp2x type // touchscreen
	string currBackdrop;

	~GMenu2X();
	void quit();
	void quit_nosave();
	void main(bool autoStart);
	void settings();
	void settings_date();
	void reinit();
	void reinit_save();
	void poweroffDialog();
	void resetSettings();
	void cpuSettings();
	void showManual();

	void setSkin(string skin, bool clearSC = true);
	void skinMenu();
	void skinColors();
	uint32_t onChangeSkin() { return 1; }

	bool inputCommonActions(bool &inputAction);

	bool autoStart;
	bool actionPerformed = false;

	void cls(Surface *s = NULL, bool flip = true);

	void about();
	void viewLog();
	void viewAutoStart();
	void contextMenu();
	void changeWallpaper();
	void changeSelectorDir();

	bool saveScreenshot(string path);
	void drawSlider(int val, int min, int max, Surface &icon, Surface &bg);

	void setInputSpeed();

	void writeConfig();
	void writeSkinConfig();
	void writeTmp(int selelem = -1, const string &selectordir = "");

	void initMenu();
	void addLink();
	void editLink();
	void deleteLink();
	void addSection();
	void renameSection();
	void deleteSection();

	string setBackground(Surface *bg, string wallpaper);

	int drawButton(Button *btn, int x = 5, int y = -8);
	int drawButton(Surface *s, const string &btn, const string &text = "", int x = 5, int y = -8);
	int drawButtonRight(Surface *s, const string &btn, const string &text = "", int x = 5, int y = -8);
	void drawScrollBar(uint32_t pagesize, uint32_t totalsize, uint32_t pagepos, SDL_Rect scrollRect, const uint8_t align = HAlignRight);

	static uint32_t timerFlip(uint32_t interval, void *param = NULL);

	virtual void setScaleMode(unsigned int mode) { };
	virtual void setTVOut(unsigned int mode) { };
	virtual void setCPU(uint32_t mhz) { };
	virtual void ledOn() { };
	virtual void ledOff() { };
	virtual int setVolume(int val, bool popup = false);
	virtual void setKbdLayout(int val) { };
	virtual void setTefix(int val) { };
	virtual int getVolume() { return 0; };
	virtual int getBacklight() { return -1; };
	virtual int setBacklight(int val, bool popup = false);
	virtual string hwPreLinkLaunch() { return ""; };
	virtual void enableTerminal() { };
	virtual void setGamma(int value) { };
};

#endif
