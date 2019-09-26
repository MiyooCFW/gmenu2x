#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <SDL.h>
#include "gmenu2x.h"

class PowerManager {
public:
	PowerManager(GMenu2X *gmenu2x, uint32_t suspendTimeout, uint32_t powerTimeout);
	~PowerManager();
	void setSuspendTimeout(uint32_t suspendTimeout);
	void setPowerTimeout(uint32_t powerTimeout);
	void clearTimer();
	void resetSuspendTimer();
	void resetPowerTimer();
	static uint32_t doSuspend(uint32_t interval, void *param = NULL);
	static uint32_t doPowerOff(uint32_t interval, void *param = NULL);
	bool suspendActive;
	SDL_TimerID powerTimer; // = NULL;

private:
	GMenu2X *gmenu2x;
	uint32_t suspendTimeout, powerTimeout;
	static PowerManager *instance;
};

#endif
