#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <string>
#include <vector>

class GMenu2X;
class Surface;

typedef std::vector<std::vector<std::string>> buttons_t;

class Dialog
{
public:
	Dialog(GMenu2X *gmenu2x, const std::string &title = "", const std::string &description = "", const std::string &icon = "");

protected:
	std::string title, description, icon;

	~Dialog();

	Surface *bg;
	GMenu2X *gmenu2x;
	buttons_t buttons;

	void drawTopBar(Surface *s, const std::string &title = "", const std::string &description = "", const std::string &icon = "");
	void drawBottomBar(Surface *s, buttons_t buttons = {});
	void drawDialog(Surface *s, bool top = true, bool bottom = true);
};

#endif
