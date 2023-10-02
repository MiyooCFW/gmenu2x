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
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <math.h>
#include <fstream>

#include "linkapp.h"
#include "menu.h"
#include "debug.h"
#include "messagebox.h"
#include "powermanager.h"
#include "utilities.h"

using namespace std;

Menu::Menu(GMenu2X *gmenu2x):
gmenu2x(gmenu2x) {
	iFirstDispSection = 0;

	readSections();

	setSectionIndex(0);
}

Menu::~Menu() {
	freeLinks();
}

void Menu::readSections() {
	DIR *dirp;
	struct dirent *dptr;

	mkdir("sections", 0777);

	if ((dirp = opendir("sections")) == NULL) {
		return;
	}

	sections.clear();
	links.clear();

	while ((dptr = readdir(dirp))) {
		if (dptr->d_name[0] == '.') {
			continue;
		}

		string filepath = (string)"sections/" + dptr->d_name;
		if (dir_exists(filepath)) {
			sections.push_back((string)dptr->d_name);
			linklist ll;
			links.push_back(ll);
		}
	}

	addSection("settings");
	addSection("applications");

	closedir(dirp);
	sort(sections.begin(),sections.end(), case_less());
}

void Menu::readLinks() {
	vector<string> linkfiles;

	iLink = 0;
	iFirstDispRow = 0;

	DIR *dirp;
	struct dirent *dptr;

	for (uint32_t i = 0; i < links.size(); i++) {
		links[i].clear();
		linkfiles.clear();

		if ((dirp = opendir(sectionPath(i).c_str())) == NULL) {
			continue;
		}

		while ((dptr = readdir(dirp))) {
			if (dptr->d_name[0] == '.') {
				continue;
			}
			string filepath = sectionPath(i) + dptr->d_name;
			if (filepath.substr(filepath.size() - 5, 5) == "-opkg") {
				continue;
			}

			linkfiles.push_back(filepath);
		}

		sort(linkfiles.begin(), linkfiles.end(), case_less());
		for (uint32_t x = 0; x < linkfiles.size(); x++) {
			LinkApp *link = new LinkApp(gmenu2x, linkfiles[x].c_str());
			if (link->targetExists()) {
				links[i].push_back(link);
			} else {
				delete link;
			}
		}

		closedir(dirp);
	}
}

uint32_t Menu::firstDispRow() {
	return iFirstDispRow;
}

// SECTION MANAGEMENT
void Menu::freeLinks() {
	for (vector<linklist>::iterator section = links.begin(); section < links.end(); section++) {
		for (linklist::iterator link = section->begin(); link < section->end(); link++) {
			delete *link;
		}
	}
}

linklist *Menu::sectionLinks(int i) {
	if (i < 0 || i > (int)links.size()) {
		i = selSectionIndex();
	}

	if (i < 0 || i > (int)links.size()) {
		return NULL;
	}

	return &links[i];
}

void Menu::decSectionIndex() {
	setSectionIndex(iSection - 1);
}

void Menu::incSectionIndex() {
	setSectionIndex(iSection + 1);
}

uint32_t Menu::firstDispSection() {
	return iFirstDispSection;
}

int Menu::selSectionIndex() {
	return iSection;
}

const string &Menu::selSection() {
	return sections[iSection];
}

const string Menu::selSectionName() {
	string sectionname = sections[iSection];
	string::size_type pos = sectionname.find(".");

	if (sectionname == "applications") {
		return "apps";
	}

	if (sectionname == "emulators") {
		return "emus";
	}

	if (pos != string::npos && pos > 0 && pos < sectionname.length()) {
		return sectionname.substr(pos + 1);
	}

	return sectionname;
}

int Menu::sectionNumItems() {
	if (gmenu2x->skinConfInt["sectionBar"] == SB_LEFT || gmenu2x->skinConfInt["sectionBar"] == SB_RIGHT) {
		return (gmenu2x->h - 40) / gmenu2x->skinConfInt["sectionBarSize"];
	}

	if (gmenu2x->skinConfInt["sectionBar"] == SB_TOP || gmenu2x->skinConfInt["sectionBar"] == SB_BOTTOM) {
		return (gmenu2x->w - 40) / gmenu2x->skinConfInt["sectionBarSize"];
	}

	return (gmenu2x->w / gmenu2x->skinConfInt["sectionBarSize"]) - 1;
}

void Menu::setSectionIndex(int i) {
	if (i < 0) {
		i = sections.size() - 1;
	} else if (i >= (int)sections.size()) {
		i = 0;
	}

	iSection = i;

	int numRows = sectionNumItems() - 1;

	if (i >= (int)iFirstDispSection + numRows) {
		iFirstDispSection = i - numRows;
	} else if (i < (int)iFirstDispSection) {
		iFirstDispSection = i;
	}

	iLink = 0;
	iFirstDispRow = 0;
}

string Menu::sectionPath(int section) {
	if (section < 0 || section > (int)sections.size()) {
		section = iSection;
	}

	return "sections/" + sections[section] + "/";
}

