#include "button.h"
#include "gmenu2x.h"

using namespace std;
using namespace fastdelegate;

Button::Button(Touchscreen &ts, bool doubleClick):
ts(ts), action(MakeDelegate(this, &Button::voidAction)), rect((SDL_Rect) { 0, 0, 0, 0 }), doubleClick(doubleClick), lastTick(0) {}

uint16_t Button::paint() {
	return 0;
	// if (ts.inRect(rect))
		// if (!paintHover()) return 0;
}

bool Button::paintHover() {
	return false;
}

bool Button::isPressed() {
	return ts.pressed() && ts.inRect(rect);
}

bool Button::isReleased() {
	return ts.released() && ts.inRect(rect);
}

bool Button::handleTS() {
	if (isReleased()) {
		if (doubleClick) {
			int tickNow = SDL_GetTicks();
			if (tickNow - lastTick < 400)
				exec();
			lastTick = tickNow;
		} else {
			exec();
		}
		return true;
	}
	return false;
}

void Button::exec() {
	ts.setHandled();
	action();
}

SDL_Rect Button::getRect() {
	return rect;
}

void Button::setSize(int w, int h) {
	rect.w = w;
	rect.h = h;
}

void Button::setPosition(int x, int y) {
	rect.x = x;
	rect.y = y;
}

void Button::setAction(ButtonAction action) {
	this->action = action;
}
