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
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include <sys/statvfs.h>
#include <errno.h>

// #include <sys/fcntl.h> //for battery

//for browsing the filesystem
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

//for soundcard
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "linkapp.h"
#include "menu.h"
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
#include "menusettingint.h"
#include "menusettingmultiint.h"
#include "menusettingbool.h"
#include "menusettingrgba.h"
#include "menusettingstring.h"
#include "menusettingmultistring.h"
#include "menusettingfile.h"
#include "menusettingimage.h"
#include "menusettingdir.h"
#include "imageviewerdialog.h"
#include "batteryloggerdialog.h"
#include "linkscannerdialog.h"
#include "menusettingdatetime.h"
#include "debug.h"
#include "bootAssets.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>


#include <sys/mman.h>

#include <ctime>
#include <sys/time.h>   /* for settimeofday() */


#define sync() sync(); system("sync");

// background color (RGB)
#define R 122
#define G 129
#define B 16

// delay until the logo starts being visible (unit: frames) (60 frames = 1 sec)
#define ANIMDELAY 1

// time from the moment the logo stops moving and sound is played until the logo app closes (unit: milliseconds)
#define ENDDELAY 2000

//speed at which the logo moves (unit: pixels per frame)
#define ANIMSPEED 1


const char *CARD_ROOT = "/mnt/"; //Note: Add a trailing /!
const int CARD_ROOT_LEN = 1;

static GMenu2X *app;

using std::ifstream;
using std::ofstream;
using std::stringstream;
using namespace fastdelegate;

// Note: Keep this in sync with the enum!
static const char *colorNames[NUM_COLORS] = {
	"topBarBg",
	"listBg",
	"bottomBarBg",
	"selectionBg",
	"messageBoxBg",
	"messageBoxBorder",
	"messageBoxSelection",
	"font",
	"fontOutline",
	"fontAlt",
	"fontAltOutline"
};

#if defined(TARGET_MIYOO)
uint32_t oc_table[] = {
  0x00c81802,
  0x00cc1013,
  0x00cc1001,
  0x00d01902,
  0x00d00c12,
  0x00d80b23,
  0x00d81101,
  0x00d80833,
  0x00d81a02,
  0x00d80811,
  0x00d81113,
  0x00d80220,
  0x00d80521,
  0x00d80822,
  0x00d80800,
  0x00e01b02,
  0x00e00d12,
  0x00e00632,
  0x00e41201,
  0x00e41213,
  0x00e81c02,
  0x00ea0c23,
  0x00f00922,
  0x00f00900,
  0x00f01301,
  0x00f00933,
  0x00f00911,
  0x00f01313,
  0x00f01d02,
  0x00f00431,
  0x00f00e12,
  0x00f00410,
  0x00f81e02,
  0x00fc0d23,
  0x00fc0621,
  0x00fc1413,
  0x00fc1401,
  0x01000732,
  0x01001f02,
  0x01000f12,
  0x01081501,
  0x01080a11,
  0x01080a22,
  0x01081513,
  0x01080a00,
  0x01080a33,
  0x010e0e23,
  0x01101012,
  0x01141601,
  0x01141613,
  0x01200531,
  0x01200f23,
  0x01200832,
  0x01200b11,
  0x01200230,
  0x01200721,
  0x01200b22,
  0x01200320,
  0x01201701,
  0x01200b33,
  0x01200b00,
  0x01201713,
  0x01201112,
  0x01200510,
  0x012c1801,
  0x012c1813,
  0x01301212,
  0x01321023,
  0x01380c11,
  0x01380c22,
  0x01381901,
  0x01380c00,
  0x01380c33,
  0x01381913,
  0x01401312,
  0x01400932,
  0x01441123,
  0x01441a13,
  0x01441a01,
  0x01440821,
  0x01501b13,
  0x01501412,
  0x01500631,
  0x01500610,
  0x01500d22,
  0x01500d00,
  0x01501b01,
  0x01500d33,
  0x01500d11,
  0x01561223,
  0x015c1c01,
  0x015c1c13,
  0x01601512,
  0x01600a32,
  0x01680921,
  0x01681d01,
  0x01680420,
  0x01680e22,
  0x01681d13,
  0x01680e00,
  0x01681323,
  0x01680e33,
  0x01680e11,
  0x01701612,
  0x01741e01,
  0x01741e13,
  0x017a1423,
  0x01800f33,
  0x01800710,
  0x01800731,
  0x01800f11,
  0x01801712,
  0x01800330,
  0x01800f00,
  0x01801f01,
  0x01800f22,
  0x01800b32,
  0x01801f13,
  0x018c0a21,
  0x018c1523,
  0x01901812,
  0x01981022,
  0x01981000,
  0x01981033,
  0x01981011,
  0x019e1623,
  0x01a01912,
  0x01a00c32,
  0x01b00520,
  0x01b00810,
  0x01b01111,
  0x01b01723,
  0x01b00831,
  0x01b01100,
  0x01b01122,
  0x01b00b21,
  0x01b01a12,
  0x01b01133,
  0x01c01b12,
  0x01c00d32,
  0x01c21823,
  0x01c81233,
  0x01c81211,
  0x01c81222,
  0x01c81200,
  0x01d01c12,
  0x01d41923,
  0x01d40c21,
  0x01e00931,
  0x01e01333,
  0x01e00430,
  0x01e00e32,
  0x01e01311,
  0x01e00910,
  0x01e01300,
  0x01e01322,
  0x01e01d12,
  0x01e61a23,
  0x01f01e12,
  0x01f81400,
  0x01f81b23,
  0x01f81433,
  0x01f81411,
  0x01f80d21,
  0x01f80620,
  0x01f81422,
  0x02001f12,
  0x02000f32,
  0x020a1c23,
  0x02101522,
  0x02101500,
  0x02101533,
  0x02101511,
  0x02100a10,
  0x02100a31,
  0x021c0e21,
  0x021c1d23,
  0x02201032,
  0x02281611,
  0x02281622,
  0x02281600,
  0x02281633,
  0x022e1e23,
  0x02401722,
  0x02400b10,
  0x02401733,
  0x02401700,
  0x02400b31,
  0x02400530,
  0x02401132,
  0x02401711,
  0x02400f21,
  0x02400720,
  0x02401f23,
  0x02581800,
  0x02581833,
  0x02581811,
  0x02581822,
  0x02601232,
  0x02641021,
  0x02701911,
  0x02700c10,
  0x02700c31,
  0x02701922,
  0x02701900,
  0x02701933,
  0x02801332,
  0x02881a11,
  0x02880820,
  0x02881121,
  0x02881a22,
  0x02881a00,
  0x02881a33,
  0x02a00d31,
  0x02a01432,
  0x02a01b11,
  0x02a00630,
  0x02a01b00,
  0x02a01b22,
  0x02a00d10,
  0x02a01b33,
  0x02ac1221,
  0x02b81c00,
  0x02b81c33,
  0x02b81c11,
  0x02b81c22,
  0x02c01532,
  0x02d01d00,
  0x02d01d22,
  0x02d00920,
  0x02d01d33,
  0x02d00e10,
  0x02d00e31,
  0x02d01d11,
  0x02d01321,
  0x02e01632,
  0x02e81e00,
  0x02e81e33,
  0x02e81e11,
  0x02e81e22,
  0x02f41421,
  0x03000730,
  0x03000f10,
  0x03001f11,
  0x03000f31,
  0x03001f00,
  0x03001f22,
  0x03001732,
  0x03001f33,
  0x03180a20,
  0x03181521,
  0x03201832,
  0x03301031,
  0x03301010,
  0x033c1621,
  0x03401932,
  0x03600830,
  0x03601721,
  0x03601a32,
  0x03600b20,
  0x03601110,
  0x03601131,
  0x03801b32,
  0x03841821
};
int oc_choices[] = {408,414,448,450,456,464,468,486,496,512,522,528,558,576,608,640,704,736,800,896};
int oc_choices_size = sizeof(oc_choices)/sizeof(int);
#endif

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

char *hwVersion() {
	static char buf[10] = { 0 };
	FILE *f = fopen("/dev/mmcblk0", "r");
	fseek(f, 440, SEEK_SET); // Set the new position at 10
	if (f) {
		for (int i = 0; i < 4; i++) {
			int c = fgetc(f); // Get character
			snprintf(buf + strlen(buf), sizeof(buf), "%02X", c);
		}
	}
	fclose(f);

	// printf("FW Checksum: %s\n", buf);
	return buf;
}

char *ms2hms(uint32_t t, bool mm = true, bool ss = true) {
	static char buf[10];

	t = t / 1000;
	int s = (t % 60);
	int m = (t % 3600) / 60;
	int h = (t % 86400) / 3600;
	// int d = (t % (86400 * 30)) / 86400;

	if (!ss) sprintf(buf, "%02d:%02d", h, m);
	else if (!mm) sprintf(buf, "%02d", h);
	else sprintf(buf, "%02d:%02d:%02d", h, m, s);
	return buf;
};

void printbin(const char *id, int n) {
	printf("%s: 0x%08x ", id, n);
	for(int i = 31; i >= 0; i--) {
		printf("%d", !!(n & 1 << i));
		if (!(i % 8)) printf(" ");
	}
	printf("\e[0K\n");
}

static void quit_all(int err) {
	delete app;
	exit(err);
}

int memdev = 0;
#if defined(TARGET_RS97) || defined(TARGET_MIYOO)
	volatile uint32_t *memregs;
#else
	volatile uint16_t *memregs;
#endif

enum mmc_status {
	MMC_REMOVE, MMC_INSERT, MMC_ERROR
};

int16_t curMMCStatus, preMMCStatus;
int16_t getMMCStatus(void) {
#if !defined(TARGET_MIYOO)
	if (memdev > 0) return !(memregs[0x10500 >> 2] >> 0 & 0b1);
#endif
	return MMC_ERROR;
}

enum udc_status {
	UDC_REMOVE, UDC_CONNECT, UDC_ERROR
};

int udcConnectedOnBoot;
int16_t getUDCStatus(void) {
#if !defined(TARGET_MIYOO)
	if (memdev > 0) return (memregs[0x10300 >> 2] >> 7 & 0b1);
#endif
	return UDC_ERROR;
}

int16_t tvOutPrev = false, tvOutConnected;
bool getTVOutStatus() {
#if !defined(TARGET_MIYOO)
	if (memdev > 0) return !(memregs[0x10300 >> 2] >> 25 & 0b1);
#endif
	return false;
}

enum vol_mode_t {
	VOLUME_MODE_MUTE, VOLUME_MODE_PHONES, VOLUME_MODE_NORMAL
};
int16_t volumeModePrev, volumeMode;
uint8_t getVolumeMode(uint8_t vol) {
#if !defined(TARGET_MIYOO)
	if (!vol) return VOLUME_MODE_MUTE;
	else if (memdev > 0 && !(memregs[0x10300 >> 2] >> 6 & 0b1)) return VOLUME_MODE_PHONES;
#endif
	return VOLUME_MODE_NORMAL;
}

GMenu2X::~GMenu2X() {
	confStr["datetime"] = getDateTime();

	writeConfig();

	quit();

	delete menu;
	delete s;
	delete font;
	delete titlefont;
}

#define MIFB_BASE                   0x100
#define MIFB_GET_DMA                _IOWR (MIFB_BASE, 1, unsigned long)
#define MIFB_REFILL_MODE            _IOWR (MIFB_BASE, 2, unsigned long)
#define MIFB_FLUSH_DMA              _IOWR (MIFB_BASE, 3, unsigned long)
#define MIFB_REFILL_FULL_AUTO       0
#define MIFB_REFILL_HALF_AUTO       1
#define MIFB_REFILL_MANUAL_UPDATE   2
void GMenu2X::quit() {
  int fd = open("/dev/fb0", O_RDWR);
  ioctl(fd, MIFB_REFILL_MODE, MIFB_REFILL_HALF_AUTO);
  close(fd);

	fflush(NULL);
	sc.clear();
	s->free();
	SDL_CloseAudio();
	SDL_Quit();
	hwDeinit();
}

int main(int argc, char * argv[]) {
	INFO("GMenu2X starting: If you read this message in the logs, check http://mtorromeo.github.com/gmenu2x/troubleshooting.html for a solution");

	signal(SIGINT, &quit_all);
	signal(SIGSEGV,&quit_all);
	signal(SIGTERM,&quit_all);
	bool autoStart = false;
	bool bootLogo = false;
	for (int i = 0; i < argc; i++){
       		if(strcmp(argv[i],"--autostart")==0) {
			INFO("Woo Autostart");
			autoStart = true;
		} else if (strcmp(argv[i],"--logo")==0) {
			INFO("BOOT LOGO");
			bootLogo = true;	
		}
	}
	app = new GMenu2X();
	//!app->input[PAGEUP]
	DEBUG("Starting main()");
	app->main(autoStart, bootLogo);
	return 0;
}

bool GMenu2X::bootAnimaton() {
	Surface *logoimg;
	SDL_RWops *RWops;
	logoimg = sc.skinRes("bootlogo.png");
	if(!logoimg) {
		RWops = SDL_RWFromMem(png_logo_pocketgo, sizeof(png_logo_pocketgo));
		logoimg = new Surface(IMG_LoadPNG_RW(RWops), s->format(), 0);
	}
    	if (!logoimg) {
        	printf("Error loading logo: %s\n", IMG_GetError());
        	return false;
    	}
	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 1024);
        Mix_AllocateChannels(2);
        SDL_RWops *RWops2;
        Mix_Chunk *logosound;

        RWops2 = SDL_RWFromMem(wav_logosound, sizeof(wav_logosound));
	string wavpath = path + "skins/" + confStr["skin"] + "/bootlogo.wav";
	logosound = Mix_LoadWAV(wavpath.c_str());
	if (!logosound) {
    		logosound = Mix_LoadWAV_RW(RWops2, 1);
	}
    	if (!logosound) {
    	    printf("Error loading sound: %s\n", Mix_GetError());
    	    return false;
    	}
        int dest_y = (resY / 2 ) - (logoimg->raw->h / 2);
        uint32_t curr_time = SDL_GetTicks();
        uint32_t old_time = curr_time;
        uint32_t color = SDL_MapRGB(s->format(), R, G, B);
	bool returnVal = true;
	bool playSound = true;
        for (int i = 0 - logoimg->raw->h - ANIMDELAY; i <= dest_y; i = i + ANIMSPEED) {
		input.update(false);
		if (input[CONFIRM] || input[CANCEL]) {
			playSound = false;
			break;
		} else if (input[MODIFIER]) {
			playSound = false;
			returnVal = false;
			break;
		}

                //SDL_FillRect(s, &rect, color);
		s->box((SDL_Rect){0, 0, resX, resY}, (RGBAColor){R, G, B, 255});
                //SDL_BlitSurface(logoimg, NULL, s, &dstrect);
		logoimg->blit(s,(resX - logoimg->raw->w) / 2,i);
                if (i == dest_y) {
                        Mix_PlayChannel(-1, logosound, 0);
                }
                while (curr_time < old_time + 16) {
                        curr_time = SDL_GetTicks();
                }
                old_time = curr_time;
                s->flip();
	}


	s->flip();
	if(playSound) {
		SDL_Delay(confInt["bootLogoDelay"]*100);
	}
	s->box((SDL_Rect){0, 0, resX, resY}, (RGBAColor){R, G, B, 255});
        //SDL_FreeRW(RWops2);
        //SDL_FreeRW(RWops2);
	logoimg->free();

        Mix_FreeChunk(logosound);
	Mix_CloseAudio();

        
        SDL_CloseAudio();
	return returnVal;
}

// GMenu2X *GMenu2X::instance = NULL;
GMenu2X::GMenu2X() {
	// instance = this;
	//Detect firmware version and type
	//load config data
	readConfig();

	halfX = resX/2;
	halfY = resY/2;

	path = "";
	getExePath();

#if defined(TARGET_GP2X) || defined(TARGET_WIZ) || defined(TARGET_CAANOO) || defined(TARGET_RS97) || defined(TARGET_MIYOO)
	hwInit();
#endif

#if !defined(TARGET_PC)
	setenv("SDL_NOMOUSE", "1", 1);
#endif
	setenv("SDL_FBCON_DONT_CLEAR", "1", 0);
	setDateTime();

	//Screen
	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_JOYSTICK|SDL_INIT_AUDIO) < 0 ) {
		ERROR("Could not initialize SDL: %s", SDL_GetError());
		quit();
	}

	s = new Surface();
