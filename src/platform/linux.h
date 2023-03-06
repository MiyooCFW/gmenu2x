#ifndef HW_PC_H
#define HW_PC_H

#include <linux/soundcard.h>

volatile uint16_t *memregs;
uint8_t memdev = 0;
int SOUND_MIXER_READ = SOUND_MIXER_READ_PCM;
int SOUND_MIXER_WRITE = SOUND_MIXER_WRITE_PCM;

uint16_t getDevStatus() {
	FILE *f;
	char buf[10000];
	if (f = fopen("/proc/bus/input/devices", "r")) {
	// if (f = fopen("/proc/bus/input/handlers", "r")) {
		size_t sz = fread(buf, sizeof(char), 10000, f);
		fclose(f);
		return sz;
	}
	return 0;
}

uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	printf("%s:%d: %s\n", __FILE__, __LINE__, __func__);
	numJoy = getDevStatus();
	if (numJoyPrev != numJoy) {
		numJoyPrev = numJoy;
		InputManager::pushEvent(JOYSTICK_CONNECT);
	}

	return 0;
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

int16_t getBatteryLevel() {
	return 0;
}

uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
	return 6;
}

uint8_t getVolumeMode(uint8_t vol) {
	return VOLUME_MODE_NORMAL;
}

class GMenuNX : public GMenu2X {
private:
	void hwInit() {
		CPU_MENU = 528;
		CPU_LINK = 600;
		CPU_MAX = 700;
		CPU_MIN = 500;
		CPU_STEP = 5;

		w = 480;
		h = 272;
	}

	uint16_t getBatteryLevel() {
		return 6;
	};
};

#endif
