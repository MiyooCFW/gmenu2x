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
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#include "linkapp.h"
#include "menu.h"
#include "selector.h"
#include "messagebox.h"
#include "debug.h"

using namespace std;

extern char** environ;

LinkApp::LinkApp(GMenu2X *gmenu2x, const char* file):
Link(gmenu2x, MakeDelegate(this, &LinkApp::run)), file(file) {
	setCPU(gmenu2x->confInt["cpuLink"]);

#if defined(HW_GAMMA)
	//G
	setGamma(0);
	// wrapper = false;
	// dontleave = false;
	// setVolume(-1);
	// useRamTimings = false;
	// useGinge = false;
#endif

	if (((float)(gmenu2x->w)/gmenu2x->h) != (4.0f/3.0f)) _scalemode = 3; // 4:3 by default
	scalemode = _scalemode;

	string line;
	ifstream infile(file, ios_base::in);
	while (getline(infile, line, '\n')) {
		line = trim(line);
		if (line == "") continue;
		if (line[0] == '#') continue;

		string::size_type position = line.find("=");
		string name = trim(line.substr(0, position));
		string value = trim(line.substr(position + 1));

		if (name == "exec") setExec(value);
		else if (name == "title") setTitle(value);
		else if (name == "description") setDescription(value);
		else if (name == "icon") setIcon(value);
		else if (name == "opk[icon]") icon_opk = value;
		else if (name == "params") setParams(value);
		else if (name == "home") setHomeDir(value);
		else if (name == "manual") setManual(value);
		else if (name == "clock") setCPU(atoi(value.c_str()));

#if defined(HW_GAMMA)
		// else if (name == "wrapper" && value == "true") // wrapper = true;
		// else if (name == "dontleave" && value == "true") // dontleave = true;
		// else if (name == "volume") // setVolume(atoi(value.c_str()));
		// else if (name == "useramtimings" && value == "true") // useRamTimings = true;
		// else if (name == "useginge" && value == "true") // useGinge = true;
		else if (name == "gamma") setGamma(atoi(value.c_str()));
#endif
		else if (name == "selectordir") setSelectorDir(value);
		else if (name == "selectorbrowser" && value == "false") setSelectorBrowser(false);
		else if (name == "scalemode") setScaleMode(atoi(value.c_str()));
		else if (name == "selectorfilter") setSelectorFilter(value);
		else if (name == "selectorscreens") setSelectorScreens(value);
		else if (name == "selectoraliases") setAliasFile(value);
		else if (name == "selectorelement") setSelectorElement(atoi(value.c_str()));
		else if ((name == "consoleapp") || (name == "terminal")) setTerminal(value == "true");
		else if (name == "backdrop") setBackdrop(value);
		// else WARNING("Unrecognized option: '%s'", name.c_str());
	}
	infile.close();

	is_opk = (file_ext(exec, true) == ".opk");

	if (iconPath.empty()) iconPath = searchIcon();
	if (manualPath.empty()) manualPath = searchManual();
	if (backdropPath.empty()) backdropPath = searchBackdrop();
}

const string LinkApp::searchManual() {
	if (!manualPath.empty()) return manualPath;
	string filename = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) filename = exec.substr(0, pos);
	filename += ".man.txt";

	string dname = dir_name(exec) + "/";

	string dirtitle = dname + base_name(dir_name(exec)) + ".man.txt";
	string linktitle = base_name(file, true);
	linktitle = dname + linktitle + ".man.txt";

	if (file_exists(linktitle)) return linktitle;
	if (file_exists(filename)) return filename;
	if (file_exists(dirtitle)) return dirtitle;

	return "";
}

const string LinkApp::searchBackdrop() {
	if (!backdropPath.empty() || !gmenu2x->confInt["skinBackdrops"]) return backdropPath;
	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0, pos);
	string exectitle = base_name(execicon);
	string dirtitle = base_name(dir_name(exec));
	string linktitle = base_name(file);
	string sublinktitle = base_name(file);

	pos = linktitle.find(".");
	if (pos != string::npos) sublinktitle = linktitle.substr(0, pos);

	pos = linktitle.rfind(".");
	if (pos != string::npos) linktitle = linktitle.substr(0, pos);

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + sublinktitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + sublinktitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = dir_name(exec) + "/backdrop.png";
	if (file_exists(backdropPath)) return backdropPath;

	return "";
}

