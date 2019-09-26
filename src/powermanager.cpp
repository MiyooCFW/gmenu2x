#include "powermanager.h"
#include "messagebox.h"

PowerManager *PowerManager::instance = NULL;

PowerManager::PowerManager(GMenu2X *gmenu2x, uint32_t suspendTimeout, uint32_t powerTimeout) {
	instance = this;
	this->suspendTimeout = suspendTimeout;
	this->powerTimeout = powerTimeout;
	this->gmenu2x = gmenu2x;
	this->suspendActive = false;
	this->powerTimer = NULL;
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
	if (powerTimer != NULL) {
    SDL_RemoveTimer(powerTimer);
  }
	powerTimer = NULL;
};

void PowerManager::resetSuspendTimer() {
	clearTimer();
	powerTimer = SDL_AddTimer(this->suspendTimeout * 1e3, doSuspend, NULL);
};

void PowerManager::resetPowerTimer() {
	clearTimer();
  if(this->powerTimeout){
	  powerTimer = SDL_AddTimer(this->powerTimeout * 60e3, doPowerOff, NULL);
  }
};

uint32_t PowerManager::doSuspend(uint32_t interval, void *param) {
	if (interval > 0) {
		MessageBox mb(PowerManager::instance->gmenu2x, PowerManager::instance->gmenu2x->tr["Suspend"]);
		mb.setAutoHide(500);
		mb.exec();

		PowerManager::instance->gmenu2x->setBacklight(0);
		PowerManager::instance->gmenu2x->setTVOut("OFF");
		PowerManager::instance->gmenu2x->setCPU(PowerManager::instance->gmenu2x->confInt["cpuMin"]);
		PowerManager::instance->resetPowerTimer();
		PowerManager::instance->suspendActive = true;
		return interval;
	}

	PowerManager::instance->gmenu2x->setCPU(PowerManager::instance->gmenu2x->confInt["cpuMenu"]);
	PowerManager::instance->gmenu2x->setTVOut(PowerManager::instance->gmenu2x->TVOut);
	PowerManager::instance->gmenu2x->setBacklight(max(10, PowerManager::instance->gmenu2x->confInt["backlight"]));
	PowerManager::instance->suspendActive = false;
	PowerManager::instance->resetSuspendTimer();
	return interval;
};

uint32_t PowerManager::doPowerOff(uint32_t interval, void *param) {
#if !defined(TARGET_PC)
	system("sync; poweroff");
#endif
	return interval;
};
