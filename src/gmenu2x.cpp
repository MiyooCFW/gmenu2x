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

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <signal.h>

#include <sys/ioctl.h>

#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/fb.h>

#include "linkapp.h"
#include "fonthelper.h"
#include "surface.h"
#include "browsedialog.h"
#include "powermanager.h"
#include "gmenu2x.h"
#include "filelister.h"
#include "iconbutton.h"
#include "messagebox.h"
#include "inputdialog.h"
#include "settingsdialog.h"
#include "wallpaperdialog.h"
#include "textdialog.h"
#include "terminaldialog.h"
#include "menusettingint.h"
#include "menusettingbool.h"
#include "menusettingrgba.h"
#include "menusettingmultiint.h"
#include "menusettingstring.h"
#include "menusettingmultistring.h"
#include "menusettingfile.h"
#include "menusettingimage.h"
#include "menusettingdir.h"
#include "imageviewerdialog.h"
#include "menusettingdatetime.h"
#include "debug.h"

#if defined(OPK_SUPPORT)
	#include "opkscannerdialog.h"
	#include <libopk.h>
#endif

using std::ifstream;
using std::ofstream;
using std::stringstream;
using namespace fastdelegate;

#define sync() sync(); system("sync &");

enum vol_mode_t {
	VOLUME_MODE_MUTE, VOLUME_MODE_PHONES, VOLUME_MODE_NORMAL
};

string fwType = "";

uint16_t mmcPrev, mmcStatus;
uint16_t udcPrev = UDC_REMOVE, udcStatus;
uint16_t tvOutPrev = TV_REMOVE, tvOutStatus;
uint16_t volumeModePrev, volumeMode;
uint16_t batteryIcon = 3;
uint8_t numJoyPrev, numJoy; // number of connected joysticks

int CPU_MENU = 0;
int CPU_LINK = 0;
int CPU_MAX = 0;
int CPU_EDGE = 0;
int CPU_MIN = 0;
int CPU_STEP = 0;
int LAYOUT_VERSION = 0;
int LAYOUT_VERSION_MAX = 0;
int TEFIX = -1;
int TEFIX_MAX = -1;

const char *CARD_ROOT = getenv("HOME");

#if defined(TARGET_RETROFW)
	#include "platform/retrofw.h"
#elif defined(TARGET_RG350)
	#include "platform/rg350.h"
#elif defined(TARGET_MIYOO)
	#include "platform/miyoo.h"
#elif defined(TARGET_GP2X) || defined(TARGET_WIZ) || defined(TARGET_CAANOO)
	#include "platform/gp2x.h"
#else //if defined(TARGET_LINUX)
	#include "platform/linux.h"
#endif

#ifndef DEFAULT_CPU
#define DEFAULT_CPU 0
#endif
#ifndef DEFAULT_LAYOUT
#define DEFAULT_LAYOUT 0
#endif
#ifndef DEFAULT_TEFIX
#define DEFAULT_TEFIX -1
#endif

#include "menu.h"

// Note: Keep this in sync with the enum!
static const char *colorNames[NUM_COLORS] = {
	"topBarBg",
	"listBg",
	"bottomBarBg",
	"selectionBg",
	"previewBg",
	"messageBoxBg",
	"messageBoxBorder",
	"messageBoxSelection",
	"font",
	"fontOutline",
	"fontAlt",
	"fontAltOutline"
};

static enum color stringToColor(const string &name) {
	for (uint32_t i = 0; i < NUM_COLORS; i++) {
		if (strcmp(colorNames[i], name.c_str()) == 0) {
			return (enum color)i;
		}
	}
	return (enum color)-1;
}

static const char *colorToString(enum color c) {
	return colorNames[c];
}

GMenu2X *GMenu2X::instance = NULL;

static void quit_all(int err) {
	delete GMenu2X::instance;
	exit(err);
}

int main(int argc, char * argv[]) {
	INFO("Starting GMenuNX...");

	signal(SIGINT,  &quit_all);
	signal(SIGSEGV, &quit_all);
	signal(SIGTERM, &quit_all);

	bool autoStart = false;
	for (int i = 0; i < argc; i++){
       		if(strcmp(argv[i],"--autostart")==0) {
			INFO("Launching Autostart");
			autoStart = true;
		}
	}
	int fd = open("/dev/tty0", O_RDONLY);
	if (fd > 0) {
		ioctl(fd, VT_UNLOCKSWITCH, 1);
		ioctl(fd, KDSETMODE, KD_TEXT);
		ioctl(fd, KDSKBMODE, K_XLATE);
		close(fd);
	}

	usleep(1000);

	GMenu2X::instance = new GMenuNX();
	GMenu2X::instance->main(autoStart);

	return 0;
}

GMenu2X::~GMenu2X() {
	get_date_time(); // update sw clock
	confStr["datetime"] = get_date_time();
	
	writeConfig();
	
	quit();
	delete menu;
	delete s;
	delete font;
	delete titlefont;
}

void GMenu2X::quit() {
	s->flip(); s->flip(); s->flip(); // flush buffers

	powerManager->clearTimer();
	get_date_time(); // update sw clock
	confStr["datetime"] = get_date_time();
//#if defined(HW_LIDVOL)
// 	setBacklight(getBacklight());
// 	setVolume(getVolume());
//#endif	
	writeConfig();

	s->free();

	font->free();
	titlefont->free();

	fflush(NULL);
	SDL_Quit();
	hwDeinit();
}

void GMenu2X::quit_nosave() {
	s->flip(); s->flip(); s->flip(); // flush buffers

	powerManager->clearTimer();

	s->free();

	font->free();
	titlefont->free();

	fflush(NULL);
	SDL_Quit();
	hwDeinit();
}

void GMenu2X::main(bool autoStart) {
	hwInit();

	chdir(exe_path().c_str());

	readConfig();

	setScaleMode(0);
#if defined(HW_LIDVOL)
	if (setBacklight(getBacklight()) == 0) setBacklight(50);
	setVolume(getVolume());
#else
	if (setBacklight(confInt["backlight"]) == 0) setBacklight(50);
	setVolume(confInt["globalVolume"]);
#endif
	setKbdLayout(confInt["keyboardLayoutMenu"]);
	setTefix(confInt["tefixMenu"]);

	setenv("SDL_FBCON_DONT_CLEAR", "1", 0);
	setenv("SDL_NOMOUSE", "1", 1);
	string prevDateTime = confStr["datetime"];	
	string freshDateTime = get_date_time();
		if (prevDateTime > freshDateTime) {
			set_date_time(prevDateTime.c_str());
		}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {
		ERROR("Could not initialize SDL: %s", SDL_GetError());
		quit();
		return;
	}
	SDL_ShowCursor(SDL_DISABLE);

	input.init(exe_path() + "/input.conf");

	setInputSpeed();

	SDL_Surface *screen = SDL_SetVideoMode(this->w, this->h, this->bpp, SDL_HWSURFACE |
		#ifdef SDL_TRIPLEBUF
			SDL_TRIPLEBUF
		#else
			SDL_DOUBLEBUF
		#endif
	);
	s = new Surface();

	s->enableVirtualDoubleBuffer(screen);

	setSkin(confStr["skin"], true);

	powerManager = new PowerManager(this, confInt["backlightTimeout"], confInt["powerTimeout"]);

	// overclock CPU only after first surface has been draw (hopefully won't interfere fb0 init execution)
	setCPU(confInt["cpuMenu"]);

	srand(time(0));  // Seed the rand with current time to get different number sequences
	int randomInt = rand() % 10; // Generate a random val={0..x} to print "Hint" msg occasionally
	//Hint messages
	if (confInt["showHints"] == 1) {
		if (confStr["lastCommand"] == "" || confStr["lastDirectory"] == "") {
			switch (randomInt) {
				case 0: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: Press 'Y' now quickly to reset gmenu2x.cfg"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();
				break;
				}
				case 1: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: Hold 'X' to change Date & Time"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();		
				break;
				}
				case 2: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: Hold 'SELECT' to disable TV-output"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();		
				break;
				}
				case 3: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: Hold 'START' to enter Suspend Mode"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();		
				break;
				}
				case 4: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: You can AutoStart any game/app!? See settings"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();		
				break;
				}
				case 5: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: Hold 'Y' to restart GMenu2X"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();		
				break;
				}
				default: {
				MessageBox mb(this, tr["Loading"]);
				mb.setAutoHide(1);
				mb.setBgAlpha(0);
				mb.exec();
				break;
				}
			}
		} else if (!confInt["dialogAutoStart"]) {
			switch (randomInt) {
				case 0: case 1: case 2: {
				MessageBox mb(this, tr["Loading."]+"\n"+tr["Hint: Press 'Y' now quickly to reset gmenu2x.cfg"]);
				mb.setAutoHide(1000);
				mb.setBgAlpha(0);
				mb.exec();
				break;
				}
				default: {
				MessageBox mb(this, tr["Loading"]);
				mb.setAutoHide(1);
				mb.setBgAlpha(0);
				mb.exec();
				break;
				}
			}
		} else {
			MessageBox mb(this, tr["Loading"]);
			mb.setAutoHide(1);
			mb.setBgAlpha(0);
			mb.exec();
		}
	} else {
		MessageBox mb(this, tr["Loading"]);
		mb.setAutoHide(1);
		mb.setBgAlpha(0);
		mb.exec();
	}

	input.update(false);
	if (input[MANUAL]){ //Reset GMenu2X settings
		string tmppath = exe_path() + "/gmenu2x.conf";
		unlink(tmppath.c_str());
		reinit();
	}

	menu = new Menu(this);
	initMenu();

	tvOutStatus = getTVOutStatus();
	mmcPrev = mmcStatus = getMMCStatus();
	udcStatus = getUDCStatus();
	numJoyPrev = numJoy = getDevStatus();
	volumeModePrev = volumeMode = getVolumeMode(confInt["globalVolume"]);

	if (confInt["dialogAutoStart"] && confStr["lastCommand"] != "" && confStr["lastDirectory"] != "") {
		viewAutoStart();
	}

	if(confStr["lastCommand"] != "" && confStr["lastDirectory"] != "")  {
		INFO("Starting autostart()");
		INFO("conf %s %s",confStr["lastDirectory"].c_str(),confStr["lastCommand"].c_str());
		INFO("autostart %s %s",confStr["lastDirectory"],confStr["lastCommand"]);
		setCPU(confInt["lastCPU"]);
		setKbdLayout(confInt["lastKeyboardLayout"]);
		setTefix(confInt["lastTefix"]);
		chdir(confStr["lastDirectory"].c_str());
		quit();
		system("killall espeak");
		string prevCmd = confStr["lastCommand"].c_str();
		string tmppath = exe_path() + "/gmenu2x.conf";
		string writeDateCmd = "; sed -i \"1s/.*/datetime=\\\"$(date +\\%\\F\\ %H:%M)\\\"/\" ";
#if defined(TARGET_LINUX)
		string exitCmd = "; exit" ;
#else
		string exitCmd = "; sync; mount -o remount,ro $HOME; poweroff";
#endif
		string launchCmd = prevCmd + writeDateCmd + tmppath + exitCmd;
		execlp("/bin/sh", "/bin/sh", "-c", launchCmd.c_str(),  NULL);
	}
	currBackdrop = confStr["wallpaper"];
	confStr["wallpaper"] = setBackground(s, currBackdrop);
	bg = new Surface(s);
	
	if (readTmp() && confInt["outputLogs"]) {
		viewLog();
	}
	
	input.dropEvents();

	SDL_TimerID hwCheckTimer = SDL_AddTimer(1000, hwCheck, NULL);

	powerManager->resetSuspendTimer();

	// recover last session
	if (lastSelectorElement >= 0 && menu->selLinkApp() != NULL && (!menu->selLinkApp()->getSelectorDir().empty() || !lastSelectorDir.empty())) {
		if (confInt["skinBackdrops"] & BD_DIALOG)
			setBackground(bg, currBackdrop);
		else
			setBackground(bg, confStr["wallpaper"]);
		menu->selLinkApp()->selector(lastSelectorElement, lastSelectorDir);
	}

	menu->exec();
}

