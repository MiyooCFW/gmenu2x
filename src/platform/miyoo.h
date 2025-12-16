#ifndef HW_MIYOO_H
#define HW_MIYOO_H

#include <linux/magic.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/swap.h>
#include <sys/reboot.h>
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
#define MIYOO_SND_JACK_STATUS _IOWR(0x102, 0, unsigned long)
#define MIYOO_TV_JACK_STATUS  _IOWR(0x103, 0, unsigned long)
#define MIYOO_KBD_GET_HOTKEY  _IOWR(0x100, 0, unsigned long)
#define MIYOO_KBD_SET_VER     _IOWR(0x101, 0, unsigned long)
#define MIYOO_KBD_LOCK_KEY    _IOWR(0x102, 0, unsigned long) //unused
#define MIYOO_LAY_SET_VER     _IOWR(0x103, 0, unsigned long)
#define MIYOO_KBD_GET_VER     _IOWR(0x104, 0, unsigned long)
#define MIYOO_LAY_GET_VER     _IOWR(0x105, 0, unsigned long)
#define MIYOO_KBD_SET_HOTKEY  _IOWR(0x106, 0, unsigned long)
#define MIYOO_FB0_PUT_OSD     _IOWR(0x100, 0, unsigned long)
#define MIYOO_FB0_SET_MODE    _IOWR(0x101, 0, unsigned long)
#define MIYOO_FB0_GET_VER     _IOWR(0x102, 0, unsigned long)
#define MIYOO_FB0_SET_FLIP    _IOWR(0x103, 0, unsigned long) //unused
#define MIYOO_FB0_SET_FPBP    _IOWR(0x104, 0, unsigned long)
#define MIYOO_FB0_GET_FPBP    _IOWR(0x105, 0, unsigned long)
#define MIYOO_FB0_SET_TEFIX   _IOWR(0x106, 0, unsigned long)
#define MIYOO_FB0_GET_TEFIX   _IOWR(0x107, 0, unsigned long)

#define MIYOO_HOME_DIR        "/mnt"
#define MIYOO_ROMS_DIR        "/roms"
#define MIYOO_FBP_FILE        "/mnt/.fpbp.conf"
#define MIYOO_LID_FILE        "/mnt/.backlight.conf"
#define MIYOO_VOL_FILE        "/mnt/.volume.conf"
#define MIYOO_BUTTON_FILE     "/mnt/.buttons.conf"
#define MIYOO_BATTERY_FILE    "/mnt/.batterylow.conf"
#define MIYOO_DATE_FILE       "/mnt/.date.conf"
#define MIYOO_TVMODE_FILE     "/mnt/.tvmode"
#define MIYOO_OPTIONS_FILE    "/boot/options.cfg"
#define MIYOO_LID_CONF        "/sys/devices/platform/backlight/backlight/backlight/brightness"
#define MIYOO_BATTERY         "/sys/devices/platform/soc/1c23400.battery/power_supply/miyoo-battery/voltage_now"
#define MIYOO_BATTERY_STATUS  "/sys/class/power_supply/miyoo-battery/status"
#define MIYOO_USB_STATE       "/sys/devices/platform/soc/1c13000.usb/musb-hdrc.1.auto/udc/musb-hdrc.1.auto/state"
#define MIYOO_USB_SUSPEND     "/sys/devices/platform/soc/1c13000.usb/musb-hdrc.1.auto/gadget/suspended"
#define MIYOO_SND_FILE        "/dev/miyoo_snd"
#define MIYOO_FB0_FILE        "/dev/miyoo_fb0"
#define MIYOO_KBD_FILE        "/dev/miyoo_kbd"
#define MIYOO_VIR_FILE        "/dev/miyoo_vir"
#define MIYOO_SWAP_FILE       "/dev/mmcblk0p3"
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
string status, state, suspended;
char *cmd_testfs_p5 = NULL;
int32_t tickBattery = 0;

struct statfs fs;

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

uint8_t getUDCStatus() {
	status = file_read(MIYOO_BATTERY_STATUS);
	state = file_read(MIYOO_USB_STATE);
	suspended = file_read(MIYOO_USB_SUSPEND);

	//INFO("SYS_USB_MODE=%s",SYS_USB_MODE);
	if (SYS_USB_MODE != NULL && sysUSBmode == "Unknown") {
		sysUSBmode = SYS_USB_MODE;
	}
	//INFO("sysUSBmode=%s",sysUSBmode);

	if (sysUSBmode == "mtp") sysUSBmode = "Storage";
	else if (sysUSBmode == "hid") sysUSBmode = "HID";
	else if (sysUSBmode == "serial") sysUSBmode = "Serial";
	else if (sysUSBmode == "net") sysUSBmode = "Networking";
	else if (sysUSBmode == "host") sysUSBmode = "Host";

	if (state == "configured" && suspended == "0") return UDC_CONNECT;
	if (status == "Charging") return UDC_CHARGE;
	return UDC_REMOVE;
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
	if ((val > 4300) || (val < 0) || (getUDCStatus() == UDC_CHARGE)) return 6; // >100% - max voltage 4320
	if (val > 4100) return 5; // 100% - fully charged 4150
	if (val > 3900) return 4; // 80%
	if (val > 3800) return 3; // 60%
	if (val > 3700) return 2; // 40%
	if (val > 3520) return 1; // 20%
	return 0; // 0% :(
}