// LINKS MANAGEMENT
bool Menu::addActionLink(uint32_t section, const string &title, fastdelegate::FastDelegate0<> action, const string &description, const string &icon) {
	if (section >= sections.size()) {
		return false;
	}

	Link *linkact = new Link(gmenu2x, action);
	linkact->setTitle(title);
	linkact->setDescription(description);
	linkact->setIcon("skin:icons/" + icon);
	linkact->setBackdrop("skin:backdrops/" + icon);

	sectionLinks(section)->push_back(linkact);
	return true;
}

bool Menu::addLink(string exec) {
	string section = selSection();
	string title = base_name(exec, true);
	string linkpath = unique_filename("sections/" + section + "/" + title, ".lnk");

	// Reduce title length to fit the link width
	if ((int)gmenu2x->font->getTextWidth(title) > linkWidth) {
		while ((int)gmenu2x->font->getTextWidth(title + "..") > linkWidth) {
			title = title.substr(0, title.length() - 1);
		}
		title += "..";
	}

	INFO("Adding link: '%s'", linkpath.c_str());

	LinkApp *link = new LinkApp(gmenu2x, linkpath.c_str());
	if (!exec.empty()) link->setExec(exec);
	if (!title.empty()) link->setTitle(title);
	link->save();

	int isection = find(sections.begin(), sections.end(), section) - sections.begin();
	if (isection >= 0 && isection < (int)sections.size()) {
		links[isection].push_back(link);
		setLinkIndex(links[isection].size() - 1);
	}

	return true;
}

bool Menu::addSection(const string &sectionName) {
	string sectiondir = "sections/" + sectionName;
	if (mkdir(sectiondir.c_str(), 0777) == 0) {
		sections.push_back(sectionName);
		linklist ll;
		links.push_back(ll);
		return true;
	}
	string tmpfile = sectiondir + "/.section";
	FILE *fp = fopen(tmpfile.c_str(), "wb+");
	if (fp) fclose(fp);
	return false;
}

void Menu::deleteSelectedLink() {
	string iconpath = selLink()->getIconPath();

	INFO("Deleting link '%s'", selLink()->getTitle().c_str());

	if (selLinkApp() != NULL) unlink(selLinkApp()->getFile().c_str());
	sectionLinks()->erase(sectionLinks()->begin() + selLinkIndex());
	setLinkIndex(selLinkIndex());

	for (uint32_t i = 0; i < sections.size(); i++) {
		for (uint32_t j = 0; j < sectionLinks(i)->size(); j++) {
			if (iconpath == sectionLinks(i)->at(j)->getIconPath()) {
				return; // icon in use by another link; return here.
			}
		}
	}

	gmenu2x->sc.del(iconpath);
}

void Menu::deleteSelectedSection() {
	INFO("Deleting section '%s'", selSection().c_str());

	string iconpath = "sections/" + selSection() + ".png";

	links.erase(links.begin() + selSectionIndex());
	sections.erase(sections.begin() + selSectionIndex());
	setSectionIndex(0); //reload sections

	for (uint32_t i = 0; i < sections.size(); i++) {
		if (iconpath == getSectionIcon(i)) {
			return; // icon in use by another section; return here.
		}
	}

	gmenu2x->sc.del(iconpath);
}

bool Menu::linkChangeSection(uint32_t linkIndex, uint32_t oldSectionIndex, uint32_t newSectionIndex) {
	if (oldSectionIndex < sections.size() && newSectionIndex < sections.size() && linkIndex < sectionLinks(oldSectionIndex)->size()) {
		sectionLinks(newSectionIndex)->push_back(sectionLinks(oldSectionIndex)->at(linkIndex));
		sectionLinks(oldSectionIndex)->erase(sectionLinks(oldSectionIndex)->begin() + linkIndex);
		// Select the same link in the new position
		setSectionIndex(newSectionIndex);
		setLinkIndex(sectionLinks(newSectionIndex)->size() - 1);
		return true;
	}
	return false;
}

void Menu::pageUp() {
	// PAGEUP with left
	if ((int)(iLink - linkRows - 1) < 0) {
		setLinkIndex(0);
	} else {
		setLinkIndex(iLink - linkRows + 1);
	}
}

void Menu::pageDown() {
	// PAGEDOWN with right
	if (iLink + linkRows > sectionLinks()->size()) {
		setLinkIndex(sectionLinks()->size() - 1);
	} else {
		setLinkIndex(iLink + linkRows - 1);
	}
}

void Menu::linkLeft() {
	setLinkIndex(iLink - 1);
}

void Menu::linkRight() {
	setLinkIndex(iLink + 1);
}

void Menu::linkUp() {
	int l = iLink - linkCols;
	if (l < 0) {
		uint32_t rows = (uint32_t)ceil(sectionLinks()->size() / (double)linkCols);
		l += (rows * linkCols);
		if (l >= (int)sectionLinks()->size()) {
			l -= linkCols;
		}
	}
	setLinkIndex(l);
}

void Menu::linkDown() {
	uint32_t l = iLink + linkCols;
	if (l >= sectionLinks()->size()) {
		uint32_t rows = (uint32_t)ceil(sectionLinks()->size() / (double)linkCols);
		uint32_t curCol = (uint32_t)ceil((iLink+1) / (double)linkCols);
		if (rows > curCol) {
			l = sectionLinks()->size() - 1;
		} else {
			l %= linkCols;
		}
	}
	setLinkIndex(l);
}