const string LinkApp::searchIcon() {
	string iconpath = gmenu2x->sc.getSkinFilePath(icon, false);
	if (!iconpath.empty()) return iconpath;

	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0, pos);
	string exectitle = base_name(execicon);
	string dirtitle = base_name(dir_name(exec));
	string linktitle = base_name(file);

	vector<string> linkparts;
	split(linkparts, linktitle, ".");

	if (linkparts.size() > 2) {
		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[0] + "." + linkparts[1] + ".png", false);
		if (!iconpath.empty()) return iconpath;

		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[1] + "." + linkparts[0] + ".png", false);
		if (!iconpath.empty()) return iconpath;
	}

	if (linkparts.size() > 1) {
		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[1] + ".png", false);
		if (!iconpath.empty()) return iconpath;

		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[0] + ".png", false);
		if (!iconpath.empty()) return iconpath;
	}

	iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linktitle + ".png", false);
	if (!iconpath.empty()) return iconpath;

	iconpath = gmenu2x->sc.getSkinFilePath("icons/" + exectitle + ".png", false);
	if (!iconpath.empty()) return iconpath;

	iconpath = gmenu2x->sc.getSkinFilePath("icons/" + dirtitle + ".png", false);
	if (!iconpath.empty()) return iconpath;

	iconpath = dir_name(exec) + "/" + exectitle + ".png";
	if (file_exists(iconpath)) return iconpath;

	iconpath = execicon + ".png";
	if (file_exists(iconpath)) return iconpath;

#if defined(OPK_SUPPORT)
	if (isOPK()) {
		return exec + "#" + icon_opk;
	} else
#endif

	return gmenu2x->sc.getSkinFilePath("icons/generic.png");
}

void LinkApp::setCPU(int mhz) {
	clock = mhz;
	if (clock != 0) clock = constrain(clock, gmenu2x->confInt["cpuMin"], gmenu2x->confInt["cpuMax"]);
	edited = true;
}

#if defined(HW_GAMMA)
void LinkApp::setGamma(int gamma) {
	gamma = constrain(gamma, 0, 100);
	edited = true;
}
#endif

bool LinkApp::targetExists() {
#if defined(TARGET_LINUX)
	return true; //For displaying elements during testing on pc
#endif
	string target = exec;
	if (!exec.empty() && exec[0] != '/' && !homedir.empty())
		target = homedir + "/" + exec;

	return file_exists(target);
}

bool LinkApp::save() {
	if (!edited) return false;
	int pos = icon.find('#'); // search for "opkfile.opk#icon.png"
	if (pos != string::npos) {
		icon_opk = icon.substr(pos + 1);
	}

	ofstream f(file.c_str());
	if (f.is_open()) {
		if (title != "")			f << "title="			<< title			<< endl;
		if (description != "")		f << "description="		<< description		<< endl;
		if (icon != "")				f << "icon="			<< icon				<< endl;
		if (icon_opk != "")			f << "opk[icon]="		<< icon_opk			<< endl;
		if (exec != "")				f << "exec="			<< exec				<< endl;
		if (params != "")			f << "params="			<< params			<< endl;
		if (homedir != "")			f << "home="			<< homedir			<< endl;
		if (manual != "")			f << "manual="			<< manual			<< endl;
		if (clock != 0 && clock != gmenu2x->confInt["cpuLink"])
									f << "clock="			<< clock			<< endl;
		// if (useRamTimings)		f << "useramtimings=true"					<< endl;
		// if (useGinge)			f << "useginge=true"						<< endl;
		// if (volume > 0)			f << "volume="			<< volume			<< endl;
#if defined(HW_GAMMA)
		if (gamma != 0)				f << "gamma="			<< gamma			<< endl;
#endif

		if (selectordir != "")		f << "selectordir="		<< selectordir		<< endl;
		if (!selectorbrowser)		f << "selectorbrowser=false"				<< endl; // selectorbrowser = true by default
		if (scalemode != _scalemode)	f << "scalemode="	<< scalemode		<< endl; // scalemode = 0 by default
		if (selectorfilter != "")	f << "selectorfilter="	<< selectorfilter	<< endl;
		if (selectorscreens != "")	f << "selectorscreens="	<< selectorscreens	<< endl;
		if (selectorelement > 0)	f << "selectorelement="	<< selectorelement	<< endl;
		if (aliasfile != "")		f << "selectoraliases="	<< aliasfile		<< endl;
		if (backdrop != "")			f << "backdrop="		<< backdrop			<< endl;
		if (terminal)				f << "terminal=true"						<< endl;
		f.close();
		return true;
	}

	ERROR("Error while opening the file '%s' for write.", file.c_str());
	return false;
}