#if defined(TARGET_GP2X) || defined(TARGET_WIZ) || defined(TARGET_CAANOO)
	//I'm forced to use SW surfaces since with HW there are issuse with changing the clock frequency
	SDL_Surface *dbl = SDL_SetVideoMode(resX, resY, confInt["videoBpp"], SDL_SWSURFACE);
	s->enableVirtualDoubleBuffer(dbl);
	SDL_ShowCursor(0);
#elif defined(TARGET_RS97)
	SDL_ShowCursor(0);
	s->ScreenSurface = SDL_SetVideoMode(320, 240, confInt["videoBpp"], SDL_HWSURFACE | SDL_DOUBLEBUF);
	s->raw = SDL_CreateRGBSurface(SDL_SWSURFACE, resX, resY, confInt["videoBpp"], 0, 0, 0, 0);
#elif defined(TARGET_MIYOO)
	SDL_ShowCursor(0);
	s->ScreenSurface = SDL_SetVideoMode(320, 240, confInt["videoBpp"], SDL_HWSURFACE | SDL_DOUBLEBUF);
	s->raw = SDL_CreateRGBSurface(SDL_SWSURFACE, resX, resY, confInt["videoBpp"], 0, 0, 0, 0);
#else
	s->raw = SDL_SetVideoMode(resX, resY, confInt["videoBpp"], SDL_HWSURFACE | SDL_DOUBLEBUF);
#endif
	s->box((SDL_Rect){0, 0, resX, resY}, (RGBAColor){R, G, B, 255});
	s->flip();
	//if (autoStart) {
	//}

	setWallpaper(confStr["wallpaper"]);

	setSkin(confStr["skin"], false, true);

	powerManager = new PowerManager(this, confInt["backlightTimeout"], confInt["powerTimeout"]);

#if !defined(TARGET_MIYOO)
	MessageBox mb(this,tr["Loading"]);
	mb.setAutoHide(1);
	mb.exec();
#endif

	setBacklight(confInt["backlight"]);

	input.init(path + "input.conf");
	setInputSpeed();

#if defined(TARGET_GP2X)
	initServices();
	setGamma(confInt["gamma"]);
	applyDefaultTimings();
#elif defined(TARGET_RS97)
	system("ln -sf $(mount | grep int_sd | cut -f 1 -d ' ') /tmp/.int_sd");
	tvOutConnected = getTVOutStatus();
	preMMCStatus = curMMCStatus = getMMCStatus();
	udcConnectedOnBoot = getUDCStatus();
#endif
	volumeModePrev = volumeMode = getVolumeMode(confInt["globalVolume"]);
	
	initMenu();
	readTmp();
	setCPU(confInt["cpuMenu"]);

	input.setWakeUpInterval(1000);

	//recover last session
	if (lastSelectorElement >- 1 && menu->selLinkApp() != NULL && (!menu->selLinkApp()->getSelectorDir().empty() || !lastSelectorDir.empty()))
		menu->selLinkApp()->selector(lastSelectorElement, lastSelectorDir);
}

void GMenu2X::main(bool autoStart,bool bootLogo) {
	int i = 0, x = 0, y = 0, ix = 0, iy = 0;
	uint32_t tickBattery = -4800, tickNow; //, tickMMC = 0; //, tickUSB = 0;
	string prevBackdrop = confStr["wallpaper"], currBackdrop = confStr["wallpaper"];

	int8_t brightnessIcon = 5;
	Surface *iconBrightness[6] = {
		sc.skinRes("imgs/brightness/0.png"),
		sc.skinRes("imgs/brightness/1.png"),
		sc.skinRes("imgs/brightness/2.png"),
		sc.skinRes("imgs/brightness/3.png"),
		sc.skinRes("imgs/brightness/4.png"),
		sc.skinRes("imgs/brightness.png"),
	};

	int8_t batteryIcon = 3;
	Surface *iconBattery[7] = {
		sc.skinRes("imgs/battery/0.png"),
		sc.skinRes("imgs/battery/1.png"),
		sc.skinRes("imgs/battery/2.png"),
		sc.skinRes("imgs/battery/3.png"),
		sc.skinRes("imgs/battery/4.png"),
		sc.skinRes("imgs/battery/5.png"),
		sc.skinRes("imgs/battery/ac.png"),
	};

	Surface *iconVolume[3] = {
		sc.skinRes("imgs/mute.png"),
		sc.skinRes("imgs/phones.png"),
		sc.skinRes("imgs/volume.png"),
	};

	Surface *iconSD = sc.skinRes("imgs/sd1.png"),
			*iconManual = sc.skinRes("imgs/manual.png"),
			*iconCPU = sc.skinRes("imgs/cpu.png"),
			*iconMenu = sc.skinRes("imgs/menu.png");

#if defined(TARGET_RS97)
	if (udcConnectedOnBoot == UDC_CONNECT) checkUDC();
	if (curMMCStatus == MMC_INSERT) mountSd(true);
#endif
	if (confInt["bootLogoShow"] || (autoStart&& confStr["lastCommand"] != "" && confStr["lastDirectory"] != "" )) {
		autoStart = bootAnimaton();
	}
	//autoStart = false;
	if(autoStart && confStr["lastCommand"] != "" && confStr["lastDirectory"] != "")  {
		INFO("Starting autostart()");
		INFO("conf %s %s",confStr["lastDirectory"].c_str(),confStr["lastCommand"].c_str());
		INFO("autostart %s %s",confStr["lastDirectory"],confStr["lastCommand"]);
		setCPU(confInt["lastCPU"]);
		chdir(confStr["lastDirectory"].c_str());
		quit();
		execlp("/bin/sh", "/bin/sh", "-c", confStr["lastCommand"].c_str(), NULL);
	}

	//s->box((SDL_Rect){0, 0, resX, resY}, (RGBAColor){0, 0, 0, 255});
	//setWallpaper(confStr["wallpaper"]);

	//s->flip();

	INFO("Start Loop");
	bool quit = false;
	while (!quit) {
		tickNow = SDL_GetTicks();

		s->box((SDL_Rect){0, 0, resX, resY}, (RGBAColor){0, 0, 0, 255});
		sc[currBackdrop]->blit(s,0,0);

		// SECTIONS
		if (confInt["sectionBar"]) {
			s->box(sectionBarRect, skinConfColors[COLOR_TOP_BAR_BG]);

			x = sectionBarRect.x; y = sectionBarRect.y;
			for (i = menu->firstDispSection(); i < menu->getSections().size() && i < menu->firstDispSection() + menu->sectionNumItems(); i++) {
				if (confInt["sectionBar"] == SB_LEFT || confInt["sectionBar"] == SB_RIGHT) {
					y = (i - menu->firstDispSection()) * skinConfInt["sectionBarSize"];
				} else {
					x = (i - menu->firstDispSection()) * skinConfInt["sectionBarSize"];
				}

				if (menu->selSectionIndex() == (int)i) {
					s->box(x, y, skinConfInt["sectionBarSize"], skinConfInt["sectionBarSize"], skinConfColors[COLOR_SELECTION_BG]);
					if(confInt["sectionLetters"]) {
					s->write(font, tr.translate(menu->getSectionLetter(i)), x + skinConfInt["sectionBarSize"]/2,y+font->getHeight()/2, HAlignCenter | VAlignMiddle, skinConfColors[COLOR_BOTTOM_BAR_BG], skinConfColors[COLOR_FONT_OUTLINE]);
					} else {
						sc[menu->getSectionIcon(i)]->blit(s, {x, y, skinConfInt["sectionBarSize"], skinConfInt["sectionBarSize"]}, HAlignCenter | VAlignMiddle);
					}
				} else {

					if(confInt["sectionLetters"]) {
						s->write(font, tr.translate(menu->getSectionLetter(i)), x + skinConfInt["sectionBarSize"]/2,y+font->getHeight()/2, HAlignCenter | VAlignMiddle);
					} else {
						sc[menu->getSectionIcon(i)]->blit(s, {x, y, skinConfInt["sectionBarSize"], skinConfInt["sectionBarSize"]}, HAlignCenter | VAlignMiddle);
					}
				}
			}
		}

		// LINKS
		s->setClipRect(linksRect);
		s->box(linksRect, skinConfColors[COLOR_LIST_BG]);

		i = menu->firstDispRow() * linkCols;

		if (linkCols == 1) {
			// LIST
			ix = linksRect.x;
			for (y = 0; y < linkRows && i < menu->sectionLinks()->size(); y++, i++) {
				iy = linksRect.y + y * linkHeight;
				string shortName = tr.translate(menu->sectionLinks()->at(i)->getTitle());
				string name = shortName;
				if(confInt["shortNames"]) {
					string description = tr.translate(menu->sectionLinks()->at(i)->getDescription());
					if(description.length() > 0) {
						name = name + " - " + description;
						if (name.length() > 40) {
                                                	name = name.substr(0,38) + "...";
                                        	}
					}
				}
				int alignment = HAlignLeft;
				int hOffset = 2;
				if(confInt["textAlignment"]==TA_CENTER) {
					alignment = HAlignCenter;
					hOffset = (resX/2) - hOffset + 2;
				} 


				bool selected = i == (int)menu->selLinkIndex();
				if (selected){
					s->box(ix, iy, linksRect.w, linkHeight, skinConfColors[COLOR_FONT]);
				}

				if(!confInt["hideIcons"]){
					sc[menu->sectionLinks()->at(i)->getIconPath()]->blit(s, {ix, iy, 36, linkHeight}, HAlignCenter | VAlignMiddle);
					hOffset += 32;
				}

				if (selected) {
					s->write(titlefont, name, ix + linkSpacing + hOffset, iy + titlefont->getHeight()/2, alignment | VAlignMiddle, skinConfColors[COLOR_BOTTOM_BAR_BG], skinConfColors[COLOR_FONT_OUTLINE]);
				} else {
					s->write(titlefont, shortName, ix + linkSpacing + hOffset, iy + titlefont->getHeight()/2, alignment | VAlignMiddle);
				}

				// Warning: this writes on top of the title when menu rows is > 5
				// todo: restore the bottom bar, and the behavior of showing the description there for a second or two after selecting a link
				if(selected && !confInt["hideDescription"]) {
					s->write(font, tr.translate(menu->sectionLinks()->at(i)->getDescription()), ix + linkSpacing + hOffset, iy + linkHeight - linkSpacing/2, alignment | VAlignBottom);
				}

			}
		} else {
			for (y = 0; y < linkRows; y++) {
				for (x = 0; x < linkCols && i < menu->sectionLinks()->size(); x++, i++) {
					ix = linksRect.x + x * linkWidth  + (x + 1) * linkSpacing;
					iy = linksRect.y + y * linkHeight + (y + 1) * linkSpacing;

					s->setClipRect({ix, iy, linkWidth, linkHeight});

					if (i == (uint32_t)menu->selLinkIndex())
						s->box(ix, iy, linkWidth, linkHeight, skinConfColors[COLOR_SELECTION_BG]);
					if(!confInt["hideIcons"]){
						sc[menu->sectionLinks()->at(i)->getIconPath()]->blit(s, {ix + 2, iy + 2, linkWidth - 4, linkHeight - 4}, HAlignCenter | VAlignMiddle);
					}

					s->write(font, tr.translate(menu->sectionLinks()->at(i)->getTitle()), ix + linkWidth/2, iy + linkHeight - 2, HAlignCenter | VAlignBottom);
				}
			}
		}
		s->clearClipRect();

		drawScrollBar(linkRows, menu->sectionLinks()->size()/linkCols + ((menu->sectionLinks()->size()%linkCols==0) ? 0 : 1), menu->firstDispRow(), linksRect);

		// TRAY DEBUG
		// s->box(sectionBarRect.x + sectionBarRect.w - 38 + 0 * 20, sectionBarRect.y + sectionBarRect.h - 18,16,16, strtorgba("ffff00ff"));
		// s->box(sectionBarRect.x + sectionBarRect.w - 38 + 1 * 20, sectionBarRect.y + sectionBarRect.h - 18,16,16, strtorgba("00ff00ff"));
		// s->box(sectionBarRect.x + sectionBarRect.w - 38, sectionBarRect.y + sectionBarRect.h - 38,16,16, strtorgba("0000ffff"));
		// s->box(sectionBarRect.x + sectionBarRect.w - 18, sectionBarRect.y + sectionBarRect.h - 38,16,16, strtorgba("ff00ffff"));

		currBackdrop = confStr["wallpaper"];
		if (menu->selLink() != NULL && menu->selLinkApp() != NULL && !menu->selLinkApp()->getBackdropPath().empty() && sc.add(menu->selLinkApp()->getBackdropPath()) != NULL) {
			currBackdrop = menu->selLinkApp()->getBackdropPath();
		}

		//Background
		if (prevBackdrop != currBackdrop) {
			INFO("New backdrop: %s", currBackdrop.c_str());
			sc.del(prevBackdrop);
			prevBackdrop = currBackdrop;
			// input.setWakeUpInterval(1);
			continue;
		}

		if (confInt["sectionBar"]) {
#if !defined(TARGET_MIYOO)
			// TRAY 0,0
			iconVolume[volumeMode]->blit(s, sectionBarRect.x + sectionBarRect.w - 38, sectionBarRect.y + sectionBarRect.h - 38);
#endif
			// TRAY 1,0
			if (tickNow - tickBattery >= 5000) {
				// TODO: move to hwCheck
				tickBattery = tickNow;
				batteryIcon = getBatteryLevel();
			}
			if (batteryIcon > 5) batteryIcon = 6;
			iconBattery[batteryIcon]->blit(s, sectionBarRect.x + sectionBarRect.w - 18, sectionBarRect.y + sectionBarRect.h - 18);

			// TRAY iconTrayShift,1
			int iconTrayShift = 0;
#if !defined(TARGET_MIYOO)
			if (curMMCStatus == MMC_INSERT) {
				iconSD->blit(s, sectionBarRect.x + sectionBarRect.w - 38 + iconTrayShift * 20, sectionBarRect.y + sectionBarRect.h - 38);
				iconTrayShift++;
			}
#endif
      if (menu->selLink() != NULL) {
				if (menu->selLinkApp() != NULL) {
					if (!menu->selLinkApp()->getManualPath().empty() && iconTrayShift < 2) {
						// Manual indicator
						iconManual->blit(s, sectionBarRect.x + sectionBarRect.w - 38 + iconTrayShift * 20, sectionBarRect.y + sectionBarRect.h - 18);
						iconTrayShift++;
					}

					if (menu->selLinkApp()->clock() != confInt["cpuMenu"] && iconTrayShift < 2) {
						// CPU indicator
						iconCPU->blit(s, sectionBarRect.x + sectionBarRect.w - 38 + iconTrayShift * 20, sectionBarRect.y + sectionBarRect.h - 18);
						iconTrayShift++;
					}
				}
			}

#if !defined(TARGET_MIYOO)
			if (iconTrayShift < 2) {
				brightnessIcon = confInt["backlight"]/20;
				if (brightnessIcon > 4 || iconBrightness[brightnessIcon] == NULL) brightnessIcon = 5;
				iconBrightness[brightnessIcon]->blit(s, sectionBarRect.x + sectionBarRect.w - 38 + iconTrayShift * 20, sectionBarRect.y + sectionBarRect.h - 18);
				iconTrayShift++;
			}
#endif
			/*if (iconTrayShift < 2) {
				// Menu indicator
				iconMenu->blit(s, sectionBarRect.x + sectionBarRect.w - 38 + iconTrayShift * 20, sectionBarRect.y + sectionBarRect.h - 18);
				iconTrayShift++;
			}*/
		}
		s->flip();

		bool inputAction = input.update();
		if (input.combo()) {
			confInt["sectionBar"] = ((confInt["sectionBar"] + 1) % 5);
			if (!confInt["sectionBar"]) confInt["sectionBar"]++;
			initMenu();
			MessageBox mb(this,tr["CHEATER! ;)"]);
			mb.setBgAlpha(0);
			mb.setAutoHide(200);
			mb.exec();
			input.setWakeUpInterval(1);
			continue;
		}
		// input.setWakeUpInterval(0);

		if (inputCommonActions(inputAction)) continue;

		if ( input[CONFIRM] && menu->selLink() != NULL ) {
			setVolume(confInt["globalVolume"]);

			if (menu->selLinkApp() != NULL && menu->selLinkApp()->getSelectorDir().empty()) {
				MessageBox mb(this, tr["Launching "] + menu->selLink()->getTitle().c_str(), menu->selLink()->getIconPath());
				mb.setAutoHide(500);
				mb.exec();
			}

			menu->selLink()->run();
		}
		else if ( input[SETTINGS] ) settings();
		else if ( input[MENU]     ) contextMenu();
		// LINK NAVIGATION
		else if ( input[LEFT ] && linkCols == 1) menu->pageUp();
		else if ( input[RIGHT] && linkCols == 1) menu->pageDown();
		else if ( input[LEFT ] ) menu->linkLeft();
		else if ( input[RIGHT] ) menu->linkRight();
		else if ( input[UP   ] ) menu->linkUp();
		else if ( input[DOWN ] ) menu->linkDown();
		// SECTION
		else if ( input[SECTION_PREV] ) menu->decSectionIndex();
		else if ( input[SECTION_NEXT] ) menu->incSectionIndex();

		// VOLUME SCALE MODIFIER
#if defined(TARGET_GP2X)
		else if ( fwType=="open2x" && input[CANCEL] ) {
			volumeMode = constrain(volumeMode - 1, -VOLUME_MODE_MUTE - 1, VOLUME_MODE_NORMAL);
			if (volumeMode < VOLUME_MODE_MUTE)
				volumeMode = VOLUME_MODE_NORMAL;
			switch(volumeMode) {
				case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
				case VOLUME_MODE_PHONES: setVolumeScaler(volumeScalerPhones); break;
				case VOLUME_MODE_NORMAL: setVolumeScaler(volumeScalerNormal); break;
			}
			setVolume(confInt["globalVolume"]);
		}
#endif
		// SELLINKAPP SELECTED
		else if (input[MANUAL] && menu->selLinkApp() != NULL) showManual(); // menu->selLinkApp()->showManual();


		// On Screen Help
		// else if (input[MODIFIER]) {
		// 	s->box(10,50,300,162, skinConfColors[COLOR_MESSAGE_BOX_BG]);
		// 	s->rectangle( 12,52,296,158, skinConfColors[COLOR_MESSAGE_BOX_BORDER] );
		// 	int line = 60; s->write( font, tr["CONTROLS"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["A: Confirm action"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["B: Cancel action"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["X: Show manual"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["L, R: Change section"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["Select: Modifier"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["Start: Contextual menu"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["Select+Start: Options menu"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["Backlight: Adjust backlight level"], 20, line);
		// 	line += font->getHeight() + 5; s->write( font, tr["Power: Toggle speaker on/off"], 20, line);
		// 	s->flip();
		// 	bool close = false;
		// 	while (!close) {
		// 		input.update();
		// 		if (input[MODIFIER] || input[CONFIRM] || input[CANCEL]) close = true;
		// 	}
		// }
	}

	// delete btnContextMenu;
	// btnContextMenu = NULL;
}

