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

#include "debug.h"
#include "inputmanager.h"
#include "utilities.h"
#include "gmenu2x.h"

#include <fstream>

using namespace std;

extern uint8_t numJoy; // number of connected joysticks

enum InputManagerMappingTypes {
	MAPPING_TYPE_BUTTON,
	MAPPING_TYPE_AXIS,
	MAPPING_TYPE_KEYPRESS
};

static SDL_TimerID timer = NULL;

uint8_t *keystate = SDL_GetKeyState(NULL);

InputManager::InputManager() {}

InputManager::~InputManager() {
	SDL_RemoveTimer(timer); timer = NULL;
	for (uint32_t x = 0; x < joysticks.size(); x++)
		if (SDL_JoystickOpened(x))
			SDL_JoystickClose(joysticks[x]);
}

void InputManager::initJoysticks(bool reinit) {
	if (reinit) {
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	}

	joysticks.clear();
	int nj = SDL_NumJoysticks();
	INFO("%d joysticks found", nj);
	for (int x = 0; x < nj; x++) {
		SDL_Joystick *joy = SDL_JoystickOpen(x);
		if (joy) {
			INFO("Initialized joystick: '%s'", SDL_JoystickName(x));
			joysticks.push_back(joy);
		}
		else WARNING("Failed to initialize joystick: %i", x);
	}
}

void InputManager::init(const string &conffile) {
	setActionsCount(NUM_ACTIONS);
	initJoysticks(false);
	SDL_EnableKeyRepeat(0, 0);

	if (!file_exists(conffile)) {
		ERROR("File not found: %s", conffile.c_str());
		return;
	}

	ifstream inf(conffile.c_str(), ios_base::in);
	if (!inf.is_open()) {
		ERROR("Could not open %s", conffile.c_str());
		return;
	}

	int action, linenum = 0;
	string line, name, value;
	string::size_type pos;
	vector<string> values;

	for (uint32_t x = UDC_CONNECT; x < NUM_ACTIONS; x++) {
		InputMap map;
		map.type = MAPPING_TYPE_KEYPRESS;
		map.value = x - UDC_CONNECT + SDLK_WORLD_0;
		actions[x].maplist.push_back(map);
	}

	while (getline(inf, line, '\n')) {
		linenum++;
		pos = line.find("=");
		name = trim(line.substr(0, pos));
		value = trim(line.substr(pos + 1));

		if (name == "up")                action = UP;
		else if (name == "down")         action = DOWN;
		else if (name == "left")         action = LEFT;
		else if (name == "right")        action = RIGHT;
		else if (name == "modifier")     action = MODIFIER;
		else if (name == "confirm")      action = CONFIRM;
		else if (name == "cancel")       action = CANCEL;
		else if (name == "manual")       action = MANUAL;
		else if (name == "dec")          action = DEC;
		else if (name == "inc")          action = INC;
		else if (name == "section_prev") action = SECTION_PREV;
		else if (name == "section_next") action = SECTION_NEXT;
		else if (name == "pageup")       action = PAGEUP;
		else if (name == "pagedown")     action = PAGEDOWN;
		else if (name == "settings")     action = SETTINGS;
		else if (name == "volup")        action = VOLUP;
		else if (name == "voldown")      action = VOLDOWN;
		else if (name == "backlight")    action = BACKLIGHT;
		else if (name == "power")        action = POWER;
		else if (name == "menu")         action = MENU;
		else if (name == "speaker") {}
		else {
			ERROR("%s:%d Unknown action '%s'.", conffile.c_str(), linenum, name.c_str());
			continue;
		}

		split(values, value, ",");
		if (values.size() >= 2) {
			if (values[0] == "joystickbutton" && values.size() == 3) {
				InputMap map;
				map.type = MAPPING_TYPE_BUTTON;
				map.num = atoi(values[1].c_str());
				map.value = atoi(values[2].c_str());
				map.treshold = 0;
				actions[action].maplist.push_back(map);
				// INFO("ADDING: joystickbutton %d  %d ", map.num, map.value);
			} else if (values[0] == "joystickaxis" && values.size() == 4) {
				InputMap map;
				map.type = MAPPING_TYPE_AXIS;
				map.num = atoi(values[1].c_str());
				map.value = atoi(values[2].c_str());
				map.treshold = atoi(values[3].c_str());
				actions[action].maplist.push_back(map);
			} else if (values[0] == "keyboard") {
				InputMap map;
				map.type = MAPPING_TYPE_KEYPRESS;
				map.value = atoi(values[1].c_str());
				actions[action].maplist.push_back(map);
			} else {
				ERROR("%s:%d Invalid syntax or unsupported mapping type '%s'.", conffile.c_str(), linenum, value.c_str());
				continue;
			}

		} else {
			ERROR("%s:%d Every definition must have at least 2 values (%s).", conffile.c_str(), linenum, value.c_str());
			continue;
		}
	}
	inf.close();
}