int Menu::selLinkIndex() {
	return iLink;
}

Link *Menu::selLink() {
	if (sectionLinks()->size() == 0) {
		return NULL;
	}

	return sectionLinks()->at(iLink);
}

LinkApp *Menu::selLinkApp() {
	return dynamic_cast<LinkApp*>(selLink());
}

void Menu::setLinkIndex(int i) {
	if (i < 0) {
		i = sectionLinks()->size() - 1;
	} else if (i >= (int)sectionLinks()->size()) {
		i = 0;
	}

	int perPage = linkCols * linkRows;
	if (linkRows == 1) {
		if (i < iFirstDispRow) {
			iFirstDispRow = i;
		} else if (i >= iFirstDispRow + perPage) {
			iFirstDispRow = i - perPage + 1;
		}
	} else {
		int page = i / linkCols;
		if (i < iFirstDispRow) {
			iFirstDispRow = page * linkCols;
		} else if (i >= iFirstDispRow + perPage) {
			iFirstDispRow = page * linkCols - linkCols * (linkRows - 1);
		}
	}

	if (iFirstDispRow < 0) {
		iFirstDispRow = 0;
	}

	iLink = i;
}

void Menu::renameSection(int index, const string &name) {
	// section directory doesn't exists
	string oldsection = "sections/" + selSection();
	string newsection = "sections/" + name;

	if (oldsection != newsection && rename(oldsection.c_str(), newsection.c_str()) == 0) {
		sections[index] = name;
	}

}

int Menu::getSectionIndex(const string &name) {
	return distance(sections.begin(), find(sections.begin(), sections.end(), name));
}

const string Menu::getSectionIcon(int i) {
	if (i < 0) i = iSection;

	string sectionIcon = gmenu2x->sc.getSkinFilePath("sections/" + sections[i] + ".png", false);
	if (!sectionIcon.empty()) {
		return sectionIcon;
	}

	string::size_type pos = sections[i].rfind(".");
	if (pos != string::npos) {
		string subsectionIcon = gmenu2x->sc.getSkinFilePath("sections/" + sections[i].substr(pos) + ".png", false);
		if (!subsectionIcon.empty()) {
			return subsectionIcon;
		}
	}

	pos = sections[i].find(".");
	if (pos != string::npos) {
		string mainsectionIcon = gmenu2x->sc.getSkinFilePath("sections/" + sections[i].substr(0, pos) + ".png", false);
		if (!mainsectionIcon.empty()) {
			return mainsectionIcon;
		}
	}

	return gmenu2x->sc.getSkinFilePath("icons/section.png");
}

void Menu::initLayout() {
	// LINKS rect
	gmenu2x->linksRect = (SDL_Rect){0, 0, gmenu2x->w, gmenu2x->h};
	gmenu2x->sectionBarRect = (SDL_Rect){0, 0, gmenu2x->w, gmenu2x->h};

	if (gmenu2x->skinConfInt["sectionBar"]) {
		if (gmenu2x->skinConfInt["sectionBar"] == SB_LEFT || gmenu2x->skinConfInt["sectionBar"] == SB_RIGHT) {
			gmenu2x->sectionBarRect.x = (gmenu2x->skinConfInt["sectionBar"] == SB_RIGHT) * (gmenu2x->w - gmenu2x->skinConfInt["sectionBarSize"]);
			gmenu2x->sectionBarRect.w = gmenu2x->skinConfInt["sectionBarSize"];
			gmenu2x->linksRect.w = gmenu2x->w - gmenu2x->skinConfInt["sectionBarSize"];

			if (gmenu2x->skinConfInt["sectionBar"] == SB_LEFT) {
				gmenu2x->linksRect.x = gmenu2x->skinConfInt["sectionBarSize"];
			}
		} else {
			gmenu2x->sectionBarRect.y = (gmenu2x->skinConfInt["sectionBar"] == SB_BOTTOM) * (gmenu2x->h - gmenu2x->skinConfInt["sectionBarSize"]);
			gmenu2x->sectionBarRect.h = gmenu2x->skinConfInt["sectionBarSize"];
			gmenu2x->linksRect.h = gmenu2x->h - gmenu2x->skinConfInt["sectionBarSize"];

			if (gmenu2x->skinConfInt["sectionBar"] == SB_TOP || gmenu2x->skinConfInt["sectionBar"] == SB_CLASSIC) {
				gmenu2x->linksRect.y = gmenu2x->skinConfInt["sectionBarSize"];
			}
			if (gmenu2x->skinConfInt["sectionBar"] == SB_CLASSIC) {
				gmenu2x->linksRect.h -= gmenu2x->skinConfInt["bottomBarHeight"];
			}
		}
	}

	gmenu2x->listRect = (SDL_Rect){0, gmenu2x->skinConfInt["sectionBarSize"], gmenu2x->w, gmenu2x->h - gmenu2x->skinConfInt["bottomBarHeight"] - gmenu2x->skinConfInt["sectionBarSize"]};
	gmenu2x->bottomBarRect = (SDL_Rect){0, gmenu2x->h - gmenu2x->skinConfInt["bottomBarHeight"], gmenu2x->w, gmenu2x->skinConfInt["bottomBarHeight"]};

	// WIP
	linkCols = gmenu2x->skinConfInt["linkCols"];
	linkRows = gmenu2x->skinConfInt["linkRows"];

	linkWidth  = (gmenu2x->linksRect.w - (linkCols + 1) * linkSpacing) / linkCols;
	linkHeight = (gmenu2x->linksRect.h - (linkCols > 1) * (linkRows + 1) * linkSpacing) / linkRows;
}

