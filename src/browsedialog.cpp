#include "browsedialog.h"
#include "FastDelegate.h"
// #include "debug.h"
#include <algorithm>

using namespace fastdelegate;
using namespace std;

BrowseDialog::BrowseDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon)
: Dialog(gmenu2x), title(title), description(description), icon(icon) {
	fl = new FileLister(CARD_ROOT, true, false);
}

BrowseDialog::~BrowseDialog() {
	delete fl;
}

bool BrowseDialog::exec() {
	if (!fl) return false;

	this->bg = new Surface(gmenu2x->bg); // needed to redraw on child screen return

	Surface *iconGoUp = gmenu2x->sc.skinRes("imgs/go-up.png");
	Surface *iconFolder = gmenu2x->sc.skinRes("imgs/folder.png");
	Surface *iconFile = gmenu2x->sc.skinRes("imgs/file.png");

	string path = fl->getPath();
	if (path.empty() || !dirExists(path))
		setPath(CARD_ROOT);

	fl->browse();

	selected = 0;
	close = false;
	bool inputAction = false;

	uint32_t i, iY, firstElement = 0, animation = 0, padding = 6;
	uint32_t rowHeight = gmenu2x->font->getHeight() + 1;
	uint32_t numRows = (gmenu2x->listRect.h - 2)/rowHeight - 1;

	drawTopBar(this->bg, title, description, icon);
	drawBottomBar(this->bg);
	this->bg->box(gmenu2x->listRect, gmenu2x->skinConfColors[COLOR_LIST_BG]);

	if (!showFiles && allowSelectDirectory) {
		gmenu2x->drawButton(this->bg, "a", gmenu2x->tr["Select"]);
	} else {
		gmenu2x->drawButton(this->bg, "x", gmenu2x->tr["Folder up"],
		gmenu2x->drawButton(this->bg, "a", gmenu2x->tr["Select"],
		gmenu2x->drawButton(this->bg, "b", gmenu2x->tr["Cancel"], 5)));
	}

	uint32_t tickStart = SDL_GetTicks();
	while (!close) {
		this->bg->blit(gmenu2x->s,0,0);
		// buttonBox.paint(5);

		//Selection
		if (selected >= firstElement + numRows) firstElement = selected - numRows;
		if (selected < firstElement) firstElement = selected;

		//Files & Directories
		iY = gmenu2x->listRect.y + 1;
		for (i = firstElement; i < fl->size() && i <= firstElement + numRows; i++, iY += rowHeight) {
			if (i == selected) gmenu2x->s->box(gmenu2x->listRect.x, iY, gmenu2x->listRect.w, rowHeight, gmenu2x->skinConfColors[COLOR_SELECTION_BG]);
			if (fl->isDirectory(i)) {
				if ((*fl)[i] == "..")
					iconGoUp->blit(gmenu2x->s, gmenu2x->listRect.x + 10, iY + rowHeight/2, HAlignCenter | VAlignMiddle);
				else
					iconFolder->blit(gmenu2x->s, gmenu2x->listRect.x + 10, iY + rowHeight/2, HAlignCenter | VAlignMiddle);
			} else {
				iconFile->blit(gmenu2x->s, gmenu2x->listRect.x + 10, iY + rowHeight/2, HAlignCenter | VAlignMiddle);
			}
			gmenu2x->s->write(gmenu2x->font, (*fl)[i], gmenu2x->listRect.x + 21, iY + rowHeight/2, VAlignMiddle);
		}

		// preview
		string filename = fl->getPath() + "/" + getFile();
		string ext = getExt();

		if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif") {
			gmenu2x->s->box(320 - animation, gmenu2x->listRect.y, gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h, gmenu2x->skinConfColors[COLOR_TOP_BAR_BG]);

			gmenu2x->sc[filename]->softStretch(gmenu2x->skinConfInt["previewWidth"] - 2 * padding, gmenu2x->listRect.h - 2 * padding, true, false);
			gmenu2x->sc[filename]->blit(gmenu2x->s, {320 - animation + padding, gmenu2x->listRect.y + padding, gmenu2x->skinConfInt["previewWidth"] - 2 * padding, gmenu2x->listRect.h - 2 * padding}, HAlignCenter | VAlignMiddle, 240);

			if (animation < gmenu2x->skinConfInt["previewWidth"]) {
				animation = intTransition(0, gmenu2x->skinConfInt["previewWidth"], tickStart, 110);
				gmenu2x->s->flip();
				gmenu2x->input.setWakeUpInterval(45);
				continue;
			}
		} else {
			if (animation > 0) {
				gmenu2x->s->box(320 - animation, gmenu2x->listRect.y, gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h, gmenu2x->skinConfColors[COLOR_TOP_BAR_BG]);
				animation = gmenu2x->skinConfInt["previewWidth"] - intTransition(0, gmenu2x->skinConfInt["previewWidth"], tickStart, 80);
				gmenu2x->s->flip();
				gmenu2x->input.setWakeUpInterval(45);
				continue;
			}
		}
		gmenu2x->input.setWakeUpInterval(1000);

		gmenu2x->drawScrollBar(numRows, fl->size(), firstElement, gmenu2x->listRect);
		gmenu2x->s->flip();

		do {
			inputAction = gmenu2x->input.update();
			if (gmenu2x->inputCommonActions(inputAction)) continue;
			if (inputAction) tickStart = SDL_GetTicks();

			uint32_t action = getAction();

		// if (action == BD_ACTION_SELECT && (*fl)[selected] == "..")
			// action = BD_ACTION_GOUP;
			switch (action) {
				case BD_ACTION_CANCEL:
					cancel();
					break;
				case BD_ACTION_CLOSE:
					if (allowSelectDirectory && fl->isDirectory(selected)) confirm();
					else cancel();
					break;
				case BD_ACTION_UP:
					selected -= 1;
					if (selected < 0) selected = fl->size() - 1;
					break;
				case BD_ACTION_DOWN:
					selected += 1;
					if (selected >= fl->size()) selected = 0;
					break;
				case BD_ACTION_PAGEUP:
					selected -= numRows;
					if (selected < 0) selected = 0;
					break;
				case BD_ACTION_PAGEDOWN:
					selected += numRows;
					if (selected >= fl->size()) selected = fl->size() - 1;
					break;
				case BD_ACTION_GOUP:
					directoryUp();
					break;
				case BD_ACTION_SELECT:
					if (fl->isDirectory(selected)) {
						directoryEnter();
						break;
					}
			/* Falltrough */
				case BD_ACTION_CONFIRM:
					confirm();
					break;
				default:
					break;
			}
		} while (!inputAction);
	}
	return result;
}

