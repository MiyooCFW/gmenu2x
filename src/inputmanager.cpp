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
/*	RS97 Key Codes. 2018, pingflood
	BUTTON     GMENU          SDL             NUMERIC
	-------------------------------------------------
	X          MODIFIER       SDLK_SPACE      32
	A          CONFIRM        SDLK_LCTRL      306
	B          CANCEL         SDLK_LALT       308
	Y          MANUAL         SDLK_LSHIFT     304
	L          SECTION_PREV   SDLK_TAB        9
	R          SECTION_NEXT   SDLK_BACKSPACE  8
	START      SETTINGS       SDLK_RETURN     13
	SELECT     MENU)          SDLK_ESCAPE     27
	BACKLIGHT  BACKLIGHT      SDLK_3          51
	POWER      POWER          SDLK_END        279
*/

#include "debug.h"
#include "inputmanager.h"
#include "utilities.h"

#include <iostream>
#include <fstream>

using namespace std;

InputManager::InputManager()
	: wakeUpTimer(NULL) {
}

InputManager::~InputManager() {
	for (uint32_t x = 0; x < joysticks.size(); x++)
		if(SDL_JoystickOpened(x))
			SDL_JoystickClose(joysticks[x]);
}

void InputManager::init(const string &conffile) {
	initJoysticks();
	if (!readConfFile(conffile))
		ERROR("InputManager initialization from config file failed.");
}

void InputManager::initJoysticks() {
	//SDL_JoystickEventState(SDL_IGNORE);

	int numJoy = SDL_NumJoysticks();
	// INFO("%d joysticks found", numJoy);
	for (int x = 0; x < numJoy; x++) {
		SDL_Joystick *joy = SDL_JoystickOpen(x);
		if (joy) {
			INFO("Initialized joystick: '%s'", SDL_JoystickName(x));
			joysticks.push_back(joy);
		}
		else WARNING("Failed to initialize joystick: %i", x);
	}
}

bool InputManager::readConfFile(const string &conffile) {
	setActionsCount(20); // plus 2 for BACKLIGHT and POWER

	if (!fileExists(conffile)) {
		ERROR("File not found: %s", conffile.c_str());
		return false;
	}

	ifstream inf(conffile.c_str(), ios_base::in);
	if (!inf.is_open()) {
		ERROR("Could not open %s", conffile.c_str());
		return false;
	}

	int action, linenum = 0;
	string line, name, value;
	string::size_type pos;
	vector<string> values;

	while (getline(inf, line, '\n')) {
		linenum++;
		pos = line.find("=");
		name = trim(line.substr(0,pos));
		value = trim(line.substr(pos + 1,line.length()));

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
		else if (name == "menu")         action = MENU;
		else if (name == "volup")        action = VOLUP;
		else if (name == "voldown")      action = VOLDOWN;
		else if (name == "backlight")    action = BACKLIGHT;
		else if (name == "power")        action = POWER;
		else if (name == "speaker") {}
		else {
			ERROR("%s:%d Unknown action '%s'.", conffile.c_str(), linenum, name.c_str());
			return false;
		}

		split(values, value, ",");
		if (values.size() >= 2) {

			if (values[0] == "joystickbutton" && values.size() == 3) {
				InputMap map;
				map.type = InputManager::MAPPING_TYPE_BUTTON;
				map.num = atoi(values[1].c_str());
				map.value = atoi(values[2].c_str());
				map.treshold = 0;
				actions[action].maplist.push_back(map);
			} else if (values[0] == "joystickaxis" && values.size() == 4) {
				InputMap map;
				map.type = InputManager::MAPPING_TYPE_AXIS;
				map.num = atoi(values[1].c_str());
				map.value = atoi(values[2].c_str());
				map.treshold = atoi(values[3].c_str());
				actions[action].maplist.push_back(map);
			} else if (values[0] == "keyboard") {
				InputMap map;
				map.type = InputManager::MAPPING_TYPE_KEYPRESS;
				map.value = atoi(values[1].c_str());
				actions[action].maplist.push_back(map);
			} else {
				ERROR("%s:%d Invalid syntax or unsupported mapping type '%s'.", conffile.c_str(), linenum, value.c_str());
				return false;
			}

		} else {
			ERROR("%s:%d Every definition must have at least 2 values (%s).", conffile.c_str(), linenum, value.c_str());
			return false;
		}
	}
	inf.close();
	return true;
}