bool GMenu2X::inputCommonActions(bool &inputAction) {
	if (powerManager->suspendActive) {
		// SUSPEND ACTIVE
		while (!(input[POWER] || input[SETTINGS] || input[UDC_CONNECT] || input[UDC_REMOVE] || input[MMC_INSERT] || input[MMC_REMOVE])) {
			input.update();
		}

		powerManager->doSuspend(0);
		input[WAKE_UP] = true;

		if (!(input[UDC_REMOVE] || input[UDC_CONNECT] || input[MMC_INSERT] || input[MMC_REMOVE])) {
			return true;
		}
	}

	if (inputAction) powerManager->resetSuspendTimer();

	uint32_t button_hold = SDL_GetTicks();

	int wasActive = 0;

	while (input[MODIFIER]) { // MODIFIER HOLD
		wasActive = MODIFIER;

		input.update();

		if (SDL_GetTicks() - button_hold > 1000) {
#if defined(HW_LIDVOL)
			confInt["backlight"] = getBacklight();
			confInt["globalVolume"] = getVolume();
#endif
			wasActive = 0;
			settings_date();
			powerManager->doSuspend(0);
		}
	}

	while (input[MANUAL]) { // MANUAL HOLD
		wasActive = MANUAL;

		input.update();

		if (SDL_GetTicks() - button_hold > 1000) {
			wasActive = 0;
			reinit();
		}
	}

	while (input[SETTINGS]) { // SETTINGS HOLD
		wasActive = SETTINGS;

		input.update();

		if (SDL_GetTicks() - button_hold > 1000 && !actionPerformed) {
			wasActive = 0;
			actionPerformed = true;
			powerManager->doSuspend(1);
		}
	}
	

	while (input[MENU]) { // MENU HOLD
		wasActive = MENU;

		input.update();

		if (SDL_GetTicks() - button_hold > 1000) {
			wasActive = 0;
			setTVoff();
		} else if (input[SETTINGS]) {
			wasActive = SCREENSHOT;
		} else if (input[BACKLIGHT_HOTKEY]) {
			wasActive = BACKLIGHT;
		} else if (input[VOLUME_HOTKEY]) {
			wasActive = VOLUP;
		} else if (input[POWER]) {
			wasActive = UDC_CONNECT;
		} else {
			continue;
		}
		break;
	}

	input[wasActive] = true;

	if (input[POWER]) {
		poweroffDialog();
	} else if (input[SCREENSHOT]) {
		if (!saveScreenshot(confStr["homePath"])) {
			ERROR("Can't save screenshot");
			return true;
		}
		MessageBox mb(this, tr["Screenshot saved"]);
		mb.setAutoHide(1000);
		mb.exec();

	} else if (input[VOLUP] || input[VOLDOWN]) {
		setVolume(getVolume(), true);

	} else if (input[BACKLIGHT]) {
		setBacklight(getBacklight(), true);

	} else if (input[UDC_CONNECT]) {
		powerManager->setPowerTimeout(0);
		batteryIcon = 6;
		udcDialog(UDC_CONNECT);

	} else if (input[UDC_REMOVE]) {
		udcDialog(UDC_REMOVE);
		iconInet = NULL;
		batteryIcon = getBatteryStatus(getBatteryLevel(), confInt["minBattery"], confInt["maxBattery"]);
		powerManager->setPowerTimeout(confInt["powerTimeout"]);

	} else if (input[TV_CONNECT]) {
		tvOutDialog();

	} else if (input[TV_REMOVE]) {
		tvOutDialog(TV_OFF);

	} else if (input[JOYSTICK_CONNECT]) {
		input.initJoysticks(true);

	// } else if (input[PHONES_CONNECT]) {
	// 	// tvOutDialog(TV_OFF);
	// 	WARNING("volume mode changed");
	// 	return true;

	} else if (input[MMC_INSERT] || input[MMC_REMOVE]) {
		confInt["section"] = menu->selSectionIndex();
		confInt["link"] = menu->selLinkIndex();
		initMenu();

	} else {
		return false;
	}
	return true;
}

void GMenu2X::cls(Surface *s, bool flip) {
	if (s == NULL) {
		s = this->s;
	}

	s->box((SDL_Rect){0, 0, s->width(), s->height()}, (RGBAColor){0, 0, 0, 255});

	if (flip) {
		s->flip();
	}
}

string GMenu2X::setBackground(Surface *bg, string wallpaper) {
	if (!sc.exists(wallpaper)) { // search and scale background
		if (wallpaper.empty() || sc[wallpaper] == NULL) {
			DEBUG("Searching wallpaper");
			FileLister fl("skins/Default/wallpapers", false, true);
			fl.setFilter(".png,.jpg,.jpeg,.bmp");
			fl.browse();
			wallpaper = "skins/Default/wallpapers/" + fl.getFiles()[0];
		}
		if (sc[wallpaper] == NULL) return "";
		if (confStr["bgscale"] == "Stretch") sc[wallpaper]->softStretch(this->w, this->h, SScaleStretch);
		else if (confStr["bgscale"] == "Crop") sc[wallpaper]->softStretch(this->w, this->h, SScaleMax);
		else if (confStr["bgscale"] == "Aspect") sc[wallpaper]->softStretch(this->w, this->h, SScaleFit);
	}

	cls(bg, false);
	sc[wallpaper]->blit(bg, (this->w - sc[wallpaper]->width()) / 2, (this->h - sc[wallpaper]->height()) / 2);
	return wallpaper;
}

void GMenu2X::initFont() {
	string skinFont = confStr["skinFont"] == "Default" ? "skins/Default/font.ttf" : sc.getSkinFilePath("font.ttf");

	delete font;
	font = new FontHelper(skinFont, skinConfInt["fontSize"], skinConfColors[COLOR_FONT], skinConfColors[COLOR_FONT_OUTLINE]);
	if (!font->font) {
		delete font;
		font = new FontHelper("skins/Default/font.ttf", skinConfInt["fontSize"], skinConfColors[COLOR_FONT], skinConfColors[COLOR_FONT_OUTLINE]);
	}

	delete titlefont;
	titlefont = new FontHelper(skinFont, skinConfInt["fontSizeTitle"], skinConfColors[COLOR_FONT], skinConfColors[COLOR_FONT_OUTLINE]);
	if (!titlefont->font) {
		delete titlefont;
		titlefont = new FontHelper("skins/Default/font.ttf", skinConfInt["fontSizeTitle"], skinConfColors[COLOR_FONT], skinConfColors[COLOR_FONT_OUTLINE]);
	}
}

void GMenu2X::initMenu() {
	// Menu structure handler
	menu->initLayout();
	menu->readSections();
	menu->readLinks();

	for (uint32_t i = 0; i < menu->getSections().size(); i++) {
		// Add virtual links in the applications section
		if (menu->getSections()[i] == "applications") {
			menu->addActionLink(i, tr["Explorer"], MakeDelegate(this, &GMenu2X::explorer), tr["Browse files and launch apps"], "explorer.png");

#if defined(HW_EXT_SD)
			menu->addActionLink(i, tr["Umount"], MakeDelegate(this, &GMenu2X::umountSdDialog), tr["Umount external media device"], "eject.png");
#endif
		}
		// Add virtual links in the setting section
		else if (menu->getSections()[i] == "settings") {
			menu->addActionLink(i, tr["Settings"], MakeDelegate(this, &GMenu2X::settings), tr["Configure system"], "configure.png");
			menu->addActionLink(i, tr["Skin"], MakeDelegate(this, &GMenu2X::skinMenu), tr["Appearance & skin settings"], "skin.png");
#if defined(TARGET_GP2X)
			if (fwType == "open2x") {
				menu->addActionLink(i, "Open2x", MakeDelegate(this, &GMenu2X::settingsOpen2x), tr["Configure Open2x system settings"], "o2xconfigure.png");
			}
			menu->addActionLink(i, "USB SD", MakeDelegate(this, &GMenu2X::activateSdUsb), tr["Activate USB on SD"], "usb.png");
			if (fwType == "gph" && !f200) {
				menu->addActionLink(i, "USB Nand", MakeDelegate(this, &GMenu2X::activateNandUsb), tr["Activate USB on NAND"], "usb.png");
			}
#endif
			if (file_exists(exe_path() + "/log.txt")) {
				menu->addActionLink(i, tr["Log Viewer"], MakeDelegate(this, &GMenu2X::viewLog), tr["Displays last launched program's output"], "ebook.png");
			}

			menu->addActionLink(i, tr["About"], MakeDelegate(this, &GMenu2X::about), tr["Info about GMenuNX"], "about.png");
			menu->addActionLink(i, tr["Power"], MakeDelegate(this, &GMenu2X::poweroffDialog), tr["Power menu"], "exit.png");
			menu->addActionLink(i, tr["CPU Settings"], MakeDelegate(this, &GMenu2X::cpuSettings), tr["Config CPU clock"], "cpu.png");
		}
	}
	menu->setSectionIndex(confInt["section"]);
	menu->setLinkIndex(confInt["link"]);
}

void GMenu2X::settings_date() {
	powerManager->clearTimer();

	// int prevgamma = confInt["gamma"];

	vector<string> opFactory;
	opFactory.push_back(">>");
	string tmp = ">>";

	SettingsDialog sd(this, ts, tr["SW Clock"], "skin:icons/clock.png");
	sd.allowCancel = true;

	string prevDateTime = confStr["datetime"] = get_date_time();
	sd.addSetting(new MenuSettingDateTime(this, tr["Date & Time"], tr["Set system's software clock"], &confStr["datetime"]));

	if (sd.exec() && sd.edited() && sd.save) {

		writeConfig();
	}
		string freshDateTime = confStr["datetime"];
		if (prevDateTime != confStr["datetime"]) {
			set_date_time(freshDateTime.c_str());
			powerManager->doSuspend(0);
			}
}

void GMenu2X::settings() {
	powerManager->clearTimer();

#if defined(HW_LIDVOL)
	confInt["backlight"] = getBacklight();
	confInt["globalVolume"] = getVolume();
#endif
	// int prevgamma = confInt["gamma"];
	FileLister fl_tr("translations");
	fl_tr.browse();
	fl_tr.insertFile("English");
	string lang = tr.lang();
	if (lang == "") lang = "English";

	vector<string> opFactory;
	opFactory.push_back(">>");
	string tmp = ">>";

	SettingsDialog sd(this, ts, tr["Settings"], "skin:icons/configure.png");
	sd.allowCancel = true;

	sd.addSetting(new MenuSettingMultiString(this, tr["Language"], tr["Set the language used by GMenuNX"], &lang, &fl_tr.getFiles()));

	string prevDateTime = confStr["datetime"] = get_date_time();
	sd.addSetting(new MenuSettingDateTime(this, tr["Date & Time"], tr["Set system's date & time"], &confStr["datetime"]));
	sd.addSetting(new MenuSettingDir(this, tr["Home path"],	tr["Set as home for launched links"], &confStr["homePath"]));

#if defined(HW_UDC)
	vector<string> usbMode;
	usbMode.push_back("Ask");
	usbMode.push_back("Storage");
	usbMode.push_back("Charger");
	sd.addSetting(new MenuSettingMultiString(this, tr["USB mode"], tr["Define default USB mode"], &confStr["usbMode"], &usbMode));
#endif

#if defined(HW_TVOUT)
	vector<string> tvMode;
	tvMode.push_back("Ask");
	tvMode.push_back("NTSC");
	tvMode.push_back("PAL");
	tvMode.push_back("OFF");
	sd.addSetting(new MenuSettingMultiString(this, tr["TV mode"], tr["Define default TV mode"], &confStr["tvMode"], &tvMode));
#endif

	sd.addSetting((MenuSettingInt *)(new MenuSettingInt(this, tr["Suspend timeout"], tr["Seconds until suspend the device when inactive"], &confInt["backlightTimeout"], 30, 0, 300))->setOff(9));
	sd.addSetting((MenuSettingInt *)(new MenuSettingInt(this, tr["Power timeout"], tr["Minutes to poweroff system if inactive"], &confInt["powerTimeout"], 10, 0, 60))->setOff(0));
	sd.addSetting(new MenuSettingInt(this, tr["Backlight"], tr["Set LCD backlight"], &confInt["backlight"], 50, 10, 100, 10));
	sd.addSetting(new MenuSettingInt(this, tr["Audio volume"], tr["Set the default audio volume"], &confInt["globalVolume"], 50, 0, 90, 10));
	sd.addSetting(new MenuSettingInt(this, tr["Keyboard layout"], tr["Set the default A/B/X/Y layout"], &confInt["keyboardLayoutMenu"], DEFAULT_LAYOUT, 1, confInt["keyboardLayoutMax"]));
	sd.addSetting(new MenuSettingInt(this, tr["TEfix method"], tr["Set the default tearing FIX method"], &confInt["tefixMenu"], DEFAULT_TEFIX, 0, confInt["tefixMax"]));
	sd.addSetting(new MenuSettingBool(this, tr["Remember selection"], tr["Remember the last selected section, link and file"], &confInt["saveSelection"]));
	sd.addSetting(new MenuSettingBool(this, tr["Autostart"], tr["Run last app on restart"], &confInt["saveAutoStart"]));
	sd.addSetting(new MenuSettingBool(this, tr["Hints"], tr["Show \"Hint\" messages"], &confInt["showHints"]));
	sd.addSetting(new MenuSettingBool(this, tr["Output logs"], tr["Logs the link's output to read with Log Viewer"], &confInt["outputLogs"]));
	sd.addSetting(new MenuSettingMultiString(this, tr["Reset settings"], tr["Choose settings to reset back to defaults"], &tmp, &opFactory, 0, MakeDelegate(this, &GMenu2X::resetSettings)));

	if (sd.exec() && sd.edited() && sd.save) {
		writeConfig();
		if (lang == "English") lang = "";
		if (confStr["lang"] != lang) {
			confStr["lang"] = lang;
			tr.setLang(lang);
		}
	}

	setBacklight(confInt["backlight"], false);

#if defined(TARGET_GP2X)
	if (prevgamma != confInt["gamma"]) {
		setGamma(confInt["gamma"]);
	}
#endif
	string freshDateTime = confStr["datetime"];
	if (prevDateTime != confStr["datetime"]) {
		set_date_time(freshDateTime.c_str());
		powerManager->doSuspend(0);
	}
	
	powerManager->setSuspendTimeout(confInt["backlightTimeout"]);
	powerManager->setPowerTimeout(confInt["powerTimeout"]);

	if (sd.exec() && sd.edited() && sd.save) {
		if (confInt["saveAutoStart"]) confInt["dialogAutoStart"] = 1;
		reinit_save();
	}
}