void Menu::drawList() {
	int i = firstDispRow();

	int ix = gmenu2x->linksRect.x;
	for (int y = 0; y < linkRows && i < sectionLinks()->size(); y++, i++) {
		int iy = gmenu2x->linksRect.y + y * linkHeight;

		if (i == (uint32_t)selLinkIndex()) {
			gmenu2x->s->box(ix, iy, gmenu2x->linksRect.w, linkHeight, gmenu2x->skinConfColors[COLOR_SELECTION_BG]);
		}

		Surface *icon = gmenu2x->sc[sectionLinks()->at(i)->getIconPath()];
		if (icon == NULL) {
			icon = gmenu2x->sc["skin:icons/generic.png"];
		}

		if (icon->width() > 32 || icon->height() > linkHeight - 4) {
			icon->softStretch(32, linkHeight - 4, SScaleFit);
		}

		icon->blit(gmenu2x->s, {ix + 2, iy + 2, 32, linkHeight - 4}, HAlignCenter | VAlignMiddle);
		gmenu2x->s->write(gmenu2x->titlefont, gmenu2x->tr[sectionLinks()->at(i)->getTitle()], ix + linkSpacing + 36, iy + gmenu2x->titlefont->getHeight()/2, VAlignMiddle);
		gmenu2x->s->write(gmenu2x->font, gmenu2x->tr[sectionLinks()->at(i)->getDescription()], ix + linkSpacing + 36, iy + linkHeight - linkSpacing/2, VAlignBottom);
	}

	if (sectionLinks()->size() > linkRows) {
		gmenu2x->drawScrollBar(1, sectionLinks()->size(), selLinkIndex(), gmenu2x->linksRect, HAlignRight);
	}
}

void Menu::drawGrid() {
	int i = firstDispRow();

	for (int y = 0; y < linkRows; y++) {
		for (int x = 0; x < linkCols && i < sectionLinks()->size(); x++, i++) {
			int ix = gmenu2x->linksRect.x + x * linkWidth  + (x + 1) * linkSpacing;
			int iy = gmenu2x->linksRect.y + y * linkHeight + (y + 1) * linkSpacing;

			Surface *icon = gmenu2x->sc[sectionLinks()->at(i)->getIconPath()];
			if (icon == NULL) {
				icon = gmenu2x->sc["skin:icons/generic.png"];
			}

			if (icon->width() > linkWidth || icon->height() > linkHeight) {
				icon->softStretch(linkWidth, linkHeight, SScaleFit);
			}

			if (i == (uint32_t)selLinkIndex()) {
				if (iconBGon != NULL && icon->width() <= iconBGon->width() && icon->height() <= iconBGon->height()) {
					iconBGon->blit(gmenu2x->s, ix + (linkWidth + iconPadding) / 2, iy + (linkHeight + iconPadding) / 2, HAlignCenter | VAlignMiddle, 50);
				} else {
					gmenu2x->s->box(ix + (linkWidth - min(linkWidth, icon->width())) / 2 - 4, iy + (linkHeight - min(linkHeight, icon->height())) / 2 - 4, min(linkWidth, icon->width()) + 8, min(linkHeight, icon->height()) + 8, gmenu2x->skinConfColors[COLOR_SELECTION_BG]);
				}

			} else if (iconBGoff != NULL && icon->width() <= iconBGoff->width() && icon->height() <= iconBGoff->height()) {
				iconBGoff->blit(gmenu2x->s, {ix + iconPadding/2, iy + iconPadding/2, linkWidth - iconPadding, linkHeight - iconPadding}, HAlignCenter | VAlignMiddle);
			}

			icon->blit(gmenu2x->s, {ix + iconPadding/2, iy + iconPadding/2, linkWidth - iconPadding, linkHeight - iconPadding}, HAlignCenter | VAlignMiddle);

			if (gmenu2x->skinConfInt["linkLabel"]) {
				SDL_Rect labelRect;
				labelRect.x = ix + 2 + linkWidth/2;
				labelRect.y = iy + (linkHeight + min(linkHeight, icon->height()))/2;
				labelRect.w = linkWidth - iconPadding;
				labelRect.h = linkHeight - iconPadding;
				gmenu2x->s->write(gmenu2x->font, gmenu2x->tr[sectionLinks()->at(i)->getTitle()], labelRect, HAlignCenter | VAlignMiddle);
			}
		}
	}

	if (linkRows == 1 && sectionLinks()->size() > linkCols) {
		gmenu2x->drawScrollBar(1, sectionLinks()->size(), selLinkIndex(), gmenu2x->linksRect, VAlignBottom);
	} else if (sectionLinks()->size() > linkCols * linkRows) {
		gmenu2x->drawScrollBar(1, sectionLinks()->size()/linkCols + 1, selLinkIndex()/linkCols, gmenu2x->linksRect, HAlignRight);
	}
}

