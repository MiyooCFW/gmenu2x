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

#include "textdialog.h"
#include "messagebox.h"
// #include "debug.h"

using namespace std;

#include <fstream>
#include <sstream>

TextDialog::TextDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon, const string &backdrop):
Dialog(gmenu2x, title, description, icon), backdrop(backdrop) {}

void TextDialog::preProcess() {
	uint32_t i = 0;
	string row;

	split(text, rawText, "\n");

	while (i < text.size()) {
		//clean this row
		row = trim(text.at(i));

		//check if this row is not too long
		if (gmenu2x->font->getTextWidth(row) > gmenu2x->w - 15) {
			vector<string> words;
			split(words, row, " ");

			uint32_t numWords = words.size();
			//find the maximum number of rows that can be printed on screen
			while (gmenu2x->font->getTextWidth(row) > gmenu2x->w - 15 && numWords > 0) {
				numWords--;
				row = "";
				for (uint32_t x = 0; x < numWords; x++)
					row += words[x] + " ";
				row = trim(row);
			}

			//if numWords==0 then the string must be printed as-is, it cannot be split
			if (numWords > 0) {
				//replace with the shorter version
				text.at(i) = row;

				//build the remaining text in another row
				row = "";
				for (uint32_t x = numWords; x < words.size(); x++)
					row += words[x] + " ";
				row = trim(row);

				if (!row.empty())
					text.insert(text.begin() + i + 1, row);
			}
		}
		i++;
	}
}

int TextDialog::drawText(vector<string> *text, int32_t firstCol, int32_t firstRow, uint32_t rowsPerPage) {
	drawDialog(gmenu2x->s);

	gmenu2x->s->setClipRect(gmenu2x->listRect);
	int mx = 0;

	if (firstRow < 0 && text->size() >= rowsPerPage) firstRow = text->size() - rowsPerPage;
	if (firstRow < 0) firstRow = 0;

	int fh = gmenu2x->font->getHeight();

	for (uint32_t i = firstRow; i < firstRow + rowsPerPage && i < text->size(); i++) {
		int y = gmenu2x->listRect.y + (i - firstRow) * fh;
		mx = max(mx, gmenu2x->font->getTextWidth(text->at(i)));

		if (text->at(i) == "----") { // draw a line
			gmenu2x->s->box(5, y - 1 + fh / 2, gmenu2x->w - 10, 1, 255, 255, 255, 130);
			gmenu2x->s->box(5, y + fh / 2, gmenu2x->w - 10, 1, 0, 0, 0, 130);
		} else {
			gmenu2x->font->write(gmenu2x->s, text->at(i), 5 + firstCol, y);
		}
	}

	gmenu2x->s->clearClipRect();
	gmenu2x->drawScrollBar(rowsPerPage, text->size(), firstRow, gmenu2x->listRect);

	gmenu2x->s->flip();

	return mx;
}

void TextDialog::exec() {
	if (gmenu2x->sc[backdrop] != NULL) gmenu2x->sc[backdrop]->blit(this->bg,0,0);

	preProcess();

	bool inputAction = false;
	rowsPerPage = gmenu2x->listRect.h / gmenu2x->font->getHeight();

	if (gmenu2x->sc[this->icon] == NULL)
		this->icon = "skin:icons/ebook.png";

	buttons.push_back({"dpad", gmenu2x->tr["Scroll"]});
	buttons.push_back({"b", gmenu2x->tr["Exit"]});

	drawDialog(gmenu2x->s);

	while (true) {
		if (gmenu2x->confStr["previewMode"] == "Backdrop") {
			if (!backdrop.empty())
				gmenu2x->setBackground(this->bg, backdrop);
			else
				gmenu2x->bg->blit(this->bg,0,0);
		}

		lineWidth = drawText(&text, firstCol, firstRow, rowsPerPage);

		do {
			inputAction = gmenu2x->input.update();
			if (gmenu2x->inputCommonActions(inputAction)) continue;

			if (gmenu2x->input[UP] && firstRow > 0) firstRow--;
			else if (gmenu2x->input[DOWN] && firstRow + rowsPerPage < text.size()) firstRow++;
			else if (gmenu2x->input[RIGHT] && firstCol > -1 * (lineWidth - gmenu2x->listRect.w) - 10) firstCol -= 30;
			else if (gmenu2x->input[LEFT] && firstCol < 0) firstCol += 30;
			else if (gmenu2x->input[PAGEUP] || (gmenu2x->input[LEFT] && firstCol == 0)) {
				if (firstRow >= rowsPerPage - 1)
					firstRow -= rowsPerPage - 1;
				else
					firstRow = 0;
			}
			else if (gmenu2x->input[PAGEDOWN] || (gmenu2x->input[RIGHT] && firstCol == 0)) {
				if (firstRow + rowsPerPage * 2 - 1 < text.size())
					firstRow += rowsPerPage - 1;
				else
					firstRow = max(0, text.size() - rowsPerPage);
			}
			else if (gmenu2x->input[SETTINGS] || gmenu2x->input[CANCEL]) return;
		} while (!inputAction);
	}
}

void TextDialog::appendText(const string &text) {
	this->rawText += text;
}

void TextDialog::appendFile(const string &file) {
	ifstream t(file);
	stringstream buf;
	buf << t.rdbuf();

	this->rawText += buf.str();
}
