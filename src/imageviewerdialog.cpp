#include "imageviewerdialog.h"
#include "debug.h"

ImageViewerDialog::ImageViewerDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon, const string &path)
: Dialog(gmenu2x), title(title), description(description), icon(icon), path(path)
{}

void ImageViewerDialog::exec() {
	Surface image(path);

	bool close = false, inputAction = false;
	int offsetX = 0, offsetY = 0;

	drawTopBar(this->bg, title, description, icon);
	drawBottomBar(this->bg);

	gmenu2x->drawButton(this->bg, "right", gmenu2x->tr["Pan"],
	gmenu2x->drawButton(this->bg, "down", "",
	gmenu2x->drawButton(this->bg, "up", "",
	gmenu2x->drawButton(this->bg, "left", "",
	gmenu2x->drawButton(this->bg, "start", gmenu2x->tr["Exit"],
	5))-12)-14)-12);

	this->bg->blit(gmenu2x->s,0,0);

	while (!close) {
			this->bg->blit(gmenu2x->s, 0, 0);
			gmenu2x->s->setClipRect(gmenu2x->listRect);
			image.blit(gmenu2x->s, gmenu2x->listRect.x + offsetX, gmenu2x->listRect.y + offsetY);
			gmenu2x->s->flip();
			gmenu2x->s->clearClipRect();

		do {
			inputAction = gmenu2x->input.update();
			if (gmenu2x->inputCommonActions(inputAction)) continue;

			if ( gmenu2x->input[MANUAL] || gmenu2x->input[CANCEL] || gmenu2x->input[SETTINGS] ) close = true;
			else if ( gmenu2x->input[LEFT] && offsetX < 0) {
				offsetX += gmenu2x->listRect.w/3;
				if (offsetX > 0) offsetX = 0;
			}
			else if ( gmenu2x->input[RIGHT] && image.raw->w + offsetX > gmenu2x->listRect.w) {
				offsetX -=  gmenu2x->listRect.w/3;
				if (image.raw->w + offsetX < gmenu2x->listRect.w) offsetX = gmenu2x->listRect.w - image.raw->w;
			}
			else if ( gmenu2x->input[UP] && offsetY < 0) {
				offsetY +=  gmenu2x->listRect.h/3;
				if (offsetY > 0) offsetY = 0;
			}
			else if ( gmenu2x->input[DOWN] && image.raw->w + offsetY > gmenu2x->listRect.h) {
				offsetY -=  gmenu2x->listRect.h/3;
				if (image.raw->h + offsetY < gmenu2x->listRect.h) offsetY = gmenu2x->listRect.h - image.raw->h;
			}
		} while (!inputAction);
	}
}
