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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include "linkapp.h"
#include "menu.h"
#include "selector.h"
#include "debug.h"

using namespace std;

LinkApp::LinkApp(GMenu2X *gmenu2x_, InputManager &inputMgr_, const char* linkfile):
	Link(gmenu2x_, MakeDelegate(this, &LinkApp::run)),
	inputMgr(inputMgr_)
{
	manual = manualPath = "";
	file = linkfile;
	// wrapper = false;
	// dontleave = false;
	setCPU(gmenu2x->confInt["cpuMenu"]);
	// setVolume(-1);

#if defined(TARGET_GP2X)
	//G
	setGamma(0);
#endif

	selectordir = "";
	selectorfilter = "";
	icon = iconPath = "";
	selectorbrowser = true;
	// useRamTimings = false;
	// useGinge = false;
	workdir = "";
	backdrop = backdropPath = "";

	string line;
	ifstream infile (linkfile, ios_base::in);
	while (getline(infile, line, '\n')) {
		line = trim(line);
		if (line == "") continue;
		if (line[0] == '#') continue;

		string::size_type position = line.find("=");
		string name = trim(line.substr(0,position));
		string value = trim(line.substr(position+1));
		if (name == "title") {
			title = value;
		} else if (name == "description") {
			description = value;
		} else if (name == "icon") {
			setIcon(value);
		} else if (name == "exec") {
			exec = value;
		} else if (name == "params") {
			params = value;
		} else if (name == "workdir") {
			workdir = value;
		} else if (name == "manual") {
			setManual(value);
		// } else if (name == "wrapper" && value == "true") {
			// wrapper = true;
		// } else if (name == "dontleave" && value == "true") {
			// dontleave = true;
		} else if (name == "clock") {
			setCPU( atoi(value.c_str()) );

#if defined(TARGET_GP2X)
		//G
		} else if (name == "gamma") {
			setGamma( atoi(value.c_str()) );
#endif
		// } else if (name == "volume") {
			// setVolume( atoi(value.c_str()) );
		} else if (name == "selectordir") {
			setSelectorDir( value );
		} else if (name == "selectorbrowser" && value == "true") {
			selectorbrowser = true;
		} else if (name == "selectorbrowser" && value == "false") {
			selectorbrowser = false;
		// } else if (name == "useramtimings" && value == "true") {
			// useRamTimings = true;
		// } else if (name == "useginge" && value == "true") {
			// useGinge = true;
		} else if (name == "selectorfilter") {
			setSelectorFilter( value );
		} else if (name == "selectorscreens") {
			setSelectorScreens( value );
		} else if (name == "selectoraliases") {
			setAliasFile( value );
		} else if (name == "backdrop") {
			setBackdrop(value);
			// WARNING("BACKDROP: '%s'", backdrop.c_str());
		} else {
			WARNING("Unrecognized option: '%s'", name.c_str());
			break;
		}
	}
	infile.close();

	if (iconPath.empty()) searchIcon();

	edited = false;
}

const string &LinkApp::searchManual() {
	if (!manualPath.empty()) return manualPath;
	string filename = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) filename = exec.substr(0, pos);
	filename += ".man.txt";

	string dname = dir_name(exec) + "/";

	string dirtitle = dname + base_name(dir_name(exec)) + ".man.txt";
	string linktitle = base_name(file);
	pos = linktitle.rfind(".");
	if (pos != string::npos) linktitle = linktitle.substr(0, pos);
	linktitle = dname + linktitle + ".man.txt";

	if (fileExists(linktitle))
		manualPath = linktitle;
	else if (fileExists(filename))
		manualPath = filename;
	else if (fileExists(dirtitle))
		manualPath = dirtitle;

	return manualPath;
}

const string &LinkApp::searchBackdrop() {
	if (!backdropPath.empty() || !gmenu2x->confInt["skinBackdrops"]) return backdropPath;
	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0, pos);
	string exectitle = base_name(execicon);
	string dirtitle = base_name(dir_name(exec));
	string linktitle = base_name(file);
	pos = linktitle.rfind(".");
	if (pos != string::npos) linktitle = linktitle.substr(0, pos);

