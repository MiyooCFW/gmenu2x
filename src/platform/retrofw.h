#ifndef HW_RETROFW_H
#define HW_RETROFW_H

#include <sys/mman.h>
#include <linux/soundcard.h>

/*	RetroGame Key Codes. pingflood, 2018
	BUTTON     GMENU          SDL             NUMERIC   GPIO
	-----------------------------------------------------------------------------
	X          MODIFIER       SDLK_SPACE      32        !(mem[PEPIN] >> 07 & 1)
	A          CONFIRM        SDLK_LCTRL      306       !(mem[PDPIN] >> 22 & 1)
	B          CANCEL         SDLK_LALT       308       !(mem[PDPIN] >> 23 & 1)
	Y          MANUAL         SDLK_LSHIFT     304       !(mem[PEPIN] >> 11 & 1)
	L          SECTION_PREV   SDLK_TAB        9         !(mem[PBPIN] >> 23 & 1)
	R          SECTION_NEXT   SDLK_BACKSPACE  8         !(mem[PDPIN] >> 24 & 1)
	START      SETTINGS       SDLK_RETURN     13         (mem[PDPIN] >> 18 & 1)
	SELECT     MENU           SDLK_ESCAPE     27         (mem[PDPIN] >> 17 & 1)
	BACKLIGHT  BACKLIGHT      SDLK_3          51        !(mem[PDPIN] >> 21 & 1)
	POWER      POWER          SDLK_END        279       !(mem[PAPIN] >> 30 & 1)
	UP         UP             SDLK_UP         273       !(mem[PBPIN] >> 25 & 1)
	DOWN       DOWN           SDLK_DOWN       274       !(mem[PBPIN] >> 24 & 1)
	RIGHT      RIGHT          SDLK_RIGHT      275       !(mem[PBPIN] >> 26 & 1)
	LEFT       LEFT           SDLK_LEFT       276       !(mem[PDPIN] >> 00 & 1)
*/

volatile uint32_t *mem;
volatile uint8_t memdev = 0;
int32_t tickBattery = 0;

#define BASE    0x10000000
#define CPPCR   (0x10 >> 2)
#define PAPIN	(0x10000 >> 2)
#define PBPIN	(0x10100 >> 2)
#define PCPIN	(0x10200 >> 2)
#define PDPIN	(0x10300 >> 2)
#define PEPIN	(0x10400 >> 2)
#define PFPIN	(0x10500 >> 2)

uint16_t getMMCStatus() {
	if (memdev > 0 && !(mem[PFPIN] >> 0 & 1)) return MMC_INSERT;
	return MMC_REMOVE;
}

uint16_t getUDCStatus() {
	// if (memdev > 0 && ((mem[PDPIN] >> 7 & 1) || (mem[PEPIN] >> 13 & 1))) return UDC_CONNECT;
	if (memdev > 0 && (mem[PDPIN] >> 7 & 1)) return UDC_CONNECT;
	return UDC_REMOVE;
}

uint16_t getTVOutStatus() {
	if (memdev > 0) {
		if (fwType == "RETROARCADE" && !(mem[PDPIN] >> 6 & 1)) return TV_CONNECT;
		if (!(mem[PDPIN] >> 25 & 1)) return TV_CONNECT;
	}
	return TV_REMOVE;
}

uint16_t getDevStatus() {
	char buf[10000];
	if (FILE *f = fopen("/proc/bus/input/devices", "r")) {
		size_t val = fread(buf, sizeof(char), 10000, f);
		fclose(f);
		return val;
	}
	return 0;
}

int32_t getBatteryLevel() {
	int val = -1;
	if (FILE *f = fopen("/proc/jz/battery", "r")) {
		fscanf(f, "%i", &val);
		fclose(f);
	}
	return val;
}

uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
	if ((val > 10000) || (val < 0)) return 6;
	if (val > 4000) return 5; // 100%
	if (val > 3900) return 4; // 80%
	if (val > 3800) return 3; // 60%
	if (val > 3700) return 2; // 40%
	if (val > 3520) return 1; // 20%
	return 0; // 0% :(
}

uint16_t getTVOut() {
	int val = 0;
	if (FILE *f = fopen("/proc/jz/tvout", "r")) {
		fscanf(f, "%i", &val);
		fclose(f);
	}
	return val;
}