void InputManager::setActionsCount(int count) {
	actions.clear();
	for (int x = 0; x < count; x++) {
		InputManagerAction action;
		action.active = false;
		action.interval = 0;
		// action.last = 0;
		action.timer = NULL;
		actions.push_back(action);
	}
}

bool InputManager::update(bool wait) {
	bool anyactions = false;
	SDL_JoystickUpdate();

	events.clear();
	SDL_Event event;

	if (wait) {
		SDL_WaitEvent(&event);
		if (event.type == SDL_KEYUP) anyactions = true;
		SDL_Event evcopy = event;
		events.push_back(evcopy);
	}
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_KEYUP) anyactions = true;
		SDL_Event evcopy = event;
		events.push_back(evcopy);
	}

	int32_t now = SDL_GetTicks();
	for (uint32_t x = 0; x < actions.size(); x++) {
		actions[x].active = isActive(x);
		if (actions[x].active) {
			memcpy(input_combo, input_combo + 1, sizeof(input_combo) - 1); // eegg
			input_combo[sizeof(input_combo) - 1] = x; // eegg
			if (actions[x].timer == NULL) actions[x].timer = SDL_AddTimer(actions[x].interval, wakeUp, NULL);
			anyactions = true;
			// actions[x].last = now;
		} else {
			if (actions[x].timer != NULL) {
				SDL_RemoveTimer(actions[x].timer);
				actions[x].timer = NULL;
			}
			// actions[x].last = 0;
		}
	}

	return anyactions;
}

bool InputManager::combo() { // eegg
	return !memcmp(input_combo, konami, sizeof(input_combo));
}

void InputManager::dropEvents() {
	for (uint32_t x = 0; x < actions.size(); x++) {
		actions[x].active = false;
		if (actions[x].timer != NULL) {
			SDL_RemoveTimer(actions[x].timer);
			actions[x].timer = NULL;
		}
	}
}

uint32_t InputManager::checkRepeat(uint32_t interval, void *_data) {
	RepeatEventData *data = (RepeatEventData *)_data;
	InputManager *im = (class InputManager*)data->im;
	SDL_JoystickUpdate();
	if (im->isActive(data->action)) {
		SDL_PushEvent( im->fakeEventForAction(data->action) );
		return interval;
	} else {
		im->actions[data->action].timer = NULL;
		return 0;
	}
}

SDL_Event *InputManager::fakeEventForAction(int action) {
	MappingList mapList = actions[action].maplist;
	// Take the first mapping. We only need one of them.
	InputMap map = *mapList.begin();

	SDL_Event *event = new SDL_Event();
	switch (map.type) {
		case InputManager::MAPPING_TYPE_BUTTON:
			event->type = SDL_JOYBUTTONDOWN;
			event->jbutton.type = SDL_JOYBUTTONDOWN;
			event->jbutton.which = map.num;
			event->jbutton.button = map.value;
			event->jbutton.state = SDL_PRESSED;
		break;
		case InputManager::MAPPING_TYPE_AXIS:
			event->type = SDL_JOYAXISMOTION;
			event->jaxis.type = SDL_JOYAXISMOTION;
			event->jaxis.which = map.num;
			event->jaxis.axis = map.value;
			event->jaxis.value = map.treshold;
		break;
		case InputManager::MAPPING_TYPE_KEYPRESS:
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = (SDLKey)map.value;
		break;
	}
	return event;
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

void InputManager::setWakeUpInterval(int ms) {
	if (wakeUpTimer != NULL)
		SDL_RemoveTimer(wakeUpTimer);

	if (ms > 0)
		wakeUpTimer = SDL_AddTimer(ms, wakeUp, NULL);
}

uint32_t InputManager::wakeUp(uint32_t interval, void *_data) {
	SDL_Event *event = new SDL_Event();
	event->type = SDL_WAKEUPEVENT;
	SDL_PushEvent( event );
	return interval;
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
			case InputManager::MAPPING_TYPE_BUTTON:
				if (map.num < joysticks.size() && SDL_JoystickGetButton(joysticks[map.num], map.value))
					return true;
			break;
			case InputManager::MAPPING_TYPE_AXIS:
				if (map.num < joysticks.size()) {
					int axyspos = SDL_JoystickGetAxis(joysticks[map.num], map.value);
					if (map.treshold < 0 && axyspos < map.treshold) return true;
					if (map.treshold > 0 && axyspos > map.treshold) return true;
				}
			break;
			case InputManager::MAPPING_TYPE_KEYPRESS:
				uint8_t *keystate = SDL_GetKeyState(NULL);
				return keystate[map.value];
			break;
		}
	}
	return false;
}