void GMenu2X::resetSettings() {
	bool
		reset_gmenu = false,
		reset_skin = false,
		reset_icon = false,
		/* reset_homedir = false, */
		reset_manual = false,
		reset_parameter = false,
		reset_backdrop = false,
		reset_filter = false,
		reset_directory = false,
		reset_boxart = false,
		reset_cpu = false;

	SettingsDialog sd(this, ts, tr["Reset settings"], "skin:icons/configure.png");
	sd.allowCancel_link_nomb = true;
	sd.addSetting(new MenuSettingBool(this, tr["GMenuNX"], tr["Reset GMenuNX settings"], &reset_gmenu));
	sd.addSetting(new MenuSettingBool(this, tr["Default skin"], tr["Reset Default skin settings back to default"], &reset_skin));
	sd.addSetting(new MenuSettingBool(this, tr["Icons"], tr["Reset link's icon back to default"], &reset_icon));
	sd.addSetting(new MenuSettingBool(this, tr["Manuals"], tr["Unset link's manual"], &reset_manual));
	// sd.addSetting(new MenuSettingBool(this, tr["Home directory"], tr["Unset link's home directory"], &reset_homedir));
	sd.addSetting(new MenuSettingBool(this, tr["Parameters"], tr["Unset link's additional parameters"], &reset_parameter));
	sd.addSetting(new MenuSettingBool(this, tr["Backdrops"], tr["Unset link's backdrops"], &reset_backdrop));
	sd.addSetting(new MenuSettingBool(this, tr["Filters"], tr["Unset link's selector file filters"], &reset_filter));
	sd.addSetting(new MenuSettingBool(this, tr["Directories"], tr["Unset link's selector directory"], &reset_directory));
	sd.addSetting(new MenuSettingBool(this, tr["Box art"], tr["Unset link's selector box art path"], &reset_boxart));
	
	if (CPU_MAX != CPU_MIN) {
		sd.addSetting(new MenuSettingBool(this, tr["CPU speed"], tr["Reset link's custom CPU speed back to default"], &reset_cpu));
	  }

	if (sd.exec() && sd.edited() && sd.save) {
		MessageBox mb(this, tr["Changes will be applied to ALL"]+"\n"+tr["apps and GMenuNX. Are you sure?"], "skin:icons/exit.png");
		mb.setButton(CANCEL, tr["Cancel"]);
		mb.setButton(MANUAL,  tr["Yes"]);
		if (mb.exec() != MANUAL) return;

		for (uint32_t s = 0; s < menu->getSections().size(); s++) {
			for (uint32_t l = 0; l < menu->sectionLinks(s)->size(); l++) {
				menu->setSectionIndex(s);
				menu->setLinkIndex(l);
				bool islink = menu->selLinkApp() != NULL;
				// WARNING("APP: %d %d %d %s", s, l, islink, menu->sectionLinks(s)->at(l)->getTitle().c_str());
				if (!islink) continue;
				if (reset_cpu)			menu->selLinkApp()->setCPU();
				if (reset_icon)			menu->selLinkApp()->setIcon("");
				// if (reset_homedir)		menu->selLinkApp()->setHomeDir("");
				if (reset_manual)		menu->selLinkApp()->setManual("");
				if (reset_parameter) 	menu->selLinkApp()->setParams("");
				if (reset_filter) 		menu->selLinkApp()->setSelectorFilter("");
				if (reset_directory) 	menu->selLinkApp()->setSelectorDir("");
				if (reset_boxart) 		menu->selLinkApp()->setSelectorScreens("");
				if (reset_backdrop) 	menu->selLinkApp()->setBackdrop("");
				if (reset_cpu || reset_icon || reset_manual || reset_parameter || reset_backdrop || reset_filter || reset_directory || reset_boxart )
					menu->selLinkApp()->save();
			}
		}
		if (reset_skin) {
			string tmppath = exe_path() + "/skins/Default/skin.conf";
			unlink(tmppath.c_str());
		}
		if (reset_gmenu) {
			string tmppath = exe_path() + "/gmenu2x.conf";
			unlink(tmppath.c_str());
		}
		reinit();
	}
}

void GMenu2X::cpuSettings() {  
SettingsDialog sd(this, ts, tr["CPU setup"], "skin:icons/cpu.png");
sd.allowCancel = true;
#if defined(MULTI_INT)
sd.addSetting(new MenuSettingMultiInt(this, tr["Default CPU clock"], tr["Set the default working CPU frequency"], &confInt["cpuMenu"], oc_choices, oc_choices_size, CPU_MENU, CPU_MIN, CPU_MENU));
sd.addSetting(new MenuSettingMultiInt(this, tr["Maximum CPU clock"], tr["Maximum overclock for launching links"], &confInt["cpuMax"], oc_choices, oc_choices_size, CPU_MAX, CPU_EDGE, CPU_MAX));
sd.addSetting(new MenuSettingMultiInt(this, tr["Minimum CPU clock"], tr["Minimum underclock used in Suspend mode"], &confInt["cpuMin"], oc_choices, oc_choices_size, CPU_MIN, CPU_MIN, CPU_MAX));
sd.addSetting(new MenuSettingMultiInt(this, tr["Link CPU clock"], tr["Set LinkApp default CPU frequency"], &confInt["cpuLink"], oc_choices, oc_choices_size, CPU_MENU, CPU_MIN, CPU_MAX));
#else
sd.addSetting(new MenuSettingInt(this, tr["Default CPU clock"], tr["Set the default working CPU frequency"], &confInt["cpuMenu"], 672, 480, 864, 48));
sd.addSetting(new MenuSettingInt(this, tr["Maximum CPU clock"], tr["Maximum overclock for launching links"], &confInt["cpuMax"], 864, 720, 1248, 48));
sd.addSetting(new MenuSettingInt(this, tr["Minimum CPU clock"], tr["Minimum underclock used in Suspend mode"], &confInt["cpuMin"], 192, 48, 720, 48));
sd.addSetting(new MenuSettingInt(this, tr["Link CPU clock"], tr["Set LinkApp default CPU frequency"], &confInt["cpuLink"], 720, 480, 1248, 48));
sd.addSetting(new MenuSettingInt(this, tr["Step for clock values"], tr["Set Step default CPU frequency"], &confInt["cpuStep"], 48, 48, 240, 48));
#endif
if (sd.exec() && sd.edited() && sd.save) {
		setCPU(confInt["cpuMenu"]);
 		writeConfig();
 	}
  }

bool GMenu2X::readTmp() {
	lastSelectorElement = -1;
	ifstream tmp("/tmp/gmenu2x.tmp", ios_base::in);
	if (!tmp.is_open()) return false;

	string line;

	while (getline(tmp, line, '\n')) {
		string::size_type pos = line.find("=");
		string name = trim(line.substr(0, pos));
		string value = trim(line.substr(pos + 1));

		if (name == "section") menu->setSectionIndex(atoi(value.c_str()));
		else if (name == "link") menu->setLinkIndex(atoi(value.c_str()));
		else if (name == "selectorelem") lastSelectorElement = atoi(value.c_str());
		else if (name == "selectordir") lastSelectorDir = value;
		// else if (name == "TVOut") TVOut = atoi(value.c_str());
		else if (name == "tvOutPrev") tvOutPrev = atoi(value.c_str());
		else if (name == "udcPrev") udcPrev = atoi(value.c_str());
		else if (name == "currBackdrop") currBackdrop = value;
		else if (name == "explorerLastDir") confStr["explorerLastDir"] = value;
	}
	// if (TVOut > 2) TVOut = 0;
	tmp.close();
	unlink("/tmp/gmenu2x.tmp");
	return true;
}

void GMenu2X::writeTmp(int selelem, const string &selectordir) {
	ofstream tmp("/tmp/gmenu2x.tmp");
	if (tmp.is_open()) {
		tmp << "section=" << menu->selSectionIndex() << endl;
		tmp << "link=" << menu->selLinkIndex() << endl;
		if (selelem >- 1) tmp << "selectorelem=" << selelem << endl;
		if (selectordir != "") tmp << "selectordir=" << selectordir << endl;
		tmp << "udcPrev=" << udcPrev << endl;
		tmp << "tvOutPrev=" << tvOutPrev << endl;
		// tmp << "TVOut=" << TVOut << endl;
		tmp << "currBackdrop=" << currBackdrop << endl;
		if (!confStr["explorerLastDir"].empty()) tmp << "explorerLastDir=" << confStr["explorerLastDir"] << endl;
		tmp.close();
	}
}

