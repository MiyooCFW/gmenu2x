#ifndef HW_BITTBOY_H
#define HW_BITTBOY_H

#include <sys/mman.h>

/*	BittBoy Key Codes. pingflood, 2019
	BUTTON     GMENU          SDL             NUMERIC   GPIO
	-----------------------------------------------------------------------------
	A          CONFIRM        SDLK_LCTRL      306
	B          CANCEL         SDLK_SPACE      32
	TA         MANUAL         SDLK_LALT       308
	TB         MODIFIER       SDLK_LSHIFT     304
	R          SECTION_NEXT   SDLK_RCTRL      305
	START      SETTINGS       SDLK_RETURN     13
	SELECT     MENU           SDLK_ESCAPE     27
	UP         UP             SDLK_UP         273
	DOWN       DOWN           SDLK_DOWN       274
	RIGHT      RIGHT          SDLK_RIGHT      275
	LEFT       LEFT           SDLK_LEFT       276

	Pocket-Go Key Codes. pingflood, 2019
	BUTTON     GMENU          SDL             NUMERIC   GPIO
	-----------------------------------------------------------------------------
	A          CONFIRM        SDLK_LALT       308
	B          CANCEL         SDLK_LCTRL      306
	X          MODIFIER       SDLK_LSHIFT     304
	Y          MANUAL         SDLK_SPACE      32
	L          SECTION_PREV   SDLK_TAB        9
	R          SECTION_NEXT   BACKSPACE       8
	RESET      POWER          SDLK_RCTRL      305
	START      SETTINGS       SDLK_RETURN     13
	SELECT     MENU           SDLK_ESCAPE     27
	UP         UP             SDLK_UP         273
	DOWN       DOWN           SDLK_DOWN       274
	RIGHT      RIGHT          SDLK_RIGHT      275
	LEFT       LEFT           SDLK_LEFT       276
*/

#define MIYOO_SND_SET_VOLUME  _IOWR(0x100, 0, unsigned long)
#define MIYOO_KBD_GET_HOTKEY  _IOWR(0x100, 0, unsigned long)
#define MIYOO_KBD_SET_VER     _IOWR(0x101, 0, unsigned long)
#define MIYOO_FB0_GET_VER     _IOWR(0x102, 0, unsigned long)
#define MIYOO_FB0_PUT_OSD     _IOWR(0x100, 0, unsigned long)
int fb0, kbd;

static uint32_t oc_table[] = {
  ((96 * 2) << 16) | ((1 << 8) | (3 << 4)), 
  ((96 * 3) << 16) | ((2 << 8) | (3 << 4)), 
  ((96 * 4) << 16) | ((3 << 8) | (3 << 4)), 
  ((96 * 5) << 16) | ((4 << 8) | (3 << 4)), 
  ((96 * 6) << 16) | ((5 << 8) | (3 << 4)), 
  ((96 * 7) << 16) | ((6 << 8) | (3 << 4)), 
  ((96 * 8) << 16) | ((7 << 8) | (3 << 4)), 
  ((96 * 9) << 16) | ((8 << 8) | (3 << 4)), 
  ((96 * 10) << 16) | ((9 << 8) | (3 << 4)), 
  ((96 * 11) << 16) | ((10 << 8) | (3 << 4)), 
  ((96 * 12) << 16) | ((11 << 8) | (3 << 4)), 
  ((96 * 13) << 16) | ((12 << 8) | (3 << 4)), 
  ((96 * 14) << 16) | ((13 << 8) | (3 << 4)), 
  ((96 * 15) << 16) | ((14 << 8) | (3 << 4)), 
};

// #define MIYOO_LID_FILE "/mnt/.backlight.conf"
// static int read_conf(const char *file)
// {
// 	int val = 5;
// 	char buf[10] = {0};
// 	int fd = open(file, O_RDWR);
// 	if (fd < 0) val = -1;
// 	else {
// 		read(fd, buf, sizeof(buf));
// 		for (int i = 0; i < strlen(buf); i++) {
// 			if(buf[i] == '\r' || buf[i] == '\n' || buf[i] == ' ') {
// 				buf[i] = 0;
// 			}
// 		}
// 		val = atoi(buf);
// 	}
// 	close(fd);
// 	return val;
// }