void Menu::drawSectionBar() {
	gmenu2x->s->box(gmenu2x->sectionBarRect, gmenu2x->skinConfColors[COLOR_TOP_BAR_BG]);

	int ix = 0, iy = 0, sy = 0;
	int x = gmenu2x->sectionBarRect.x;
	int y = gmenu2x->sectionBarRect.y;
	int sx = (selSectionIndex() - firstDispSection()) * gmenu2x->skinConfInt["sectionBarSize"];

	if (gmenu2x->skinConfInt["sectionBar"] == SB_CLASSIC) {
		ix = (gmenu2x->w - gmenu2x->skinConfInt["sectionBarSize"] * min(sectionNumItems(), getSections().size())) / 2;
	}

	for (int i = firstDispSection(); i < getSections().size() && i < firstDispSection() + sectionNumItems(); i++) {
		if (gmenu2x->skinConfInt["sectionBar"] == SB_LEFT || gmenu2x->skinConfInt["sectionBar"] == SB_RIGHT) {
			y = (i - firstDispSection()) * gmenu2x->skinConfInt["sectionBarSize"];
		} else {
			x = (i - firstDispSection()) * gmenu2x->skinConfInt["sectionBarSize"] + ix;
		}

		if (selSectionIndex() == (int)i) {
			sx = x;
			sy = y;
			gmenu2x->s->box(sx, sy, gmenu2x->skinConfInt["sectionBarSize"], gmenu2x->skinConfInt["sectionBarSize"], gmenu2x->skinConfColors[COLOR_SELECTION_BG]);
		}

		gmenu2x->sc[getSectionIcon(i)]->blit(gmenu2x->s, {x, y, gmenu2x->skinConfInt["sectionBarSize"], gmenu2x->skinConfInt["sectionBarSize"]}, HAlignCenter | VAlignMiddle);
	}

	if (gmenu2x->skinConfInt["sectionLabel"] && SDL_GetTicks() - section_changed < 1400) {
		if (gmenu2x->skinConfInt["sectionBar"] == SB_LEFT) {
			gmenu2x->s->write(gmenu2x->font, gmenu2x->tr[selSectionName()], sx, sy + gmenu2x->skinConfInt["sectionBarSize"], HAlignLeft | VAlignBottom);
		} else if (gmenu2x->skinConfInt["sectionBar"] == SB_RIGHT) {
			gmenu2x->s->write(gmenu2x->font, gmenu2x->tr[selSectionName()], sx + gmenu2x->skinConfInt["sectionBarSize"], sy + gmenu2x->skinConfInt["sectionBarSize"], HAlignRight | VAlignBottom);
		} else {
			gmenu2x->s->write(gmenu2x->font, gmenu2x->tr[selSectionName()], sx + gmenu2x->skinConfInt["sectionBarSize"] / 2 , sy + gmenu2x->skinConfInt["sectionBarSize"], HAlignCenter | VAlignBottom);
		}
	} else {
		SDL_RemoveTimer(sectionChangedTimer); sectionChangedTimer = NULL;
	}

	if (gmenu2x->skinConfInt["sectionBar"] == SB_CLASSIC) {
		if (iconL != NULL) iconL->blit(gmenu2x->s, 0, 0, HAlignLeft | VAlignTop);
		if (iconR != NULL) iconR->blit(gmenu2x->s, gmenu2x->w, 0, HAlignRight | VAlignTop);
	}
}

