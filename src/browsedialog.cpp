#include "browsedialog.h"
#include "messagebox.h"
#include "debug.h"
#include "utilities.h"
#include "powermanager.h"
#include "inputmanager.h"
using namespace std;
extern const char *CARD_ROOT;

SDL_TimerID alphanum_timer = NULL;

uint32_t hideAlphaNum(uint32_t interval, void *param) {
	SDL_RemoveTimer(alphanum_timer); alphanum_timer = NULL;
	InputManager::wakeUp(0, (void*)false);
	return 0;
}


BrowseDialog::BrowseDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon):
Dialog(gmenu2x, title, description, icon) {
	setPath(gmenu2x->confStr["homePath"]);
	srand(SDL_GetTicks());
}

bool BrowseDialog::exec() {
	this->bg = new Surface(gmenu2x->bg); // needed to redraw on child screen return

	Surface *iconGoUp = gmenu2x->sc["skin:imgs/go-up.png"];
	Surface *iconFolder = gmenu2x->sc["skin:imgs/folder.png"];
	Surface *iconFile = gmenu2x->sc["skin:imgs/file.png"];
	Surface *iconSd = gmenu2x->sc["skin:imgs/sd.png"];
	Surface *iconCur;

	uint32_t i, iY, firstElement = 0, padding = 6;
	int32_t animation = 0;
	uint32_t rowHeight = gmenu2x->font->getHeight() + 1;
	uint32_t numRows = (gmenu2x->listRect.h - 2) / rowHeight - 1;

	if (path.empty() || !dir_exists(path))
		setPath(gmenu2x->confStr["homePath"]);

	directoryEnter(path);

	string preview = getPreview(selected);

	// this->description = getFilter();

	while (true) {
		if (selected < 0) selected = this->size() - 1;
		if (selected >= this->size()) selected = 0;

		bool inputAction = false;

		buttons.clear();
		if (alphanum_timer != NULL) {
			int c = toupper(getFileName(selected).at(0));
			if (!isalpha(c)) c = '#';
			string sel(1, c);
			buttons.push_back({"skin:imgs/manual.png", strreplace("#ABCDEFGHIJKLMNOPQRSTUVWXYZ", sel, " < " + sel + " > ")});
		} else {
			buttons.push_back({"select", gmenu2x->tr["Menu"]});
			buttons.push_back({"b", gmenu2x->tr["Cancel"]});

		if (!showFiles && allowSelectDirectory)
			buttons.push_back({"start", gmenu2x->tr["Select"]});
		else if ((allowEnterDirectory && isDirectory(selected)) || !isDirectory(selected))
			buttons.push_back({"a", gmenu2x->tr["Select"]});

		if (showDirectories && allowDirUp && path != "/")
			buttons.push_back({"x", gmenu2x->tr["Folder up"]});

		if (gmenu2x->confStr["previewMode"] == "Backdrop") {
			if (!(preview.empty() || preview == "#"))
				gmenu2x->setBackground(this->bg, preview);
			else
				gmenu2x->bg->blit(this->bg,0,0);
			}
		}

		this->description = path;

		drawDialog(gmenu2x->s);

		if (!size()) {
			MessageBox mb(gmenu2x, gmenu2x->tr["This directory is empty"]);
			mb.setAutoHide(1);
			mb.setBgAlpha(0);
			mb.exec();
		} else {
			// Selection
			if (selected >= firstElement + numRows) firstElement = selected - numRows;
			if (selected < firstElement) firstElement = selected;

			// Files & Directories
			iY = gmenu2x->listRect.y + 1;
			for (i = firstElement; i < size() && i <= firstElement + numRows; i++, iY += rowHeight) {
				if (i == selected) gmenu2x->s->box(gmenu2x->listRect.x, iY, gmenu2x->listRect.w, rowHeight, gmenu2x->skinConfColors[COLOR_SELECTION_BG]);
				
				iconCur = iconFile;

				if (isDirectory(i)) {
					if (getFile(i) == "..")
						iconCur = iconGoUp;
					else if ((path == "/" && getFileName(i) == "media") || path == "/media")
						iconCur = iconSd;
					else
						iconCur = iconFolder;
				}

				iconCur->blit(gmenu2x->s, gmenu2x->listRect.x + 10, iY + rowHeight/2, HAlignCenter | VAlignMiddle);

				gmenu2x->s->write(gmenu2x->font, getFileName(i), gmenu2x->listRect.x + 21, iY + rowHeight/2, VAlignMiddle);
			}

			gmenu2x->allyTTS(getFileName(selected).c_str(), FAST_GAP_TTS, FAST_SPEED_TTS, 0);

			if (gmenu2x->confStr["previewMode"] != "Backdrop") {
				Surface anim = new Surface(gmenu2x->s);
				if (preview.empty() || preview == "#") { // hide preview
					 while (animation > 0) {
						animation -= gmenu2x->skinConfInt["previewWidth"] / 8;

						if (animation < 0)
							animation = 0;

						anim.blit(gmenu2x->s,0,0);
						gmenu2x->s->box(gmenu2x->w - animation, gmenu2x->listRect.y, gmenu2x->skinConfInt["previewWidth"] + 2 * padding, gmenu2x->listRect.h, gmenu2x->skinConfColors[COLOR_PREVIEW_BG]);
						gmenu2x->s->flip();
						SDL_Delay(10);
					};
				} else { // show preview
					if (!gmenu2x->sc.exists(preview + "scaled")) {
						Surface *previm = new Surface(preview);
						gmenu2x->sc.add(previm, preview + "scaled");
						if (gmenu2x->confStr["bgscale"] == "Stretch") gmenu2x->sc[preview + "scaled"]->softStretch(gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h - 2 * padding, SScaleStretch);
						else if (gmenu2x->confStr["bgscale"] == "Crop") gmenu2x->sc[preview + "scaled"]->softStretch(gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h - 2 * padding, SScaleMax);
						else if (gmenu2x->confStr["bgscale"] == "Aspect") gmenu2x->sc[preview + "scaled"]->softStretch(gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h - 2 * padding, SScaleFit);
					}

					do {
						animation += gmenu2x->skinConfInt["previewWidth"] / 8;

						if (animation > gmenu2x->skinConfInt["previewWidth"] + 2 * padding)
							animation = gmenu2x->skinConfInt["previewWidth"] + 2 * padding;

						anim.blit(gmenu2x->s,0,0);
						gmenu2x->s->box(gmenu2x->w - animation, gmenu2x->listRect.y, gmenu2x->skinConfInt["previewWidth"] + 2 * padding, gmenu2x->listRect.h, gmenu2x->skinConfColors[COLOR_PREVIEW_BG]);
						gmenu2x->sc[preview + "scaled"]->blit(gmenu2x->s, {gmenu2x->w - animation + padding, gmenu2x->listRect.y + padding, gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h - 2 * padding}, HAlignCenter | VAlignMiddle, gmenu2x->h);
						gmenu2x->s->flip();
						SDL_Delay(10);
					} while (animation < gmenu2x->skinConfInt["previewWidth"] + 2 * padding);
				}
			}
			gmenu2x->drawScrollBar(numRows, size(), firstElement, gmenu2x->listRect);
			gmenu2x->s->flip();
		}

		do {
			inputAction = gmenu2x->input.update();
			} while (!inputAction);

		if (gmenu2x->inputCommonActions(inputAction)) continue;

		SDL_RemoveTimer(alphanum_timer); alphanum_timer = NULL;

		if (gmenu2x->input[UP]) {
			selected--;
		} else if (gmenu2x->input[DOWN]) {
			selected++;
		} else if (gmenu2x->input[LEFT]) {
			selected -= numRows;
			if (selected < 0) selected = 0;
		} else if (gmenu2x->input[RIGHT]) {
			selected += numRows;
			if (selected >= this->size()) selected = this->size() - 1;
		} else if (gmenu2x->input[PAGEDOWN]) {
			alphanum_timer = SDL_AddTimer(1500, hideAlphaNum, (void*)false);
			int cur = toupper(getFileName(selected).at(0));
			while ((selected < this->size() - 1) && ++selected && cur == toupper(getFileName(selected).at(0))) {
			}
		} else if (gmenu2x->input[PAGEUP]) {
			alphanum_timer = SDL_AddTimer(1500, hideAlphaNum, (void*)false);
			int cur = toupper(getFileName(selected).at(0));
			while (selected > 0 && selected-- && cur == toupper(getFileName(selected).at(0))) {
			}
		} else if (showDirectories && allowDirUp && (gmenu2x->input[MODIFIER] || (gmenu2x->input[CONFIRM] && getFile(selected) == ".."))) { /*Directory Up */
			selected = 0;
				preview = "";
			if (browse_history.size() > 0) {
				selected = browse_history.back();
				browse_history.pop_back();
			}
			directoryEnter(path + "/..");
		} else if (gmenu2x->input[CONFIRM]) {
			if (allowEnterDirectory && isDirectory(selected)) {
				browse_history.push_back(selected);
				directoryEnter(getFilePath(selected));
				selected = 0;
				} else {
					return true;
			}
		} else if (gmenu2x->input[SETTINGS] && allowSelectDirectory) {
			return true;
		} else if (gmenu2x->input[CANCEL] || gmenu2x->input[SETTINGS]) {
			if (!((gmenu2x->confStr["previewMode"] != "Backdrop") && !(preview.empty() || preview == "#")))
				return false; // close only if preview is empty.
			preview = "";
		} else if (gmenu2x->input[MANUAL]) {
			alphanum_timer = SDL_AddTimer(1500, hideAlphaNum, (void*)false);
			selected = (rand() % fileCount()) + dirCount();
		} else if (gmenu2x->input[MENU]) {
			contextMenu();
			preview = getPreview(selected);
		}
	if (gmenu2x->input[UP] || gmenu2x->input[DOWN] || gmenu2x->input[LEFT] || gmenu2x->input[RIGHT] || gmenu2x->input[PAGEUP] || gmenu2x->input[PAGEDOWN] || gmenu2x->input[MANUAL]) {
			preview = getPreview(selected);
		}
	}
}