bool GMenu2X::inputCommonActions(bool &inputAction) {
	// INFO("SDL_GetTicks(): %d\tsuspendActive: %d", SDL_GetTicks(), powerManager->suspendActive);

	if (powerManager->suspendActive) {
		// SUSPEND ACTIVE
		input.setWakeUpInterval(0);
		while (!input[POWER]) {
			input.update();
		}
		powerManager->doSuspend(0);
		input.setWakeUpInterval(1000);
		return true;
	}

	if (inputAction) powerManager->resetSuspendTimer();
	input.setWakeUpInterval(1000);

	hwCheck();

	bool wasActive = false;
	while (input[POWER]) {
		wasActive = true;
		input.update();
		if (input[POWER]) {
			// HOLD POWER BUTTON
			poweroffDialog();
			return true;
		}
	}

	if (wasActive) {
		powerManager->doSuspend(1);
		return true;
	}

	while (input[MENU]) {
		wasActive = true;
		input.update();
		if (input[SECTION_NEXT]) {
			// SCREENSHOT
			if (!saveScreenshot()) { ERROR("Can't save screenshot"); return true; }
			MessageBox mb(this, tr["Screenshot saved"]);
			mb.setAutoHide(1000);
			mb.exec();
			return true;
		} else if (input[SECTION_PREV]) {
			// VOLUME / MUTE
			setVolume(confInt["globalVolume"], true);
			return true;
#ifdef TARGET_RS97
		} else if (input[POWER]) {
			udcConnectedOnBoot = UDC_CONNECT;
			checkUDC();
			return true;
#endif
		}
	}

	input[MENU] = wasActive; // Key was active but no combo was pressed

	if ( input[BACKLIGHT] ) {
		setBacklight(confInt["backlight"], true);
		return true;
	}

	return false;
}

void GMenu2X::hwInit() {
#if defined(TARGET_GP2X) || defined(TARGET_WIZ) || defined(TARGET_CAANOO) || defined(TARGET_RS97) || defined(TARGET_MIYOO)
	memdev = open("/dev/mem", O_RDWR);
	if (memdev < 0) WARNING("Could not open /dev/mem");
#endif

	if (memdev > 0) {
#if defined(TARGET_GP2X)
		memregs = (uint16_t*)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, 0xc0000000);
		MEM_REG = &memregs[0];

		//Fix tv-out
		if (memregs[0x2800 >> 1] & 0x100) {
			memregs[0x2906 >> 1] = 512;
			//memregs[0x290C >> 1]=640;
			memregs[0x28E4 >> 1] = memregs[0x290C >> 1];
		}
		memregs[0x28E8 >> 1] = 239;

#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
		memregs = (uint16_t*)mmap(0, 0x20000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, 0xc0000000);
#elif defined(TARGET_RS97)
		memregs = (uint32_t*)mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, 0x10000000);
#elif defined(TARGET_MIYOO)
		memregs = (uint32_t*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, 0x01c20000);
#endif
		if (memregs == MAP_FAILED) {
			ERROR("Could not mmap hardware registers!");
			close(memdev);
		}
	}

#if defined(TARGET_GP2X)
	if (fileExists("/etc/open2x")) fwType = "open2x";
	else fwType = "gph";

	f200 = fileExists("/dev/touchscreen/wm97xx");

	//open2x
	savedVolumeMode = 0;
	volumeScalerNormal = VOLUME_SCALER_NORMAL;
	volumeScalerPhones = VOLUME_SCALER_PHONES;
	o2x_usb_net_on_boot = false;
	o2x_usb_net_ip = "";
	o2x_ftp_on_boot = false;
	o2x_telnet_on_boot = false;
	o2x_gp2xjoy_on_boot = false;
	o2x_usb_host_on_boot = false;
	o2x_usb_hid_on_boot = false;
	o2x_usb_storage_on_boot = false;
	usbnet = samba = inet = web = false;
	if (fwType=="open2x") {
		readConfigOpen2x();
		//	VOLUME MODIFIER
		switch(volumeMode) {
			case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
			case VOLUME_MODE_PHONES: setVolumeScaler(volumeScalerPhones); break;
			case VOLUME_MODE_NORMAL: setVolumeScaler(volumeScalerNormal); break;
		}
	}
	readCommonIni();
	cx25874 = 0;
	batteryHandle = 0;
	// useSelectionPng = false;

	batteryHandle = open(f200 ? "/dev/mmsp2adc" : "/dev/batt", O_RDONLY);
	//if wm97xx fails to open, set f200 to false to prevent any further access to the touchscreen
	if (f200) f200 = ts.init();
#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
	// get access to battery device
	batteryHandle = open("/dev/pollux_batt", O_RDONLY);
#endif
	INFO("System Init Done!");
}

void GMenu2X::hwDeinit() {
#if defined(TARGET_GP2X)
	if (memdev > 0) {
		//Fix tv-out
		if (memregs[0x2800 >> 1] & 0x100) {
			memregs[0x2906 >> 1] = 512;
			memregs[0x28E4 >> 1] = memregs[0x290C >> 1];
		}

		memregs[0x28DA >> 1] = 0x4AB;
		memregs[0x290C >> 1] = 640;
	}
	if (f200) ts.deinit();
	if (batteryHandle != 0) close(batteryHandle);

	if (memdev > 0) {
		memregs = NULL;
		close(memdev);
	}
#endif
}

void GMenu2X::setWallpaper(const string &wallpaper) {
	if (bg != NULL) delete bg;

	bg = new Surface(s);
	bg->box((SDL_Rect){0, 0, resX, resY}, (RGBAColor){0, 0, 0, 0});

	confStr["wallpaper"] = wallpaper;
	if (wallpaper.empty() || sc.add(wallpaper) == NULL) {
		DEBUG("Searching wallpaper");

		FileLister fl("skins/" + confStr["skin"] + "/wallpapers", false, true);
		fl.setFilter(".png,.jpg,.jpeg,.bmp");
		fl.browse();
		if (fl.getFiles().size() <= 0 && confStr["skin"] != "Default")
			fl.setPath("skins/Default/wallpapers", true);
		if (fl.getFiles().size() > 0)
			confStr["wallpaper"] = fl.getPath() + "/" + fl.getFiles()[0];
	}
	sc[wallpaper]->blit(bg, 0, 0);
}

void GMenu2X::initLayout() {
	// LINKS rect
	linksRect = (SDL_Rect){0, 0, resX, resY};
	sectionBarRect = (SDL_Rect){0, 0, resX, resY};

	if (confInt["sectionBar"]) {
		// x = 0; y = 0;
		if (confInt["sectionBar"] == SB_LEFT || confInt["sectionBar"] == SB_RIGHT) {
			sectionBarRect.x = (confInt["sectionBar"] == SB_RIGHT)*(resX - skinConfInt["sectionBarSize"]);
			sectionBarRect.w = skinConfInt["sectionBarSize"];
			linksRect.w = resX - skinConfInt["sectionBarSize"];

			if (confInt["sectionBar"] == SB_LEFT) {
				linksRect.x = skinConfInt["sectionBarSize"];
			}
		} else {
			sectionBarRect.y = (confInt["sectionBar"] == SB_BOTTOM)*(resY - skinConfInt["sectionBarSize"]);
			sectionBarRect.h = skinConfInt["sectionBarSize"];
			linksRect.h = resY - skinConfInt["sectionBarSize"];

			if (confInt["sectionBar"] == SB_TOP) {
				linksRect.y = skinConfInt["sectionBarSize"];
			}
		}
	}

	listRect = (SDL_Rect){0, skinConfInt["topBarHeight"], resX, resY - skinConfInt["bottomBarHeight"] - skinConfInt["topBarHeight"]};

	// WIP
	linkCols = confInt["linkCols"];
	linkRows = confInt["linkRows"];

	linkWidth  = (linksRect.w - (linkCols + 1 ) * linkSpacing) / linkCols;
	linkHeight = (linksRect.h - (linkCols > 1) * (linkRows    + 1 ) * linkSpacing) / linkRows;
}

void GMenu2X::initFont() {
	if (font != NULL) delete font;
	if (titlefont != NULL) delete titlefont;

	font = new FontHelper(sc.getSkinFilePath("font.ttf"), skinConfInt["fontSize"], skinConfColors[COLOR_FONT], skinConfColors[COLOR_FONT_OUTLINE]);
	titlefont = new FontHelper(sc.getSkinFilePath("font.ttf"), skinConfInt["fontSizeTitle"], skinConfColors[COLOR_FONT], skinConfColors[COLOR_FONT_OUTLINE]);
}

void GMenu2X::initMenu() {
	if (menu != NULL) delete menu;
	initLayout();

	//Menu structure handler
	menu = new Menu(this);

	for (uint32_t i = 0; i < menu->getSections().size(); i++) {
		//Add virtual links in the applications section
		if (menu->getSections()[i] == "applications") {
			menu->addActionLink(i, tr["Explorer"], MakeDelegate(this, &GMenu2X::explorer), tr["Browse files and launch apps"], "skin:icons/explorer.png");
#if !defined(TARGET_PC)
			if (getBatteryLevel() > 5) // show only if charging
#endif
				menu->addActionLink(i, tr["Battery Logger"], MakeDelegate(this, &GMenu2X::batteryLogger), tr["Log battery power to battery.csv"], "skin:icons/ebook.png");
		}

		//Add virtual links in the setting section
		else if (menu->getSections()[i] == "settings") {
			menu->addActionLink(i, tr["Settings"], MakeDelegate(this, &GMenu2X::settings), tr["Configure system"], "skin:icons/configure.png");
			menu->addActionLink(i, tr["Skin"], MakeDelegate(this, &GMenu2X::skinMenu), tr["Appearance & skin settings"], "skin:icons/skin.png");
#if defined(TARGET_GP2X)
			if (fwType == "open2x")
				menu->addActionLink(i, "Open2x", MakeDelegate(this, &GMenu2X::settingsOpen2x), tr["Configure Open2x system settings"], "skin:icons/o2xconfigure.png");
			menu->addActionLink(i, "USB SD", MakeDelegate(this, &GMenu2X::activateSdUsb), tr["Activate USB on SD"], "skin:icons/usb.png");
			if (fwType == "gph" && !f200)
				menu->addActionLink(i, "USB Nand", MakeDelegate(this, &GMenu2X::activateNandUsb), tr["Activate USB on NAND"], "skin:icons/usb.png");
#elif defined(TARGET_RS97)
			//menu->addActionLink(i, "Format", MakeDelegate(this, &GMenu2X::formatSd), tr["Format internal SD"], "skin:icons/format.png");
			if (curMMCStatus == MMC_INSERT)
				menu->addActionLink(i, tr["Umount"], MakeDelegate(this, &GMenu2X::umountSdDialog), tr["Umount external SD"], "skin:icons/eject.png");
#endif

			if (fileExists(path + "log.txt"))
				menu->addActionLink(i, tr["Log Viewer"], MakeDelegate(this, &GMenu2X::viewLog), tr["Displays last launched program's output"], "skin:icons/ebook.png");

			menu->addActionLink(i, tr["About"], MakeDelegate(this, &GMenu2X::about), tr["Info about system"], "skin:icons/about.png");
			menu->addActionLink(i, tr["Power"], MakeDelegate(this, &GMenu2X::poweroffDialog), tr["Power menu"], "skin:icons/exit.png");
		}
	}
	menu->setSectionIndex(confInt["section"]);
	menu->setLinkIndex(confInt["link"]);
	menu->loadIcons();
}

void GMenu2X::settings() {
	int curGlobalVolume = confInt["globalVolume"];
//G
	// int prevgamma = confInt["gamma"];
	// bool showRootFolder = fileExists("/mnt/root");

	FileLister fl_tr("translations");
	fl_tr.browse();
	fl_tr.insertFile("English");
	string lang = tr.lang();
	if (lang == "") lang = "English";

	vector<string> encodings;
	// encodings.push_back("OFF");
	encodings.push_back("NTSC");
	encodings.push_back("PAL");

	vector<string> batteryType;
	batteryType.push_back("BL-5B");
	batteryType.push_back("Linear");

	vector<string> opFactory;
	opFactory.push_back(">>");
	string tmp = ">>";

	string prevDateTime = confStr["datetime"] = getDateTime();

	SettingsDialog sd(this, ts, tr["Settings"], "skin:icons/configure.png");
	sd.addSetting(new MenuSettingMultiString(this, tr["Language"], tr["Set the language used by GMenu2X"], &lang, &fl_tr.getFiles()));
	sd.addSetting(new MenuSettingDateTime(this, tr["Date & Time"], tr["Set system's date & time"], &confStr["datetime"]));
	sd.addSetting(new MenuSettingMultiString(this, tr["Battery profile"], tr["Set the battery discharge profile"], &confStr["batteryType"], &batteryType));
	sd.addSetting(new MenuSettingBool(this, tr["Save last selection"], tr["Save the last selected link and section on exit"], &confInt["saveSelection"]));
	sd.addSetting(new MenuSettingBool(this, tr["Save autostart"], tr["Save the last run app for autostart"], &confInt["saveAutoStart"]));
	sd.addSetting(new MenuSettingBool(this, tr["Output logs"], tr["Logs the link's output to read with Log Viewer"], &confInt["outputLogs"]));
	sd.addSetting(new MenuSettingInt(this,tr["Screen timeout"], tr["Seconds to turn display off if inactive"], &confInt["backlightTimeout"], 30, 10, 300));
	sd.addSetting(new MenuSettingInt(this,tr["Power timeout"], tr["Minutes to poweroff system if inactive"], &confInt["powerTimeout"], 10, 0, 300));
#if !defined(TARGET_MIYOO)
	sd.addSetting(new MenuSettingInt(this,tr["Backlight"], tr["Set LCD backlight"], &confInt["backlight"], 70, 1, 100));
	sd.addSetting(new MenuSettingInt(this, tr["Audio volume"], tr["Set the default audio volume"], &confInt["globalVolume"], 60, 0, 100));
#endif

#if defined(TARGET_RS97)
	sd.addSetting(new MenuSettingMultiString(this, tr["TV-out"], tr["TV-out signal encoding"], &confStr["TVOut"], &encodings));
#endif
	sd.addSetting(new MenuSettingMultiString(this, tr["CPU settings"], tr["Define CPU and overclock settings"], &tmp, &opFactory, 0, MakeDelegate(this, &GMenu2X::cpuSettings)));
	sd.addSetting(new MenuSettingMultiString(this, tr["Reset settings"], tr["Choose settings to reset back to defaults"], &tmp, &opFactory, 0, MakeDelegate(this, &GMenu2X::resetSettings)));

	if (sd.exec() && sd.edited() && sd.save) {
		if (curGlobalVolume != confInt["globalVolume"]) setVolume(confInt["globalVolume"]);

		if (lang == "English") lang = "";
		if (confStr["lang"] != lang) {
			confStr["lang"] = lang;
			tr.setLang(lang);
		}

		setBacklight(confInt["backlight"], false);
		writeConfig();

		powerManager->resetSuspendTimer();
		powerManager->setSuspendTimeout(confInt["backlightTimeout"]);
		powerManager->setPowerTimeout(confInt["powerTimeout"]);

#if defined(TARGET_GP2X)
		if (prevgamma != confInt["gamma"]) setGamma(confInt["gamma"]);
#endif

		if (prevDateTime != confStr["datetime"]) restartDialog();
	}
}