void GMenu2X::readConfig() {
#if defined (__BUILDTIME__)
#define xstr(s) str(s)
#define str(s) #s
#endif
	string conf = exe_path() + "/gmenu2x.conf";
	// Defaults *** Sync with default values in writeConfig
	confInt["saveSelection"] = 1;
	confInt["dialogAutoStart"] = 1;
	confInt["showHints"] = 1;
	confStr["datetime"] = xstr(__BUILDTIME__);
	confInt["skinBackdrops"] = 1;
	confStr["homePath"] = CARD_ROOT;
	confInt["keyboardLayoutMenu"] = LAYOUT_VERSION;
	confInt["keyboardLayoutMax"] = LAYOUT_VERSION_MAX;
	confInt["tefixMenu"] = TEFIX;
	confInt["tefixMax"] = TEFIX_MAX;
	confStr["bgscale"] = "Crop";
	confStr["skinFont"] = "Custom";
	confInt["backlightTimeout"] = 30;
	confInt["powerTimeout"] = 0;
#if !defined(HW_LIDVOL)
	confInt["backlight"] = 50;
	confInt["globalVolume"] = 50;
#endif	
	confInt["cpuMenu"] = CPU_MENU;
	confInt["cpuMax"] = CPU_MAX;
	confInt["cpuMin"] = CPU_MIN;
	confInt["cpuLink"] = CPU_LINK;
	confInt["cpuStep"] = CPU_STEP;

	// input.update(false);

	// if (!input[SETTINGS] && file_exists(conf)) {

	ifstream cfg(conf.c_str(), ios_base::in);
	if (cfg.is_open()) {
		string line;
		while (getline(cfg, line, '\n')) {
			string::size_type pos = line.find("=");
			string name = trim(line.substr(0, pos));
			string value = trim(line.substr(pos + 1));

			if (value.length() > 1 && value.at(0) == '"' && value.at(value.length() - 1) == '"')
				confStr[name] = value.substr(1, value.length() - 2);
				confInt[name] = atoi(value.c_str());
		}
		cfg.close();
	}

	if (!confStr["lang"].empty()) {
		tr.setLang(confStr["lang"]);
		string voiceTTS = tr["TTS voice"];
		INFO("voice is set to %s",voiceTTS.c_str());
		if (voiceTTS != "TTS voice" && !voiceTTS.empty()) VOICE_TTS = voiceTTS;
	}
	if (!confStr["wallpaper"].empty() && !file_exists(confStr["wallpaper"])) confStr["wallpaper"] = "";
	if (confStr["skin"].empty() || !dir_exists("skins/" + confStr["skin"])) confStr["skin"] = "Default";

	evalIntConf(&confInt["backlightTimeout"], 30, 0, 300);
	evalIntConf(&confInt["powerTimeout"], 10, 0, 60);
	evalIntConf(&confInt["outputLogs"], 0, 0, 1 );
	// evalIntConf(&confInt["cpuMax"], 2000, 200, 2000 );
	// evalIntConf(&confInt["cpuMin"], 342, 200, 1200 );
	// evalIntConf(&confInt["cpuMenu"], 528, 200, 1200 );
	evalIntConf(&confInt["globalVolume"], 60, 0, 100 );
	evalIntConf(&confInt["backlight"], 70, 1, 100);
	evalIntConf(&confInt["minBattery"], 3550, 1, 10000);
	evalIntConf(&confInt["maxBattery"], 3720, 1, 10000);

	if (!confInt["saveSelection"]) {
		confInt["section"] = 0;
		confInt["link"] = 0;
	}
}

void GMenu2X::writeConfig() {
	if (confInt["saveSelection"] && menu != NULL) {
		confInt["section"] = menu->selSectionIndex();
		confInt["link"] = menu->selLinkIndex();
	}

	string conf = exe_path() + "/gmenu2x.conf";
	ofstream cfg(conf.c_str());
	if (cfg.is_open()) {
		for (ConfStrHash::iterator curr = confStr.begin(); curr != confStr.end(); curr++) {
			if (
				// deprecated
				curr->first == "sectionBarPosition" ||
				curr->first == "tvoutEncoding" ||
				curr->first == "explorerLastDir" ||
				curr->first == "defaultDir" ||

				// defaults
				(curr->first == "cpuMenu" && curr->second.empty()) ||
				(curr->first == "cpuMax" && curr->second.empty()) ||
				(curr->first == "cpuMin" && curr->second.empty()) ||
				(curr->first == "cpuLink" && curr->second.empty()) ||
				(curr->first == "cpuStep" && curr->second.empty()) ||
				(curr->first == "datetime" && curr->second.empty()) ||
				(curr->first == "homePath" && curr->second == CARD_ROOT) ||
				(curr->first == "skin" && curr->second == "Default") ||
				(curr->first == "previewMode" && curr->second == "Miniature") ||
				(curr->first == "skinFont" && curr->second == "Custom") ||
				(curr->first == "usbMode" && curr->second == "Ask") ||
				(curr->first == "tvMode" && curr->second == "Ask") ||
				(curr->first == "lang" && curr->second.empty()) ||
				(curr->first == "lang" && curr->second.empty()) ||
				(curr->first == "bgscale" && curr->second.empty()) ||
				(curr->first == "bgscale" && curr->second == "Crop") ||

				curr->first.empty() || curr->second.empty()
			) continue;

			cfg << curr->first << "=\"" << curr->second << "\"" << endl;
		}

		for (ConfIntHash::iterator curr = confInt.begin(); curr != confInt.end(); curr++) {
			if (
				// deprecated
				curr->first == "batteryLog" ||
				curr->first == "maxClock" ||
				curr->first == "minClock" ||
				curr->first == "menuClock" ||
				curr->first == "TVOut" ||

				// moved to skin conf
				curr->first == "linkCols" ||
				curr->first == "linkRows" ||
				curr->first == "sectionBar" ||
				curr->first == "sectionLabel" ||
				curr->first == "linkLabel" ||

				// defaults
				(curr->first == "skinBackdrops" && curr->second == 1) ||
				(curr->first == "backlightTimeout" && curr->second == 30) ||
				(curr->first == "powerTimeout" && curr->second == 10) ||
				(curr->first == "outputLogs" && curr->second == 0) ||
				// (curr->first == "cpuMin" && curr->second == 342) ||
				// (curr->first == "cpuMax" && curr->second == 740) ||
				// (curr->first == "cpuMenu" && curr->second == 528) ||
				(curr->first == "minBattery" && curr->second == 3550) ||
				(curr->first == "maxBattery" && curr->second == 3720) ||
				(curr->first == "saveSelection" && curr->second == 1) ||
				(curr->first == "section" && curr->second == 0) ||
				(curr->first == "link" && curr->second == 0) ||

				curr->first.empty()
			) continue;

			cfg << curr->first << "=" << curr->second << endl;
		}
		cfg.close();
	}
	sync();
}

void GMenu2X::writeSkinConfig() {
	string skinconf = exe_path() + "/skins/" + confStr["skin"] + "/skin.conf";
	ofstream inf(skinconf.c_str());
	if (!inf.is_open()) return;

	for (ConfStrHash::iterator curr = skinConfStr.begin(); curr != skinConfStr.end(); curr++) {
		if (curr->first.empty() || curr->second.empty()) {
			continue;
		}

		inf << curr->first << "=\"" << curr->second << "\"" << endl;
	}

	for (ConfIntHash::iterator curr = skinConfInt.begin(); curr != skinConfInt.end(); curr++) {
		if (
			curr->first == "titleFontSize" ||
			curr->first == "sectionBarHeight" ||
			curr->first == "linkHeight" ||
			curr->first == "selectorPreviewX" ||
			curr->first == "selectorPreviewY" ||
			curr->first == "selectorPreviewWidth" ||
			curr->first == "selectorPreviewHeight" ||
			curr->first == "selectorX" ||
			curr->first == "linkItemHeight" ||
			curr->first == "topBarHeight" ||

			(curr->first == "previewWidth" && curr->second == 128) ||
			(curr->first == "linkCols" && curr->second == 4) ||
			(curr->first == "linkRows" && curr->second == 4) ||
			(curr->first == "sectionBar" && curr->second == SB_CLASSIC) ||
			(curr->first == "sectionLabel" && curr->second == 1) ||
			(curr->first == "linkLabel" && curr->second == 1) ||
			(curr->first == "showDialogIcon" && curr->second == 1) ||

			curr->first.empty()
		) {
			continue;
		}

		inf << curr->first << "=" << curr->second << endl;
	}

	for (int i = 0; i < NUM_COLORS; ++i) {
		inf << colorToString((enum color)i) << "=" << rgbatostr(skinConfColors[i]) << endl;
	}

	inf.close();

	sync();
}

void GMenu2X::setSkin(string skin, bool clearSC) {
	input.update(false);
	if (input[SETTINGS]) skin = "Default";

	confStr["skin"] = skin;

	// Clear previous skin settings
	skinConfStr.clear();
	skinConfInt.clear();

	// Defaults *** Sync with default values in writeConfig
	skinConfInt["sectionBar"] = SB_CLASSIC;
	skinConfInt["sectionLabel"] = 1;
	skinConfInt["linkLabel"] = 1;
	skinConfInt["showDialogIcon"] = 1;

	// clear collection and change the skin path
	if (clearSC) {
		sc.clear();
	}

	sc.setSkin(skin);

	// reset colors to the default values
	skinConfColors[COLOR_TOP_BAR_BG] = (RGBAColor){255,255,255,130};
	skinConfColors[COLOR_LIST_BG] = (RGBAColor){255,255,255,0};
	skinConfColors[COLOR_BOTTOM_BAR_BG] = (RGBAColor){255,255,255,130};
	skinConfColors[COLOR_SELECTION_BG] = (RGBAColor){255,255,255,130};
	skinConfColors[COLOR_PREVIEW_BG] = (RGBAColor){253,1,252,0};;
	skinConfColors[COLOR_MESSAGE_BOX_BG] = (RGBAColor){255,255,255,255};
	skinConfColors[COLOR_MESSAGE_BOX_BORDER] = (RGBAColor){80,80,80,255};
	skinConfColors[COLOR_MESSAGE_BOX_SELECTION] = (RGBAColor){160,160,160,255};
	skinConfColors[COLOR_FONT] = (RGBAColor){255,255,255,255};
	skinConfColors[COLOR_FONT_OUTLINE] = (RGBAColor){0,0,0,200};
	skinConfColors[COLOR_FONT_ALT] = (RGBAColor){253,1,252,0};
	skinConfColors[COLOR_FONT_ALT_OUTLINE] = (RGBAColor){253,1,252,0};

	// load skin settings
	skin = "skins/" + skin + "/skin.conf";

	ifstream skinconf(skin.c_str(), ios_base::in);
	if (skinconf.is_open()) {
		string line;
		while (getline(skinconf, line, '\n')) {
			line = trim(line);
			// DEBUG("skinconf: '%s'", line.c_str());
			string::size_type pos = line.find("=");
			string name = trim(line.substr(0, pos));
			string value = trim(line.substr(pos + 1));

			if (value.length() > 0) {
				if (value.length() > 1 && value.at(0) == '"' && value.at(value.length() - 1) == '"') {
					skinConfStr[name] = value.substr(1, value.length() - 2);
				} else if (value.at(0) == '#') {
					// skinConfColor[name] = strtorgba(value.substr(1,value.length()) );
					skinConfColors[stringToColor(name)] = strtorgba(value);
				} else if (name.length() > 6 && name.substr(name.length() - 6, 5) == "Color") {
					value += name.substr(name.length() - 1);
					name = name.substr(0, name.length() - 6);
					if (name == "selection" || name == "topBar" || name == "bottomBar" || name == "messageBox") name += "Bg";
					if (value.substr(value.length() - 1) == "R") skinConfColors[stringToColor(name)].r = atoi(value.c_str());
					if (value.substr(value.length() - 1) == "G") skinConfColors[stringToColor(name)].g = atoi(value.c_str());
					if (value.substr(value.length() - 1) == "B") skinConfColors[stringToColor(name)].b = atoi(value.c_str());
					if (value.substr(value.length() - 1) == "A") skinConfColors[stringToColor(name)].a = atoi(value.c_str());
				} else {
					skinConfInt[name] = atoi(value.c_str());
				}
			}
		}
		skinconf.close();
	}

	// (poor) HACK: ensure some colors have a default value
	if (skinConfColors[COLOR_FONT_ALT].r == 253 && skinConfColors[COLOR_FONT_ALT].g == 1 && skinConfColors[COLOR_FONT_ALT].b == 252 && skinConfColors[COLOR_FONT_ALT].a == 0) {
		skinConfColors[COLOR_FONT_ALT] = skinConfColors[COLOR_FONT];
	}
	if (skinConfColors[COLOR_FONT_ALT_OUTLINE].r == 253 && skinConfColors[COLOR_FONT_ALT_OUTLINE].g == 1 && skinConfColors[COLOR_FONT_ALT_OUTLINE].b == 252 && skinConfColors[COLOR_FONT_ALT_OUTLINE].a == 0) {
		skinConfColors[COLOR_FONT_ALT_OUTLINE] = skinConfColors[COLOR_FONT_OUTLINE];
	}
	if (skinConfColors[COLOR_PREVIEW_BG].r == 253 && skinConfColors[COLOR_PREVIEW_BG].g == 1 && skinConfColors[COLOR_PREVIEW_BG].b == 252 && skinConfColors[COLOR_PREVIEW_BG].a == 0) {
		skinConfColors[COLOR_PREVIEW_BG] = skinConfColors[COLOR_TOP_BAR_BG];
		skinConfColors[COLOR_PREVIEW_BG].a == 170;
	}

	// prevents breaking current skin until they are updated
	if (!skinConfInt["fontSizeTitle"] && skinConfInt["titleFontSize"] > 0) skinConfInt["fontSizeTitle"] = skinConfInt["titleFontSize"];

	evalIntConf(&skinConfInt["sectionBarSize"], 40, 1, this->w);
	evalIntConf(&skinConfInt["bottomBarHeight"], 16, 1, this->h);
	evalIntConf(&skinConfInt["previewWidth"], 128, 1, this->w);
	evalIntConf(&skinConfInt["fontSize"], 12, 6, 60);
	evalIntConf(&skinConfInt["fontSizeTitle"], 20, 1, 60);
	evalIntConf(&skinConfInt["sectionBar"], SB_CLASSIC, 0, 5);
	evalIntConf(&skinConfInt["linkCols"], 4, 1, 8);
	evalIntConf(&skinConfInt["linkRows"], 4, 1, 8);

	initFont();
}