// auto backdrop
	if (!gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".png").empty())
		backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".png");
	else if (!gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".jpg").empty())
		backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".jpg");
	else if (!gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".png").empty())
		backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".png");
	else if (!gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".jpg").empty())
		backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".jpg");
	else if (!gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".png").empty())
		backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".png");
	else if (!gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".jpg").empty())
		backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".jpg");

	return backdropPath;
}

const string &LinkApp::searchIcon() {
	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0, pos);
	execicon += ".png";
	string exectitle = base_name(execicon);
	string dirtitle = base_name(dir_name(exec)) + ".png";
	string linktitle = base_name(file);
	pos = linktitle.rfind(".");
	if (pos != string::npos) linktitle = linktitle.substr(0, pos);
	linktitle += ".png";

	if (!gmenu2x->sc.getSkinFilePath("icons/" + linktitle).empty())
		iconPath = gmenu2x->sc.getSkinFilePath("icons/" + linktitle);
	else if (!gmenu2x->sc.getSkinFilePath("icons/" + exectitle).empty())
		iconPath = gmenu2x->sc.getSkinFilePath("icons/" + exectitle);
	else if (!gmenu2x->sc.getSkinFilePath("icons/" + dirtitle).empty())
		iconPath = gmenu2x->sc.getSkinFilePath("icons/" + dirtitle);
	else if (fileExists(execicon))
		iconPath = execicon;
	else
		iconPath = gmenu2x->sc.getSkinFilePath("icons/generic.png");

	return iconPath;
}

int LinkApp::clock() {
	return iclock;
}

void LinkApp::setCPU(int mhz) {
	iclock = mhz;
	if (iclock != 0) {
    iclock = constrain(iclock, gmenu2x->confInt["cpuMin"], gmenu2x->confInt["cpuMax"]);
  }
	edited = true;
}

#if defined(TARGET_GP2X)
//G
int LinkApp::gamma() {
	return igamma;
}

void LinkApp::setGamma(int gamma) {
	igamma = constrain(gamma, 0, 100);
	edited = true;
}
// /G
#endif

void LinkApp::setBackdrop(const string selectedFile) {
	backdrop = backdropPath = selectedFile;
	edited = true;
}

bool LinkApp::targetExists() {
#if defined(TARGET_PC)
	return true; //For displaying elements during testing on pc
#endif

	string target = exec;
	if (!exec.empty() && exec[0] != '/' && !workdir.empty())
		target = workdir + "/" + exec;

	return fileExists(target);
}

bool LinkApp::save() {
	if (!edited) return false;

	ofstream f(file.c_str());
	if (f.is_open()) {
		if (title != ""        ) f << "title="           << title           << endl;
		if (description != ""  ) f << "description="     << description     << endl;
		if (icon != ""         ) f << "icon="            << icon            << endl;
		if (exec != ""         ) f << "exec="            << exec            << endl;
		if (params != ""       ) f << "params="          << params          << endl;
		if (workdir != ""      ) f << "workdir="         << workdir         << endl;
		if (manual != ""       ) f << "manual="          << manual          << endl;
		if (iclock != 0        ) f << "clock="           << iclock          << endl;
		// if (useRamTimings      ) f << "useramtimings=true"                  << endl;
		// if (useGinge           ) f << "useginge=true"                       << endl;
		// if (ivolume > 0        ) f << "volume="          << ivolume         << endl;

#if defined(TARGET_GP2X)
		//G
		if (igamma != 0        ) f << "gamma="           << igamma          << endl;
#endif

		if (selectordir != ""    ) f << "selectordir="     << selectordir     << endl;
		if (selectorbrowser      ) f << "selectorbrowser=true"                << endl;
		if (!selectorbrowser     ) f << "selectorbrowser=false"               << endl;
		if (selectorfilter != "" ) f << "selectorfilter="  << selectorfilter  << endl;
		if (selectorscreens != "") f << "selectorscreens=" << selectorscreens << endl;
		if (aliasfile != ""      ) f << "selectoraliases=" << aliasfile       << endl;
		if (backdrop != ""       ) f << "backdrop="        << backdrop        << endl;
		// if (wrapper              ) f << "wrapper=true"                        << endl;
		// if (dontleave            ) f << "dontleave=true"                      << endl;
		f.close();
		return true;
	} else
		ERROR("Error while opening the file '%s' for write.", file.c_str());
	return false;
}