void Menu::drawStatusBar() {
	int iconTrayShift = 0;

	const int iconWidth = 16, pctWidth = gmenu2x->font->getTextWidth("100");
	char buf[32]; int x = 0;

	gmenu2x->s->box(gmenu2x->bottomBarRect, gmenu2x->skinConfColors[COLOR_BOTTOM_BAR_BG]);

	if (!iconDescription.empty() && SDL_GetTicks() - icon_changed < 300) {
		x = iconPadding;
		iconManual->blit(gmenu2x->s, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle);
		x += iconWidth + iconPadding;
		gmenu2x->s->write(gmenu2x->font, iconDescription.c_str(), x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);
	} else {
		SDL_RemoveTimer(iconChangedTimer); iconChangedTimer = NULL;

		// Volume indicator
		// TODO: use drawButton(gmenu2x->s, iconVolume[volumeMode], confInt["globalVolume"], x);
#if defined(HW_LIDVOL)
		{ stringstream ss; ss << gmenu2x->getVolume() /*<< "%"*/; ss.get(&buf[0], sizeof(buf)); }
#else
		{ stringstream ss; ss << gmenu2x->confInt["globalVolume"] /*<< "%"*/; ss.get(&buf[0], sizeof(buf)); }
#endif
		x = iconPadding; // 1 * (iconWidth + 2 * iconPadding) + iconPadding + 1 * pctWidth;
		iconVolume[volumeMode]->blit(gmenu2x->s, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle);
		x += iconWidth + iconPadding;
		gmenu2x->s->write(gmenu2x->font, buf, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

		// Brightness indicator
#if defined(HW_LIDVOL)
		{ stringstream ss; ss << gmenu2x->getBacklight() /*<< "%"*/; ss.get(&buf[0], sizeof(buf)); }
#else
		{ stringstream ss; ss << gmenu2x->confInt["backlight"] /*<< "%"*/; ss.get(&buf[0], sizeof(buf)); }
#endif
		x += iconPadding + pctWidth;
		iconBrightness[brightnessIcon]->blit(gmenu2x->s, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle);
		x += iconWidth + iconPadding;
		gmenu2x->s->write(gmenu2x->font, buf, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);

		// // Menu indicator
		// iconMenu->blit(gmenu2x->s, iconPadding, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle);
		// sc["skin:imgs/debug.png"]->blit(gmenu2x->s, gmenu2x->bottomBarRect.w - iconTrayShift * (iconWidth + iconPadding) - iconPadding, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, HAlignRight | VAlignMiddle);

		// Battery indicator
		iconBattery[batteryIcon]->blit(gmenu2x->s, gmenu2x->bottomBarRect.w - iconTrayShift * (iconWidth + iconPadding) - iconPadding, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, HAlignRight | VAlignMiddle);
		iconTrayShift++;

		// SD Card indicator
		if (mmcStatus == MMC_INSERT) {
			iconSD->blit(gmenu2x->s, gmenu2x->bottomBarRect.w - iconTrayShift * (iconWidth + iconPadding) - iconPadding, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, HAlignRight | VAlignMiddle);
			iconTrayShift++;
		}

		// Network indicator
		if (gmenu2x->iconInet != NULL) {
			gmenu2x->iconInet->blit(gmenu2x->s, gmenu2x->bottomBarRect.w - iconTrayShift * (iconWidth + iconPadding) - iconPadding, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, HAlignRight | VAlignMiddle);
			iconTrayShift++;
		}

		if (selLink() != NULL) {
			if (selLinkApp() != NULL) {
				if (!selLinkApp()->getManualPath().empty()) {
					// Manual indicator
					iconManual->blit(gmenu2x->s, gmenu2x->bottomBarRect.w - iconTrayShift * (iconWidth + iconPadding) - iconPadding, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, HAlignRight | VAlignMiddle);
				}

				if (CPU_MAX != CPU_MIN) {
					// CPU indicator
					{ stringstream ss; ss << selLinkApp()->getCPU() << "MHz"; ss.get(&buf[0], sizeof(buf)); }
					x += iconPadding + pctWidth;
					iconCPU->blit(gmenu2x->s, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle);
					x += iconWidth + iconPadding;
					gmenu2x->s->write(gmenu2x->font, buf, x, gmenu2x->bottomBarRect.y + gmenu2x->bottomBarRect.h / 2, VAlignMiddle, gmenu2x->skinConfColors[COLOR_FONT_ALT], gmenu2x->skinConfColors[COLOR_FONT_ALT_OUTLINE]);
				}
			}
		}
	}
}

void Menu::drawIconTray() {
	int iconTrayShift = 0;

	// TRAY DEBUG
	// s->box(sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + 0 * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18,16,16, strtorgba("ffff00ff"));
	// s->box(sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + 1 * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18,16,16, strtorgba("00ff00ff"));
	// s->box(sectionBarRect.x + gmenu2x->sectionBarRect.w - 38, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 38,16,16, strtorgba("0000ffff"));
	// s->box(sectionBarRect.x + gmenu2x->sectionBarRect.w - 18, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 38,16,16, strtorgba("ff00ffff"));

	// TRAY 0,0
	iconVolume[volumeMode]->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 38, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 38);

	// TRAY 1,0
	iconBattery[batteryIcon]->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 18, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 38);

	// TRAY iconTrayShift,1
	if (mmcStatus == MMC_INSERT) {
		iconSD->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + iconTrayShift * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18);
		iconTrayShift++;
	}

	if (selLink() != NULL) {
		if (selLinkApp() != NULL) {
			if (!selLinkApp()->getManualPath().empty() && iconTrayShift < 2) {
				// Manual indicator
				iconManual->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + iconTrayShift * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18);
				iconTrayShift++;
			}

			if (CPU_MAX != CPU_MIN) {
				if (selLinkApp()->getCPU() != gmenu2x->confInt["cpuMenu"] && iconTrayShift < 2) {
					// CPU indicator
					iconCPU->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + iconTrayShift * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18);
					iconTrayShift++;
				}
			}
		}
	}

	if (iconTrayShift < 2) {
		brightnessIcon = gmenu2x->confInt["backlight"] / 20;
		if (brightnessIcon > 4 || iconBrightness[brightnessIcon] == NULL) {
			brightnessIcon = 5;
		}
		iconBrightness[brightnessIcon]->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + iconTrayShift * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18);
		iconTrayShift++;
	}

	if (iconTrayShift < 2) {
		// Menu indicator
		iconMenu->blit(gmenu2x->s, gmenu2x->sectionBarRect.x + gmenu2x->sectionBarRect.w - 38 + iconTrayShift * 20, gmenu2x->sectionBarRect.y + gmenu2x->sectionBarRect.h - 18);
		iconTrayShift++;
	}
}