void GMenu2X::skinMenu() {
	bool save = false;
	int selected = 0;
	string initSkin = confStr["skin"];
	string prevSkin = "/";

	FileLister fl_sk("skins", true, false);
	fl_sk.addExclude("..");
	fl_sk.browse();

	vector<string> wpLabel;
	wpLabel.push_back(">>");
	string tmp = ">>";

	vector<string> sbStr;
	sbStr.push_back("OFF");
	sbStr.push_back("Left");
	sbStr.push_back("Bottom");
	sbStr.push_back("Right");
	sbStr.push_back("Top");
	sbStr.push_back("Classic");
	int sbPrev = skinConfInt["sectionBar"];
	string sectionBar = sbStr[skinConfInt["sectionBar"]];

	int linkColsPrev = skinConfInt["linkCols"];
	int linkRowsPrev = skinConfInt["linkRows"];

	vector<string> bgScale;
	bgScale.push_back("Original");
	bgScale.push_back("Crop");
	bgScale.push_back("Aspect");
	bgScale.push_back("Stretch");
	string bgScalePrev = confStr["bgscale"];

	vector<string> bdStr;
	bdStr.push_back("OFF");
	bdStr.push_back("Menu only");
	bdStr.push_back("Dialog only");
	bdStr.push_back("Menu & Dialog");
	int bdPrev = confInt["skinBackdrops"];
	string skinBackdrops = bdStr[confInt["skinBackdrops"]];

	vector<string> skinFont;
	skinFont.push_back("Custom");
	skinFont.push_back("Default");
	string skinFontPrev = confStr["skinFont"];

	vector<string> previewMode;
	previewMode.push_back("Miniature");
	previewMode.push_back("Backdrop");

	vector<string> wallpapers;
	string wpPath = confStr["wallpaper"];
	confStr["tmp_wallpaper"] = "";

	do {
		if (prevSkin != confStr["skin"] || skinFontPrev != confStr["skinFont"]) {

			prevSkin = confStr["skin"];
			skinFontPrev = confStr["skinFont"];

			setSkin(confStr["skin"], false);
			sectionBar = sbStr[skinConfInt["sectionBar"]];

			confStr["tmp_wallpaper"] = (confStr["tmp_wallpaper"].empty() || skinConfStr["wallpaper"].empty()) ? base_name(confStr["wallpaper"]) : skinConfStr["wallpaper"];
			wallpapers.clear();

			FileLister fl_wp("skins/" + confStr["skin"] + "/wallpapers");
			fl_wp.setFilter(".png,.jpg,.jpeg,.bmp");
			fl_wp.browse();
			wallpapers = fl_wp.getFiles();

			if (confStr["skin"] != "Default") {
				fl_wp.setPath("skins/Default/wallpapers");
				fl_wp.browse();
				wallpapers.insert(wallpapers.end(), fl_wp.getFiles().begin(), fl_wp.getFiles().end());
			}

			sc.del("skin:icons/skin.png");
			sc.del("skin:imgs/buttons/left.png");
			sc.del("skin:imgs/buttons/right.png");
			sc.del("skin:imgs/buttons/a.png");
		}

		wpPath = "skins/" + confStr["skin"] + "/wallpapers/" + confStr["tmp_wallpaper"];
		if (!file_exists(wpPath)) wpPath = "skins/Default/wallpapers/" + confStr["tmp_wallpaper"];
		if (!file_exists(wpPath)) wpPath = "skins/" + confStr["skin"] + "/wallpapers/" + wallpapers.at(0);
		if (!file_exists(wpPath)) wpPath = "skins/Default/wallpapers/" + wallpapers.at(0);

		setBackground(bg, wpPath);

		SettingsDialog sd(this, ts, tr["Skin"], "skin:icons/skin.png");
		sd.selected = selected;
		sd.allowCancel_nomb = true;
		sd.addSetting(new MenuSettingMultiString(this, tr["Skin"], tr["Set the skin used by GMenuNX"], &confStr["skin"], &fl_sk.getDirectories(), MakeDelegate(this, &GMenu2X::onChangeSkin)));
		sd.addSetting(new MenuSettingMultiString(this, tr["Wallpaper"], tr["Select an image to use as a wallpaper"], &confStr["tmp_wallpaper"], &wallpapers, MakeDelegate(this, &GMenu2X::onChangeSkin), MakeDelegate(this, &GMenu2X::changeWallpaper)));
		sd.addSetting(new MenuSettingMultiString(this, tr["Background"], tr["How to scale wallpaper, backdrops and game art"], &confStr["bgscale"], &bgScale));
		sd.addSetting(new MenuSettingMultiString(this, tr["Preview mode"], tr["How to show image preview and game art"], &confStr["previewMode"], &previewMode));
		sd.addSetting(new MenuSettingMultiString(this, tr["Skin colors"], tr["Customize skin colors"], &tmp, &wpLabel, MakeDelegate(this, &GMenu2X::onChangeSkin), MakeDelegate(this, &GMenu2X::skinColors)));
		sd.addSetting(new MenuSettingMultiString(this, tr["Skin backdrops"], tr["Automatic load backdrops from skin pack"], &skinBackdrops, &bdStr));
		sd.addSetting(new MenuSettingMultiString(this, tr["Font face"], tr["Override the skin font face"], &confStr["skinFont"], &skinFont, MakeDelegate(this, &GMenu2X::onChangeSkin)));
		sd.addSetting(new MenuSettingInt(this, tr["Font size"], tr["Size of text font"], &skinConfInt["fontSize"], 12, 6, 60));
		sd.addSetting(new MenuSettingInt(this, tr["Title font size"], tr["Size of title's text font"], &skinConfInt["fontSizeTitle"], 20, 1, 60));
		sd.addSetting(new MenuSettingMultiString(this, tr["Section bar layout"], tr["Set the layout and position of the Section Bar"], &sectionBar, &sbStr));
		sd.addSetting(new MenuSettingInt(this, tr["Section bar size"], tr["Size of section and top bar"], &skinConfInt["sectionBarSize"], 40, 1, this->w));
		sd.addSetting(new MenuSettingInt(this, tr["Bottom bar height"], tr["Height of bottom bar"], &skinConfInt["bottomBarHeight"], 16, 1, this->h));
		sd.addSetting(new MenuSettingInt(this, tr["Menu columns"], tr["Number of columns of links in main menu"], &skinConfInt["linkCols"], 4, 1, 8));
		sd.addSetting(new MenuSettingInt(this, tr["Menu rows"], tr["Number of rows of links in main menu"], &skinConfInt["linkRows"], 4, 1, 8));
		sd.addSetting(new MenuSettingBool(this, tr["Link label"], tr["Show link labels in main menu"], &skinConfInt["linkLabel"]));
		sd.addSetting(new MenuSettingBool(this, tr["Section label"], tr["Show the active section label in main menu"], &skinConfInt["sectionLabel"]));
		sd.exec();

		selected = sd.selected;
		save = sd.save;
	} while (!save);

	if (skinBackdrops == "OFF") confInt["skinBackdrops"] = BD_OFF;
	else if (skinBackdrops == "Menu & Dialog") confInt["skinBackdrops"] = BD_MENU | BD_DIALOG;
	else if (skinBackdrops == "Menu only") confInt["skinBackdrops"] = BD_MENU;
	else if (skinBackdrops == "Dialog only") confInt["skinBackdrops"] = BD_DIALOG;

	if (sectionBar == "OFF") skinConfInt["sectionBar"] = SB_OFF;
	else if (sectionBar == "Right") skinConfInt["sectionBar"] = SB_RIGHT;
	else if (sectionBar == "Top") skinConfInt["sectionBar"] = SB_TOP;
	else if (sectionBar == "Bottom") skinConfInt["sectionBar"] = SB_BOTTOM;
	else if (sectionBar == "Left") skinConfInt["sectionBar"] = SB_LEFT;
	else skinConfInt["sectionBar"] = SB_CLASSIC;

	confStr["tmp_wallpaper"] = "";
	confStr["wallpaper"] = wpPath;
	writeSkinConfig();
	writeConfig();

	if (
		bdPrev != confInt["skinBackdrops"] ||
		initSkin != confStr["skin"] ||
		bgScalePrev != confStr["bgscale"] ||
		linkColsPrev != skinConfInt["linkCols"] ||
		linkRowsPrev != skinConfInt["linkRows"] ||
		sbPrev != skinConfInt["sectionBar"]
	) reinit();
}

void GMenu2X::skinColors() {
	bool save = false;
	do {
		setSkin(confStr["skin"], false);

		SettingsDialog sd(this, ts, tr["Skin Colors"], "skin:icons/skin.png");
		sd.allowCancel_nomb = true;
		sd.addSetting(new MenuSettingRGBA(this, tr["Top/Section Bar"], tr["Color of the top and section bar"], &skinConfColors[COLOR_TOP_BAR_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["List Body"], tr["Color of the list body"], &skinConfColors[COLOR_LIST_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Bottom Bar"], tr["Color of the bottom bar"], &skinConfColors[COLOR_BOTTOM_BAR_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Selection"], tr["Color of the selection and other interface details"], &skinConfColors[COLOR_SELECTION_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Box Art"], tr["Color of the box art background"], &skinConfColors[COLOR_PREVIEW_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Message Box"], tr["Background color of the message box"], &skinConfColors[COLOR_MESSAGE_BOX_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Msg Box Border"], tr["Border color of the message box"], &skinConfColors[COLOR_MESSAGE_BOX_BORDER]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Msg Box Selection"], tr["Color of the selection of the message box"], &skinConfColors[COLOR_MESSAGE_BOX_SELECTION]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Font"], tr["Color of the font"], &skinConfColors[COLOR_FONT]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Font Outline"], tr["Color of the font's outline"], &skinConfColors[COLOR_FONT_OUTLINE]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Alt Font"], tr["Color of the alternative font"], &skinConfColors[COLOR_FONT_ALT]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Alt Font Outline"], tr["Color of the alternative font outline"], &skinConfColors[COLOR_FONT_ALT_OUTLINE]));
		sd.exec();
		save = sd.save;
	} while (!save);
	writeSkinConfig();
}

void GMenu2X::about() {
#if defined (__COMMIT_HASH__) || defined (__BUILDROOT_HASH__) || defined (__CFW_HASH__)
#define xstr(s) str(s)
#define str(s) #s
#endif
	vector<string> text;
	// string temp = "", buf;

	// temp = tr["Build date: "] + __DATE__ + "\n";

	// float battlevel = getBatteryStatus();
	// { stringstream ss; ss.precision(2); ss << battlevel/1000; ss >> buf; }
	// temp += tr["Battery: "] + ((battlevel < 0 || battlevel > 10000) ? tr["Charging"] : buf + " V") + "\n";
	// temp += tr["Uptime: "] + ms2hms(SDL_GetTicks()) + "\n";

	// temp += "----\n";
#if defined (__COMMIT_HASH__) && defined (__BUILDROOT_HASH__) && defined (__CFW_HASH__)
	TextDialog td(this, "MiyooCFW build", tr[""] + "CFW-ver:" + xstr(__CFW_HASH__) + " BR2:" + xstr(__BUILDROOT_HASH__) + " GM2X:" xstr(__COMMIT_HASH__), "skin:icons/about.png");
#elif defined (__COMMIT_HASH__) && defined (__BUILDROOT_HASH__)
	TextDialog td(this, "GMenu2X BR2 package", tr["Build"] + ": " + __DATE__ + " BR2:" + xstr(__BUILDROOT_HASH__) + " GM2X:" xstr(__COMMIT_HASH__), "skin:icons/about.png");
#elif defined (__COMMIT_HASH__)
	TextDialog td(this, "GMenu2X upstream", tr["Build"] + ": " + __DATE__ + " " + __TIME__ + " with commit:" xstr(__COMMIT_HASH__), "skin:icons/about.png");
#else
	TextDialog td(this, "GMenu2X", tr["Build"] + ": " + __DATE__ + " " + __TIME__, "skin:icons/about.png");
#endif
	// td.appendText(temp);
	td.appendFile("about.txt");
	td.exec();
}

