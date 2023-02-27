#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <string>
#include "button.h"

using std::string;

class GMenu2X;
class Surface;

class IconButton : public Button {
protected:
	GMenu2X *gmenu2x;
	int labelPosition, labelMargin;
	string icon, label;
	uint16_t labelHAlign, labelVAlign;
	void recalcSize();
	SDL_Rect iconRect, labelRect;
	Surface *iconSurface;
	void updateSurfaces();

public:
	enum icon_button_position {
		DISP_RIGHT,
		DISP_LEFT,
		DISP_TOP,
		DISP_BOTTOM
	};

	IconButton(GMenu2X *gmenu2x, const string &icon, const string &label="");
	virtual ~IconButton() {};

	virtual uint16_t paint();
	virtual bool paintHover();

	virtual void setPosition(int x, int y);

	const string &getLabel();
	void setLabel(const string &label);
	void setLabelPosition(int pos, int margin);

	const string &getIcon();
	void setIcon(const string &icon);

	void setAction(ButtonAction action);
};

#endif
