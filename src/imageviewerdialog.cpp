#include "imageviewerdialog.h"
#include "debug.h"

ImageViewerDialog::ImageViewerDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon, const string &path):
Dialog(gmenu2x, title, description, icon), path(path) {}

void ImageViewerDialog::exec() {
	Surface image(path);

	bool inputAction = false;
	int offsetX = 0, offsetY = 0;

	buttons.push_back({"dpad", gmenu2x->tr["Pan"]});
	buttons.push_back({"b", gmenu2x->tr["Exit"]});
	drawDialog(this->bg);

	while (true) {
		this->bg->blit(gmenu2x->s, 0, 0);
		gmenu2x->s->setClipRect(gmenu2x->listRect);
		image.blit(gmenu2x->s, gmenu2x->listRect.x + offsetX, gmenu2x->listRect.y + offsetY);
		gmenu2x->s->flip();
		gmenu2x->s->clearClipRect();

		do {
			inputAction = gmenu2x->input.update();
			if (gmenu2x->inputCommonActions(inputAction)) continue;

			if (gmenu2x->input[CANCEL] || gmenu2x->input[SETTINGS]) return;
			else if (gmenu2x->input[LEFT] && offsetX < 0) {
				offsetX += gmenu2x->listRect.w / 3;
				if (offsetX > 0) offsetX = 0;
			}
			else if (gmenu2x->input[RIGHT] && image.raw->w + offsetX > gmenu2x->listRect.w) {
				offsetX -=  gmenu2x->listRect.w / 3;
				if (image.raw->w + offsetX < gmenu2x->listRect.w) offsetX = gmenu2x->listRect.w - image.raw->w;
			}
			else if (gmenu2x->input[UP] && offsetY < 0) {
				offsetY +=  gmenu2x->listRect.h / 3;
				if (offsetY > 0) offsetY = 0;
			}
			else if (gmenu2x->input[DOWN] && image.raw->w + offsetY > gmenu2x->listRect.h) {
				offsetY -=  gmenu2x->listRect.h / 3;
				if (image.raw->h + offsetY < gmenu2x->listRect.h) offsetY = gmenu2x->listRect.h - image.raw->h;
			}
		} while (!inputAction);
	}
}