void GMenu2X::viewLog() {
	string logfile = exe_path() + "/log.txt";
	if (!file_exists(logfile)) return;

	TextDialog td(this, tr["Log Viewer"], tr["Last launched program's output"], "skin:icons/ebook.png");
	td.appendFile(exe_path() + "/log.txt");
	td.exec();

	MessageBox mb(this, tr["Delete the log file?"], "skin:icons/ebook.png");
	mb.setButton(MANUAL, tr["Delete and disable"]);
	mb.setButton(CONFIRM, tr["Delete"]);
	mb.setButton(CANCEL,  tr["No"]);
	int res = mb.exec();

	switch (res) {
		case MANUAL:
			confInt["outputLogs"] = false;
			writeConfig();

		case CONFIRM:
			ledOn();
			unlink(logfile.c_str());
			sync();
			initMenu();
			ledOff();
			break;
	}
}

void GMenu2X::viewAutoStart() {
	MessageBox mb(this, tr["Continue with the AutoStart selected app?"]);
	mb.setButton(CONFIRM, tr["Yes"]);
	mb.setButton(CANCEL,  tr["No"]);
	mb.setButton(MODIFIER,  tr["Remove this dialog!"]);
	mb.setButton(MANUAL,  tr["Poweroff"]);
	int res = mb.exec();

	switch (res) {
			case CANCEL:
				confInt["saveAutoStart"] = 0;
				confStr["lastDirectory"] = "";
				confInt["dialogAutoStart"] = 0;
				reinit_save();
			case MODIFIER:
				confInt["dialogAutoStart"] = 0;
				reinit_save();
			case MANUAL:
				quit();
#if !defined(TARGET_LINUX)
				system("sync; mount -o remount,ro $HOME; poweroff");
#endif
				break;
	}
}

void GMenu2X::changeWallpaper() {
	WallpaperDialog wp(this, tr["Wallpaper"], tr["Select an image to use as a wallpaper"], "skin:icons/wallpaper.png");
	if (wp.exec() && confStr["wallpaper"] != wp.wallpaper) {
		confStr["wallpaper"] = wp.wallpaper;
		confStr["tmp_wallpaper"] = base_name(confStr["wallpaper"]);
	}
}

void GMenu2X::showManual() {
	string linkTitle = menu->selLinkApp()->getTitle();
	string linkDescription = menu->selLinkApp()->getDescription();
	string linkIcon = menu->selLinkApp()->getIcon();
	string linkManual = menu->selLinkApp()->getManualPath();
	string linkBackdrop = confInt["skinBackdrops"] | BD_DIALOG ? menu->selLinkApp()->getBackdropPath() : "";
	string linkExec = menu->selLinkApp()->getExec();

	if (linkManual == "") return;

	TextDialog td(this, linkTitle, linkDescription, linkIcon); //, linkBackdrop);

	if (file_exists(linkManual)) {
		string ext = linkManual.substr(linkManual.size() - 4, 4);
		if (ext == ".png" || ext == ".bmp" || ext == ".jpg" || ext == "jpeg") {
			ImageViewerDialog im(this, linkTitle, linkDescription, linkIcon, linkManual);
			im.exec();
			return;
		}

		td.appendFile(linkManual);
#if defined(OPK_SUPPORT)
	} else if (file_ext(linkExec, true) == ".opk") {
		void *buf; size_t len;
		struct OPK *opk = opk_open(linkExec.c_str());

		if (!opk) {
			ERROR("Unable to open OPK");
			return;
		}

		if (opk_extract_file(opk, linkManual.c_str(), &buf, &len) < 0) {
			ERROR("Unable to extract file: %s\n", linkManual.c_str());
			return;
		}

		opk_close(opk);

		char* str = strdup((const char*)buf);
		str[len] = 0;

		td.appendText(str);
#endif // OPK_SUPPORT
	} else {
		return;
	}

	td.exec();
}

void GMenu2X::explorer() {
	BrowseDialog bd(this, tr["Explorer"], tr["Select a file or application"]);
	bd.showDirectories = true;
	bd.showFiles = true;

	if (confInt["saveSelection"]) bd.setPath(confStr["explorerLastDir"]);

			string command = cmdclean(bd.getFilePath(bd.selected));
			if (confInt["saveAutoStart"]) {
				confInt["lastCPU"] = confInt["cpuMenu"];
				confInt["lastKeyboardLayout"] = confInt["keyboardLayoutMenu"];
				confStr["lastCommand"] = command.c_str();
				confStr["lastDirectory"] = bd.getFilePath().c_str();
				writeConfig();
			}
	while (bd.exec()) {
		string ext = bd.getExt(bd.selected);
		if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || ext == ".bmp") {
			ImageViewerDialog im(this, tr["Image viewer"], bd.getFile(bd.selected), "icons/explorer.png", bd.getFilePath(bd.selected));
			im.exec();
		} else if (ext == ".txt" || ext == ".conf" || ext == ".me" || ext == ".md" || ext == ".xml" || ext == ".log" || ext == ".ini") {
			TextDialog td(this, tr["Text viewer"], bd.getFile(bd.selected), "skin:icons/ebook.png");
			td.appendFile(bd.getFilePath(bd.selected));
			td.exec();
#if defined(IPK_SUPPORT)
		} else if (ext == ".ipk" && file_exists("/usr/bin/opkg")) {
			ipkInstall(bd.getFilePath(bd.selected));
#endif
#if defined(OPK_SUPPORT)
		} else if (ext == ".opk") {
			opkInstall(bd.getFilePath(bd.selected));
#endif
		} else if (ext == ".sh") {
			TerminalDialog td(this, tr["Terminal"], "sh " + cmdclean(bd.getFileName(bd.selected)), "skin:icons/terminal.png");
			td.exec(bd.getFilePath(bd.selected));
		} else if (ext == ".zip" && file_exists("/usr/bin/unzip") && (bd.getFile(bd.selected).rfind("gmenu2x-skin-", 0) == 0) || (bd.getFile(bd.selected).rfind("gmenunx-skin-", 0) == 0)) {
			string path = bd.getFilePath(bd.selected);
			string skinname = base_name(path, true).substr(13); // strip gmenu2x-skin- and .zip
			if (skinname.size() > 1) {
				TerminalDialog td(this, tr["Skin installer"], tr["Installing skin"] + " " + skinname, "skin:icons/skin.png");
				string cmd = "rm -rf \"skins/" + skinname + "/\"; mkdir -p \"skins/" + skinname + "/\"; unzip \"" + path + "\" -d \"skins/" + skinname + "/\"; if [ `ls \"skins/" + skinname + "\" | wc -l` == 1 ]; then subdir=`ls \"skins/" + skinname + "\"`; mv \"skins/" + skinname + "\"/$subdir/* \"skins/" + skinname + "/\"; rmdir \"skins/" + skinname + "\"/$subdir; fi; sync";
				td.exec(cmd);
				setSkin(skinname, false);
			}
		} else if (ext == ".zip" && file_exists("/usr/bin/unzip")) {
			TerminalDialog td(this, tr["Zip content"], bd.getFileName(bd.selected), "skin:icons/terminal.png");
			td.exec("unzip -l " + cmdclean(bd.getFilePath(bd.selected)));
		} else {
			if (confInt["saveSelection"] && (confInt["section"] != menu->selSectionIndex() || confInt["link"] != menu->selLinkIndex())) {
				writeConfig();
			}

			string command = cmdclean(bd.getFilePath(bd.selected));
			string params = "";
			if (confInt["saveAutoStart"]) {
				confInt["lastCPU"] = confInt["cpuMenu"];
				confInt["lastKeyboardLayout"] = confInt["keyboardLayoutMenu"];
				confStr["lastCommand"] = command.c_str();
				confStr["lastDirectory"] = bd.getFilePath().c_str();
				writeConfig();
			}

			if (confInt["outputLogs"]) {
				if (file_exists("/usr/bin/gdb")) {
					params = "-batch -ex \"run\" -ex \"bt\" --args " + command;
					command = "gdb";
				}
				params += " 2>&1 | tee " + cmdclean(exe_path()) + "/log.txt";
			}

			LinkApp *link = new LinkApp(this, "explorer.lnk~");
			link->setExec(command);
			link->setParams(params);
			link->setIcon("skin:icons/terminal.png");
			link->setTitle(bd.getFile(bd.selected));
			link->launch();
			return;
		}
	}

	confStr["explorerLastDir"] = bd.getPath();
}

bool GMenu2X::saveScreenshot(string path) {
#if !defined(TARGET_MIYOO) //for performance sake
	if (file_exists("/usr/bin/fbgrab")) {
		path = unique_filename(path + "/screenshot", ".png");
		path = "/usr/bin/fbgrab " + path;
		return system(path.c_str()) == 0;
	}
#endif

	path = unique_filename(path + "/screenshot", ".bmp");
	return SDL_SaveBMP(s->raw, path.c_str()) == 0;
}

void GMenu2X::reinit(bool showDialog) {
	if (showDialog) {
		MessageBox mb(this, tr["GMenuNX will restart to apply"]+"\n"+tr["the settings. Continue?"], "skin:icons/exit.png");
		mb.setButton(CONFIRM, tr["Restart"]);
		mb.setButton(CANCEL,  tr["Cancel"]);
		if (mb.exec() == CANCEL) return;
	}

	quit_nosave();
	WARNING("Re-launching gmenu2x");
	chdir(exe_path().c_str());
	execlp("./gmenu2x", "./gmenu2x", NULL);
}

void GMenu2X::reinit_save() {

	quit();
	WARNING("Re-launching gmenu2x");
	chdir(exe_path().c_str());
	execlp("./gmenu2x", "./gmenu2x", NULL);
}

void GMenu2X::poweroffDialog() {
#if !defined(NO_REBOOT)
	MessageBox mb(this, tr["Poweroff or reboot the device?"], "skin:icons/exit.png");
	mb.setButton(SECTION_NEXT, tr["Reboot"]);
#else
	MessageBox mb(this, tr["Poweroff the device?"], "skin:icons/exit.png");
#endif
	mb.setButton(CONFIRM, tr["Poweroff"]);
	mb.setButton(CANCEL,  tr["Cancel"]);
	int res = mb.exec();
	writeConfig();
	switch (res) {
		case CONFIRM: {
			MessageBox mb(this, tr["Poweroff"]);
			mb.setAutoHide(1);
			mb.exec();
		//	setVolume(0);
			quit();
#if !defined(TARGET_LINUX)
			system("sync; mount -o remount,ro $HOME; poweroff");
#endif
			break;
		}
		case SECTION_NEXT: {
			MessageBox mb(this, tr["Rebooting"]);
			mb.setAutoHide(1);
			mb.exec();
		//	setVolume(0);
			quit();
#if !defined(TARGET_LINUX)
			system("sync; mount -o remount,ro $HOME; reboot");
#endif
			break;
		}
	}
}

void GMenu2X::umountSdDialog() {
	BrowseDialog bd(this, tr["Umount"], tr["Umount external media device"], "skin:icons/eject.png");
	bd.showDirectories = true;
	bd.showFiles = false;
	bd.allowDirUp = false;
	bd.allowEnterDirectory = false;
	bd.setPath("/media");
	bd.exec();
}

void GMenu2X::contextMenu() {
	vector<MenuOption> options;
	if (menu->selLinkApp() != NULL) {
		options.push_back((MenuOption){tr["Edit"] + " " + menu->selLink()->getTitle(), MakeDelegate(this, &GMenu2X::editLink)});
		options.push_back((MenuOption){tr["Delete"] + " " + menu->selLink()->getTitle(), MakeDelegate(this, &GMenu2X::deleteLink)});
	}

	options.push_back((MenuOption){tr["Add link"], 			MakeDelegate(this, &GMenu2X::addLink)});
	options.push_back((MenuOption){tr["Add section"],		MakeDelegate(this, &GMenu2X::addSection)});
	options.push_back((MenuOption){tr["Rename section"],	MakeDelegate(this, &GMenu2X::renameSection)});
	options.push_back((MenuOption){tr["Delete section"],	MakeDelegate(this, &GMenu2X::deleteSection)});

#if defined(OPK_SUPPORT)
	options.push_back((MenuOption){tr["Update OPK links"],	MakeDelegate(this, &GMenu2X::opkScanner)});
#endif

	MessageBox mb(this, options);
}