void GMenu2X::resetSettings() {
	bool	reset_gmenu = true,
			reset_skin = true,
			reset_icon = false,
			reset_manual = false,
			reset_parameter = false,
			reset_backdrop = false,
			reset_filter = false,
			reset_directory = false,
			reset_preview = false,
			reset_cpu = false;

	string tmppath = "";

	SettingsDialog sd(this, ts, tr["Reset settings"], "skin:icons/configure.png");
	sd.addSetting(new MenuSettingBool(this, tr["GMenuNX"], tr["Reset GMenuNX settings"], &reset_gmenu));
	sd.addSetting(new MenuSettingBool(this, tr["Default skin"], tr["Reset Default skin settings back to default"], &reset_skin));
	sd.addSetting(new MenuSettingBool(this, tr["Icons"], tr["Reset link's icon back to default"], &reset_icon));
	sd.addSetting(new MenuSettingBool(this, tr["Manuals"], tr["Unset link's manual"], &reset_manual));
	sd.addSetting(new MenuSettingBool(this, tr["Parameters"], tr["Unset link's additional parameters"], &reset_parameter));
	sd.addSetting(new MenuSettingBool(this, tr["Backdrops"], tr["Unset link's backdrops"], &reset_backdrop));
	sd.addSetting(new MenuSettingBool(this, tr["Filters"], tr["Unset link's selector file filters"], &reset_filter));
	sd.addSetting(new MenuSettingBool(this, tr["Directories"], tr["Unset link's selector directory"], &reset_directory));
	sd.addSetting(new MenuSettingBool(this, tr["Previews"], tr["Unset link's selector previews path"], &reset_preview));
	sd.addSetting(new MenuSettingBool(this, tr["CPU speed"], tr["Reset link's custom CPU speed back to default"], &reset_cpu));

	if (sd.exec() && sd.edited() && sd.save) {
		MessageBox mb(this, tr["Changes will be applied to ALL\napps and GMenuNX. Are you sure?"], "skin:icons/exit.png");
		mb.setButton(CONFIRM, tr["Cancel"]);
		mb.setButton(SECTION_NEXT,  tr["Confirm"]);
		if (mb.exec() != SECTION_NEXT) return;

		for (uint32_t s = 0; s < menu->getSections().size(); s++) {
			for (uint32_t l = 0; l < menu->sectionLinks(s)->size(); l++) {
				menu->setSectionIndex(s);
				menu->setLinkIndex(l);
				bool islink = menu->selLinkApp() != NULL;
				// WARNING("APP: %d %d %d %s", s, l, islink, menu->sectionLinks(s)->at(l)->getTitle().c_str());
				if (!islink) continue;
				if (reset_cpu)			menu->selLinkApp()->setCPU();
				if (reset_icon)			menu->selLinkApp()->setIcon("");
				if (reset_manual)		menu->selLinkApp()->setManual("");
				if (reset_parameter) 	menu->selLinkApp()->setParams("");
				if (reset_filter) 		menu->selLinkApp()->setSelectorFilter("");
				if (reset_directory) 	menu->selLinkApp()->setSelectorDir("");
				if (reset_preview) 		menu->selLinkApp()->setSelectorScreens("");
				if (reset_backdrop) 	menu->selLinkApp()->setBackdrop("");
				if (reset_icon || reset_manual || reset_parameter || reset_backdrop || reset_filter || reset_directory || reset_preview )
					menu->selLinkApp()->save();
			}
		}
		if (reset_skin) {
			tmppath = path + "skins/Default/skin.conf";
			unlink(tmppath.c_str());
		}
		if (reset_gmenu) {
			tmppath = path + "gmenu2x.conf";
			unlink(tmppath.c_str());
		}
		restartDialog();
	}
}

void GMenu2X::cpuSettings() {
	SettingsDialog sd(this, ts, tr["CPU settings"], "skin:icons/configure.png");
	sd.addSetting(new MenuSettingInt(this, tr["Default CPU clock"], tr["Set the default working CPU frequency"], &confInt["cpuMenu"], confInt["cpuMenu"], confInt["cpuMenu"], confInt["cpuMenu"], 6));
	sd.addSetting(new MenuSettingInt(this, tr["Maximum CPU clock "], tr["Maximum overclock for launching links"], &confInt["cpuMax"], confInt["cpuMax"], confInt["cpuMax"], confInt["cpuMax"], 6));
	sd.addSetting(new MenuSettingInt(this, tr["Minimum CPU clock "], tr["Minimum underclock used in Suspend mode"], &confInt["cpuMin"], confInt["cpuMin"], confInt["cpuMin"], confInt["cpuMin"], 6));

	if (sd.exec() && sd.edited() && sd.save) {
		setCPU(confInt["cpuMenu"]);
		writeConfig();
	}
}

void GMenu2X::readTmp() {
	lastSelectorElement = -1;
	if (!fileExists("/tmp/gmenu2x.tmp")) return;
	ifstream inf("/tmp/gmenu2x.tmp", ios_base::in);
	if (!inf.is_open()) return;
	string line, name, value;

	while (getline(inf, line, '\n')) {
		string::size_type pos = line.find("=");
		name = trim(line.substr(0,pos));
		value = trim(line.substr(pos+1,line.length()));
		if (name == "section") menu->setSectionIndex(atoi(value.c_str()));
		else if (name == "link") menu->setLinkIndex(atoi(value.c_str()));
		else if (name == "selectorelem") lastSelectorElement = atoi(value.c_str());
		else if (name == "selectordir") lastSelectorDir = value;
		else if (name == "TVOut") TVOut = value;
		else if (name == "tvOutPrev") tvOutPrev = atoi(value.c_str());
	}
	if (TVOut != "NTSC" && TVOut != "PAL") TVOut = "OFF";
	udcConnectedOnBoot = 0;
	inf.close();
	unlink("/tmp/gmenu2x.tmp");
}

void GMenu2X::writeTmp(int selelem, const string &selectordir) {
	string conffile = "/tmp/gmenu2x.tmp";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		inf << "section=" << menu->selSectionIndex() << endl;
		inf << "link=" << menu->selLinkIndex() << endl;
		if (selelem >- 1) inf << "selectorelem=" << selelem << endl;
		if (selectordir != "") inf << "selectordir=" << selectordir << endl;
		inf << "tvOutPrev=" << tvOutPrev << endl;
		inf << "TVOut=" << TVOut << endl;
		inf.close();
	}
}

void GMenu2X::readConfig() {
	string conffile = path + "gmenu2x.conf";

	// Defaults
	confStr["batteryType"] = "BL-5B";
	confStr["datetime"] = __BUILDTIME__;
	confInt["saveSelection"] = 1;
	confInt["saveAutoStart"] = 1;

	if (fileExists(conffile)) {
		ifstream inf(conffile.c_str(), ios_base::in);
		if (inf.is_open()) {
			string line;
			while (getline(inf, line, '\n')) {
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0, pos));
				string value = trim(line.substr(pos + 1, line.length()));

				if (value.length() > 1 && value.at(0) == '"' && value.at(value.length() - 1) == '"')
					confStr[name] = value.substr(1,value.length()-2);
				else
					confInt[name] = atoi(value.c_str());
			}
			inf.close();
		}
	}

	if (confStr["TVOut"] != "PAL") confStr["TVOut"] = "NTSC";
	if (!confStr["lang"].empty()) tr.setLang(confStr["lang"]);
	if (!confStr["wallpaper"].empty() && !fileExists(confStr["wallpaper"])) confStr["wallpaper"] = "";
	if (confStr["skin"].empty() || !dirExists("skins/" + confStr["skin"])) confStr["skin"] = "Default";

	evalIntConf( &confInt["backlightTimeout"], 30, 10, 300);
	evalIntConf( &confInt["powerTimeout"], 10, 0, 300);
	evalIntConf( &confInt["outputLogs"], 0, 0, 1 );
	evalIntConf( &confInt["cpuMax"], 642, 200, 1200 );
	evalIntConf( &confInt["cpuMin"], 342, 6, 1200 );
	evalIntConf( &confInt["cpuMenu"], 600, 200, 1200 );
	evalIntConf( &confInt["globalVolume"], 60, 1, 100 );
	evalIntConf( &confInt["videoBpp"], 16, 8, 32 );
	evalIntConf( &confInt["backlight"], 70, 1, 100);
	evalIntConf( &confInt["minBattery"], 0, 1, 10000);
	evalIntConf( &confInt["maxBattery"], 4500, 1, 10000);
	evalIntConf( &confInt["sectionBar"], SB_LEFT, 1, 4);
	evalIntConf( &confInt["textAlignment"], 0,0,1);
	evalIntConf( &confInt["linkCols"], 1, 1, 8);
	evalIntConf( &confInt["linkRows"], 5, 1, 18);

	if (!confInt["saveSelection"]) {
		confInt["section"] = 0;
		confInt["link"] = 0;
	}

	if (!confInt["saveAutoStart"]) {
		confStr["lastCommand"] = "";
		confStr["lastDirectory"] = "";
		confInt["lastCPU"] = confInt["cpuMenu"];
	}

	resX = constrain( confInt["resolutionX"], 320, 1920 );
	resY = constrain( confInt["resolutionY"], 240, 1200 );
}

void GMenu2X::writeConfig() {
	ledOn();
	if (confInt["saveSelection"] && menu != NULL) {
		confInt["section"] = menu->selSectionIndex();
		confInt["link"] = menu->selLinkIndex();
	}

	string conffile = path + "gmenu2x.conf";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		for (ConfStrHash::iterator curr = confStr.begin(); curr != confStr.end(); curr++) {
			if (curr->first == "sectionBarPosition" || curr->first == "tvoutEncoding") continue;
			inf << curr->first << "=\"" << curr->second << "\"" << endl;
		}

		for (ConfIntHash::iterator curr = confInt.begin(); curr != confInt.end(); curr++) {
			if (curr->first == "batteryLog" || curr->first == "maxClock" || curr->first == "minClock" || curr->first == "menuClock") continue;
			inf << curr->first << "=" << curr->second << endl;
		}
		inf.close();
		sync();
	}

#if defined(TARGET_GP2X)
		if (fwType == "open2x" && savedVolumeMode != volumeMode)
			writeConfigOpen2x();
#endif
	ledOff();
}

void GMenu2X::writeSkinConfig() {
	ledOn();
	string conffile = path + "skins/" + confStr["skin"] + "/skin.conf";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		for (ConfStrHash::iterator curr = skinConfStr.begin(); curr != skinConfStr.end(); curr++)
			inf << curr->first << "=\"" << curr->second << "\"" << endl;

		for (ConfIntHash::iterator curr = skinConfInt.begin(); curr != skinConfInt.end(); curr++) {
			if (curr->first == "titleFontSize" || curr->first == "sectionBarHeight" || curr->first == "linkHeight" || curr->first == "selectorPreviewX" || curr->first == "selectorPreviewY" || curr->first == "selectorPreviewWidth" || curr->first == "selectorPreviewHeight" || curr->first == "selectorX" || curr->first == "linkItemHeight" ) continue;
			inf << curr->first << "=" << curr->second << endl;
		}

		for (int i = 0; i < NUM_COLORS; ++i) {
			inf << colorToString((enum color)i) << "=" << rgbatostr(skinConfColors[i]) << endl;
		}

		inf.close();
		sync();
	}
	ledOff();
}

