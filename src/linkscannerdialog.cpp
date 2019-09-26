#include "linkscannerdialog.h"
#include "debug.h"

LinkScannerDialog::LinkScannerDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon)
	: Dialog(gmenu2x)
{
	this->title = title;
	this->description = description;
	this->icon = icon;
}

void LinkScannerDialog::exec() {
	bool close = false;
	string str = "";
	stringstream ss;
	uint32_t lineY = gmenu2x->listRect.y;
	vector<string> files;

	drawTopBar(this->bg, title, description, icon);
	drawBottomBar(this->bg);
	this->bg->box(gmenu2x->listRect, gmenu2x->skinConfColors[COLOR_LIST_BG]);

	gmenu2x->drawButton(this->bg, "start", gmenu2x->tr["Exit"]);

	this->bg->blit(gmenu2x->s,0,0);

	gmenu2x->s->write(gmenu2x->font, gmenu2x->tr["Scanning..."], gmenu2x->listRect.x + 4, lineY);

	lineY += gmenu2x->font->getHeight();
	gmenu2x->s->write(gmenu2x->font, gmenu2x->tr["/mnt/int_sd"], gmenu2x->listRect.x + 4, lineY);
	gmenu2x->s->flip();

	//scanPath("/mnt/int_sd", &files);

	lineY += gmenu2x->font->getHeight();
	gmenu2x->s->write(gmenu2x->font, gmenu2x->tr["/mnt/ext_sd"], gmenu2x->listRect.x + 4, lineY);
	gmenu2x->s->flip();

	//scanPath("/mnt/ext_sd", &files);

	ss << files.size();
	ss >> str;

	lineY += gmenu2x->font->getHeight();
	gmenu2x->s->write(gmenu2x->font, gmenu2x->tr.translate("$1 files found.", str.c_str(), NULL), gmenu2x->listRect.x + 4, lineY);
	gmenu2x->s->flip();

	if (files.size() > 0) {

		lineY += gmenu2x->font->getHeight();
		gmenu2x->s->write(gmenu2x->font, gmenu2x->tr["Creating links..."], gmenu2x->listRect.x + 4, lineY);
		gmenu2x->s->flip();

		string path, file;
		string::size_type pos;
		uint32_t linkCount = 0;

		// ledOn();
		for (size_t i = 0; i < files.size(); ++i) {
			pos = files[i].rfind("/");

			if (pos != string::npos && pos > 0) {
				path = files[i].substr(0, pos + 1);
				file = files[i].substr(pos + 1, files[i].length());
				if (gmenu2x->menu->addLink(path, file, "linkscanner"))
					linkCount++;
			}
		}

		ss.clear();
		ss << linkCount;
		ss >> str;

		lineY += gmenu2x->font->getHeight();

		gmenu2x->s->write(gmenu2x->font, gmenu2x->tr.translate("$1 links created.", str.c_str(), NULL), gmenu2x->listRect.x + 4, lineY);
		gmenu2x->s->flip();
	}

	sync();

	while (!close) {
		bool inputAction = gmenu2x->input.update();
		if ( gmenu2x->input[SETTINGS] || gmenu2x->input[CANCEL] ) close = true;
	}
}

void LinkScannerDialog::scanPath(string path, vector<string> *files) {
	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath, ext;

	if (path[path.length()-1]!='/') path += "/";
	if ((dirp = opendir(path.c_str())) == NULL) return;

	while ((dptr = readdir(dirp))) {
		if (dptr->d_name[0] == '.')
			continue;
		filepath = path + dptr->d_name;
		int statRet = stat(filepath.c_str(), &st);
		if (S_ISDIR(st.st_mode))
			scanPath(filepath, files);

		if (statRet != -1) {
			ext = filepath.substr(filepath.length()-4,4);

			if (ext == ".dge" || ext == ".gpu" || ext == ".gpe" || filepath.substr(filepath.length()-3,3) == ".sh")
				files->push_back(filepath);
		}
	}
	closedir(dirp);
}
