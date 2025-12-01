#include "powermanager.h"

PowerManager *PowerManager::instance = NULL;

PowerManager::PowerManager(GMenu2X *gmenu2x, uint32_t suspendTimeout, uint32_t powerTimeout):
gmenu2x(gmenu2x), suspendTimeout(suspendTimeout), powerTimeout(powerTimeout) {
	instance = this;

	resetSuspendTimer();
}

PowerManager::~PowerManager() {
	clearTimer();
	instance = NULL;
}

void PowerManager::setSuspendTimeout(uint32_t suspendTimeout) {
	this->suspendTimeout = suspendTimeout;
	resetSuspendTimer();
};

void PowerManager::setPowerTimeout(uint32_t powerTimeout) {
	this->powerTimeout = powerTimeout;
	resetSuspendTimer();
};

void PowerManager::clearTimer() {
	SDL_RemoveTimer(powerTimer); powerTimer = NULL;
};

void PowerManager::resetSuspendTimer() {
	clearTimer();
	if (this->suspendTimeout > 0)
		powerTimer = SDL_AddTimer(this->suspendTimeout * 1e3, doSuspend, NULL);
};

void PowerManager::resetPowerTimer() {
	clearTimer();
	if (this->powerTimeout > 0)
		powerTimer = SDL_AddTimer(this->powerTimeout * 60e3, doPowerOff, NULL);
};

uint32_t PowerManager::doSuspend(uint32_t interval, void *param) {
	if (interval > 0) {
#if defined(HW_LIDVOL)
		PowerManager::instance->gmenu2x->confInt["backlight"] = PowerManager::instance->gmenu2x->getBacklight();
		PowerManager::instance->gmenu2x->confInt["globalVolume"] = PowerManager::instance->gmenu2x->getVolume();
//		INFO("%i", PowerManager::instance->gmenu2x->confInt["backlight"]);
//		INFO("%i", PowerManager::instance->gmenu2x->confInt["globalVolume"]);
#endif
		PowerManager::instance->gmenu2x->setBacklight(0);
//		PowerManager::instance->gmenu2x->setVolume(0);
		PowerManager::instance->resetPowerTimer();
		PowerManager::instance->gmenu2x->cls();
		if (!PowerManager::instance->gmenu2x->isUsbConnected())
			PowerManager::instance->gmenu2x->setCPU(PowerManager::instance->gmenu2x->confInt["cpuMin"]);
		PowerManager::instance->suspendActive = true;
		return interval;
	}

//	INFO("%i", PowerManager::instance->gmenu2x->confInt["backlight"]);
//	INFO("%i", PowerManager::instance->gmenu2x->confInt["globalVolume"]);
	PowerManager::instance->gmenu2x->setBacklight(max(10, PowerManager::instance->gmenu2x->confInt["backlight"]));
	PowerManager::instance->gmenu2x->setVolume(PowerManager::instance->gmenu2x->confInt["globalVolume"]);
	PowerManager::instance->gmenu2x->setCPU(PowerManager::instance->gmenu2x->confInt["cpuMenu"]);
	PowerManager::instance->suspendActive = false;
	PowerManager::instance->resetSuspendTimer();
	PowerManager::instance->gmenu2x->actionPerformed = false;
	return interval;
};

uint32_t PowerManager::doPowerOff(uint32_t interval, void *param) {
	//system("sync; poweroff");
	PowerManager::instance->gmenu2x->shutdownOS(true);
	return interval;
};
