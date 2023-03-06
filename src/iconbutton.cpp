#include "iconbutton.h"
#include "gmenu2x.h"
#include "surface.h"
#include "debug.h"

using namespace std;
using namespace fastdelegate;

IconButton::IconButton(GMenu2X *gmenu2x, const string &icon, const string &label):
Button(gmenu2x->ts), gmenu2x(gmenu2x), icon(icon), label(label) {
	labelPosition = IconButton::DISP_RIGHT;
	labelMargin = 2;
	updateSurfaces();
}

void IconButton::updateSurfaces() {
	iconSurface = gmenu2x->sc[icon];
	recalcSize();
}

void IconButton::setPosition(int x, int y) {
	if (rect.x != x || rect.y != y) {
		Button::setPosition(x,y);
		recalcSize();
	}
}

uint16_t IconButton::paint() {
	return gmenu2x->drawButton(gmenu2x->s, this->icon, this->label, rect.x, rect.y);
}

bool IconButton::paintHover() {
	return true;
}

void IconButton::recalcSize() {
	uint32_t h = 0, w = 0;
	uint32_t margin = labelMargin;

	if (iconSurface == NULL || label == "")
		margin = 0;

	if (iconSurface != NULL) {
		w += iconSurface->raw->w;
		h += iconSurface->raw->h;
		iconRect.w = w;
		iconRect.h = h;
		iconRect.x = rect.x;
		iconRect.y = rect.y;
	} else {
		iconRect.x = 0;
		iconRect.y = 0;
		iconRect.w = 0;
		iconRect.h = 0;
	}

	if (label != "") {
		labelRect.w = gmenu2x->font->getTextWidth(label);
		labelRect.h = gmenu2x->font->getHeight();
		if (labelPosition == IconButton::DISP_LEFT || labelPosition == IconButton::DISP_RIGHT) {
			w += margin + labelRect.w;
			labelHAlign = HAlignLeft;
			labelVAlign = VAlignMiddle;
		} else {
			h += margin + labelRect.h;
			labelHAlign = HAlignCenter;
			labelVAlign = VAlignTop;
		}

		switch (labelPosition) {
			case IconButton::DISP_BOTTOM:
				labelRect.x = iconRect.x + iconRect.w/2;
				labelRect.y = iconRect.y + iconRect.h + margin;
			break;
			case IconButton::DISP_TOP:
				labelRect.x = iconRect.x + iconRect.w/2;
				labelRect.y = rect.y;
				iconRect.y += labelRect.h + margin;
			break;
			case IconButton::DISP_LEFT:
				labelRect.x = rect.x;
				labelRect.y = rect.y+h/2;
				iconRect.x += labelRect.w + margin;
			break;
			default:
				labelRect.x = iconRect.x + iconRect.w + margin;
				labelRect.y = rect.y+h/2;
			break;
		}
	}
	setSize(w, h);
}

const string &IconButton::getLabel() {
	return label;
}

void IconButton::setLabel(const string &label) {
	this->label = label;
}

void IconButton::setLabelPosition(int pos, int margin) {
	labelPosition = pos;
	labelMargin = margin;
	recalcSize();
}

const string &IconButton::getIcon() {
	return icon;
}

void IconButton::setIcon(const string &icon) {
	this->icon = icon;
	updateSurfaces();
}

void IconButton::setAction(ButtonAction action) {
	this->action = action;
}