uint16_t getTVOutMode() {
	int val = TV_OFF;
	if (FILE *f = fopen(MIYOO_TVMODE_FILE, "r")) {
		fscanf(f, "%i", &val);
		fclose(f);
	}
	return val;
}

uint8_t getMMCStatus() {
	return MMC_REMOVE;
}

uint8_t getTVOutStatus() {
	int status = 0;

	if (SYS_TVOUT != NULL) {
		sysTVout = SYS_TVOUT;
	}

	snd = open(MIYOO_SND_FILE, O_RDWR);
	if (snd > 0) {
		ioctl(snd, MIYOO_TV_JACK_STATUS, &status);
		close(snd);
	} else {
		WARNING("Could not open " MIYOO_SND_FILE);
	}
	//INFO("TV status=%i", status);
	if (status)	return TV_CONNECT;
	return TV_REMOVE;
}

uint8_t getDevStatus() {
	return 0;
}

uint8_t getVolumeMode(uint8_t vol) {
	return VOLUME_MODE_NORMAL;
}

uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	tickBattery++;
	if (tickBattery > 30) { // update battery level every 30 hwChecks
		tickBattery = 0;
		batteryIcon = getBatteryStatus(getBatteryLevel(), 0, 0);
	}

	if (tickBattery > 1) {
		udcStatus = getUDCStatus();
		if (udcPrev != udcStatus) {
			udcPrev = udcStatus;
			InputManager::pushEvent(udcStatus);
			return 2000;
		}
		tvOutStatus = getTVOutStatus();
		if (tvOutPrev != tvOutStatus) {
			tvOutPrev = tvOutStatus;
			InputManager::pushEvent(tvOutStatus);
			return 2000;
		}
	}
	return interval;
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
		if (mount(MIYOO_ROMS_DIR, MIYOO_ROMS_DIR, NULL, MS_REMOUNT, NULL) != 0)
			ERROR("remounting in default options " MIYOO_ROMS_DIR " failed");
		getKbdLayoutHW();
		getTefixHW();
		w = 320;
		h = 240;
		INFO("MIYOO");
	}

	void tvOutDialog(int16_t mode) {
			if (mode < 0) {
				int option;

				mode = TV_OFF;

				if (confStr["tvMode"] == "NTSC") option = CONFIRM;
				else if (confStr["tvMode"] == "PAL") option = MANUAL;
				else if (confStr["tvMode"] == "OFF") option = CANCEL;
				else {
					MessageBox mb(this, tr["JACK connection detected.\nEnable TV-output?"], "skin:icons/tv.png");
					mb.setButton(CONFIRM, tr["TV-NTSC"]);
					mb.setButton(MANUAL,  tr["TV-PAL"]);
					mb.setButton(CANCEL,  tr["Cancel"]);
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

			if (mode != getTVOutMode()) {
			 	setTVOut(mode);
			}
	}

	void udcDialog(int udcStatus) {
		if (udcStatus == UDC_REMOVE) {
			INFO("USB disconnected...");
			//system("");
			return;
		}

		int option = CANCEL;
		if (confStr["usbMode"] == "Storage") option = CONFIRM;
		else if (confStr["usbMode"] == "HID") option = MODIFIER;
		else if (confStr["usbMode"] == "Serial") option = MANUAL;
		else if (confStr["usbMode"] == "Networking") option = MENU;
		else if (confStr["usbMode"] == "Default") option = CANCEL;
		else if (!(confInt["usbHost"]) && udcStatus != -1) {
			// shouldn't see below prompt in host or in default
			MessageBox mb(this, tr["USB mode change"]);
			mb.setButton(CANCEL,  tr["Cancel"]);
			mb.setButton(CONFIRM, tr["Storage"]);
			mb.setButton(MODIFIER, tr["HID"]);
			mb.setButton(MANUAL, tr["Serial"]);
			mb.setButton(MENU, tr["NET"]);
			option = mb.exec();
		}

		if (udcStatus == UDC_HOST) option = -1;
		

		if (option == CANCEL && !(udcStatus == -1 && confStr["usbMode"] == "Ask")) {
			INFO("Continuing with default USB mode");
			return;
		}
		
		string usbcommand = "";
		MessageBox mb(this, tr["Loading"]);
		mb.setAutoHide(1);
		mb.setBgAlpha(0);
		mb.exec();
		input.update(false);
		if (option == CONFIRM || (udcStatus == -1 && confStr["usbMode"] == "Ask")) { // storage
			INFO("Enabling MTP storage device");
			usbcommand = "mtp";
		} else if (option == MODIFIER) { // hid
			INFO("Enabling HID device");
			usbcommand = "hid";
		} else if (option == MANUAL) { // serial
			INFO("Enabling Serial Console on device");
			usbcommand = "serial";
		} else if (option == MENU) { // networking
			INFO("Enabling USB Networking on device");
			usbcommand = "net";
		} else { // host
			// if (option == -1)
			INFO("Enabling host device");
			usbcommand = "host";
		}
		pid_t son = fork();
		if (!son) {
			execlp("/bin/sh", "/bin/sh", "-c", ("exec /usr/bin/usb-mode " + usbcommand).c_str(), NULL);
		}
		wait(NULL);
		sysUSBmode = usbcommand;
		writeTmp();
	
		return;
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
		// sprintf(buf, "echo %i > " MIYOO_VOL_FILE, vol);
		// system(buf);
		if (FILE *f = fopen(MIYOO_VOL_FILE, "w")){
			fprintf(f, "%i", vol);
			fclose(f);
		}
		
		volumeMode = getVolumeMode(val);

		return val;
	}
	
	bool isUsbConnected() {
		string state = file_read(MIYOO_USB_STATE);
		string suspended = file_read(MIYOO_USB_SUSPEND);
		return (state == "configured" && suspended == "0");
	}

	int setBacklight(int val, bool popup = false) {
		val = GMenu2X::setBacklight(val, popup);
		int lid = val / 10;
		if (lid > 10) lid = 10;
		char buf[128] = {0};
		if (getTVOutMode() != TV_OFF && getTVOutStatus() != TV_REMOVE || confInt["tvOutForce"])
			return 0;
		//sprintf(buf, "echo %i > " MIYOO_LID_FILE " && echo %i > " MIYOO_LID_CONF, lid, lid);
		//system(buf);
		if (lid != 0) {
			if (FILE *f1 = fopen(MIYOO_LID_FILE, "w")){
				fprintf(f1, "%i", lid);
				fclose(f1);
			}
		}
		if (FILE *f2 = fopen(MIYOO_LID_CONF, "w")){
			fprintf(f2, "%i", lid);
			fclose(f2);
		}
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

	void shutdownOS(bool poweroff = true) {
		write_date_file(MIYOO_DATE_FILE);
		sync();
		//inittab shutdown actions are ignored when using reboot(), thus DIY as follow
		swapoff(MIYOO_SWAP_FILE);
		if (mount(MIYOO_HOME_DIR, MIYOO_HOME_DIR, NULL, MS_REMOUNT | MS_RDONLY, NULL) != 0)
			ERROR("remounting " MIYOO_HOME_DIR " in RO failed");
		if (mount(MIYOO_ROMS_DIR, MIYOO_ROMS_DIR, NULL, MS_REMOUNT | MS_RDONLY, NULL) != 0)
			ERROR("remounting " MIYOO_ROMS_DIR " in RO failed");

		reboot(poweroff ? RB_POWER_OFF : RB_AUTOBOOT);
	}

	void setTVOut(unsigned int mode) {
		setBacklight(confInt["backlight"]);
		writeTmp();
		quit(false);
		if (FILE *f = fopen(MIYOO_TVMODE_FILE, "w")) {
			fprintf(f, "%i", mode); // fputs(val, f);
			fclose(f);
		}
		//system("sync; mount -o remount,ro $HOME; reboot");
		shutdownOS(false);
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
		if (statfs(MIYOO_ROMS_DIR, &fs) != 0) {
			ERROR("couldn't read FS type with statfs()");
		} else {
			if ((unsigned long)fs.f_type != BTRFS_SUPER_MAGIC) {
				if (mount(MIYOO_ROMS_DIR, MIYOO_ROMS_DIR, NULL, MS_REMOUNT | MS_SYNCHRONOUS, NULL) != 0)
					ERROR("remounting in sync" MIYOO_ROMS_DIR " failed");
			} else {
				INFO("detected BTRFS in " MIYOO_ROMS_DIR);
			}
		}
		return "";
	}
};

#endif
