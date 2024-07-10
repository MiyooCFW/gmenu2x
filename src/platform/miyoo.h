#ifndef HW_MIYOO_H
#define HW_MIYOO_H

#include <sys/mman.h>
#include <bitset>

// 	MiyooCFW 2.0 Key Codes. Apaczer, 2023
// 	BUTTON     GMENU          SDL             NUMERIC   GPIO
// 	-----------------------------------------------------------------------------
// 	A          CONFIRM        SDLK_LALT       308
// 	B          CANCEL         SDLK_LCTRL      306
// 	X          MODIFIER       SDLK_LSHIFT     304
// 	Y          MANUAL         SDLK_SPACE      32
// 	L1         SECTION_PREV   SDLK_TAB        9
// 	R1         SECTION_NEXT   BACKSPACE       8
// 	L2         DEC            SDLK_PAGEUP     280
// 	R2         INC            SDLK_PAGEDOWN   281
// 	L3         DEC            SDLK_RALT       307
// 	R3         INC            SDLK_RSHIFT     303
// 	RESET      POWER          SDLK_RCTRL      305
// 	START      SETTINGS       SDLK_RETURN     13
// 	SELECT     MENU           SDLK_ESCAPE     27
// 	UP         UP             SDLK_UP         273
// 	DOWN       DOWN           SDLK_DOWN       274
// 	RIGHT      RIGHT          SDLK_RIGHT      275
// 	LEFT       LEFT           SDLK_LEFT       276
//

#define MIYOO_VIR_SET_MODE    _IOWR(0x100, 0, unsigned long)
#define MIYOO_VIR_SET_VER     _IOWR(0x101, 0, unsigned long)
#define MIYOO_SND_SET_VOLUME  _IOWR(0x100, 0, unsigned long)
#define MIYOO_SND_GET_VOLUME  _IOWR(0x101, 0, unsigned long)
#define MIYOO_KBD_GET_HOTKEY  _IOWR(0x100, 0, unsigned long)
#define MIYOO_KBD_SET_VER     _IOWR(0x101, 0, unsigned long)
#define MIYOO_KBD_LOCK_KEY    _IOWR(0x102, 0, unsigned long) //unused
#define MIYOO_LAY_SET_VER     _IOWR(0x103, 0, unsigned long)
#define MIYOO_KBD_GET_VER     _IOWR(0x104, 0, unsigned long)
#define MIYOO_LAY_GET_VER     _IOWR(0x105, 0, unsigned long)
#define MIYOO_FB0_PUT_OSD     _IOWR(0x100, 0, unsigned long)
#define MIYOO_FB0_SET_MODE    _IOWR(0x101, 0, unsigned long)
#define MIYOO_FB0_GET_VER     _IOWR(0x102, 0, unsigned long)
#define MIYOO_FB0_SET_FLIP    _IOWR(0x103, 0, unsigned long) //unused
#define MIYOO_FB0_SET_FPBP    _IOWR(0x104, 0, unsigned long)
#define MIYOO_FB0_GET_FPBP    _IOWR(0x105, 0, unsigned long)
#define MIYOO_FB0_SET_TEFIX   _IOWR(0x106, 0, unsigned long)
#define MIYOO_FB0_GET_TEFIX   _IOWR(0x107, 0, unsigned long)

#define MIYOO_FBP_FILE        "/mnt/.fpbp.conf"
#define MIYOO_LID_FILE        "/mnt/.backlight.conf"
#define MIYOO_VOL_FILE        "/mnt/.volume.conf"
#define MIYOO_BUTTON_FILE     "/mnt/.buttons.conf"
#define MIYOO_BATTERY_FILE    "/mnt/.batterylow.conf"
#define MIYOO_LID_CONF        "/sys/devices/platform/backlight/backlight/backlight/brightness"
#define MIYOO_BATTERY         "/sys/class/power_supply/miyoo-battery/voltage_now"
#define MIYOO_OPTIONS_FILE    "/mnt/options.cfg"
#define MIYOO_TVOUT_FILE      "/mnt/tvout"
#define MIYOO_SND_FILE        "/dev/miyoo_snd"
#define MIYOO_FB0_FILE        "/dev/miyoo_fb0"
#define MIYOO_KBD_FILE        "/dev/miyoo_kbd"
#define MIYOO_VIR_FILE        "/dev/miyoo_vir"
#define TTS_ENGINE            "espeak"

#define MULTI_INT
#define DEFAULT_CPU 720
#define DEFAULT_LAYOUT 1
#define DEFAULT_TEFIX 0