void BrowseDialog::directoryEnter(const string &path) {
	gmenu2x->input.dropEvents(); // prevent passing input away
	gmenu2x->powerManager->clearTimer();

	this->description = path;
	buttons.clear();
	buttons.push_back({"skin:imgs/manual.png", gmenu2x->tr["Loading.. Please wait.."]});

	drawDialog(gmenu2x->s);

	SDL_TimerID flipScreenTimer = SDL_AddTimer(500, GMenu2X::timerFlip, (void*)false);

	setPath(path);
	browse();
	onChangeDir();

	SDL_RemoveTimer(flipScreenTimer); flipScreenTimer = NULL;
	gmenu2x->powerManager->resetSuspendTimer();
}

const std::string BrowseDialog::getFileName(uint32_t i) {
	return getFile(i);
}
const std::string BrowseDialog::getParams(uint32_t i) {
	return "";
}
const std::string BrowseDialog::getPreview(uint32_t i) {
	string ext = getExt(i);
	if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || ext == ".bmp") return getFilePath(i);
	return "";
}

void BrowseDialog::contextMenu() {
	vector<MenuOption> options;

	string ext = getExt(selected);

	customOptions(options);

	if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || ext == ".bmp")
		options.push_back((MenuOption){gmenu2x->tr["Set as wallpaper"], MakeDelegate(this, &BrowseDialog::setWallpaper)});

	if (path == "/roms" && getFile(selected) != ".." && isDirectory(selected))
		options.push_back((MenuOption){gmenu2x->tr["Umount"], MakeDelegate(this, &BrowseDialog::umountDir)});

	if (path != CARD_ROOT)
		options.push_back((MenuOption){gmenu2x->tr["Go to"] + " " + CARD_ROOT, MakeDelegate(this, &BrowseDialog::exploreHome)});

	if (path != "/mnt/roms")
		options.push_back((MenuOption){gmenu2x->tr["Go to"] + " /mnt/roms", MakeDelegate(this, &BrowseDialog::exploreMedia)});

	if (isFile(selected))
		options.push_back((MenuOption){gmenu2x->tr["Delete"], MakeDelegate(this, &BrowseDialog::deleteFile)});

	MessageBox mb(gmenu2x, options);
}