// static void info_fb0(int fb0, int lid, int vol, int show_osd)
// {
//   unsigned long val;

//   val = (show_osd ? 0x80000000 : 0x00000000) | (vol << 16) | (lid);
//   ioctl(fb0, MIYOO_FB0_PUT_OSD, val);
// }

// int SOUND_MIXER_READ = SOUND_MIXER_READ_PCM;
// int SOUND_MIXER_WRITE = SOUND_MIXER_WRITE_PCM;

volatile uint32_t *mem;
volatile uint8_t memdev = 0;
int32_t tickBattery = 0;

int32_t getBatteryLevel() {
	int val = -1;
	if (FILE *f = fopen("/sys/devices/platform/soc/1c23400.battery/power_supply/miyoo-battery/voltage_now", "r")) {
		fscanf(f, "%i", &val);
		fclose(f);
	}
	return val;
}

uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
	if ((val > 4300) || (val < 0)) return 6; // >100% - max voltage 4320
	if (val > 4100) return 5; // 100% - fully charged 4150
	if (val > 3900) return 4; // 80%
	if (val > 3800) return 3; // 60%
	if (val > 3700) return 2; // 40%
	if (val > 3520) return 1; // 20%
	return 0; // 0% :(
}

uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	tickBattery++;
	if (tickBattery > 30) { // update battery level every 30 hwChecks
		tickBattery = 0;
		batteryIcon = getBatteryStatus(getBatteryLevel(), 0, 0);
	}
	return interval;
}

uint8_t getMMCStatus() {
	return MMC_REMOVE;
}

uint8_t getUDCStatus() {
	return UDC_REMOVE;
}

uint8_t getTVOutStatus() {
	return TV_REMOVE;
}

uint8_t getDevStatus() {
	return 0;
}

uint8_t getVolumeMode(uint8_t vol) {
	return VOLUME_MODE_NORMAL;
}

class GMenuNX : public GMenu2X {
private:
	void hwDeinit() {
		if (memdev > 0) {
			close(memdev);
		}
	}

	void hwInit() {
		CPU_MENU = 672;
		CPU_LINK = 672;
		CPU_MAX = 864;
		CPU_MIN = 192;
		CPU_STEP = 96;

		batteryIcon = getBatteryStatus(getBatteryLevel(), 0, 0);
		// setenv("HOME", "/mnt", 1);
		system("mount -o remount,async /mnt");

		memdev = open("/dev/mem", O_RDWR);
		if (memdev > 0) {
			mem = (uint32_t*)mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, 0x01c20000);
			if (mem == MAP_FAILED) {
				ERROR("Could not mmap hardware registers!");
				close(memdev);
			}
		} else {
			WARNING("Could not open /dev/mem");
		}
		w = 320;
		h = 240;
		INFO("BITTBOY");
	}

	int getBacklight() {
		int val = -1;
		FILE *f = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "r");
		if (f) {
			fscanf(f, "%i", &val);
			fclose(f);
		}
		return val;
	}

public:
	int setVolume(int val, bool popup = false) {
		val = GMenu2X::setVolume(val, popup);

		uint32_t snd = open("/dev/miyoo_snd", O_RDWR);

		if (snd) {
			int vol = val / 10;
			if (vol > 9) vol = 9;
			ioctl(snd, MIYOO_SND_SET_VOLUME, vol);
			close(snd);
		}
		volumeMode = getVolumeMode(val);

		return val;
	}

	int setBacklight(int val, bool popup = false) {
		val = GMenu2X::setBacklight(val, popup);
		char buf[128] = {0};
		sprintf(buf, "echo %d > /sys/devices/platform/backlight/backlight/backlight/brightness", val / 10);
		system(buf);
		return val;
	}

	void setCPU(uint32_t mhz) {
		uint32_t v;
		uint32_t total = sizeof(oc_table) / sizeof(oc_table[0]);

		for (int x = 0; x < total; x++) {
			if ((oc_table[x] >> 16) >= mhz) {
				mem[0] = (1 << 31) | (oc_table[x] &  0x0000ffff);
				break;
			}
		}

		INFO("Set CPU clock: %d(0x%08x)", mhz, v);
	}

	string hwPreLinkLaunch() {
		system("mount -o remount,sync /mnt");
		return "";
	}
};

#endif