static uint32_t oc_table[] = {
// F1C100S PLL_CPU Control Register.
// 24MHz*N*K/(M*P) ; N = (Nf+1)<=32 ; K = (Kf+1)<=4 ; M = (Mf+1)<=4 ; P = (00: /1 | 01: /2 | 10: /4); --> CPU_PLL output must be in 200MHz~2.6GHz range
// 27:18 are 10bit non-affecting space thus starting to read "int mhz" value here "(MHz << 18)" up to last 32bit.
// ((24 * N * K) << 18) | (Nf << 8) | (Kf << 4) | (Mf << 0) | (Pf << 16),
//
	(216 << 18) | (8 << 8) | (0 << 4),// 216MHz = 24MHz*9*1/(1*1)
	(240 << 18) | (9 << 8) | (0 << 4),
	(264 << 18) | (10 << 8) | (0 << 4),
	(288 << 18) | (11 << 8) | (0 << 4),
	(312 << 18) | (12 << 8) | (0 << 4),
	(336 << 18) | (13 << 8) | (0 << 4),
	(360 << 18) | (14 << 8) | (0 << 4),
	(384 << 18) | (15 << 8) | (0 << 4),
	(408 << 18) | (16 << 8) | (0 << 4),
	(432 << 18) | (17 << 8) | (0 << 4),
	(456 << 18) | (18 << 8) | (0 << 4),
	(480 << 18) | (19 << 8) | (0 << 4),
	(504 << 18) | (20 << 8) | (0 << 4),
	(528 << 18) | (21 << 8) | (0 << 4),
	(552 << 18) | (22 << 8) | (0 << 4),
	(576 << 18) | (23 << 8) | (0 << 4),
	(600 << 18) | (24 << 8) | (0 << 4),
	(624 << 18) | (25 << 8) | (0 << 4),
	(648 << 18) | (26 << 8) | (0 << 4),
	(672 << 18) | (27 << 8) | (0 << 4),
	(696 << 18) | (28 << 8) | (0 << 4),
	(DEFAULT_CPU << 18) | (29 << 8) | (0 << 4),
	(744 << 18) | (30 << 8) | (0 << 4),
	(768 << 18) | (31 << 8) | (0 << 4),
	(792 << 18) | (10 << 8) | (2 << 4),
	(816 << 18) | (16 << 8) | (1 << 4),
	(864 << 18) | (17 << 8) | (1 << 4),
	(912 << 18) | (18 << 8) | (1 << 4),
	(936 << 18) | (12 << 8) | (2 << 4),
	(960 << 18) | (19 << 8) | (1 << 4),
	(1008 << 18) | (20 << 8) | (1 << 4)// 1.008GHz = 24MHz*21*2/(1*1)
};
int oc_choices[] = {
	216,240,264,288,312,336,360,384,408,432,
	456,480,504,528,552,576,600,624,648,672,
	696,720,744,768,792,816,864,912,936,960,
	1008,9999
}; // last value[] is dummy point - do not modify
int oc_choices_size = sizeof(oc_choices)/sizeof(int);

int kbd, fb0, snd;
int32_t tickBattery = 0;

void setTVoff() {
	system("rm " MIYOO_TVOUT_FILE " ; sync ; reboot");
}

int getKbdLayoutHW() {
	kbd = open(MIYOO_KBD_FILE, O_RDWR);
	if (kbd > 0) {
		ioctl(kbd, MIYOO_LAY_GET_VER, &LAYOUT_VERSION);
		int val = LAYOUT_VERSION;
		close(kbd);
		return val;
	} else {
		WARNING("Could not open " MIYOO_KBD_FILE);
		return 0;
	}
}

int getTefixHW() {
	fb0 = open(MIYOO_FB0_FILE, O_RDWR);
	if (fb0 > 0) {
		ioctl(fb0, MIYOO_FB0_GET_TEFIX, &TEFIX);
		int val = TEFIX;
		close(fb0);
		return val;
	} else {
		WARNING("Could not open " MIYOO_FB0_FILE);
		return -1;
	}
}