void LinkApp::run() {
	uint32_t start = SDL_GetTicks();
	while (gmenu2x->input[CONFIRM]) {
		gmenu2x->input.update();
		if (SDL_GetTicks() - start > 1400) {
			// hold press -> inverted
			if (selectordir != "")
				return launch();
			return selector();
		}
	}

	// quick press -> normal
	if (selectordir != "")
		return selector();
	return launch();
}

void LinkApp::selector(int startSelection, const string &selectorDir) {
	//Run selector interface
	Selector bd(gmenu2x, this->getTitle(), this->getDescription(), this->getIconPath(), this);
	bd.showDirectories = this->getSelectorBrowser();

	if (selectorDir != "") bd.directoryEnter(selectorDir);
	else bd.directoryEnter(this->getSelectorDir());

	bd.setFilter(this->getSelectorFilter());

	if (startSelection > 0) bd.selected = startSelection;
	else bd.selected = this->getSelectorElement();

	if (bd.exec()) {
		gmenu2x->writeTmp(bd.selected, bd.getPath());

		string s = "";
		s += this->getSelectorDir().back();
		if (s != "/") {
			setSelectorDir(bd.getPath());
			setSelectorElement(bd.selected);
			save();
		}

		params = trim(params + " " + bd.getParams(bd.selected));

		launch(bd.getFile(bd.selected), bd.getPath());
	}
}