uint16_t getVolumeMode(uint8_t vol) {
	if (!vol) return VOLUME_MODE_MUTE;
	if (memdev > 0 && !(mem[PDPIN] >> 6 & 1)) return VOLUME_MODE_PHONES;
	return VOLUME_MODE_NORMAL;
}

void printbin(const char *id, int n) {
	printf("%s: 0x%08x ", id, n);
	for(int i = 31; i >= 0; i--) {
		printf("%d", !!(n & 1 << i));
		if (!(i % 8)) printf(" ");
	}
	printf("\e[0K\n");
}

uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	tickBattery++;
	if (tickBattery > 30) { // update battery level every 30 hwChecks
		tickBattery = 0;
		batteryIcon = getBatteryStatus(getBatteryLevel(), 0, 0);
	}

	if (memdev > 0 && tickBattery > 2) {
		udcStatus = getUDCStatus();
		if (udcPrev != udcStatus) {
			udcPrev = udcStatus;
			InputManager::pushEvent(udcStatus);
			return 2000;
		}

		mmcStatus = getMMCStatus();
		if (mmcPrev != mmcStatus) {
			mmcPrev = mmcStatus;
			InputManager::pushEvent(mmcStatus);
			if (mmcStatus == MMC_REMOVE) {
				system("umount -fl /mnt &> /dev/null");
			}
			return 2000;
		}

		volumeMode = getVolumeMode(1);
		if (volumeModePrev != volumeMode) {
			volumeModePrev = volumeMode;
			InputManager::pushEvent(PHONES_CONNECT);
			return 2000;
		}

		if (fwType == "RETROARCADE") {
			numJoy = getDevStatus();
			if (numJoyPrev != numJoy) {
				numJoyPrev = numJoy;
				InputManager::pushEvent(JOYSTICK_CONNECT);
				return 3000;
			}
		}

		tvOutStatus = getTVOutStatus();
		if (tvOutPrev != tvOutStatus) {
			tvOutPrev = tvOutStatus;
			InputManager::pushEvent(tvOutStatus);
			return 2000;
		}

		// volumeMode = getVolumeMode(confInt["globalVolume"]);
		// if (volumeModePrev != volumeMode && volumeMode == VOLUME_MODE_PHONES) {
		// 	setVolume(min(70, confInt["globalVolume"]), true);
		// }
		// volumeModePrev = volumeMode;
	}
	return interval;
}

class GMenuNX : public GMenu2X {
private:
	void hwDeinit() {
		if (memdev > 0) {
			close(memdev);
		}
	}

	void hwInit() {
		CPU_MENU = 528;
		CPU_LINK = 600;
		CPU_MAX = CPU_MENU * 2;
		CPU_MIN = CPU_MENU / 2;
		CPU_STEP = 6;

		batteryIcon = getBatteryStatus(getBatteryLevel(), 0, 0);

		system("[ -d /home/retrofw ] && mount -o remount,async /home/retrofw");
#if defined(OPK_SUPPORT)
		system("umount -fl /mnt &> /dev/null");
#endif

		memdev = open("/dev/mem", O_RDWR);
		if (memdev > 0) {
			mem = (uint32_t*)mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, BASE);
			if (mem == MAP_FAILED) {
				ERROR("Could not mmap hardware registers!");
				close(memdev);
			}
		} else {
			WARNING("Could not open /dev/mem");
		}

		struct fb_var_screeninfo vinfo;
		int fbdev = open("/dev/fb0", O_RDWR);
		if (fbdev >= 0 && ioctl(fbdev, FBIOGET_VSCREENINFO, &vinfo) >= 0) {
			w = vinfo.width;
			h = vinfo.height;
			close(fbdev);
		}

		if (w == 320 && h == 480) h = 240;

		if (FILE *f = fopen("/proc/jz/gpio", "r")) {
			char buf[7];
			fread(buf, sizeof(char), 7, f);
			fclose(f);
			if (!strncmp(buf, "480x272", 7)) {
				fwType = "RETROARCADE";
			}
		}