void InputManager::setActionsCount(int count) {
	actions.clear();
	for (int x = 0; x < count; x++) {
		InputManagerAction action;
		action.active = false;
		action.interval = 150;
		actions.push_back(action);
	}
}

bool InputManager::update(bool wait) {
	bool anyactions = false;
	uint32_t x;
	SDL_Event event;

	SDL_JoystickUpdate();

	if (wait) SDL_WaitEvent(&event);
	else if (!SDL_PollEvent(&event)) return false;

	if (timer && (event.type == SDL_JOYAXISMOTION || event.type == SDL_JOYHATMOTION)) {
		dropEvents(false);
		return false;
	}

	dropEvents();

	x = event.key.keysym.sym;
	if (!x) x = event.key.keysym.scancode;

	switch (event.type) {
		case SDL_KEYDOWN:
			keystate[x] = true;
			break;
		case SDL_KEYUP:
			anyactions = true;
			keystate[x] = false;
			break;
		case SDL_USEREVENT:
			if (event.user.code == WAKE_UP)
				anyactions = true;
			break;
	}

	int active = -1;
	for (x = 0; x < actions.size(); x++) {
		actions[x].active = isActive(x);
		if (actions[x].active) {
			memcpy(input_combo, input_combo + 1, sizeof(input_combo) - 1); input_combo[sizeof(input_combo) - 1] = x; // eegg
			anyactions = true;
			active = x;
		}
	}

	if (active >= 0) {
		SDL_RemoveTimer(timer);
		timer = SDL_AddTimer(actions[active].interval, wakeUp, (void*)false);
	}

	x = 0;
	while (SDL_PollEvent(&event) && x++ < 30) {
		if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
			keystate[event.key.keysym.sym] = false;
		}
	}

	return anyactions;
}

bool InputManager::combo() { // eegg
	return !memcmp(input_combo, konami, sizeof(input_combo));
}

void InputManager::dropEvents(bool drop_timer) {
	if (drop_timer) {
		SDL_RemoveTimer(timer); timer = NULL;
	}

	for (uint32_t x = 0; x < actions.size(); x++) {
		actions[x].active = false;
	}
}

uint32_t pushEventEnd(uint32_t interval, void *action) {
	SDL_Event event;
	event.type = SDL_KEYUP;
	event.key.state = SDL_RELEASED;
	event.key.keysym.sym = (SDLKey)((size_t)action - UDC_CONNECT + SDLK_WORLD_0);
	SDL_PushEvent(&event);
	return 0;
}

void InputManager::pushEvent(int action) {
	WARNING("EVENT START");
	SDL_Event event;
	event.type = SDL_KEYDOWN;
	event.key.state = SDL_PRESSED;
	event.key.keysym.sym = (SDLKey)(action - UDC_CONNECT + SDLK_WORLD_0);
	SDL_PushEvent(&event);
	SDL_AddTimer(50, pushEventEnd, (void*)action);
}

int InputManager::count() {
	return actions.size();
}

void InputManager::setInterval(int ms, int action) {
	if (action < 0)
		for (uint32_t x = 0; x < actions.size(); x++)
			actions[x].interval = ms;
	else if ((uint32_t)action < actions.size())
		actions[action].interval = ms;
}

uint32_t InputManager::wakeUp(uint32_t interval, void *repeat) {
	SDL_RemoveTimer(timer); timer = NULL;
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = WAKE_UP;
	SDL_PushEvent(&event);
	// if ((bool*) repeat) return interval;
	return 0;
}

bool &InputManager::operator[](int action) {
	// if (action<0 || (uint32_t)action>=actions.size()) return false;
	return actions[action].active;
}

bool InputManager::isActive(int action) {
	MappingList mapList = actions[action].maplist;
	for (MappingList::const_iterator it = mapList.begin(); it != mapList.end(); ++it) {
		InputMap map = *it;
		switch (map.type) {
			case MAPPING_TYPE_BUTTON:
				for (int j = 0; j < joysticks.size(); j++) {
					if (SDL_JoystickGetButton(joysticks[j], map.value)) return true;
				}
				break;
			case MAPPING_TYPE_AXIS:
				if (map.num < joysticks.size()) {
					int axyspos = SDL_JoystickGetAxis(joysticks[map.num], map.value);
					if (map.treshold < 0 && axyspos < map.treshold) return true;
					if (map.treshold > 0 && axyspos > map.treshold) return true;
				}
				break;
			case MAPPING_TYPE_KEYPRESS:
				if (keystate[map.value]) return true;
				break;
		}
	}
	return false;
}