void GMenu2X::addLink() {
	BrowseDialog bd(this, tr["Add link"], tr["Select an application"]);
	bd.showDirectories = true;
	bd.showFiles = true;
	string filter = ".dge,.gpu,.gpe,.sh,.bin,.elf,";

#if defined(IPK_SUPPORT)
	filter = ".ipk," + filter;
#endif
#if defined(OPK_SUPPORT)
	filter = ".opk," + filter;
#endif

	bd.setFilter(filter);
	while (bd.exec()) {
		string ext = bd.getExt(bd.selected);

#if defined(IPK_SUPPORT)
		if (ext == ".ipk" && file_exists("/usr/bin/opkg")) {
			ipkInstall(bd.getFilePath(bd.selected));
		} else
#endif
#if defined(OPK_SUPPORT)
		if (ext == ".opk") {
			opkInstall(bd.getFilePath(bd.selected));
		} else
#endif
		if (menu->addLink(bd.getFilePath(bd.selected))) {
			editLink();
			return;
		}
	}
	sync();
}

void GMenu2X::changeSelectorDir() {
	BrowseDialog bd(this, tr["Selector Path"], tr["Directory to start the selector"]);
	bd.showDirectories = true;
	bd.showFiles = false;
	bd.allowSelectDirectory = true;
	if (bd.exec() && bd.getPath() != "/") {
		confStr["tmp_selector"] = bd.getPath() + "/";
	}
}