void LinkApp::launch(const string &selectedFile, string dir) {
	MessageBox mb(gmenu2x, gmenu2x->tr["Launching"] + " " + this->getTitle().c_str(), this->getIconPath());
	mb.setAutoHide(1);
	mb.exec();

	string command = cmdclean(exec);

	if (selectedFile.empty()) {
		gmenu2x->writeTmp();
	} else {
		if (dir.empty()) {
			dir = getSelectorDir();
		}

		if (params.empty()) {
			params = cmdclean(dir + "/" + selectedFile);
		} else {
			string origParams = params;
			params = strreplace(params, "[selFullPath]", cmdclean(dir + "/" + selectedFile));
			params = strreplace(params, "\%f", cmdclean(dir + "/" + selectedFile));
			params = strreplace(params, "[selPath]", cmdclean(dir));
			params = strreplace(params, "[selFile]", cmdclean(selectedFile));
			params = strreplace(params, "[selFileNoExt]", cmdclean(base_name(selectedFile, true)));
			params = strreplace(params, "[selExt]", cmdclean(file_ext(selectedFile, false)));
			if (params == origParams) params += " " + cmdclean(dir + "/" + selectedFile);
		}
	}

	INFO("Executing '%s' (%s %s)", title.c_str(), exec.c_str(), params.c_str());

#if defined(OPK_SUPPORT)
	if (isOPK()) {
		string opk_mount = "umount -fl /mnt &> /dev/null; mount -o loop " + command + " /mnt";
		system(opk_mount.c_str());
		chdir("/mnt"); // Set correct working directory

		command = "/mnt/" + params;
		params = "";
	}
	else
#endif
	{
		chdir(dir_name(exec).c_str()); // Set correct working directory
	}

	// Check to see if permissions are desirable
	struct stat fstat;
	if (!stat(command.c_str(), &fstat)) {
		struct stat newstat = fstat;
		if (S_IRUSR != (fstat.st_mode & S_IRUSR)) newstat.st_mode |= S_IRUSR;
		if (S_IXUSR != (fstat.st_mode & S_IXUSR)) newstat.st_mode |= S_IXUSR;
		if (fstat.st_mode != newstat.st_mode) chmod(command.c_str(), newstat.st_mode);
	} // else, well.. we are no worse off :)

	if (params != "") command += " " + params;

	if (gmenu2x->confInt["saveSelection"] && (gmenu2x->confInt["section"] != gmenu2x->menu->selSectionIndex() || gmenu2x->confInt["link"] != gmenu2x->menu->selLinkIndex())) {
		gmenu2x->writeConfig();
	}

	if (getCPU() != gmenu2x->confInt["cpuMenu"]) gmenu2x->setCPU(getCPU());

#if defined(TARGET_GP2X)
	//if (useRamTimings) gmenu2x->applyRamTimings();
	// if (useGinge) {
		// string ginge_prep = exe_path() + "/ginge/ginge_prep";
		// if (file_exists(ginge_prep)) command = cmdclean(ginge_prep) + " " + command;
	// }
	if (fwType == "open2x") gmenu2x->writeConfigOpen2x();
#endif

#if defined(HW_GAMMA)
	if (gamma() != 0 && gamma() != gmenu2x->confInt["gamma"]) gmenu2x->setGamma(gamma());
#endif

	gmenu2x->setScaleMode(scalemode);

	command = gmenu2x->hwPreLinkLaunch() + command;

	if (gmenu2x->confInt["outputLogs"]) {
		params = "echo " + cmdclean(command) + " > " + cmdclean(exe_path()) + "/log.txt";
		system(params.c_str());
		command += " 2>&1 | tee -a " + cmdclean(exe_path()) + "/log.txt";
	}

	// params = this->getHomeDir();
	params = gmenu2x->confStr["homePath"];

	if (!params.empty() && dir_exists(params)) {
		command = "HOME=" + params + " " + command;
	}

	gmenu2x->quit();

	if (getTerminal()) gmenu2x->enableTerminal();

	// execle("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL, environ);
	execlp("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);

	//if execution continues then something went wrong and as we already called SDL_Quit we cannot continue
	//try relaunching gmenu2x
	chdir(exe_path().c_str());
	execlp("./gmenu2x", "./gmenu2x", NULL);
}

void LinkApp::setExec(const string &exec) {
	this->exec = exec;
	edited = true;
}

void LinkApp::setParams(const string &params) {
	this->params = params;
	edited = true;
}

void LinkApp::setHomeDir(const string &homedir) {
	this->homedir = homedir;
	edited = true;
}

void LinkApp::setManual(const string &manual) {
	this->manual = manualPath = manual;
	edited = true;
}

void LinkApp::setSelectorDir(const string &selectordir) {
	edited = this->selectordir != selectordir;
	this->selectordir = selectordir;
	// if (this->selectordir != "") this->selectordir = real_path(this->selectordir);
}

void LinkApp::setSelectorBrowser(bool value) {
	selectorbrowser = value;
	edited = true;
}

void LinkApp::setTerminal(bool value) {
	terminal = value;
	edited = true;
}

void LinkApp::setScaleMode(int value) {
	scalemode = value;
	edited = true;
}

void LinkApp::setSelectorFilter(const string &selectorfilter) {
	this->selectorfilter = selectorfilter;
	edited = true;
}

void LinkApp::setSelectorScreens(const string &selectorscreens) {
	this->selectorscreens = selectorscreens;
	edited = true;
}

void LinkApp::setSelectorElement(int i) {
	edited = selectorelement != i;
	selectorelement = i;
}

void LinkApp::setAliasFile(const string &aliasfile) {
	this->aliasfile = aliasfile;
	edited = true;
}

void LinkApp::renameFile(const string &name) {
	file = name;
}

// void LinkApp::setUseRamTimings(bool value) {
// 	useRamTimings = value;
// 	edited = true;
// }
// void LinkApp::setUseGinge(bool value) {
// 	useGinge = value;
// 	edited = true;
// }