void LinkApp::run() {
	if (selectordir != "") {
		selector();
	} else {
		launch();
	}
}

void LinkApp::selector(int startSelection, const string &selectorDir) {
	//Run selector interface
	Selector sel(gmenu2x, this, selectorDir);
	int selection = sel.exec(startSelection);
	if (selection != -1) {
		gmenu2x->writeTmp(selection, sel.getDir());
		launch(sel.getFile(), sel.getDir());
	}
}

void LinkApp::launch(const string &selectedFile, const string &selectedDir) {
	save();

	//Set correct working directory
	string wd = getRealWorkdir();
	if (!wd.empty())
		chdir(wd.c_str());

	//selectedFile
	if (selectedFile != "") {
		string selectedFileExtension;
		string selectedFileName;
		string dir;
		string::size_type i = selectedFile.rfind(".");
		if (i != string::npos) {
			selectedFileExtension = selectedFile.substr(i,selectedFile.length());
			selectedFileName = selectedFile.substr(0,i);
		}

		if (selectedDir == "")
			dir = getSelectorDir();
		else
			dir = selectedDir;

		if (params == "") {
			params = cmdclean(dir+"/"+selectedFile);
		} else {
			string origParams = params;
			params = strreplace(params, "[selFullPath]", cmdclean(dir + "/" + selectedFile));
			params = strreplace(params, "[selPath]", cmdclean(dir));
			params = strreplace(params, "[selFile]", cmdclean(selectedFileName));
			params = strreplace(params, "[selExt]", cmdclean(selectedFileExtension));
			if (params == origParams) params += " " + cmdclean(dir + "/" + selectedFile);
		}
	}

	//if (useRamTimings)
		//gmenu2x->applyRamTimings();
	//if (volume()>=0)
		//gmenu2x->setVolume(volume());

	INFO("Executing '%s' (%s %s)", title.c_str(), exec.c_str(), params.c_str());

	//check if we have to quit
	string command = cmdclean(exec);

	// Check to see if permissions are desirable
	struct stat fstat;
	if ( stat( command.c_str(), &fstat ) == 0 ) {
		struct stat newstat = fstat;
		if ( S_IRUSR != ( fstat.st_mode & S_IRUSR ) ) newstat.st_mode |= S_IRUSR;
		if ( S_IXUSR != ( fstat.st_mode & S_IXUSR ) ) newstat.st_mode |= S_IXUSR;
		if ( fstat.st_mode != newstat.st_mode ) chmod( command.c_str(), newstat.st_mode );
	} // else, well.. we are no worse off :)

	if (params != "") command += " " + params;
	// if (useGinge) {
		// string ginge_prep = gmenu2x->getExePath() + "/ginge/ginge_prep";
		// if (fileExists(ginge_prep)) command = cmdclean(ginge_prep) + " " + command;
	// }
	if (gmenu2x->confInt["outputLogs"]) command += " 2>&1 | tee " + cmdclean(gmenu2x->getExePath()) + "/log.txt";
	// if (wrapper) command += "; sync & cd " + cmdclean(gmenu2x->getExePath()) + "; exec ./gmenu2x";
	// if (dontleave) {
		// system(command.c_str());
	// } else
	{
		if (gmenu2x->confInt["saveSelection"] && (gmenu2x->confInt["section"] != gmenu2x->menu->selSectionIndex() || gmenu2x->confInt["link"] != gmenu2x->menu->selLinkIndex())) {
			gmenu2x->writeConfig();
		}

#if defined(TARGET_GP2X)
		if (gmenu2x->fwType == "open2x") // && gmenu2x->savedVolumeMode != gmenu2x->volumeMode)
			gmenu2x->writeConfigOpen2x();
#endif
		if (selectedFile == "") gmenu2x->writeTmp();
		if (gmenu2x->confInt["saveAutoStart"]) {
			gmenu2x->confInt["lastCPU"] = clock();
			gmenu2x->confStr["lastCommand"] = command.c_str();
			gmenu2x->confStr["lastDirectory"] = wd.c_str();
			gmenu2x->writeConfig();
		}
		gmenu2x->quit();
		if (clock() != gmenu2x->confInt["cpuMenu"]) {
      gmenu2x->setCPU(clock());
    }

#if defined(TARGET_GP2X)
		if (gamma() != 0 && gamma() != gmenu2x->confInt["gamma"]) gmenu2x->setGamma(gamma());
#endif

		execlp("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
		
    //if execution continues then something went wrong and as we already called SDL_Quit we cannot continue
		//try relaunching gmenu2x
		chdir(gmenu2x->getExePath().c_str());
		execlp("./gmenu2x", "./gmenu2x", NULL);
	}

	chdir(gmenu2x->getExePath().c_str());
}

const string &LinkApp::getExec() {
	return exec;
}

void LinkApp::setExec(const string &exec) {
	this->exec = exec;
	edited = true;
}

const string &LinkApp::getParams() {
	return params;
}

void LinkApp::setParams(const string &params) {
	this->params = params;
	edited = true;
}

const string &LinkApp::getWorkdir() {
	return workdir;
}

const string LinkApp::getRealWorkdir() {
	string wd = workdir;
	if (wd.empty()) {
		if (exec[0] != '/') {
			wd = gmenu2x->getExePath();
		} else {
			string::size_type pos = exec.rfind("/");
			if (pos != string::npos)
				wd = exec.substr(0,pos);
		}
	}
	return wd;
}

void LinkApp::setWorkdir(const string &workdir) {
	this->workdir = workdir;
	edited = true;
}

const string &LinkApp::getManual() {
	return manual;
}

void LinkApp::setManual(const string &manual) {
	this->manual = manualPath = manual;
	edited = true;
}

const string &LinkApp::getSelectorDir() {
	return selectordir;
}

void LinkApp::setSelectorDir(const string &selectordir) {
	this->selectordir = selectordir;
	// if (this->selectordir!="" && this->selectordir[this->selectordir.length()-1]!='/') this->selectordir += "/";
	if (this->selectordir != "") this->selectordir = real_path(this->selectordir);
	edited = true;
}

bool LinkApp::getSelectorBrowser() {
	return selectorbrowser;
}

void LinkApp::setSelectorBrowser(bool value) {
	selectorbrowser = value;
	edited = true;
}

const string &LinkApp::getSelectorFilter() {
	return selectorfilter;
}

void LinkApp::setSelectorFilter(const string &selectorfilter) {
	this->selectorfilter = selectorfilter;
	edited = true;
}

const string &LinkApp::getSelectorScreens() {
	return selectorscreens;
}

void LinkApp::setSelectorScreens(const string &selectorscreens) {
	this->selectorscreens = selectorscreens;
	edited = true;
}

const string &LinkApp::getAliasFile() {
	return aliasfile;
}

void LinkApp::setAliasFile(const string &aliasfile) {
	if (fileExists(aliasfile)) {
		this->aliasfile = aliasfile;
		edited = true;
	}
}

void LinkApp::renameFile(const string &name) {
	file = name;
}

// bool LinkApp::getUseRamTimings() {
// 	return useRamTimings;
// }

// void LinkApp::setUseRamTimings(bool value) {
// 	useRamTimings = value;
// 	edited = true;
// }

// bool LinkApp::getUseGinge() {
// 	return useGinge;
// }

// void LinkApp::setUseGinge(bool value) {
// 	useGinge = value;
// 	edited = true;
// }