uint32_t BrowseDialog::getAction() {
	uint32_t action = BD_NO_ACTION;

	if (gmenu2x->input[CANCEL]) action = BD_ACTION_CLOSE;
	else if (gmenu2x->input[UP]) action = BD_ACTION_UP;
	else if (gmenu2x->input[PAGEUP] || gmenu2x->input[LEFT]) action = BD_ACTION_PAGEUP;
	else if (gmenu2x->input[DOWN]) action = BD_ACTION_DOWN;
	else if (gmenu2x->input[PAGEDOWN] || gmenu2x->input[RIGHT]) action = BD_ACTION_PAGEDOWN;
	else if (gmenu2x->input[MANUAL]) action = BD_ACTION_GOUP;
	else if (gmenu2x->input[CONFIRM]) action = BD_ACTION_SELECT;
	else if (gmenu2x->input[CANCEL] || gmenu2x->input[MENU]) action = BD_ACTION_CANCEL;
	return action;
}

void BrowseDialog::directoryUp() {
	string path = fl->getPath();
	string::size_type p = path.rfind("/");
	if (p == path.size() - 1) p = path.rfind("/", p - 1);
	selected = 0;
	setPath("/"+path.substr(0, p));
}

void BrowseDialog::directoryEnter() {
	string path = fl->getPath();
	setPath(path + "/" + fl->at(selected));
	selected = 0;
}

void BrowseDialog::confirm() {
	result = true;
	close = true;
}

void BrowseDialog::cancel() {
	result = false;
	close = true;
}

const std::string BrowseDialog::getExt() {
	string filename = (*fl)[selected];
	string ext = "";
	string::size_type pos = filename.rfind(".");
	if (pos != string::npos && pos > 0) {
		ext = filename.substr(pos, filename.length());
		transform(ext.begin(), ext.end(), ext.begin(), (int(*)(int)) tolower);
	}
	return ext;
}

void BrowseDialog::setPath(const string &path) {
	fl->showDirectories = showDirectories;
	fl->showFiles = showFiles;
	fl->setPath(path);
	onChangeDir();
}

const std::string &BrowseDialog::getPath() {
	return fl->getPath();
}
std::string BrowseDialog::getFile() {
	return (*fl)[selected];
}
void BrowseDialog::setFilter(const string &filter) {
	fl->setFilter(filter);
}