void GMenu2X::editLink() {
	if (menu->selLinkApp() == NULL) return;

	vector<string> pathV;
	split(pathV, menu->selLinkApp()->getFile(), "/");
	string oldSection = "";
	if (pathV.size() > 1) oldSection = pathV[pathV.size()-2];
	string newSection = oldSection;

	string linkExec = menu->selLinkApp()->getExec();
	string linkTitle = menu->selLinkApp()->getTitle();
	string linkDescription = menu->selLinkApp()->getDescription();
	string linkIcon = menu->selLinkApp()->getIcon();
	string linkManual = menu->selLinkApp()->getManual();
	string linkParams = menu->selLinkApp()->getParams();
	string linkSelFilter = menu->selLinkApp()->getSelectorFilter();
	string linkSelDir = menu->selLinkApp()->getSelectorDir();
	bool useSelector = !linkSelDir.empty();
	bool linkSelBrowser = menu->selLinkApp()->getSelectorBrowser();
	string linkSelScreens = menu->selLinkApp()->getSelectorScreens();
	string linkSelAliases = menu->selLinkApp()->getAliasFile();
	int linkClock = menu->selLinkApp()->getCPU();
	int linkKbdLayout = menu->selLinkApp()->getKbdLayout();
	int linkTefix = menu->selLinkApp()->getTefix();
	string linkBackdrop = menu->selLinkApp()->getBackdrop();
	string dialogTitle = tr["Edit"] + " " + linkTitle;
	string dialogIcon = menu->selLinkApp()->getIconPath();
	string linkDir = dir_name(linkExec);
	// string linkHomeDir = menu->selLinkApp()->getHomeDir();

	vector<string> scaleMode;
	// scaleMode.push_back("Crop");
	scaleMode.push_back("Stretch");
	scaleMode.push_back("Aspect");
	scaleMode.push_back("Original");
	if (((float)(this->w)/this->h) != (4.0f/3.0f)) scaleMode.push_back("4:3");
	string linkScaleMode = scaleMode[menu->selLinkApp()->getScaleMode()];

	vector<string> selStr;
	selStr.push_back("OFF");
	selStr.push_back("AUTO");
	if (!linkSelDir.empty())
		selStr.push_back(real_path(linkSelDir) + "/");

	string s = "";
	s += linkSelDir.back();

	if (linkSelDir.empty()) confStr["tmp_selector"] = selStr[0];
	else if (s != "/") confStr["tmp_selector"] = selStr[1];
	else confStr["tmp_selector"] = selStr[2];

	SettingsDialog sd(this, ts, dialogTitle, dialogIcon);

	// sd.addSetting(new MenuSettingFile(			this, tr["Executable"],		tr["Application this link points to"], &linkExec, ".dge,.gpu,.gpe,.sh,.bin,.opk,.elf,", linkExec, dialogTitle, dialogIcon));
	sd.allowCancel_link = true;
	sd.addSetting(new MenuSettingString(		this, tr["Title"],			tr["Link title"], &linkTitle, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingString(		this, tr["Description"],	tr["Link description"], &linkDescription, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingMultiString(	this, tr["Section"],		tr["The section this link belongs to"], &newSection, &menu->getSections()));
	sd.addSetting(new MenuSettingString(		this, tr["Parameters"],		tr["Command line arguments to pass to the application"], &linkParams, dialogTitle, dialogIcon));

	sd.addSetting(new MenuSettingImage(			this, tr["Icon"],			tr["Select a custom icon for the link"], &linkIcon, ".png,.bmp,.jpg,.jpeg,.gif", linkExec, dialogTitle, dialogIcon));

	if (CPU_MAX != CPU_MIN) {
#if defined(MULTI_INT)
		sd.addSetting(new MenuSettingMultiInt(		this, tr["CPU Clock"],		tr["CPU clock frequency when launching this link"], &linkClock, oc_choices, oc_choices_size, confInt["cpuLink"], confInt["cpuMin"], confInt["cpuMax"]));
#else
		sd.addSetting(new MenuSettingInt(		this, tr["CPU Clock"],		tr["CPU clock frequency when launching this link"], &linkClock, confInt["cpuLink"], confInt["cpuMin"], confInt["cpuMax"], confInt["cpuStep"]));
#endif
	}
	// sd.addSetting(new MenuSettingDir(			this, tr["Home Path"],		tr["Set directory as $HOME for this link"], &linkHomeDir, CARD_ROOT, dialogTitle, dialogIcon));

#if defined(HW_SCALER)
	sd.addSetting(new MenuSettingMultiString(this, tr["Scale Mode"],		tr["Hardware scaling mode"], &linkScaleMode, &scaleMode));
#endif


	sd.addSetting(new MenuSettingInt(		this, tr["Keyboard Layout"],	tr["Set the appropriate A/B/X/Y layout"], &linkKbdLayout, confInt["keyboardLayoutMenu"], 1, confInt["keyboardLayoutMax"], 1));
	sd.addSetting(new MenuSettingInt(		this, tr["TEfix method"],	tr["Set different tearing FIX method"], &linkTefix, confInt["tefixMenu"], 0, confInt["tefixMax"], 1));
	sd.addSetting(new MenuSettingMultiString(	this, tr["File Selector"],	tr["Use file browser selector"], &confStr["tmp_selector"], &selStr, NULL, MakeDelegate(this, &GMenu2X::changeSelectorDir)));
	sd.addSetting(new MenuSettingBool(			this, tr["Show Folders"],	tr["Allow the selector to change directory"], &linkSelBrowser));
	sd.addSetting(new MenuSettingString(		this, tr["File Filter"],	tr["Filter by file extension (separate with commas)"], &linkSelFilter, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingDir(			this, tr["Box Art"],		tr["Directory of the box art for the selector"], &linkSelScreens, linkDir, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingFile(			this, tr["Aliases"],		tr["File containing a list of aliases for the selector"], &linkSelAliases, ".txt,.dat", linkExec, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingImage(			this, tr["Backdrop"],		tr["Select an image backdrop"], &linkBackdrop, ".png,.bmp,.jpg,.jpeg", linkExec, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingFile(			this, tr["Manual"],			tr["Select a Manual or Readme file"], &linkManual, ".man.png,.txt,.me", linkExec, dialogTitle, dialogIcon));
#if defined(TARGET_WIZ) || defined(TARGET_CAANOO)
	bool linkUseGinge = menu->selLinkApp()->getUseGinge();
	string ginge_prep = exe_path() + "/ginge/ginge_prep";
	if (file_exists(ginge_prep))
		sd.addSetting(new MenuSettingBool(		this, tr["Use Ginge"],			tr["Compatibility layer for running GP2X applications"], &linkUseGinge ));
#endif

#if defined(HW_GAMMA)
	int linkGamma = menu->selLinkApp()->getGamma();
	sd.addSetting(new MenuSettingInt(		this, tr["Gamma"],	tr["Gamma value to set when launching this link"], &linkGamma, 0, 100 ));
#endif

	if (sd.exec() && sd.edited() && sd.save) {
		ledOn();

		menu->selLinkApp()->setExec(linkExec);
		menu->selLinkApp()->setTitle(linkTitle);
		menu->selLinkApp()->setDescription(linkDescription);
		menu->selLinkApp()->setIcon(linkIcon);
		menu->selLinkApp()->setManual(linkManual);
		menu->selLinkApp()->setParams(linkParams);
		menu->selLinkApp()->setSelectorFilter(linkSelFilter);
		menu->selLinkApp()->setSelectorElement(0);

		if (linkSelDir.empty()) linkSelDir = confStr["homePath"];
		if (confStr["tmp_selector"] == "OFF") linkSelDir = "";
		else if (confStr["tmp_selector"] == "AUTO") linkSelDir = real_path(linkSelDir);
		else linkSelDir = confStr["tmp_selector"];

		int scalemode = 0;
		if (linkScaleMode == "Aspect") scalemode = 1;
		else if (linkScaleMode == "Original") scalemode = 2;
		else if (linkScaleMode == "4:3") scalemode = 3;
		menu->selLinkApp()->setScaleMode(scalemode);

		menu->selLinkApp()->setSelectorDir(linkSelDir);
		menu->selLinkApp()->setSelectorBrowser(linkSelBrowser);
		menu->selLinkApp()->setSelectorScreens(linkSelScreens);
		menu->selLinkApp()->setAliasFile(linkSelAliases);
		menu->selLinkApp()->setBackdrop(linkBackdrop);
		menu->selLinkApp()->setCPU(linkClock);
		menu->selLinkApp()->setKbdLayout(linkKbdLayout);
		menu->selLinkApp()->setTefix(linkTefix);

#if defined(HW_GAMMA)
		menu->selLinkApp()->setGamma(linkGamma);
#endif

#if defined(TARGET_WIZ) || defined(TARGET_CAANOO)
		menu->selLinkApp()->setUseGinge(linkUseGinge);
#endif

		// if section changed move file and update link->file
		if (oldSection != newSection) {
			vector<string>::const_iterator newSectionIndex = find(menu->getSections().begin(), menu->getSections().end(), newSection);
			if (newSectionIndex == menu->getSections().end()) return;
			string newFileName = "sections/" + newSection + "/" + linkTitle;
			uint32_t x = 2;
			while (file_exists(newFileName)) {
				string id = "";
				stringstream ss; ss << x; ss >> id;
				newFileName = "sections/" + newSection + "/" + linkTitle + id;
				x++;
			}
			rename(menu->selLinkApp()->getFile().c_str(), newFileName.c_str());
			menu->selLinkApp()->renameFile(newFileName);

			INFO("New section: (%i) %s", newSectionIndex - menu->getSections().begin(), newSection.c_str());

			menu->linkChangeSection(menu->selLinkIndex(), menu->selSectionIndex(), newSectionIndex - menu->getSections().begin());
		}
		menu->selLinkApp()->save();
		sync();
		ledOff();
	}
	confInt["section"] = menu->selSectionIndex();
	confInt["link"] = menu->selLinkIndex();
	confStr["tmp_selector"] = "";
}

void GMenu2X::deleteLink() {
	int package_type = 0;
	MessageBox mb(this, tr["Delete"] + " " + menu->selLink()->getTitle() + "\n" + tr["THIS CAN'T BE UNDONE"] + "\n" + tr["Are you sure?"], menu->selLink()->getIconPath());
	string package = menu->selLinkApp()->getExec();
#if defined(OPK_SUPPORT)
	if (file_ext(package, true) == ".opk") {
		package_type = 1;
		mb.setButton(MODIFIER, tr["Uninstall OPK"]);
		goto dialog; // shameless use of goto
	}
#endif
#if defined(IPK_SUPPORT)
	package = ipkName(menu->selLinkApp()->getFile());
	if (!package.empty()) {
		package_type = 2;
		mb.setButton(MODIFIER, tr["Uninstall IPK"]);
		goto dialog; // shameless use of goto
	}
#endif

	dialog:
	mb.setButton(MANUAL, tr["Delete link"]);
	mb.setButton(CANCEL,  tr["No"]);

	switch (mb.exec()) {
		case MODIFIER:
			if (package_type == 1) {
				unlink(package.c_str());
				menu->deleteSelectedLink();
			} else if (package_type == 2) {
				system("mount -o remount,rw /");
				TerminalDialog td(this, tr["Uninstall package"], "opkg remove " + package, "skin:icons/configure.png");
				td.exec("opkg remove " + package);
				system("mount -o remount,ro /");
			}
			initMenu();
			break;
		case MANUAL:
			menu->deleteSelectedLink();
			break;
		default:
			return;
	}
}

void GMenu2X::addSection() {
	InputDialog id(this, /*ts,*/ tr["Insert a name for the new section"], "", tr["Add section"], "skin:icons/section.png");
	if (id.exec()) {
		// only if a section with the same name does not exist
		if (find(menu->getSections().begin(), menu->getSections().end(), id.getInput()) == menu->getSections().end()) {
			// section directory doesn't exists
			ledOn();
			if (menu->addSection(id.getInput())) {
				menu->setSectionIndex(menu->getSections().size() - 1); //switch to the new section
				sync();
			}
			ledOff();
		}
	}
}

void GMenu2X::renameSection() {
	InputDialog id(this, /*ts,*/ tr["Insert a new name for this section"], menu->selSection(), tr["Rename section"], menu->getSectionIcon());
	if (id.exec()) {
		menu->renameSection(menu->selSectionIndex(), id.getInput());
		sync();
	}
}

void GMenu2X::deleteSection() {
	MessageBox mb(this, tr["Delete section"] + " '" +  menu->selSectionName() + "'\n" + tr["THIS CAN'T BE UNDONE"] + "\n" + tr["Are you sure?"], menu->getSectionIcon());
	mb.setButton(MANUAL, tr["Yes"]);
	mb.setButton(CANCEL,  tr["No"]);
	if (mb.exec() != MANUAL) return;
	ledOn();
	if (rmtree(exe_path() + "/sections/" + menu->selSection())) {
		menu->deleteSelectedSection();
		sync();
	}
	ledOff();
}

#if defined(OPK_SUPPORT)
void GMenu2X::opkScanner() {
	OPKScannerDialog od(this, tr["Update OPK links"], "Scanning OPK packages", "skin:icons/configure.png");
	od.exec();
	initMenu();
}

void GMenu2X::opkInstall(string path) {
	input.update(false);
	bool debug = input[MANUAL];

	OPKScannerDialog od(this, tr["Package installer"], tr["Installing"] + " " + base_name(path), "skin:icons/configure.png");
	od.opkpath = path;
	od.exec(debug);
	initMenu();
}
#endif

#if defined(IPK_SUPPORT)
string GMenu2X::ipkName(string cmd) {
	if (!file_exists("/usr/bin/opkg"))
		return "";
	char package[128] = {0,0,0,0,0,0,0,0};
	cmd = "opkg search \"*" + base_name(cmd) + "\" | cut -f1 -d' '";
	FILE *fp = popen(cmd.c_str(), "r");
	if (fp == NULL)
		return "";

	fgets(package, sizeof(package) - 1, fp);
	pclose(fp);

	if (package == NULL)
		return "";

	return trim((string)package);
}

void GMenu2X::ipkInstall(string path) {
	system("mount -o remount,rw /");
	string cmd = "opkg install --force-reinstall --force-overwrite ";
	input.update(false);
	if (input[MANUAL]) cmd += "--force-downgrade ";
	TerminalDialog td(this, tr["Package installer"], "opkg install " + base_name(path), "skin:icons/configure.png");
	td.exec(cmd + cmdclean(path));
	system("mount -o remount,ro /");
	//system("if [ -d sections/systems ]; then mkdir -p sections/emulators.systems; cp -r sections/systems/* sections/emulators.systems/; rm -rf sections/systems; fi; sync;");
	initMenu();
}
#endif

void GMenu2X::setInputSpeed() {
	input.setInterval(150);
	input.setInterval(300, SECTION_PREV);
	input.setInterval(300, SECTION_NEXT);
	input.setInterval(300, CONFIRM);
	input.setInterval(300, CANCEL);
	input.setInterval(300, MANUAL);
	input.setInterval(300, MODIFIER);
	input.setInterval(300, PAGEUP);
	input.setInterval(300, PAGEDOWN);
	// input.setInterval(1000, SETTINGS);
	// input.setInterval(100, MENU);
	// input.setInterval(1000, POWER);
	// input.setInterval(30,  VOLDOWN);
	// input.setInterval(30,  VOLUP);
	// input.setInterval(100, INC);
	// input.setInterval(100, DEC);
	// input.setInterval(200, BACKLIGHT);
}

int GMenu2X::setVolume(int val, bool popup) {
	int volumeStep = 10;

	val = constrain(val, 0, 90);

	if (popup) {
		Surface bg(s);

		Surface *iconVolume[3] = {
			sc["skin:imgs/mute.png"],
			sc["skin:imgs/phones.png"],
			sc["skin:imgs/volume.png"],
		};

		powerManager->clearTimer();
		while (true) {
			drawSlider(val, 0, 90, *iconVolume[getVolumeMode(val)], bg);

			input.update();

			if (input[SETTINGS] || input[CONFIRM] || input[CANCEL]) {
				break;
			} else if (input[LEFT] || input[DEC] || input[VOLDOWN] || input[SECTION_PREV]) {
				val = max(0, val - volumeStep);
			} else if (input[RIGHT] || input[INC] || input[VOLUP] || input[SECTION_NEXT]) {
				val = min(90, val + volumeStep);
			}

			val = constrain(val, 0, 90);
		}

		bg.blit(s, 0, 0);
		s->flip();

		powerManager->resetSuspendTimer();
#if !defined(HW_LIDVOL)
		confInt["globalVolume"] = val;
		writeConfig();
#endif
	}

	return val;
}

int GMenu2X::setBacklight(int val, bool popup) {
	if (popup) {
		int backlightStep = 10;
		val = constrain(val, 10, 100);

		Surface bg(s);

		Surface *iconBrightness[6] = {
			sc["skin:imgs/brightness/0.png"],
			sc["skin:imgs/brightness/1.png"],
			sc["skin:imgs/brightness/2.png"],
			sc["skin:imgs/brightness/3.png"],
			sc["skin:imgs/brightness/4.png"],
			sc["skin:imgs/brightness.png"],
		};

		powerManager->clearTimer();
		while (true) {
			int brightnessIcon = val / 20;
			if (brightnessIcon > 4 || iconBrightness[brightnessIcon] == NULL) brightnessIcon = 5;

			drawSlider(val, 0, 100, *iconBrightness[brightnessIcon], bg);

			input.update();

			if (input[SETTINGS] || input[CONFIRM] || input[CANCEL]) {
				break;
			} else if (input[LEFT] || input[DEC] || input[SECTION_PREV]) {
				val = setBacklight(max(10, val - backlightStep), false);
			} else if (input[RIGHT] || input[INC] || input[SECTION_NEXT]) {
				val = setBacklight(min(100, val + backlightStep), false);
			} else if (input[BACKLIGHT]) {
				SDL_Delay(50);
				val = getBacklight();
			}

			val = constrain(val, 5, 100);
		}

		bg.blit(s, 0, 0);
		s->flip();

		powerManager->resetSuspendTimer();
#if !defined(HW_LIDVOL)
		confInt["backlight"] = val;
		writeConfig();
#endif
	}

	return val;
}

int GMenu2X::drawButton(Button *btn, int x, int y) {
	if (y < 0) y = this->h + y;
	btn->setPosition(x, y);
	btn->paint();
}

int GMenu2X::drawButton(Surface *s, const string &btn, const string &text, int x, int y) {
	if (y < 0) y = this->h + y;
	Surface *icon = sc["skin:imgs/buttons/" + btn + ".png"];
	if (icon != NULL) {
		icon->blit(s, x, y, HAlignLeft | VAlignMiddle);
		x += 19;
		if (!text.empty()) {
			s->write(font, text, x, y, VAlignMiddle, skinConfColors[COLOR_FONT_ALT], skinConfColors[COLOR_FONT_ALT_OUTLINE]);
			x += font->getTextWidth(text);
		}
	}
	return x + 6;
}

int GMenu2X::drawButtonRight(Surface *s, const string &btn, const string &text, int x, int y) {
	if (y < 0) y = this->h + y;
	Surface *icon = sc["skin:imgs/buttons/" + btn + ".png"];
	if (icon != NULL) {
		if (!text.empty()) {
			x -= font->getTextWidth(text);
			s->write(font, text, x, y, HAlignLeft | VAlignMiddle, skinConfColors[COLOR_FONT_ALT], skinConfColors[COLOR_FONT_ALT_OUTLINE]);
		}
		x -= 19;
		icon->blit(s, x, y, HAlignLeft | VAlignMiddle);
	}
	return x - 6;
}

void GMenu2X::drawScrollBar(uint32_t pagesize, uint32_t totalsize, uint32_t pagepos, SDL_Rect scrollRect, const uint8_t align) {
	if (totalsize <= pagesize) return;
	SDL_Rect bar = {2, 2, 2, 2};

	if ((align & VAlignBottom) || (align & VAlignTop)) {
		bar.w = (scrollRect.w - 3) * pagesize / totalsize;
		bar.x = (scrollRect.w - 3) * pagepos / totalsize;
		bar.x = scrollRect.x + bar.x + 3;

		if (align & VAlignTop) {
			bar.y = scrollRect.y + 2; // top
		} else {
			bar.y = scrollRect.y + scrollRect.h - 4; // bottom
		}

		if (bar.w < 8) {
			bar.w = 8;
		}

		if (bar.x + bar.w > scrollRect.x + scrollRect.w - 3) {
			bar.x = scrollRect.x + scrollRect.w - bar.w - 3;
		}
	} else { // HAlignLeft || HAlignRight
		bar.h = (scrollRect.h - 3) * pagesize / totalsize;
		bar.y = (scrollRect.h - 3) * pagepos / totalsize;
		bar.y = scrollRect.y + bar.y + 3;

		if (align & HAlignRight) {
			bar.x = scrollRect.x + scrollRect.w - 4; // right
		}

		if (bar.h < 8) {
			bar.h = 8;
		}

		if (bar.y + bar.h > scrollRect.y + scrollRect.h - 3) {
			bar.y = scrollRect.y + scrollRect.h - bar.h - 3;
		}
	}

	s->box(bar, skinConfColors[COLOR_SELECTION_BG]);
	s->rectangle(bar.x - 1, bar.y - 1, bar.w + 2, bar.h + 2, skinConfColors[COLOR_LIST_BG]);
}

void GMenu2X::drawSlider(int val, int min, int max, Surface &icon, Surface &bg) {
	SDL_Rect progress = {52, 32, this->w - 84, 8};
	SDL_Rect box = {20, 20, this->w - 40, 32};

	val = constrain(val, min, max);

	bg.blit(s,0,0);
	s->box(box, skinConfColors[COLOR_MESSAGE_BOX_BG]);
	s->rectangle(box.x + 2, box.y + 2, box.w - 4, box.h - 4, skinConfColors[COLOR_MESSAGE_BOX_BORDER]);

	icon.blit(s, 28, 28);

	s->box(progress, skinConfColors[COLOR_MESSAGE_BOX_BG]);
	s->box(progress.x + 1, progress.y + 1, val * (progress.w - 3) / max + 1, progress.h - 2, skinConfColors[COLOR_MESSAGE_BOX_SELECTION]);
	s->flip();
}

uint32_t GMenu2X::timerFlip(uint32_t interval, void *param) {
	instance->s->flip();
	return 0;
};
