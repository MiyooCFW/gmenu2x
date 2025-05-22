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

//for browsing the filesystem
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

#include "filelister.h"
#include "utilities.h"
#include "gmenu2x.h"
#include "debug.h"

using namespace std;

FileLister::FileLister(const string &startPath, bool showDirectories, bool showFiles):
showDirectories(showDirectories), showFiles(showFiles) {
	setPath(startPath);
}

void FileLister::browse(bool dirup) {
	directories.clear();
	files.clear();

	if (showDirectories || showFiles) {
		if (showDirectories && path != "/" && allowDirUp) directories.push_back("..");

		vector<string> vfilter;
		split(vfilter, getFilter(), ",");

		struct stat st;
		struct dirent **dptr;

		//string dirname = dir_name(path);
		if (dirup) path = dir_name(path);
		//INFO("browse() path=%s dirname=%s", path.c_str(), dirname.c_str());
		int i = 0, n = scandir(path.c_str(), &dptr, NULL, alphasort);

		if (n < 0) {
			ERROR("scandir(%s)", path.c_str());
			return;
		}

		while (++i < n) {
			string file = dptr[i]->d_name;
			if (file[0] == '.' || (find(excludes.begin(), excludes.end(), file) != excludes.end()))
				continue;

			if (!((dptr[i]->d_type & DT_REG) || (dptr[i]->d_type & DT_DIR)) || (dptr[i]->d_type & DT_LNK)) {
				string filepath = path + "/" + file;
				//INFO("browse() loop- filepath=%s, file=%s", filepath.c_str(), file.c_str());
				if (stat(filepath.c_str(), &st) == -1) {
					ERROR("Stat failed on '%s': '%s'", filepath.c_str(), strerror(errno));
					continue;
				}
				if (S_ISDIR(st.st_mode)) {
					dptr[i]->d_type |= DT_DIR;
				} else {
					dptr[i]->d_type |= DT_REG;
				}
			}

			if (dptr[i]->d_type & DT_DIR) {
				if (showDirectories) directories.push_back(file); // warning: do not merge the _if_
			} else if (showFiles) {
				for (vector<string>::iterator it = vfilter.begin(); it != vfilter.end(); ++it) {
					if (vfilter.size() > 1 && it->length() == 0 && (int32_t)file.rfind(".") >= 0) {
						continue;
					}

					if (it->length() <= file.length()) {
						if (!strcasecmp(file.substr(file.size() - it->length()).c_str(), it->c_str())) {
							files.push_back(file);
							break;
						}
					}
				}
			}
			free(dptr[i]);
		}
		free(dptr);
	}
}

void FileLister::setPath(const string &path) {
	this->path = real_path(path);
}
void FileLister::setFilter(const string &filter) {
	this->filter = filter;
}
uint32_t FileLister::size() {
	return files.size() + directories.size();
}
uint32_t FileLister::dirCount() {
	return directories.size();
}
uint32_t FileLister::fileCount() {
	return files.size();
}
string FileLister::operator[](uint32_t x) {
	return getFile(x);
}
string FileLister::getFile(uint32_t x) {
	if (x >= size()) return "";
	if (x < directories.size()) return directories[x];
	return files[x - directories.size()];
}
const string FileLister::getFilePath(uint32_t i) {
	return getPath() + "/" + getFile(i);
}
const string FileLister::getExt(uint32_t i) {
	return file_ext(getFile(i), true);
}
bool FileLister::isFile(uint32_t x) {
	return x >= directories.size() && x < size();
}
bool FileLister::isDirectory(uint32_t x) {
	return x < directories.size();
}
void FileLister::insertFile(const string &file) {
	files.insert(files.begin(), file);
}
void FileLister::addExclude(const string &exclude) {
	if (exclude == "..") allowDirUp = false;
	excludes.push_back(exclude);
}
