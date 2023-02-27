#include "button.h"
#include "gmenu2x.h"
#include "buttonbox.h"

ButtonBox::ButtonBox(GMenu2X *gmenu2x):
gmenu2x(gmenu2x) {}

ButtonBox::~ButtonBox() {
	for (ButtonList::const_iterator it = buttons.begin(); it != buttons.end(); ++it)
		delete *it;
}

void ButtonBox::add(Button *button) {
	buttons.push_back(button);
}

void ButtonBox::remove(uint32_t n) {
	for (uint32_t i = 0; i < n; i++) buttons.pop_back();
}

void ButtonBox::paint(uint32_t posX) {
	for (ButtonList::const_iterator it = buttons.begin(); it != buttons.end(); ++it)
		posX = gmenu2x->drawButton(*it, posX);
}

void ButtonBox::handleTS() {
	for (ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it)
		(*it)->handleTS();
}
