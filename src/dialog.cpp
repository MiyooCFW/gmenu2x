#include "dialog.h"
#include "gmenu2x.h"
#include "debug.h"
#include "menu.h"
#include "linkapp.h"

Dialog::Dialog(GMenu2X *gmenu2x, const std::string &title, const std::string &description, const std::string &icon):
gmenu2x(gmenu2x), title(title), description(description), icon(icon) {
	bg = new Surface(gmenu2x->bg);

	buttons.clear();
	gmenu2x->input.dropEvents(); // prevent passing input away
}

Dialog::~Dialog() {
	gmenu2x->input.dropEvents(); // prevent passing input away
	delete bg;
}

void Dialog::drawTopBar(Surface *s, const std::string &title, const std::string &description, const std::string &icon) {
	// Surface *bar = sc["skins/" + gmenu2x->confStr["skin"] + "/imgs/topbar.png"];
	// if (bar != NULL) bar->blit(s, 0, 0);
	// else
	s->setClipRect({0, 0, gmenu2x->w, gmenu2x->skinConfInt["sectionBarSize"]});
	s->box(0, 0, gmenu2x->w, gmenu2x->skinConfInt["sectionBarSize"], gmenu2x->skinConfColors[COLOR_TOP_BAR_BG]);

	int iconOffset = 2;

	if (!icon.empty() && gmenu2x->skinConfInt["showDialogIcon"]) { // drawTitleIcon
		iconOffset = gmenu2x->skinConfInt["sectionBarSize"];

		Surface *i = gmenu2x->sc.add(icon, icon + "dialog");

		if (i == NULL) i = gmenu2x->sc["skin:" + icon];
		if (i == NULL) i = gmenu2x->sc["skin:icons/generic.png"];

		gmenu2x->s->setClipRect({4, 4, iconOffset - 8, iconOffset - 8});
		i->blit(s, {4, 4, iconOffset - 8, iconOffset - 8}, HAlignCenter | VAlignMiddle);
		gmenu2x->s->clearClipRect();
	}

	if (!title.empty()) // writeTitle
		s->write(gmenu2x->titlefont, title, iconOffset, 2, VAlignTop, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

	if (!description.empty()) // writeSubTitle
		s->write(gmenu2x->font, description, iconOffset, gmenu2x->skinConfInt["sectionBarSize"] - 2, VAlignBottom, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

	s->clearClipRect();
}

void Dialog::drawBottomBar(Surface *s, buttons_t buttons) {
	// Surface *bar = sc["skins/" + gmenu2x->confStr["skin"] + "/imgs/bottombar.png"];
	// if (bar != NULL) bar->blit(s, 0, gmenu2x->h - bar->raw->h);
	// else
	s->box(gmenu2x->bottomBarRect, gmenu2x->skinConfColors[COLOR_BOTTOM_BAR_BG]);

	int x = gmenu2x->bottomBarRect.x + 5, y = gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2;

	for (const auto &itr: buttons) {
		Surface *btn;
		string path = itr[0];
		if (path.substr(0, 5) != "skin:") {
			path = "skin:imgs/buttons/" + path + ".png";
		}
		btn = gmenu2x->sc[path];

		string txt = itr[1];
		if (btn != NULL) {
			btn->blit(s, x, y, HAlignLeft | VAlignMiddle);
			x += btn->width() + 4;
		}
		if (!txt.empty()) {
			s->write(gmenu2x->font, txt, x, y, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);
			x += gmenu2x->font->getTextWidth(txt);
		}
		x += 6;
	}
}

void Dialog::drawDialog(Surface *s, bool top, bool bottom) {
	if (s == NULL) s = gmenu2x->s;
	string linkBackdrop = "";
	if (gmenu2x->menu->selLink() != NULL && !(gmenu2x->menu->selLink()->getBackdropPath().empty()))
		linkBackdrop = gmenu2x->menu->selLink()->getBackdropPath();
	if (gmenu2x->menu->selLinkApp() != NULL && !(gmenu2x->menu->selLinkApp()->getBackdropPath().empty()))
		linkBackdrop = gmenu2x->menu->selLinkApp()->getBackdropPath();
	if (linkBackdrop != "" && gmenu2x->confInt["skinBackdrops"] & BD_DIALOG)
		gmenu2x->setBackground(s, linkBackdrop);
	else
		gmenu2x->setBackground(s, gmenu2x->confStr["wallpaper"]); // workaround for BD_DIALOG ovewriting bg in Dialog
		//this->bg->blit(s, 0, 0);

	if (top) {
		// Replace '\n' with " "
		string::size_type pos = 0;
		while ((pos = title.find('\n', pos)) != std::string::npos) {
			title.replace(pos, 1, " ");
			pos += 1;
		}
		drawTopBar(s, title, description, icon);
	}
	if (bottom) drawBottomBar(s, buttons);
	s->box(gmenu2x->linksRect, gmenu2x->skinConfColors[COLOR_LIST_BG]);
}