void Menu::exec() {
	icon_changed = SDL_GetTicks();
	section_changed = icon_changed;

	sectionChangedTimer = SDL_AddTimer(2000, gmenu2x->input.wakeUp, (void*)false);
	iconChangedTimer = SDL_AddTimer(1000, gmenu2x->input.wakeUp, (void*)false);

	iconBrightness[0] = gmenu2x->sc["skin:imgs/brightness/0.png"],
	iconBrightness[1] = gmenu2x->sc["skin:imgs/brightness/1.png"],
	iconBrightness[2] = gmenu2x->sc["skin:imgs/brightness/2.png"],
	iconBrightness[3] = gmenu2x->sc["skin:imgs/brightness/3.png"],
	iconBrightness[4] = gmenu2x->sc["skin:imgs/brightness/4.png"],
	iconBrightness[5] = gmenu2x->sc["skin:imgs/brightness.png"],

	iconBattery[0] = gmenu2x->sc["skin:imgs/battery/0.png"],
	iconBattery[1] = gmenu2x->sc["skin:imgs/battery/1.png"],
	iconBattery[2] = gmenu2x->sc["skin:imgs/battery/2.png"],
	iconBattery[3] = gmenu2x->sc["skin:imgs/battery/3.png"],
	iconBattery[4] = gmenu2x->sc["skin:imgs/battery/4.png"],
	iconBattery[5] = gmenu2x->sc["skin:imgs/battery/5.png"],
	iconBattery[6] = gmenu2x->sc["skin:imgs/battery/ac.png"],

	iconVolume[0] = gmenu2x->sc["skin:imgs/mute.png"],
	iconVolume[1] = gmenu2x->sc["skin:imgs/phones.png"],
	iconVolume[2] = gmenu2x->sc["skin:imgs/volume.png"],

	iconSD = gmenu2x->sc["skin:imgs/sd.png"];
	iconManual = gmenu2x->sc["skin:imgs/manual.png"];
	iconCPU = gmenu2x->sc["skin:imgs/cpu.png"];
	iconMenu = gmenu2x->sc["skin:imgs/menu.png"];
	iconL = gmenu2x->sc["skins/" + gmenu2x->confStr["skin"] + "/imgs/section-l.png"];
	iconR = gmenu2x->sc["skins/" + gmenu2x->confStr["skin"] + "/imgs/section-r.png"];
	iconBGoff = gmenu2x->sc["skins/" + gmenu2x->confStr["skin"] + "/imgs/iconbg_off.png"];
	iconBGon = gmenu2x->sc["skins/" + gmenu2x->confStr["skin"] + "/imgs/iconbg_on.png"];

	while (true) {
		// BACKGROUND
		gmenu2x->currBackdrop = gmenu2x->confStr["wallpaper"];
		if (gmenu2x->confInt["skinBackdrops"] & BD_MENU) {
			if (selLink() != NULL && !selLink()->getBackdropPath().empty()) {
				gmenu2x->currBackdrop = selLink()->getBackdropPath();
			} else if (selLinkApp() != NULL && !selLinkApp()->getBackdropPath().empty()) {
				gmenu2x->currBackdrop = selLinkApp()->getBackdropPath();
			}
		}
		gmenu2x->setBackground(gmenu2x->s, gmenu2x->currBackdrop);

		// SECTIONS
		if (gmenu2x->skinConfInt["sectionBar"]) {
			drawSectionBar();

			brightnessIcon = gmenu2x->confInt["backlight"] / 20;
			if (brightnessIcon > 4 || iconBrightness[brightnessIcon] == NULL) {
				brightnessIcon = 5;
			}

			if (gmenu2x->skinConfInt["sectionBar"] == SB_CLASSIC) {
				drawStatusBar();
			} else {
				drawIconTray();
			}
		}

		// LINKS
		gmenu2x->s->setClipRect(gmenu2x->linksRect);
		gmenu2x->s->box(gmenu2x->linksRect, gmenu2x->skinConfColors[COLOR_LIST_BG]);
		if (linkCols == 1 && linkRows > 1) {
			drawList(); // LIST
		} else {
			drawGrid(); // CLASSIC
		}
		gmenu2x->s->clearClipRect();

		if (!sectionLinks()->size()) {
			MessageBox mb(gmenu2x, gmenu2x->tr["This section is empty"]);
			mb.setAutoHide(1);
			mb.setBgAlpha(0);
			mb.exec();
		}

		if (!gmenu2x->powerManager->suspendActive && !gmenu2x->input.combo()) {
			gmenu2x->s->flip();
		}

		bool inputAction = gmenu2x->input.update();

		if (gmenu2x->input.combo()) {
			gmenu2x->skinConfInt["sectionBar"] = ((gmenu2x->skinConfInt["sectionBar"]) % 5) + 1;
			initLayout();
			MessageBox mb(gmenu2x, gmenu2x->tr["CHEATER! ;)"]);
			mb.setBgAlpha(0);
			mb.setAutoHide(200);
			mb.exec();
			gmenu2x->input.dropEvents();
			SDL_AddTimer(350, gmenu2x->input.wakeUp, (void*)false);
		} else if (!gmenu2x->powerManager->suspendActive) {
			gmenu2x->s->flip();
		}

		if (gmenu2x->inputCommonActions(inputAction)) {
			continue;
		}

		if (gmenu2x->input[CANCEL] || gmenu2x->input[CONFIRM] || gmenu2x->input[SETTINGS] || gmenu2x->input[MENU]) {
			SDL_RemoveTimer(sectionChangedTimer); sectionChangedTimer = NULL;
			SDL_RemoveTimer(iconChangedTimer); iconChangedTimer = NULL;
			icon_changed = section_changed = 0;
		}

		if (gmenu2x->input[CONFIRM] && selLink() != NULL) {
			if (gmenu2x->confInt["skinBackdrops"] & BD_DIALOG) {
				gmenu2x->setBackground(gmenu2x->bg, gmenu2x->currBackdrop);
			} else {
				gmenu2x->setBackground(gmenu2x->bg, gmenu2x->confStr["wallpaper"]);
			}

			selLink()->run();
		}
		else if (gmenu2x->input[CANCEL])	continue;
		else if (gmenu2x->input[SETTINGS] && !(gmenu2x->actionPerformed))	gmenu2x->settings();
		else if (gmenu2x->input[MENU])		gmenu2x->contextMenu();

		// LINK NAVIGATION
		else if (gmenu2x->input[LEFT]  && linkCols == 1 && linkRows > 1) pageUp();
		else if (gmenu2x->input[RIGHT] && linkCols == 1 && linkRows > 1) pageDown();
		else if (gmenu2x->input[LEFT])	linkLeft();
		else if (gmenu2x->input[RIGHT])	linkRight();
		else if (gmenu2x->input[UP])	linkUp();
		else if (gmenu2x->input[DOWN])	linkDown();

		// SECTION
		else if (gmenu2x->input[SECTION_PREV]) decSectionIndex();
		else if (gmenu2x->input[SECTION_NEXT]) incSectionIndex();

		// SELLINKAPP SELECTED
		else if (gmenu2x->input[MANUAL] && selLinkApp() != NULL && !selLinkApp()->getManualPath().empty()) gmenu2x->showManual();
		// On Screen Help
		// else if (gmenu2x->input[MANUAL]) {
		// 	s->box(10,50,300,162, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BG]);
		// 	s->rectangle(12,52,296,158, gmenu2x->skinConfColors[COLOR_MESSAGE_BOX_BORDER]);
		// 	int line = 60; s->write(gmenu2x->font, gmenu2x->tr["CONTROLS"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["A: Select"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["B: Cancel"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["Y: Show manual"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["L, R: Change section"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["Start: Settings"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["Select: Menu"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["Select+Start: Save screenshot"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["Select+L: Adjust volume level"], 20, line);
		// 	line += font->getHeight() + 5; s->write(gmenu2x->font, gmenu2x->tr["Select+R: Adjust backlight level"], 20, line);
		// 	s->flip();
		// 	bool close = false;
		// 	while (!close) {
		// 		gmenu2x->input.update();
		// 		if (gmenu2x->input[MODIFIER] || gmenu2x->input[CONFIRM] || gmenu2x->input[CANCEL]) close = true;
		// 	}
		// }

		iconDescription = "";
		if (selLinkApp() != NULL) {
			iconDescription = selLinkApp()->getDescription();
			browsedialog->allyTTS(selLinkApp()->getTitle().c_str(), FAST_GAP_TTS, FAST_SPEED_TTS);
		} else if (selLink() != NULL) {
			iconDescription = selLink()->getDescription();
			browsedialog->allyTTS(selLink()->getDescription().c_str(), MEDIUM_GAP_TTS, MEDIUM_SPEED_TTS);
		}

		if (
			!iconDescription.empty() &&
			(gmenu2x->input[LEFT] || gmenu2x->input[RIGHT] || gmenu2x->input[LEFT] || gmenu2x->input[RIGHT] || gmenu2x->input[UP] || gmenu2x->input[DOWN] || gmenu2x->input[SECTION_PREV] || gmenu2x->input[SECTION_NEXT])
		) {
			icon_changed = SDL_GetTicks();
			SDL_RemoveTimer(iconChangedTimer); iconChangedTimer = NULL;
			iconChangedTimer = SDL_AddTimer(1000, gmenu2x->input.wakeUp, (void*)false);
		}

		if (gmenu2x->skinConfInt["sectionLabel"] && (gmenu2x->input[SECTION_PREV] || gmenu2x->input[SECTION_NEXT])) {
			section_changed = SDL_GetTicks();
			SDL_RemoveTimer(sectionChangedTimer); sectionChangedTimer = NULL;
			sectionChangedTimer = SDL_AddTimer(2000, gmenu2x->input.wakeUp, (void*)false);
		}
	}
}
