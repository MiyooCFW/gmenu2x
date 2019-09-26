#ifndef LINKSCANNERDIALOG_H_
#define LINKSCANNERDIALOG_H_

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

#include "gmenu2x.h"
#include "dialog.h"
#include "menu.h"
#include "messagebox.h"

using namespace std;

class LinkScannerDialog : protected Dialog {
protected:
	string title, description, icon;
	void scanPath(string path, vector<string> *files);

public:
	LinkScannerDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon);
	void exec();
};

#endif /*LINKSCANNERDIALOG_H_*/