void BrowseDialog::deleteFile() {
	MessageBox mb(gmenu2x, gmenu2x->tr["Delete"] + " '" +  getFile(selected) + "'\n" + gmenu2x->tr["THIS CAN'T BE UNDONE"] + "\n" + gmenu2x->tr["Are you sure?"], "explorer.png");
	mb.setButton(MANUAL, gmenu2x->tr["Yes"]);
	mb.setButton(CANCEL,  gmenu2x->tr["No"]);
	if (mb.exec() != MANUAL) return;
	if (!unlink(getFilePath(selected).c_str())) {
		directoryEnter(path); // refresh
		sync();
	}
}

void BrowseDialog::umountDir() {
	string umount = "sync; umount -fl " + getFilePath(selected) + " && rm -r " + getFilePath(selected);
	system(umount.c_str());
	directoryEnter(path); // refresh
}

void BrowseDialog::exploreHome() {
	selected = 0;
	directoryEnter(CARD_ROOT);
}

void BrowseDialog::exploreMedia() {
	selected = 0;
	directoryEnter("/mnt/roms");
}

void BrowseDialog::setWallpaper() {
	string src = getFilePath(selected);
	string dst = "skins/Default/wallpapers/Wallpaper" + file_ext(src, true);
	if (file_copy(src, dst)) {
		gmenu2x->confStr["wallpaper"] = dst;
		gmenu2x->writeConfig();
		gmenu2x->setBackground(gmenu2x->bg, dst);
		this->exec();
	}
}