		INFO("%s", fwType.c_str());
	}

	void udcDialog(int udcStatus) {
		if (udcStatus == UDC_REMOVE) {
			INFO("USB Disconnected. Disabling devices...");
			system("/usr/bin/retrofw stop");
			return;
		}

		int option;
		if (confStr["usbMode"] == "Storage") option = CONFIRM;
		else if (confStr["usbMode"] == "Charger") option = CANCEL;
		else {
			MessageBox mb(this, tr["USB mode"], "skin:icons/usb.png");
			mb.setButton(CANCEL,  tr["Charger"]);
			mb.setButton(CONFIRM, tr["Storage"]);
			option = mb.exec();
		}

		if (option == CONFIRM) { // storage
			INFO("Enabling storage device");
			quit();
			execlp("/bin/sh", "/bin/sh", "-c", "exec /usr/bin/retrofw storage on", NULL);
			return;
		}

		INFO("Enabling networking device");
		system("/usr/bin/retrofw network on");
		iconInet = sc["skin:imgs/inet.png"];
	}

	void tvOutDialog(int16_t mode) {
		if (!file_exists("/proc/jz/tvout")) return;

		if (mode < 0) {
			int option;

			mode = TV_OFF;

			if (confStr["tvMode"] == "NTSC") option = CONFIRM;
			else if (confStr["tvMode"] == "PAL") option = MANUAL;
			else if (confStr["tvMode"] == "OFF") option = CANCEL;
			else {
				MessageBox mb(this, tr["TV-out connected. Enable?"], "skin:icons/tv.png");
				mb.setButton(CONFIRM, tr["NTSC"]);
				mb.setButton(MANUAL,  tr["PAL"]);
				mb.setButton(CANCEL,  tr["OFF"]);
				option = mb.exec();
			}

			switch (option) {
				case CONFIRM:
					mode = TV_NTSC;
					break;
				case MANUAL:
					mode = TV_PAL;
					break;
			}
		}

		if (mode != getTVOut()) {
			setTVOut(mode);
			setBacklight(confInt["backlight"]);
			writeTmp();
			exit(0);
		}
	}

	int getBacklight() {
		int val = -1;
		if (FILE *f = fopen("/proc/jz/backlight", "r")) {
			fscanf(f, "%i", &val);
			fclose(f);
		}
		return val;
	}

public:
	int getVolume() {
		int vol = -1;
		uint32_t soundDev = open("/dev/mixer", O_RDONLY);

		if (soundDev) {
			ioctl(soundDev, SOUND_MIXER_READ_VOLUME, &vol);
			close(soundDev);
			if (vol != -1) {
				// just return one channel , not both channels, they're hopefully the same anyways
				return vol & 0xFF;
			}
		}
		return vol;
	}

	int setVolume(int val, bool popup = false) {
		val = GMenu2X::setVolume(val, popup);

		uint32_t soundDev = open("/dev/mixer", O_RDWR);
		if (soundDev) {
			int vol = (val << 8) | val;
			ioctl(soundDev, SOUND_MIXER_WRITE_VOLUME, &vol);
			close(soundDev);

		}
		volumeMode = getVolumeMode(val);

		return val;
	}

	int setBacklight(int val, bool popup = false) {
		if (val < 1 && getUDCStatus() != UDC_REMOVE /* && !getTVOut() */) {
			val = 0; // suspend only if not charging and TV out is not enabled
		} else if (popup) {
			val = GMenu2X::setBacklight(val, popup);
		}

		if (FILE *f = fopen("/proc/jz/backlight", "w")) {
			if (val == 0) {
				fprintf(f, "-"); // disable backlight button
			}
			fprintf(f, "%d", val); // fputs(val, f);
			fclose(f);
		}

		return val;
	}

	void setScaleMode(unsigned int mode) {
		/* Scaling Modes:
			0: stretch
			1: aspect
			2: original (fallback to aspect when downscale is needed)
			3: 4:3
		*/
		if (FILE *f = fopen("/proc/jz/ipu", "w")) {
			fprintf(f, "%d", mode); // fputs(val, f);
			fclose(f);
		}
	}

	void setTVOut(unsigned int mode) {
		if (FILE *f = fopen("/proc/jz/tvout", "w")) {
			fprintf(f, "%d", mode); // fputs(val, f);
			fclose(f);
		}
	}

	void setCPU(uint32_t mhz) {
		if (getTVOut()) {
			WARNING("Can't overclock when TV out is enabled.");
			return;
		}
		mhz = constrain(mhz, confInt["cpuMenu"], confInt["cpuMax"]);
		if (memdev > 0) {
			DEBUG("Setting clock to %d", mhz);
			uint32_t m = mhz / 6;
			mem[CPPCR] = (m << 24) | 0x090520;
			INFO("CPU clock: %d MHz", mhz);
		}
	}

	string hwPreLinkLaunch() {
		system("[ -d /home/retrofw ] && mount -o remount,sync /home/retrofw");
		return "";
	}
};

#endif