void GMenu2X::setSkin(const string &skin, bool resetWallpaper, bool clearSC) {
	confStr["skin"] = skin;

//Clear previous skin settings
	skinConfStr.clear();
	skinConfInt.clear();

//clear collection and change the skin path
	if (clearSC) sc.clear();
	sc.setSkin(skin);
	// if (btnContextMenu != NULL) btnContextMenu->setIcon( btnContextMenu->getIcon() );

//reset colors to the default values
	skinConfColors[COLOR_TOP_BAR_BG] = (RGBAColor){255,255,255,130};
	skinConfColors[COLOR_LIST_BG] = (RGBAColor){255,255,255,0};
	skinConfColors[COLOR_BOTTOM_BAR_BG] = (RGBAColor){255,255,255,130};
	skinConfColors[COLOR_SELECTION_BG] = (RGBAColor){255,255,255,130};
	skinConfColors[COLOR_MESSAGE_BOX_BG] = (RGBAColor){255,255,255,255};
	skinConfColors[COLOR_MESSAGE_BOX_BORDER] = (RGBAColor){80,80,80,255};
	skinConfColors[COLOR_MESSAGE_BOX_SELECTION] = (RGBAColor){160,160,160,255};
	skinConfColors[COLOR_FONT] = (RGBAColor){255,255,255,255};
	skinConfColors[COLOR_FONT_OUTLINE] = (RGBAColor){0,0,0,200};
	skinConfColors[COLOR_FONT_ALT] = (RGBAColor){253,1,252,0};
	skinConfColors[COLOR_FONT_ALT_OUTLINE] = (RGBAColor){253,1,252,0};
	//skinConfColors[COLOR_BOOTLOGO_BG] = (RGBAColor){0,0,0,255};

//load skin settings
	string skinconfname = "skins/" + skin + "/skin.conf";
	if (fileExists(skinconfname)) {
		ifstream skinconf(skinconfname.c_str(), ios_base::in);
		if (skinconf.is_open()) {
			string line;
			while (getline(skinconf, line, '\n')) {
				line = trim(line);
				// DEBUG("skinconf: '%s'", line.c_str());
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0,pos));
				string value = trim(line.substr(pos+1,line.length()));

				if (value.length() > 0) {
					if (value.length() > 1 && value.at(0) == '"' && value.at(value.length() - 1) == '"') {
							skinConfStr[name] = value.substr(1, value.length() - 2);
					} else if (value.at(0) == '#') {
						// skinConfColor[name] = strtorgba( value.substr(1,value.length()) );
						skinConfColors[stringToColor(name)] = strtorgba(value);
					} else if (name.length() > 6 && name.substr( name.length() - 6, 5 ) == "Color") {
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

			if (resetWallpaper && !skinConfStr["wallpaper"].empty() && fileExists("skins/" + skin + "/wallpapers/" + skinConfStr["wallpaper"])) {
				setWallpaper("skins/" + skin + "/wallpapers/" + skinConfStr["wallpaper"]);
				// confStr["wallpaper"] = "skins/" + skin + "/wallpapers/" + skinConfStr["wallpaper"];
				// sc[confStr["wallpaper"]]->blit(bg,0,0);
			}
		}
	}

// (poor) HACK: ensure font alt colors have a default value
	if (skinConfColors[COLOR_FONT_ALT].r == 253 && skinConfColors[COLOR_FONT_ALT].g == 1 && skinConfColors[COLOR_FONT_ALT].b == 252 && skinConfColors[COLOR_FONT_ALT].a == 0) skinConfColors[COLOR_FONT_ALT] = skinConfColors[COLOR_FONT];
	if (skinConfColors[COLOR_FONT_ALT_OUTLINE].r == 253 && skinConfColors[COLOR_FONT_ALT_OUTLINE].g == 1 && skinConfColors[COLOR_FONT_ALT_OUTLINE].b == 252 && skinConfColors[COLOR_FONT_ALT_OUTLINE].a == 0) skinConfColors[COLOR_FONT_ALT_OUTLINE] = skinConfColors[COLOR_FONT_OUTLINE];

// prevents breaking current skin until they are updated
	if (!skinConfInt["fontSizeTitle"] && skinConfInt["titleFontSize"] > 0) skinConfInt["fontSizeTitle"] = skinConfInt["titleFontSize"];

	evalIntConf( &skinConfInt["topBarHeight"], 40, 1, resY);
	evalIntConf( &skinConfInt["sectionBarSize"], 40, 1, resX);
	evalIntConf( &skinConfInt["bottomBarHeight"], 16, 1, resY);
	evalIntConf( &skinConfInt["previewWidth"], 142, 1, resX);
	evalIntConf( &skinConfInt["fontSize"], 12, 6, 60);
	evalIntConf( &skinConfInt["fontSizeTitle"], 20, 6, 60);
	evalIntConf( &skinConfInt["hideIcons"], 0, 1, 1);
	evalIntConf( &skinConfInt["invertSelected"], 0, 0, 1);
	evalIntConf( &skinConfInt["sectionLetters"], 0, 0, 1);
	evalIntConf( &skinConfInt["shortNames"], 0, 0, 1);
	evalIntConf( &skinConfInt["hideDescription"], 0, 0, 1);
	evalIntConf( &skinConfInt["bootLogoDisplay"], 1, 0, 1);
	evalIntConf( &skinConfInt["bootLogoDelay"], 12, 0, 100);

	if (menu != NULL && clearSC) menu->loadIcons();

//font
	initFont();
}

uint32_t GMenu2X::onChangeSkin() {
	return 1;
}

void GMenu2X::skinMenu() {
	bool save = false;
	int selected = 0;
	string prevSkin = confStr["skin"];
	int prevSkinBackdrops = confInt["skinBackdrops"];

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
	vector<string> taStr;
	taStr.push_back("Left");
	taStr.push_back("Center");
	int sbPrev = confInt["sectionBar"];
	string sectionBar = sbStr[confInt["sectionBar"]];
	string textAlignment = taStr[confInt["textAlignment"]];

	do {
		setSkin(confStr["skin"], false, false);

		FileLister fl_wp("skins/" + confStr["skin"] + "/wallpapers");
		fl_wp.setFilter(".png,.jpg,.jpeg,.bmp");
		vector<string> wallpapers;
		if (dirExists("skins/" + confStr["skin"] + "/wallpapers")) {
			fl_wp.browse();
			wallpapers = fl_wp.getFiles();
		}
		if (confStr["skin"] != "Default") {
			fl_wp.setPath("skins/Default/wallpapers", true);
			for (uint32_t i = 0; i < fl_wp.getFiles().size(); i++)
				wallpapers.push_back(fl_wp.getFiles()[i]);
		}

		confStr["wallpaper"] = base_name(confStr["wallpaper"]);
		string wpPrev = confStr["wallpaper"];

		sc.del("skin:icons/skin.png");
		sc.del("skin:imgs/buttons/left.png");
		sc.del("skin:imgs/buttons/right.png");
		sc.del("skin:imgs/buttons/a.png");

		SettingsDialog sd(this, ts, tr["Skin"], "skin:icons/skin.png");
		sd.selected = selected;
		sd.allowCancel = false;
		sd.addSetting(new MenuSettingMultiString(this, tr["Skin"], tr["Set the skin used by GMenu2X"], &confStr["skin"], &fl_sk.getDirectories(), MakeDelegate(this, &GMenu2X::onChangeSkin)));
		sd.addSetting(new MenuSettingMultiString(this, tr["Wallpaper"], tr["Select an image to use as a wallpaper"], &confStr["wallpaper"], &wallpapers, MakeDelegate(this, &GMenu2X::onChangeSkin), MakeDelegate(this, &GMenu2X::changeWallpaper)));
		sd.addSetting(new MenuSettingMultiString(this, tr["Skin colors"], tr["Customize skin colors"], &tmp, &wpLabel, MakeDelegate(this, &GMenu2X::onChangeSkin), MakeDelegate(this, &GMenu2X::skinColors)));
		sd.addSetting(new MenuSettingBool(this, tr["Skin backdrops"], tr["Automatic load backdrops from skin pack"], &confInt["skinBackdrops"]));
		sd.addSetting(new MenuSettingInt(this, tr["Font size"], tr["Size of text font"], &skinConfInt["fontSize"], 12, 6, 60));
		sd.addSetting(new MenuSettingInt(this, tr["Title font size"], tr["Size of title's text font"], &skinConfInt["fontSizeTitle"], 20, 6, 60));
		sd.addSetting(new MenuSettingInt(this, tr["Top bar height"], tr["Height of top bar"], &skinConfInt["topBarHeight"], 40, 1, resY));
		sd.addSetting(new MenuSettingInt(this, tr["Bottom bar height"], tr["Height of bottom bar"], &skinConfInt["bottomBarHeight"], 16, 1, resY));
		sd.addSetting(new MenuSettingInt(this, tr["Section bar size"], tr["Size of section bar"], &skinConfInt["sectionBarSize"], 40, 1, resX));
		sd.addSetting(new MenuSettingMultiString(this, tr["Section bar position"], tr["Set the position of the Section Bar"], &sectionBar, &sbStr));
		sd.addSetting(new MenuSettingInt(this, tr["Section letter"], tr["Use first letter instead of icon"], &confInt["sectionLetters"], 0, 0, 1));
		sd.addSetting(new MenuSettingInt(this, tr["Menu columns"], tr["Number of columns of links in main menu"], &confInt["linkCols"], 1, 1, 8));
		sd.addSetting(new MenuSettingInt(this, tr["Menu rows"], tr["Number of rows of links in main menu"], &confInt["linkRows"], 6, 1, 18));
		sd.addSetting(new MenuSettingMultiString(this, tr["Text Alignment"], tr["Set the alignment of the text"], &textAlignment, &taStr));
		sd.addSetting(new MenuSettingInt(this, tr["Short Names"], tr["Use short names when not selected"], &confInt["shortNames"], 0, 0, 1));
		sd.addSetting(new MenuSettingInt(this, tr["Hide Description"], tr["Hide description line"], &confInt["hideDescription"], 0, 0, 1));
		sd.addSetting(new MenuSettingInt(this, tr["Invert Selected"], tr["Invert colors of highlighted item"], &confInt["invertSelected"], 0, 0, 1));
		sd.addSetting(new MenuSettingInt(this, tr["Hide Icons"], tr["Hide icons in list"], &confInt["hideIcons"], 0, 0, 1));
		sd.addSetting(new MenuSettingInt(this, tr["Boot Logo"], tr["Show boot logo"], &confInt["bootLogoShow"], 1, 0, 1));
		sd.addSetting(new MenuSettingInt(this, tr["Boot Logo Delay"], tr["How long to delay for audio"], &confInt["bootLogoDelay"], 12, 0, 100));
		sd.exec();

		if (sc.add("skins/" + confStr["skin"] + "/wallpapers/" + confStr["wallpaper"]) != NULL)
			confStr["wallpaper"] = "skins/" + confStr["skin"] + "/wallpapers/" + confStr["wallpaper"];
		else if (sc.add("skins/Default/wallpapers/" + confStr["wallpaper"]) != NULL)
			confStr["wallpaper"] = "skins/Default/wallpapers/" + confStr["wallpaper"];
		sc.add("skins/" + confStr["skin"] + "bootlogo.png");

		setWallpaper(confStr["wallpaper"]);

		selected = sd.selected;
		save = sd.save;
	} while (!save);

	if (sectionBar == "OFF") confInt["sectionBar"] = SB_OFF;
	else if (sectionBar == "Right") confInt["sectionBar"] = SB_RIGHT;
	else if (sectionBar == "Top") confInt["sectionBar"] = SB_TOP;
	else if (sectionBar == "Bottom") confInt["sectionBar"] = SB_BOTTOM;
	else confInt["sectionBar"] = SB_LEFT;

	if (textAlignment == "Left") confInt["textAlignment"] = TA_LEFT;
	else confInt["textAlignment"] = TA_CENTER;

	writeSkinConfig();
	writeConfig();

	if (prevSkinBackdrops != confInt["skinBackdrops"] || prevSkin != confStr["skin"]) restartDialog();
	if (sbPrev != confInt["sectionBar"]) initMenu();
	initLayout();
}

void GMenu2X::skinColors() {
	bool save = false;
	do {
		setSkin(confStr["skin"], false, false);

		SettingsDialog sd(this, ts, tr["Skin Colors"], "skin:icons/skin.png");
		sd.allowCancel = false;
		sd.addSetting(new MenuSettingRGBA(this, tr["Top/Section Bar"], tr["Color of the top and section bar"], &skinConfColors[COLOR_TOP_BAR_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["List Body"], tr["Color of the list body"], &skinConfColors[COLOR_LIST_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Bottom Bar"], tr["Color of the bottom bar"], &skinConfColors[COLOR_BOTTOM_BAR_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Selection"], tr["Color of the selection and other interface details"], &skinConfColors[COLOR_SELECTION_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Message Box"], tr["Background color of the message box"], &skinConfColors[COLOR_MESSAGE_BOX_BG]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Msg Box Border"], tr["Border color of the message box"], &skinConfColors[COLOR_MESSAGE_BOX_BORDER]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Msg Box Selection"], tr["Color of the selection of the message box"], &skinConfColors[COLOR_MESSAGE_BOX_SELECTION]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Font"], tr["Color of the font"], &skinConfColors[COLOR_FONT]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Font Outline"], tr["Color of the font's outline"], &skinConfColors[COLOR_FONT_OUTLINE]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Alt Font"], tr["Color of the alternative font"], &skinConfColors[COLOR_FONT_ALT]));
		sd.addSetting(new MenuSettingRGBA(this, tr["Alt Font Outline"], tr["Color of the alternative font outline"], &skinConfColors[COLOR_FONT_ALT_OUTLINE]));
		//sd.addSetting(new MenuSettingRGBA(this, tr["Bootlogo Background"], tr["Color of the bootlogo background"], &skinConfColors[COLOR_BOOTLOGO_BG]));
		sd.exec();
		save = sd.save;
	} while (!save);
	writeSkinConfig();
}

void GMenu2X::about() {
	vector<string> text;
	string temp, batt;

	char *hms = ms2hms(SDL_GetTicks());
	int32_t battlevel = getBatteryStatus();
	char *hwv = hwVersion();

	stringstream ss; ss << battlevel; ss >> batt;

	temp = tr["Build date: "] + __DATE__ + "\n";
	temp += tr["Uptime: "] + hms + "\n";
#ifdef TARGET_RS97
	temp += tr["Battery: "] + ((battlevel < 0 || battlevel > 10000) ? tr["Charging"] : batt) + "\n";
	temp += tr["Checksum: 0x"] + hwv + "\n";
#endif
	// temp += tr["Storage:"];
	// temp += "\n    " + tr["Root: "] + getDiskFree("/");
	// temp += "\n    " + tr["Internal: "] + getDiskFree("/mnt/int_sd");
	// temp += "\n    " + tr["External: "] + getDiskFree("/mnt/ext_sd");
	temp += "----\n";

	TextDialog td(this, "GMenuNX", tr["Info about system"], "skin:icons/about.png");

	// todo: translate
	td.appendText("\nFirmware version: ");
	string versionFile = "/boot/version.txt";
	if (fileExists(versionFile)){
		td.appendFile(versionFile);
	}
	else {
		td.appendText("unknown - no " + versionFile);
	}
	td.appendText(exec("uname -srm"));

	td.appendText(temp);
	td.appendFile("about.txt");
	td.exec();
}

void GMenu2X::viewLog() {
	string logfile = path + "log.txt";
	if (!fileExists(logfile)) return;

	TextDialog td(this, tr["Log Viewer"], tr["Last launched program's output"], "skin:icons/ebook.png");
	td.appendFile(path + "log.txt");
	td.exec();

	MessageBox mb(this, tr["Do you want to delete the log file?"], "skin:icons/ebook.png");
	mb.setButton(CONFIRM, tr["Yes"]);
	mb.setButton(CANCEL,  tr["No"]);
	if (mb.exec() == CONFIRM) {
		ledOn();
		unlink(logfile.c_str());
		sync();
		menu->deleteSelectedLink();
		ledOff();
	}
}

void GMenu2X::batteryLogger() {
	BatteryLoggerDialog bl(this, tr["Battery Logger"], tr["Log battery power to battery.csv"], "skin:icons/ebook.png");
	bl.exec();
}

void GMenu2X::linkScanner() {
	LinkScannerDialog ls(this, tr["Link scanner"], tr["Scan for applications and games"], "skin:icons/configure.png");
	ls.exec();
}

void GMenu2X::changeWallpaper() {
	WallpaperDialog wp(this, tr["Wallpaper"], tr["Select an image to use as a wallpaper"], "skin:icons/wallpaper.png");
	if (wp.exec() && confStr["wallpaper"] != wp.wallpaper) {
		confStr["wallpaper"] = wp.wallpaper;
		setWallpaper(wp.wallpaper);
		writeConfig();
	}
}

void GMenu2X::showManual() {
	string linkTitle = menu->selLinkApp()->getTitle();
	string linkDescription = menu->selLinkApp()->getDescription();
	string linkIcon = menu->selLinkApp()->getIcon();
	string linkManual = menu->selLinkApp()->getManualPath();
	string linkBackdrop = menu->selLinkApp()->getBackdropPath();

	if (linkManual == "" || !fileExists(linkManual)) return;

	string ext = linkManual.substr(linkManual.size() - 4, 4);
	if (ext == ".png" || ext == ".bmp" || ext == ".jpg" || ext == "jpeg") {
		ImageViewerDialog im(this, linkTitle, linkDescription, linkIcon, linkManual);
		im.exec();
		return;
	}

	TextDialog td(this, linkTitle, linkDescription, linkIcon, linkBackdrop);
	td.appendFile(linkManual);
	td.exec();
}

void GMenu2X::explorer() {
	BrowseDialog fd(this, tr["Explorer"], tr["Select a file or application"]);
	fd.showDirectories = true;
	fd.showFiles = true;

	bool loop = true;
	while (fd.exec() && loop) {
		string ext = fd.getExt();
		if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif") {
			ImageViewerDialog im(this, tr["Image viewer"], fd.getFile(), "icons/explorer.png", fd.getPath() + "/" + fd.getFile());
			im.exec();
			continue;
		} else if (ext == ".txt" || ext == ".conf" || ext == ".me" || ext == ".md" || ext == ".xml" || ext == ".log") {
			TextDialog td(this, tr["Text viewer"], fd.getFile(), "skin:icons/ebook.png");
			td.appendFile(fd.getPath() + "/" + fd.getFile());
			td.exec();
		} else {
			if (confInt["saveSelection"] && (confInt["section"] != menu->selSectionIndex() || confInt["link"] != menu->selLinkIndex()))
				writeConfig();

			loop = false;
			//string command = cmdclean(fd.path()+"/"+fd.file) + "; sync & cd "+cmdclean(getExePath())+"; exec ./gmenu2x";
			string command = cmdclean(fd.getPath() + "/" + fd.getFile());
			if (confInt["saveAutoStart"]) {
				confInt["lastCPU"] = confInt["cpuMenu"];
				confStr["lastCommand"] = command.c_str();
				confStr["lastDirectory"] = fd.getPath().c_str();
				writeConfig();
			}
			chdir(fd.getPath().c_str());
			quit();
			setCPU(confInt["cpuMenu"]);
			execlp("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);

			//if execution continues then something went wrong and as we already called SDL_Quit we cannot continue
			//try relaunching gmenu2x
			WARNING("Error executing selected application, re-launching gmenu2x");
			chdir(getExePath().c_str());
			execlp("./gmenu2x", "./gmenu2x", NULL);
		}
	}
}

void GMenu2X::ledOn() {
#if defined(TARGET_GP2X)
	if (memdev != 0 && !f200) memregs[0x106E >> 1] ^= 16;
	//SDL_SYS_JoystickGp2xSys(joy.joystick, BATT_LED_ON);
#endif
}

void GMenu2X::ledOff() {
#if defined(TARGET_GP2X)
	if (memdev != 0 && !f200) memregs[0x106E >> 1] ^= 16;
	//SDL_SYS_JoystickGp2xSys(joy.joystick, BATT_LED_OFF);
#endif
}

void GMenu2X::hwCheck() {
#if defined(TARGET_RS97)
	if (memdev > 0) {
		// printf("\e[s\e[1;0f");
		// printbin("A", memregs[0x10000 >> 2]);
		// printbin("B", memregs[0x10100 >> 2]);
		// printbin("C", memregs[0x10200 >> 2]);
		// printbin("D", memregs[0x10300 >> 2]);
		// printbin("E", memregs[0x10400 >> 2]);
		// printbin("F", memregs[0x10500 >> 2]);
		// printf("\n\e[K\e[u");

		curMMCStatus = getMMCStatus();
		if (preMMCStatus != curMMCStatus) {
			preMMCStatus = curMMCStatus;
			string msg;

			if (curMMCStatus == MMC_INSERT) msg = tr["SD card connected"];
			else msg = tr["SD card removed"];

			MessageBox mb(this, msg, "skin:icons/eject.png");
			mb.setAutoHide(1000);
			mb.exec();

			if (curMMCStatus == MMC_INSERT) {
				mountSd(true);
				menu->addActionLink(menu->getSectionIndex("settings"), tr["Umount"], MakeDelegate(this, &GMenu2X::umountSdDialog), tr["Umount external SD"], "skin:icons/eject.png");
			} else {
				umountSd(true);
			}
		}

		tvOutConnected = getTVOutStatus();
		if (tvOutPrev != tvOutConnected) {
			tvOutPrev = tvOutConnected;

			TVOut = "OFF";
			int lcd_brightness = confInt["backlight"];

			if (tvOutConnected) {
				MessageBox mb(this, tr["TV-out connected.\nContinue?"], "skin:icons/tv.png");
				mb.setButton(SETTINGS, tr["Yes"]);
				mb.setButton(CONFIRM,  tr["No"]);

				if (mb.exec() == SETTINGS) {
					TVOut = confStr["TVOut"];
					lcd_brightness = 0;
				}
			}
			setTVOut(TVOut);
			setBacklight(lcd_brightness);
		}

		volumeMode = getVolumeMode(confInt["globalVolume"]);
		if (volumeModePrev != volumeMode && volumeMode == VOLUME_MODE_PHONES) {
			setVolume(min(70, confInt["globalVolume"]), true);
		}
		volumeModePrev = volumeMode;
	}
#endif
}

const string GMenu2X::getDateTime() {
	char       buf[80];
	time_t     now = time(0);
	struct tm  tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%F %R", &tstruct);
	return buf;
}

void GMenu2X::setDateTime() {
	int imonth, iday, iyear, ihour, iminute;
	string value = confStr["datetime"];

	sscanf(value.c_str(), "%d-%d-%d %d:%d", &iyear, &imonth, &iday, &ihour, &iminute);

	struct tm datetime = { 0 };

	datetime.tm_year = iyear - 1900;
	datetime.tm_mon  = imonth - 1;
	datetime.tm_mday = iday;
	datetime.tm_hour = ihour;
	datetime.tm_min  = iminute;
	datetime.tm_sec  = 0;

	if (datetime.tm_year < 0) datetime.tm_year = 0;

	time_t t = mktime(&datetime);

	struct timeval newtime = { t, 0 };

#if !defined(TARGET_PC)
	settimeofday(&newtime, NULL);
#endif
}

bool GMenu2X::saveScreenshot() {
	ledOn();
	uint32_t x = 0;
	string fname;

	mkdir("screenshots/", 0777);

	do {
		x++;
		// fname = "";
		stringstream ss;
		ss << x;
		ss >> fname;
		fname = "screenshots/screen" + fname + ".bmp";
	} while (fileExists(fname));
	x = SDL_SaveBMP(s->raw, fname.c_str());
	sync();
	ledOff();
	return x == 0;
}

void GMenu2X::restartDialog(bool showDialog) {
	if (showDialog) {
		MessageBox mb(this, tr["GMenuNX will restart to apply\nthe settings. Continue?"], "skin:icons/exit.png");
		mb.setButton(CONFIRM, tr["Restart"]);
		mb.setButton(CANCEL,  tr["Cancel"]);
		if (mb.exec() == CANCEL) return;
	}

	quit();
	WARNING("Re-launching gmenu2x");
	chdir(getExePath().c_str());
	execlp("./gmenu2x", "./gmenu2x", NULL);
}

void GMenu2X::poweroffDialog() {
	MessageBox mb(this, tr["Poweroff or reboot the device?"], "skin:icons/exit.png");
	mb.setButton(SECTION_NEXT, tr["Reboot"]);
	mb.setButton(CONFIRM, tr["Poweroff"]);
	mb.setButton(CANCEL,  tr["Cancel"]);
	int response = mb.exec();
	if (response == CONFIRM) {
		MessageBox mb(this, tr["Poweroff"]);
		mb.setAutoHide(500);
		mb.exec();
		setBacklight(0);
#if !defined(TARGET_PC)
		system("sync; poweroff");
#endif
	}
	else if (response == SECTION_NEXT) {
		MessageBox mb(this, tr["Rebooting"]);
		mb.setAutoHide(500);
		mb.exec();
		setBacklight(0);
#if !defined(TARGET_PC)
		system("sync; reboot");
#endif
	}
}

void GMenu2X::setTVOut(string TVOut) {
#if defined(TARGET_RS97)
	system("echo 0 > /proc/jz/tvselect"); // always reset tv out
	if (TVOut == "NTSC")		system("echo 2 > /proc/jz/tvselect");
	else if (TVOut == "PAL")	system("echo 1 > /proc/jz/tvselect");
#endif
}

void GMenu2X::mountSd(bool ext) {
  return;

	if (ext)	system("par=$(( $(readlink /tmp/.int_sd | head -c -3 | tail -c 1) ^ 1 )); par=$(ls /dev/mmcblk$par* | tail -n 1); sync; umount -fl /mnt/ext_sd; mount -t vfat -o rw,utf8 $par /mnt/ext_sd");
	else		system("par=$(readlink /tmp/.int_sd | head -c -3 | tail -c 1); par=$(ls /dev/mmcblk$par* | tail -n 1); sync; umount -fl /mnt/int_sd; mount -t vfat -o rw,utf8 $par /mnt/int_sd");
}

void GMenu2X::umountSd(bool ext) {
  return;

	if (ext)	system("sync; umount -fl /mnt/ext_sd");
	else		system("sync; umount -fl /mnt/int_sd");
}

#if defined(TARGET_RS97)
void GMenu2X::umountSdDialog() {
	MessageBox mb(this, tr["Umount SD card?"], "skin:icons/eject.png");
	mb.setButton(CONFIRM, tr["Yes"]);
	mb.setButton(CANCEL,  tr["No"]);
	if (mb.exec() == CONFIRM) {
		umountSd(true);
		menu->deleteSelectedLink();
		MessageBox mb(this, tr["SD card umounted"], "skin:icons/eject.png");
		mb.setAutoHide(1000);
		mb.exec();
	}
}

void GMenu2X::checkUDC() {
  return;

	if (getUDCStatus() == UDC_CONNECT) {
		if (!fileExists("/sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file")) {
			MessageBox mb(this, tr["This device does not support USB mount."], "skin:icons/usb.png");
			mb.setButton(CANCEL,  tr["Charger"]);
			mb.exec();
			return;
		}

		MessageBox mb(this, tr["Select USB mode:"], "skin:icons/usb.png");
		mb.setButton(CONFIRM, tr["USB Drive"]);
		mb.setButton(CANCEL,  tr["Charger"]);
		if (mb.exec() == CONFIRM) {
			umountSd(false);
			system("echo \"\" > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file; par=$(readlink /tmp/.int_sd | head -c -3 | tail -c 1); par=$(ls /dev/mmcblk$par* | tail -n 1); echo \"$par\" > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file");
			INFO("%s, connect USB disk for internal SD", __func__);

			if (getMMCStatus() == MMC_INSERT) {
				umountSd(true);
				system("echo \"\" > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun0/file; par=$(( $(readlink /tmp/.int_sd | head -c -3 | tail -c 1) ^ 1 )); par=$(ls /dev/mmcblk$par* | tail -n 1); echo \"$par\" > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun0/file");
				INFO("%s, connect USB disk for external SD", __func__);
			}

			sc[confStr["wallpaper"]]->blit(s,0,0);

			{
				MessageBox mb(this, tr["USB Drive Connected"], "skin:icons/usb.png");
				mb.setAutoHide(500);
				mb.exec();
			}

			powerManager->clearTimer();

			while (udcConnectedOnBoot == UDC_CONNECT && getUDCStatus() == UDC_CONNECT) {
				input.update();
				if ( input[MENU] && input[POWER]) udcConnectedOnBoot = UDC_REMOVE;
			}

			{
				MessageBox mb(this, tr["USB disconnected. Rebooting..."], "skin:icons/usb.png");
				mb.setAutoHide(200);
				mb.exec();
			}

			system("sync; reboot & sleep 1m");

			system("echo '' > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun0/file");
			mountSd(false);
			INFO("%s, disconnect usbdisk for internal sd", __func__);
			if (getMMCStatus() == MMC_INSERT) {
				system("echo '' > /sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file");
				mountSd(true);
				INFO("%s, disconnect USB disk for external SD", __func__);
			}
			// powerManager->resetSuspendTimer();
		}
	}
}

void GMenu2X::formatSd() {
	MessageBox mb(this, tr["Format internal SD card?"], "skin:icons/format.png");
	mb.setButton(CONFIRM, tr["Yes"]);
	mb.setButton(CANCEL,  tr["No"]);
	if (mb.exec() == CONFIRM) {
		MessageBox mb(this, tr["Formatting internal SD card..."], "skin:icons/format.png");
		mb.setAutoHide(100);
		mb.exec();

		system("/usr/bin/format_int_sd.sh");
		{ // new mb scope
			MessageBox mb(this, tr["Complete!"]);
			mb.setAutoHide(0);
			mb.exec();
		}
	}
}
#endif

void GMenu2X::contextMenu() {
	vector<MenuOption> voices;
	if (menu->selLinkApp() != NULL) {
		voices.push_back((MenuOption){tr.translate("Edit $1", menu->selLink()->getTitle().c_str(), NULL), MakeDelegate(this, &GMenu2X::editLink)});
		voices.push_back((MenuOption){tr.translate("Delete $1", menu->selLink()->getTitle().c_str(), NULL), MakeDelegate(this, &GMenu2X::deleteLink)});
	}
	voices.push_back((MenuOption){tr["Add link"], 		MakeDelegate(this, &GMenu2X::addLink)});
	voices.push_back((MenuOption){tr["Add section"],	MakeDelegate(this, &GMenu2X::addSection)});
	voices.push_back((MenuOption){tr["Rename section"],	MakeDelegate(this, &GMenu2X::renameSection)});
	voices.push_back((MenuOption){tr["Delete section"],	MakeDelegate(this, &GMenu2X::deleteSection)});
	voices.push_back((MenuOption){tr["Link scanner"],	MakeDelegate(this, &GMenu2X::linkScanner)});

	Surface bg(s);
	bool close = false, inputAction = false;
	int sel = 0;
	uint32_t i, fadeAlpha = 0, h = font->getHeight(), h2 = font->getHalfHeight();

	SDL_Rect box;
	box.h = h * voices.size() + 8;
	box.w = 0;
	for (i = 0; i < voices.size(); i++) {
		int w = font->getTextWidth(voices[i].text);
		if (w > box.w) box.w = w;
	}
	box.w += 23;
	box.x = halfX - box.w / 2;
	box.y = halfY - box.h / 2;

	uint32_t tickStart = SDL_GetTicks();
	input.setWakeUpInterval(45);
	while (!close) {
		bg.blit(s, 0, 0);

		s->box(0, 0, resX, resY, 0,0,0, fadeAlpha);
		s->box(box.x, box.y, box.w, box.h, skinConfColors[COLOR_MESSAGE_BOX_BG]);
		s->rectangle( box.x + 2, box.y + 2, box.w - 4, box.h - 4, skinConfColors[COLOR_MESSAGE_BOX_BORDER] );

		//draw selection rect
		s->box( box.x + 4, box.y + 4 + h * sel, box.w - 8, h, skinConfColors[COLOR_MESSAGE_BOX_SELECTION] );
		for (i = 0; i < voices.size(); i++)
			s->write( font, voices[i].text, box.x + 12, box.y + h2 + 3 + h * i, VAlignMiddle, skinConfColors[COLOR_FONT_ALT], skinConfColors[COLOR_FONT_ALT_OUTLINE]);

		s->flip();

		if (fadeAlpha < 200) {
			fadeAlpha = intTransition(0, 200, tickStart, 200);
			continue;
		}
		do {
			inputAction = input.update();

			if (inputCommonActions(inputAction)) continue;

			if ( input[MENU] || input[CANCEL]) close = true;
			else if ( input[UP] ) sel = (sel - 1 < 0) ? (int)voices.size() - 1 : sel - 1 ;
			else if ( input[DOWN] ) sel = (sel + 1 > (int)voices.size() - 1) ? 0 : sel + 1;
			else if ( input[LEFT] || input[PAGEUP] ) sel = 0;
			else if ( input[RIGHT] || input[PAGEDOWN] ) sel = (int)voices.size() - 1;
			else if ( input[SETTINGS] || input[CONFIRM] ) { voices[sel].action(); close = true; }
		} while (!inputAction);
	}
}

void GMenu2X::addLink() {
	BrowseDialog fd(this, tr["Add link"], tr["Select an application"]);
	fd.showDirectories = true;
	fd.showFiles = true;
	fd.setFilter(".dge,.gpu,.gpe,.sh,.bin,.elf,");
	if (fd.exec()) {
		ledOn();
		if (menu->addLink(fd.getPath(), fd.getFile())) {
			editLink();
		}
		sync();
		ledOff();
	}
}

void GMenu2X::editLink() {
	if (menu->selLinkApp() == NULL) return;

	vector<string> pathV;
	// ERROR("FILE: %s", menu->selLinkApp()->getFile().c_str());
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
	bool linkSelBrowser = menu->selLinkApp()->getSelectorBrowser();
	string linkSelScreens = menu->selLinkApp()->getSelectorScreens();
	string linkSelAliases = menu->selLinkApp()->getAliasFile();
	int linkClock = menu->selLinkApp()->clock();
	string linkBackdrop = menu->selLinkApp()->getBackdrop();
	string dialogTitle = tr.translate("Edit $1", linkTitle.c_str(), NULL);
	string dialogIcon = menu->selLinkApp()->getIconPath();

	SettingsDialog sd(this, ts, dialogTitle, dialogIcon);
	sd.addSetting(new MenuSettingFile(			this, tr["Executable"],		tr["Application this link points to"], &linkExec, ".dge,.gpu,.gpe,.sh,.bin,.elf,", CARD_ROOT, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingString(		this, tr["Title"],			tr["Link title"], &linkTitle, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingString(		this, tr["Description"],	tr["Link description"], &linkDescription, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingMultiString(	this, tr["Section"],		tr["The section this link belongs to"], &newSection, &menu->getSections()));
	sd.addSetting(new MenuSettingImage(			this, tr["Icon"],			tr["Select a custom icon for the link"], &linkIcon, ".png,.bmp,.jpg,.jpeg,.gif", dir_name(linkIcon), dialogTitle, dialogIcon));
#if defined(TARGET_MIYOO)
	sd.addSetting(new MenuSettingMultiInt(		this, tr["CPU Clock"],		tr["CPU clock frequency when launching this link"], &linkClock, oc_choices, oc_choices_size, confInt["cpuMenu"], confInt["cpuMin"], confInt["cpuMax"]));
#else
	sd.addSetting(new MenuSettingInt(			this, tr["CPU Clock"],		tr["CPU clock frequency when launching this link"], &linkClock, confInt["cpuMenu"], confInt["cpuMin"], confInt["cpuMax"], 6));
#endif
	sd.addSetting(new MenuSettingString(		this, tr["Parameters"],		tr["Command line arguments to pass to the application"], &linkParams, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingDir(			this, tr["Selector Path"],	tr["Directory to start the selector"], &linkSelDir, CARD_ROOT, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingBool(			this, tr["Show Folders"],	tr["Allow the selector to change directory"], &linkSelBrowser));
	sd.addSetting(new MenuSettingString(		this, tr["File Filter"],	tr["Filter by file extension (separate with commas)"], &linkSelFilter, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingDir(			this, tr["Screenshots"],	tr["Directory of the screenshots for the selector"], &linkSelScreens, CARD_ROOT, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingFile(			this, tr["Aliases"],		tr["File containing a list of aliases for the selector"], &linkSelAliases, ".txt,.dat", CARD_ROOT, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingImage(			this, tr["Backdrop"],		tr["Select an image backdrop"], &linkBackdrop, ".png,.bmp,.jpg,.jpeg", CARD_ROOT, dialogTitle, dialogIcon));
	sd.addSetting(new MenuSettingFile(			this, tr["Manual"],   		tr["Select a Manual or Readme file"], &linkManual, ".man.png,.txt,.me", dir_name(linkManual), dialogTitle, dialogIcon));

#if defined(TARGET_WIZ) || defined(TARGET_CAANOO)
	bool linkUseGinge = menu->selLinkApp()->getUseGinge();
	string ginge_prep = getExePath() + "/ginge/ginge_prep";
	if (fileExists(ginge_prep))
		sd.addSetting(new MenuSettingBool(        this, tr["Use Ginge"],            tr["Compatibility layer for running GP2X applications"], &linkUseGinge ));
#elif defined(TARGET_GP2X)
	//G
	int linkGamma = menu->selLinkApp()->gamma();
	sd.addSetting(new MenuSettingInt(         this, tr["Gamma (default: 0)"],   tr["Gamma value to set when launching this link"], &linkGamma, 0, 100 ));
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
		menu->selLinkApp()->setSelectorDir(linkSelDir);
		menu->selLinkApp()->setSelectorBrowser(linkSelBrowser);
		menu->selLinkApp()->setSelectorScreens(linkSelScreens);
		menu->selLinkApp()->setAliasFile(linkSelAliases);
		menu->selLinkApp()->setBackdrop(linkBackdrop);
		menu->selLinkApp()->setCPU(linkClock);
		//G
#if defined(TARGET_GP2X)
		menu->selLinkApp()->setGamma(linkGamma);
#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
		menu->selLinkApp()->setUseGinge(linkUseGinge);
#endif

		//if section changed move file and update link->file
		if (oldSection != newSection) {
			vector<string>::const_iterator newSectionIndex = find(menu->getSections().begin(), menu->getSections().end(), newSection);
			if (newSectionIndex == menu->getSections().end()) return;
			string newFileName = "sections/" + newSection + "/" + linkTitle;
			uint32_t x = 2;
			while (fileExists(newFileName)) {
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
	initMenu();
}

void GMenu2X::deleteLink() {
	if (menu->selLinkApp() != NULL) {
		MessageBox mb(this, tr.translate("Delete $1", menu->selLink()->getTitle().c_str(), NULL) + "\n" + tr["Are you sure?"], menu->selLink()->getIconPath());
		mb.setButton(CONFIRM, tr["Yes"]);
		mb.setButton(CANCEL,  tr["No"]);
		if (mb.exec() == CONFIRM) {
			ledOn();
			menu->deleteSelectedLink();
			sync();
			ledOff();
		}
	}
}

void GMenu2X::addSection() {
	InputDialog id(this, ts, tr["Insert a name for the new section"], "", tr["Add section"], "skin:icons/section.png");
	if (id.exec()) {
		//only if a section with the same name does not exist
		if (find(menu->getSections().begin(), menu->getSections().end(), id.getInput()) == menu->getSections().end()) {
			//section directory doesn't exists
			ledOn();
			if (menu->addSection(id.getInput())) {
				menu->setSectionIndex( menu->getSections().size() - 1 ); //switch to the new section
				sync();
			}
			ledOff();
		}
	}
}

void GMenu2X::renameSection() {
	InputDialog id(this, ts, tr["Insert a new name for this section"], menu->selSection(), tr["Rename section"], menu->getSectionIcon(menu->selSectionIndex()));
	if (id.exec()) {
		//only if a section with the same name does not exist & !samename
		if (menu->selSection() != id.getInput() && find(menu->getSections().begin(),menu->getSections().end(), id.getInput()) == menu->getSections().end()) {
			//section directory doesn't exists
			string newsectiondir = "sections/" + id.getInput();
			string sectiondir = "sections/" + menu->selSection();
			ledOn();
			if (rename(sectiondir.c_str(), "tmpsection")==0 && rename("tmpsection", newsectiondir.c_str())==0) {
				string oldpng = sectiondir + ".png", newpng = newsectiondir+".png";
				string oldicon = sc.getSkinFilePath(oldpng), newicon = sc.getSkinFilePath(newpng);
				if (!oldicon.empty() && newicon.empty()) {
					newicon = oldicon;
					newicon.replace(newicon.find(oldpng), oldpng.length(), newpng);

					if (!fileExists(newicon)) {
						rename(oldicon.c_str(), "tmpsectionicon");
						rename("tmpsectionicon", newicon.c_str());
						sc.move("skin:" + oldpng, "skin:" + newpng);
					}
				}
				menu->renameSection(menu->selSectionIndex(), id.getInput());
				sync();
			}
			ledOff();
		}
	}
}

void GMenu2X::deleteSection() {
	// only allow deleting of empty section to reduce accidental deletions
	if ( menu->sectionLinks()->size() > 0) {
		MessageBox mb(this, tr["Error: cannot delete menu section with links in it."]);
		mb.setButton(CONFIRM, tr["OK"]);
		mb.exec();
		return;
	}

	MessageBox mb(this, tr["Really delete this menu section?"] + "\n" + tr[menu->selSection()]);
	mb.setButton(CONFIRM, tr["Yes"]);
	mb.setButton(CANCEL,  tr["No"]);
	if (mb.exec() == CONFIRM) {
		ledOn();
		if (rmtree(path+"sections/"+menu->selSection())) {
			menu->deleteSelectedSection();
			sync();
		}
		ledOff();
	}
}

int32_t GMenu2X::getBatteryStatus() {
	char buf[32] = "-1";
#if defined(TARGET_RS97)
	FILE *f = fopen("/proc/jz/battery", "r");
	if (f) {
		fgets(buf, sizeof(buf), f);
	}
	fclose(f);
#elif defined(TARGET_MIYOO)
	FILE *f = fopen("/sys/devices/platform/soc/1c23400.battery/power_supply/miyoo-battery/voltage_now", "r");
	if (f) {
		fgets(buf, sizeof(buf), f);
	}
	fclose(f);
#endif
	return atol(buf);
}

uint16_t GMenu2X::getBatteryLevel() {
	int32_t val = getBatteryStatus();

if (confStr["batteryType"] == "BL-5B") {
	if ((val > 10000) || (val < 0)) return 6;
	else if (val > 4000) return 5; // 100%
	else if (val > 3900) return 4; // 80%
	else if (val > 3800) return 3; // 60%
	else if (val > 3730) return 2; // 40%
	else if (val > 3600) return 1; // 20%
	return 0; // 0% :(
}

#if defined(TARGET_GP2X)
	//if (batteryHandle<=0) return 6; //AC Power
	if (f200) {
		MMSP2ADC val;
		read(batteryHandle, &val, sizeof(MMSP2ADC));

		if (val.batt==0) return 5;
		if (val.batt==1) return 3;
		if (val.batt==2) return 1;
		if (val.batt==3) return 0;
		return 6;
	} else {
		int battval = 0;
		uint16_t cbv, min=900, max=0;

		for (int i = 0; i < BATTERY_READS; i ++) {
			if ( read(batteryHandle, &cbv, 2) == 2) {
				battval += cbv;
				if (cbv>max) max = cbv;
				if (cbv<min) min = cbv;
			}
		}

		battval -= min+max;
		battval /= BATTERY_READS-2;

		if (battval>=850) return 6;
		if (battval>780) return 5;
		if (battval>740) return 4;
		if (battval>700) return 3;
		if (battval>690) return 2;
		if (battval>680) return 1;
	}

#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
	uint16_t cbv;
	if ( read(batteryHandle, &cbv, 2) == 2) {
		// 0=fail, 1=100%, 2=66%, 3=33%, 4=0%
		switch (cbv) {
			case 4: return 1;
			case 3: return 2;
			case 2: return 4;
			case 1: return 5;
			default: return 6;
		}
	}
#else

	val = constrain(val, 0, 4500);

	bool needWriteConfig = false;
	int32_t max = confInt["maxBattery"];
	int32_t min = confInt["minBattery"];

	if (val > max) {
		needWriteConfig = true;
		max = confInt["maxBattery"] = val;
	}
	if (val < min) {
		needWriteConfig = true;
		min = confInt["minBattery"] = val;
	}

	if (needWriteConfig)
		writeConfig();

	if (max == min)
		return 0;

	// return 5 - 5*(100-val)/(100);
	return 5 - 5 * (max - val) / (max - min);
#endif
}

void GMenu2X::setInputSpeed() {
	input.setInterval(180);
	input.setInterval(1000, SETTINGS);
	input.setInterval(1000, MENU);
	input.setInterval(1000, CONFIRM);
	input.setInterval(1500, POWER);
	// input.setInterval(30,  VOLDOWN);
	// input.setInterval(30,  VOLUP);
	// input.setInterval(300, CANCEL);
	// input.setInterval(300, MANUAL);
	// input.setInterval(100, INC);
	// input.setInterval(100, DEC);
	// input.setInterval(500, SECTION_PREV);
	// input.setInterval(500, SECTION_NEXT);
	// input.setInterval(500, PAGEUP);
	// input.setInterval(500, PAGEDOWN);
	// input.setInterval(200, BACKLIGHT);
}

void GMenu2X::setCPU(uint32_t mhz) {
	// mhz = constrain(mhz, CPU_CLK_MIN, CPU_CLK_MAX);
	if (memdev > 0) {
		DEBUG("Setting clock to %d", mhz);

#if defined(TARGET_GP2X)
		uint32_t v, mdiv, pdiv=3, scale=0;

		#define SYS_CLK_FREQ 7372800
		mhz *= 1000000;
		mdiv = (mhz * pdiv) / SYS_CLK_FREQ;
		mdiv = ((mdiv-8)<<8) & 0xff00;
		pdiv = ((pdiv-2)<<2) & 0xfc;
		scale &= 3;
		v = mdiv | pdiv | scale;
		MEM_REG[0x910>>1] = v;

#elif defined(TARGET_CAANOO) || defined(TARGET_WIZ)
		volatile uint32_t *memregl = static_cast<volatile uint32_t*>((volatile void*)memregs);
		int mdiv, pdiv = 9, sdiv = 0;
		uint32_t v;

		#define SYS_CLK_FREQ 27
		#define PLLSETREG0   (memregl[0xF004>>2])
		#define PWRMODE      (memregl[0xF07C>>2])
		mdiv = (mhz * pdiv) / SYS_CLK_FREQ;
		if (mdiv & ~0x3ff) return;
		v = pdiv<<18 | mdiv<<8 | sdiv;

		PLLSETREG0 = v;
		PWRMODE |= 0x8000;
		for (int i = 0; (PWRMODE & 0x8000) && i < 0x100000; i++);

#elif defined(TARGET_RS97)
		uint32_t m = mhz / 6;
		memregs[0x10 >> 2] = (m << 24) | 0x090520;
		INFO("Set CPU clock: %d", mhz);

#elif defined(TARGET_MIYOO)
    uint32_t x, v;
    uint32_t total=sizeof(oc_table)/sizeof(uint32_t);

    for(x=0; x<total; x++){
      if((oc_table[x] >> 16) >= mhz){
        v = memregs[0];
        v&= 0xffff0000;
        v|= (oc_table[x] &  0x0000ffff);
        memregs[0] = v;
        break;
      }
    }
		INFO("Set CPU clock: %d(0x%08x)", mhz, v);
#endif

#if !defined(TARGET_MIYOO)
		setTVOut(TVOut);
#endif
	}
}

int GMenu2X::getVolume() {
#if defined(TARGET_MIYOO)
  return 0;
#endif

	int vol = -1;
	uint32_t soundDev = open("/dev/mixer", O_RDONLY);

	if (soundDev) {
#if defined(TARGET_RS97)
		ioctl(soundDev, SOUND_MIXER_READ_VOLUME, &vol);
#else
		ioctl(soundDev, SOUND_MIXER_READ_PCM, &vol);
#endif
		close(soundDev);
		if (vol != -1) {
			//just return one channel , not both channels, they're hopefully the same anyways
			return vol & 0xFF;
		}
	}
	return vol;
}

int GMenu2X::setVolume(int val, bool popup) {
	int volumeStep = 10;

	if (val < 0) val = 100;
	else if (val > 100) val = 0;

	if (popup) {
		bool close = false;

		Surface bg(s);

		Surface *iconVolume[3] = {
			sc.skinRes("imgs/mute.png"),
			sc.skinRes("imgs/phones.png"),
			sc.skinRes("imgs/volume.png"),
		};

		powerManager->clearTimer();
		while (!close) {
			input.setWakeUpInterval(3000);
			drawSlider(val, 0, 100, *iconVolume[getVolumeMode(val)], bg);

			close = !input.update();

			if (input[SETTINGS] || input[CONFIRM] || input[CANCEL]) close = true;
			if ( input[LEFT] || input[DEC] )		val = max(0, val - volumeStep);
			else if ( input[RIGHT] || input[INC] )	val = min(100, val + volumeStep);
			else if ( input[SECTION_PREV] )	{
													val += volumeStep;
													if (val > 100) val = 0;
			}
		}
		powerManager->resetSuspendTimer();
		confInt["globalVolume"] = val;
		writeConfig();
	}

	uint32_t soundDev = open("/dev/mixer", O_RDWR);
	if (soundDev) {
		int vol = (val << 8) | val;
#if defined(TARGET_RS97)
		ioctl(soundDev, SOUND_MIXER_WRITE_VOLUME, &vol);
#else
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
#endif
		close(soundDev);

	}
	volumeMode = getVolumeMode(val);
	return val;
}

int GMenu2X::getBacklight() {
	char buf[32] = "-1";
#if defined(TARGET_RS97)
	FILE *f = fopen("/proc/jz/lcd_backlight", "r");
	if (f) {
		fgets(buf, sizeof(buf), f);
	}
	fclose(f);
#elif defined(TARGET_MIYOO)
	FILE *f = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "r");
	if (f) {
		fgets(buf, sizeof(buf), f);
	}
	fclose(f);
#endif
	return atoi(buf);
}

#if defined(TARGET_MIYOO)
#define MIYOO_LID_FILE "/mnt/.backlight.conf"
static int read_conf(const char *file)
{
  int i, fd;
  int val = 5;
  char buf[10]={0};
  
  fd = open(file, O_RDWR);
  if(fd < 0){
    val = 5;
  }
  else{
    read(fd, buf, sizeof(buf));
    for(i=0; i<strlen(buf); i++){
      if(buf[i] == '\r'){
        buf[i] = 0;
      }
      if(buf[i] == '\n'){
        buf[i] = 0;
      }
      if(buf[i] == ' '){
        buf[i] = 0;
      }
    }
    val = atoi(buf);
  }
  close(fd);
  return val;
}
#endif

int GMenu2X::setBacklight(int val, bool popup) {
	int backlightStep = 10;

	if (val < 0) val = 100;
	else if (val > 100) val = backlightStep;

	if (popup) {
		bool close = false;

		Surface bg(s);

		Surface *iconBrightness[6] = {
			sc.skinRes("imgs/brightness/0.png"),
			sc.skinRes("imgs/brightness/1.png"),
			sc.skinRes("imgs/brightness/2.png"),
			sc.skinRes("imgs/brightness/3.png"),
			sc.skinRes("imgs/brightness/4.png"),
			sc.skinRes("imgs/brightness.png")
		};

		powerManager->clearTimer();
		while (!close) {
			input.setWakeUpInterval(3000);
			int brightnessIcon = val/20;

			if (brightnessIcon > 4 || iconBrightness[brightnessIcon] == NULL) brightnessIcon = 5;

			drawSlider(val, 0, 100, *iconBrightness[brightnessIcon], bg);

			close = !input.update();

			if ( input[SETTINGS] || input[MENU] || input[CONFIRM] || input[CANCEL] ) close = true;
			if ( input[LEFT] || input[DEC] )			val = setBacklight(max(1, val - backlightStep), false);
			else if ( input[RIGHT] || input[INC] )		val = setBacklight(min(100, val + backlightStep), false);
			else if ( input[BACKLIGHT] )				val = setBacklight(val + backlightStep, false);
		}
		powerManager->resetSuspendTimer();
		// input.setWakeUpInterval(0);
		confInt["backlight"] = val;
		writeConfig();
	}

#if defined(TARGET_RS97)
	char buf[34] = {0};
	sprintf(buf, "echo %d > /proc/jz/lcd_backlight", val);
	system(buf);
#endif

#if defined(TARGET_MIYOO)
	char buf[34] = {0};
  int lid = read_conf(MIYOO_LID_FILE);
  if(val == 0){
    lid = 0;
  }
	sprintf(buf, "echo %d > /sys/devices/platform/backlight/backlight/backlight/brightness", lid);
	system(buf);
#endif
	return val;
}

const string &GMenu2X::getExePath() {
	if (path.empty()) {
		char buf[255];
		memset(buf, 0, 255);
		int l = readlink("/proc/self/exe", buf, 255);

		path = buf;
		path = path.substr(0, l);
		l = path.rfind("/");
		path = path.substr(0, l + 1);
	}
	return path;
}

string GMenu2X::getDiskFree(const char *path) {
	string df = "N/A";
	struct statvfs b;

	if (statvfs(path, &b) == 0) {
		// Make sure that the multiplication happens in 64 bits.
		uint32_t freeMiB = ((uint64_t)b.f_bfree * b.f_bsize) / (1024 * 1024);
		uint32_t totalMiB = ((uint64_t)b.f_blocks * b.f_frsize) / (1024 * 1024);
		stringstream ss;
		if (totalMiB >= 10000) {
			ss << (freeMiB / 1024) << "." << ((freeMiB % 1024) * 10) / 1024 << "/"
			   << (totalMiB / 1024) << "." << ((totalMiB % 1024) * 10) / 1024 << "GiB";
		} else {
			ss << freeMiB << "/" << totalMiB << "MiB";
		}
		ss >> df;
	} else WARNING("statvfs failed with error '%s'.\n", strerror(errno));
	return df;
}

int GMenu2X::drawButton(Button *btn, int x, int y) {
	if (y < 0) y = resY + y;
	// y = resY - 8 - skinConfInt["bottomBarHeight"] / 2;
	btn->setPosition(x, y - 7);
	btn->paint();
	return x + btn->getRect().w + 6;
}

int GMenu2X::drawButton(Surface *s, const string &btn, const string &text, int x, int y) {
	if (y < 0) y = resY + y;
	// y = resY - skinConfInt["bottomBarHeight"] / 2;
	SDL_Rect re = {x, y, 0, 16};

	if (sc.skinRes("imgs/buttons/"+btn+".png") != NULL) {
		sc["imgs/buttons/"+btn+".png"]->blit(s, re.x + 8, re.y + 2, HAlignCenter | VAlignMiddle);
		re.w = sc["imgs/buttons/"+btn+".png"]->raw->w + 3;

		s->write(font, text, re.x + re.w, re.y, VAlignMiddle, skinConfColors[COLOR_FONT_ALT], skinConfColors[COLOR_FONT_ALT_OUTLINE]);
		re.w += font->getTextWidth(text);
	}
	return x + re.w + 6;
}

int GMenu2X::drawButtonRight(Surface *s, const string &btn, const string &text, int x, int y) {
	if (y < 0) y = resY + y;
	// y = resY - skinConfInt["bottomBarHeight"] / 2;
	if (sc.skinRes("imgs/buttons/" + btn + ".png") != NULL) {
		x -= 16;
		sc["imgs/buttons/" + btn + ".png"]->blit(s, x + 8, y + 2, HAlignCenter | VAlignMiddle);
		x -= 3;
		s->write(font, text, x, y, HAlignRight | VAlignMiddle, skinConfColors[COLOR_FONT_ALT], skinConfColors[COLOR_FONT_ALT_OUTLINE]);
		return x - 6 - font->getTextWidth(text);
	}
	return x - 6;
}

void GMenu2X::drawScrollBar(uint32_t pagesize, uint32_t totalsize, uint32_t pagepos, SDL_Rect scrollRect) {
	if (totalsize <= pagesize) return;
	
	return;
	//internal bar total height = height-2
	//bar size
	uint32_t bs = (scrollRect.h - 3) * pagesize / totalsize;
	//bar y position
	uint32_t by = (scrollRect.h - 3) * pagepos / totalsize;
	by = scrollRect.y + 3 + by;
	if ( by + bs > scrollRect.y + scrollRect.h - 4) by = scrollRect.y + scrollRect.h - 4 - bs;

	s->rectangle(scrollRect.x + scrollRect.w - 4, by, 4, bs, skinConfColors[COLOR_LIST_BG]);
	s->box(scrollRect.x + scrollRect.w - 3, by + 1, 2, bs - 2, skinConfColors[COLOR_SELECTION_BG]);
}

void GMenu2X::drawSlider(int val, int min, int max, Surface &icon, Surface &bg) {
	SDL_Rect progress = {52, 32, resX-84, 8};
	SDL_Rect box = {20, 20, resX-40, 32};

	val = constrain(val, min, max);

	bg.blit(s,0,0);
	s->box(box, skinConfColors[COLOR_MESSAGE_BOX_BG]);
	s->rectangle(box.x+2, box.y+2, box.w-4, box.h-4, skinConfColors[COLOR_MESSAGE_BOX_BORDER]);

	icon.blit(s, 28, 28);

	s->box(progress, skinConfColors[COLOR_MESSAGE_BOX_BG]);
	s->box(progress.x + 1, progress.y + 1, val * (progress.w - 3) / max + 1, progress.h - 2, skinConfColors[COLOR_MESSAGE_BOX_SELECTION]);
	s->flip();
}

#if defined(TARGET_GP2X)
void GMenu2X::gp2x_tvout_on(bool pal) {
	if (memdev != 0) {
		/*Ioctl_Dummy_t *msg;
		#define FBMMSP2CTRL 0x4619
		int TVHandle = ioctl(SDL_videofd, FBMMSP2CTRL, msg);*/
		if (cx25874!=0) gp2x_tvout_off();
		//if tv-out is enabled without cx25874 open, stop
		//if (memregs[0x2800 >> 1]&0x100) return;
		cx25874 = open("/dev/cx25874",O_RDWR);
		ioctl(cx25874, _IOW('v', 0x02, uint8_t), pal ? 4 : 3);
		memregs[0x2906 >> 1] = 512;
		memregs[0x28E4 >> 1] = memregs[0x290C >> 1];
		memregs[0x28E8 >> 1] = 239;
	}
}

void GMenu2X::gp2x_tvout_off() {
	if (memdev != 0) {
		close(cx25874);
		cx25874 = 0;
		memregs[0x2906 >> 1] = 1024;
	}
}

void GMenu2X::settingsOpen2x() {
	SettingsDialog sd(this, ts, tr["Open2x Settings"]);
	sd.addSetting(new MenuSettingBool(this, tr["USB net on boot"], tr["Allow USB networking to be started at boot time"],&o2x_usb_net_on_boot));
	sd.addSetting(new MenuSettingString(this, tr["USB net IP"], tr["IP address to be used for USB networking"],&o2x_usb_net_ip));
	sd.addSetting(new MenuSettingBool(this, tr["Telnet on boot"], tr["Allow telnet to be started at boot time"],&o2x_telnet_on_boot));
	sd.addSetting(new MenuSettingBool(this, tr["FTP on boot"], tr["Allow FTP to be started at boot time"],&o2x_ftp_on_boot));
	sd.addSetting(new MenuSettingBool(this, tr["GP2XJOY on boot"], tr["Create a js0 device for GP2X controls"],&o2x_gp2xjoy_on_boot));
	sd.addSetting(new MenuSettingBool(this, tr["USB host on boot"], tr["Allow USB host to be started at boot time"],&o2x_usb_host_on_boot));
	sd.addSetting(new MenuSettingBool(this, tr["USB HID on boot"], tr["Allow USB HID to be started at boot time"],&o2x_usb_hid_on_boot));
	sd.addSetting(new MenuSettingBool(this, tr["USB storage on boot"], tr["Allow USB storage to be started at boot time"],&o2x_usb_storage_on_boot));
//sd.addSetting(new MenuSettingInt(this, tr["Speaker Mode on boot"], tr["Set Speaker mode. 0 = Mute, 1 = Phones, 2 = Speaker"],&volumeMode,0,2,1));
	sd.addSetting(new MenuSettingInt(this, tr["Speaker Scaler"], tr["Set the Speaker Mode scaling 0-150\% (default is 100\%)"],&volumeScalerNormal,100, 0,150));
	sd.addSetting(new MenuSettingInt(this, tr["Headphones Scaler"], tr["Set the Headphones Mode scaling 0-100\% (default is 65\%)"],&volumeScalerPhones,65, 0,100));

	if (sd.exec() && sd.edited()) {
		writeConfigOpen2x();
		switch(volumeMode) {
			case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
			case VOLUME_MODE_PHONES: setVolumeScaler(volumeScalerPhones); break;
			case VOLUME_MODE_NORMAL: setVolumeScaler(volumeScalerNormal); break;
		}
		setVolume(confInt["globalVolume"]);
	}
}

void GMenu2X::readConfigOpen2x() {
	string conffile = "/etc/config/open2x.conf";
	if (!fileExists(conffile)) return;
	ifstream inf(conffile.c_str(), ios_base::in);
	if (!inf.is_open()) return;
	string line;
	while (getline(inf, line, '\n')) {
		string::size_type pos = line.find("=");
		string name = trim(line.substr(0,pos));
		string value = trim(line.substr(pos+1,line.length()));

		if (name=="USB_NET_ON_BOOT") o2x_usb_net_on_boot = value == "y" ? true : false;
		else if (name=="USB_NET_IP") o2x_usb_net_ip = value;
		else if (name=="TELNET_ON_BOOT") o2x_telnet_on_boot = value == "y" ? true : false;
		else if (name=="FTP_ON_BOOT") o2x_ftp_on_boot = value == "y" ? true : false;
		else if (name=="GP2XJOY_ON_BOOT") o2x_gp2xjoy_on_boot = value == "y" ? true : false;
		else if (name=="USB_HOST_ON_BOOT") o2x_usb_host_on_boot = value == "y" ? true : false;
		else if (name=="USB_HID_ON_BOOT") o2x_usb_hid_on_boot = value == "y" ? true : false;
		else if (name=="USB_STORAGE_ON_BOOT") o2x_usb_storage_on_boot = value == "y" ? true : false;
		else if (name=="VOLUME_MODE") volumeMode = savedVolumeMode = constrain( atoi(value.c_str()), 0, 2);
		else if (name=="PHONES_VALUE") volumeScalerPhones = constrain( atoi(value.c_str()), 0, 100);
		else if (name=="NORMAL_VALUE") volumeScalerNormal = constrain( atoi(value.c_str()), 0, 150);
	}
	inf.close();
}

void GMenu2X::writeConfigOpen2x() {
	ledOn();
	string conffile = "/etc/config/open2x.conf";
	ofstream inf(conffile.c_str());
	if (inf.is_open()) {
		inf << "USB_NET_ON_BOOT=" << ( o2x_usb_net_on_boot ? "y" : "n" ) << endl;
		inf << "USB_NET_IP=" << o2x_usb_net_ip << endl;
		inf << "TELNET_ON_BOOT=" << ( o2x_telnet_on_boot ? "y" : "n" ) << endl;
		inf << "FTP_ON_BOOT=" << ( o2x_ftp_on_boot ? "y" : "n" ) << endl;
		inf << "GP2XJOY_ON_BOOT=" << ( o2x_gp2xjoy_on_boot ? "y" : "n" ) << endl;
		inf << "USB_HOST_ON_BOOT=" << ( (o2x_usb_host_on_boot || o2x_usb_hid_on_boot || o2x_usb_storage_on_boot) ? "y" : "n" ) << endl;
		inf << "USB_HID_ON_BOOT=" << ( o2x_usb_hid_on_boot ? "y" : "n" ) << endl;
		inf << "USB_STORAGE_ON_BOOT=" << ( o2x_usb_storage_on_boot ? "y" : "n" ) << endl;
		inf << "VOLUME_MODE=" << volumeMode << endl;
		if (volumeScalerPhones != VOLUME_SCALER_PHONES) inf << "PHONES_VALUE=" << volumeScalerPhones << endl;
		if (volumeScalerNormal != VOLUME_SCALER_NORMAL) inf << "NORMAL_VALUE=" << volumeScalerNormal << endl;
		inf.close();
		sync();
	}
	ledOff();
}

void GMenu2X::activateSdUsb() {
	if (usbnet) {
		MessageBox mb(this, tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
		mb.exec();
	} else {
		MessageBox mb(this, tr["USB Enabled (SD)"],"skin:icons/usb.png");
		mb.setButton(CONFIRM, tr["Turn off"]);
		mb.exec();
		system("scripts/usbon.sh nand");
	}
}

void GMenu2X::activateNandUsb() {
	if (usbnet) {
		MessageBox mb(this, tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
		mb.exec();
	} else {
		system("scripts/usbon.sh nand");
		MessageBox mb(this, tr["USB Enabled (Nand)"],"skin:icons/usb.png");
		mb.setButton(CONFIRM, tr["Turn off"]);
		mb.exec();
		system("scripts/usboff.sh nand");
	}
}

void GMenu2X::activateRootUsb() {
	if (usbnet) {
		MessageBox mb(this,tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
		mb.exec();
	} else {
		system("scripts/usbon.sh root");
		MessageBox mb(this,tr["USB Enabled (Root)"],"skin:icons/usb.png");
		mb.setButton(CONFIRM, tr["Turn off"]);
		mb.exec();
		system("scripts/usboff.sh root");
	}
}

void GMenu2X::applyRamTimings() {
	// 6 4 1 1 1 2 2
	if (memdev!=0) {
		int tRC = 5, tRAS = 3, tWR = 0, tMRD = 0, tRFC = 0, tRP = 1, tRCD = 1;
		memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
		memregs[0x3804>>1] = ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
	}
}

void GMenu2X::applyDefaultTimings() {
	// 8 16 3 8 8 8 8
	if (memdev!=0) {
		int tRC = 7, tRAS = 15, tWR = 2, tMRD = 7, tRFC = 7, tRP = 7, tRCD = 7;
		memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
		memregs[0x3804>>1] = ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
	}
}

void GMenu2X::setGamma(int gamma) {
	float fgamma = (float)constrain(gamma,1,100)/10;
	fgamma = 1 / fgamma;
	MEM_REG[0x2880>>1] &= ~(1<<12);
	MEM_REG[0x295C>>1] = 0;

	for (int i = 0; i < 256; i++) {
		uint8_t g = (uint8_t)(255.0*pow(i/255.0,fgamma));
		uint16_t s = (g << 8) | g;
		MEM_REG[0x295E >> 1] = s;
		MEM_REG[0x295E >> 1] = g;
	}
}

void GMenu2X::setVolumeScaler(int scale) {
	scale = constrain(scale,0,MAX_VOLUME_SCALE_FACTOR);
	uint32_t soundDev = open("/dev/mixer", O_WRONLY);
	if (soundDev) {
		ioctl(soundDev, SOUND_MIXER_PRIVATE2, &scale);
		close(soundDev);
	}
}

int GMenu2X::getVolumeScaler() {
	int currentscalefactor = -1;
	uint32_t soundDev = open("/dev/mixer", O_RDONLY);
	if (soundDev) {
		ioctl(soundDev, SOUND_MIXER_PRIVATE1, &currentscalefactor);
		close(soundDev);
	}
	return currentscalefactor;
}

void GMenu2X::readCommonIni() {
	if (!fileExists("/usr/gp2x/common.ini")) return;
	ifstream inf("/usr/gp2x/common.ini", ios_base::in);
	if (!inf.is_open()) return;
	string line;
	string section = "";
	while (getline(inf, line, '\n')) {
		line = trim(line);
		if (line[0]=='[' && line[line.length()-1]==']') {
			section = line.substr(1,line.length()-2);
		} else {
			string::size_type pos = line.find("=");
			string name = trim(line.substr(0,pos));
			string value = trim(line.substr(pos+1,line.length()));

			if (section=="usbnet") {
				if (name=="enable")
					usbnet = value=="true" ? true : false;
				else if (name=="ip")
					ip = value;

			} else if (section=="server") {
				if (name=="inet")
					inet = value=="true" ? true : false;
				else if (name=="samba")
					samba = value=="true" ? true : false;
				else if (name=="web")
					web = value=="true" ? true : false;
			}
		}
	}
	inf.close();
}

void GMenu2X::initServices() {
	if (usbnet) {
		string services = "scripts/services.sh "+ip+" "+(inet?"on":"off")+" "+(samba?"on":"off")+" "+(web?"on":"off")+" &";
		system(services.c_str());
	}
}
#endif
