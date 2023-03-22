#ifndef HW_MIYOO_H
#define HW_MIYOO_H

#include <sys/mman.h>

/*	MiyooCFW 2.0 Key Codes. Apaczer, 2023
	BUTTON     GMENU          SDL             NUMERIC   GPIO
	-----------------------------------------------------------------------------
	A          CONFIRM        SDLK_LALT       308
	B          CANCEL         SDLK_LCTRL      306
	X          MODIFIER       SDLK_LSHIFT     304
	Y          MANUAL         SDLK_SPACE      32
	L1         SECTION_PREV   SDLK_TAB        9
	R1         SECTION_NEXT   BACKSPACE       8
	L2         DEC            SDLK_PAGEUP     280
	R2         INC            SDLK_PAGEDOWN   281
	L3         DEC            SDLK_RALT       307
	R3         INC            SDLK_RSHIFT     303
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
#define MIYOO_LAY_SET_VER     _IOWR(0x103, 0, unsigned long)
#define MIYOO_FB0_GET_VER     _IOWR(0x102, 0, unsigned long)
#define MIYOO_FB0_PUT_OSD     _IOWR(0x100, 0, unsigned long)

static uint32_t oc_table[] = {
  ((24 * 2 * 1) << 16) | ((0 << 8) | (3 << 3)), // 48MHz = 24*2*1
  ((24 * 2 * 2) << 16) | ((1 << 8) | (3 << 3)),
  ((24 * 3 * 2) << 16) | ((2 << 8) | (3 << 3)),
  ((24 * 4 * 2) << 16) | ((3 << 8) | (3 << 3)),
  ((24 * 5 * 2) << 16) | ((4 << 8) | (3 << 3)),
  ((24 * 6 * 2) << 16) | ((5 << 8) | (3 << 3)),
  ((24 * 7 * 2) << 16) | ((6 << 8) | (3 << 3)),
  ((24 * 8 * 2) << 16) | ((7 << 8) | (3 << 3)),
  ((24 * 9 * 2) << 16) | ((8 << 8) | (3 << 3)),
  ((24 * 10 * 2) << 16) | ((9 << 8) | (3 << 3)),
  ((24 * 11 * 2) << 16) | ((10 << 8) | (3 << 3)),
  ((24 * 12 * 2) << 16) | ((11 << 8) | (3 << 3)),
  ((24 * 13 * 2) << 16) | ((12 << 8) | (3 << 3)),
  ((24 * 14 * 2) << 16) | ((13 << 8) | (3 << 3)),
  ((24 * 15 * 2) << 16) | ((14 << 8) | (3 << 3)),
  ((24 * 16 * 2) << 16) | ((15 << 8) | (3 << 3)),
  ((24 * 17 * 2) << 16) | ((16 << 8) | (3 << 3)),
  ((24 * 18 * 2) << 16) | ((17 << 8) | (3 << 3)),
  ((24 * 19 * 2) << 16) | ((18 << 8) | (3 << 3)),
  ((24 * 20 * 2) << 16) | ((19 << 8) | (3 << 3)),
  ((24 * 21 * 2) << 16) | ((20 << 8) | (3 << 3)),
  ((24 * 22 * 2) << 16) | ((21 << 8) | (3 << 3)),
  ((24 * 23 * 2) << 16) | ((22 << 8) | (3 << 3)),
  ((24 * 24 * 2) << 16) | ((23 << 8) | (3 << 3)),
  ((24 * 25 * 2) << 16) | ((24 << 8) | (3 << 3)),
  ((24 * 26 * 2) << 16) | ((25 << 8) | (3 << 3)), // 1248MHz = 24*26*2
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
		CPU_MENU = 720;
		CPU_LINK = 720;
		CPU_MAX = 912;
		CPU_MIN = 48;
		CPU_STEP = 48;

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
		INFO("MIYOO");
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
		if (FILE *f = fopen("/mnt/tvout", "r")) {
			return 0;
		} else {
			sprintf(buf, "echo %d > /sys/devices/platform/backlight/backlight/backlight/brightness", val / 10);
		}
		system(buf);
		return val;
	}

	void setKbdLayout(int val) {
		uint32_t kbd = open("/dev/miyoo_kbd", O_RDWR);

		if (kbd) {
			if (val > 3) val = 3;
			ioctl(kbd, MIYOO_LAY_SET_VER, val);
			close(kbd);
		}
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