int32_t getBatteryLevel() {
	int val = -1;
	if (FILE *f = fopen(MIYOO_BATTERY, "r")) {
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

class GMenu2X_platform : public GMenu2X {
private:
	void hwDeinit() {
	}

	void hwInit() {
		CPU_MENU = DEFAULT_CPU;
		CPU_LINK = CPU_MENU;
		CPU_MAX = oc_choices[oc_choices_size - 2]; //omitting last value in oc_choices
		CPU_EDGE = oc_choices[oc_choices_size - 4];
		CPU_MIN = oc_choices[8]; //408MHz, below may be insufficient and result in occasional freezes
//		CPU_STEP = 1;
		LAYOUT_VERSION_MAX = 6;
		TEFIX_MAX = 3;

		batteryIcon = getBatteryStatus(getBatteryLevel(), 0, 0);
		// setenv("HOME", "/mnt", 1);
		//system("mount -o remount,async /mnt");
		getKbdLayoutHW();
		getTefixHW();
		w = 320;
		h = 240;
		INFO("MIYOO");
	}

	int getBacklight() {
		int val = -1;
		FILE *f = fopen(MIYOO_LID_CONF, "r");
		if (f) {
			fscanf(f, "%i", &val);
			fclose(f);
			val = val * 10;
		}
		return val;
	}
	
	int getVolume() {
		int val = -1;
		snd = open(MIYOO_SND_FILE, O_RDWR);
		
		if (snd > 0) {
			ioctl(snd, MIYOO_SND_GET_VOLUME, &val);
			close(snd);
			val = val * 10; 
		} else {
		WARNING("Could not open " MIYOO_SND_FILE);
		}
		return val;
	}

public:
	int setVolume(int val, bool popup = false) {
		val = GMenu2X::setVolume(val, popup);
		char buf[128] = {0};
		int vol = val / 10;
		if (vol > 9) vol = 9;
		else if (vol < 0) vol = 0;
		
		snd = open(MIYOO_SND_FILE, O_RDWR);
		if (snd > 0) {
			ioctl(snd, MIYOO_SND_SET_VOLUME, vol);
			close(snd);
		}
		sprintf(buf, "echo %i > " MIYOO_VOL_FILE, vol);
		system(buf);		
		
		volumeMode = getVolumeMode(val);

		return val;
	}

	int setBacklight(int val, bool popup = false) {
		val = GMenu2X::setBacklight(val, popup);
		int lid = val / 10;
		if (lid > 10) lid = 10;
		char buf[128] = {0};
		if (FILE *f = fopen(MIYOO_TVOUT_FILE, "r")) {
			return 0;
		} else if (lid != 0) {
			sprintf(buf, "echo %i > " MIYOO_LID_FILE " && echo %i > " MIYOO_LID_CONF, lid, lid);
		} else {
			sprintf(buf, "echo %i > " MIYOO_LID_CONF, lid);
		}
		system(buf);
		return val;
	}

	void setKbdLayout(int val) {
		int f = open(MIYOO_KBD_FILE, O_RDWR);

		if (f > 0) {
			if (val <= 0 || val > LAYOUT_VERSION_MAX) val = DEFAULT_LAYOUT;
			ioctl(f, MIYOO_LAY_SET_VER, val);
			close(f);
		} else {
			WARNING("Could not open " MIYOO_KBD_FILE);
			val = 0;
		}
	}

	void setTefix(int val) {
		int f = open(MIYOO_FB0_FILE, O_RDWR);

		if (f > 0) {
			if (val < 0 || val > TEFIX_MAX) val = DEFAULT_TEFIX;
			ioctl(f, MIYOO_FB0_SET_TEFIX, val);
			close(f);
		} else {
			WARNING("Could not open " MIYOO_FB0_FILE);
			val = -1;
		}
	}

	void setCPU(uint32_t mhz) {
		volatile uint8_t memdev = open("/dev/mem", O_RDWR);
		if (memdev > 0) {
			uint32_t *mem = (uint32_t*)mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, 0x01c20000);
			if (mem == MAP_FAILED) {
				ERROR("Could not mmap hardware registers!");
				return;
			} else {
				uint32_t total = sizeof(oc_table) / sizeof(oc_table[0]);

				for (int x = total - 1; x >= 0; x--) {
					if ((oc_table[x] >> 18) <= mhz) {
						mem[0] = (1 << 31) | (oc_table[x] & 0x0003ffff);
						uint32_t v = mem[0];
						while (std::bitset<32>(v).test(28) == 0) {
							v = mem[0];
							//INFO("PLL unstable wait for register lock");
						}
						INFO("Set CPU clock: %d(0x%08x)", mhz, v);
						break;
					}
				}
			}
			munmap(mem, 0x1000);
		} else {
			WARNING("Could not open /dev/mem");
		}
		close(memdev);
	}

	string hwPreLinkLaunch() {
		//system("mount -o remount,sync /mnt");
		return "";
	}
};

#endif
