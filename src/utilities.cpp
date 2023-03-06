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
#include <math.h>

#include <ctime>
#include <sys/time.h>   /* for settimeofday() */

#include <SDL.h>

#include <algorithm>

#include "utilities.h"
#include "debug.h"

using namespace std;

bool case_less::operator()(const string &left, const string &right) const {
	return strcasecmp(left.c_str(), right.c_str()) < 0;
}

// General tool to strip spaces from both ends:
string trim(const string &s) {
	if (s.length() == 0)
		return s;
	int b = s.find_first_not_of(" \t\n\r");
	int e = s.find_last_not_of(" \t\r\n");
	if (b == -1) // No non-spaces
		return "";
	return string(s, b, e - b + 1);
}

void string_copy(const string &s, char **cs) {
	*cs = (char*)malloc(s.length());
	strcpy(*cs, s.c_str());
}

char *string_copy(const string &s) {
	char *cs = NULL;
	string_copy(s, &cs);
	return cs;
}

bool dir_exists(const string &path) {
	struct stat s;
	return (stat(path.c_str(), &s) == 0 && s.st_mode & S_IFDIR); // exists and is dir
}

bool file_exists(const string &path) {
	struct stat s;
	return (stat(path.c_str(), &s) == 0 && s.st_mode & S_IFREG); // exists and is file
}

bool rmtree(string path) {
	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath;

	DEBUG("RMTREE: '%s'", path.c_str());

	if ((dirp = opendir(path.c_str())) == NULL) return false;
	if (path[path.length() - 1] != '/') path += "/";

	while ((dptr = readdir(dirp))) {
		filepath = dptr->d_name;
		if (filepath == "." || filepath == "..") continue;
		filepath = path + filepath;
		int statRet = stat(filepath.c_str(), &st);
		if (statRet == -1) continue;
		if (S_ISDIR(st.st_mode)) {
			if (!rmtree(filepath)) return false;
		} else {
			if (unlink(filepath.c_str()) != 0) return false;
		}
	}

	closedir(dirp);
	return rmdir(path.c_str()) == 0;
}

int max(int a, int b) {
	return a > b ? a : b;
}
float max(float a, float b) {
	return a > b ? a : b;
}
int min(int a, int b) {
	return a < b ? a : b;
}
float min(float a, float b) {
	return a < b ? a : b;
}
int constrain(int x, int imin, int imax) {
	return min(imax, max(imin, x));
}
float constrain(float x, float imin, float imax) {
	return min(imax, max(imin, x));
}

//Configuration parsing utilities
int evalIntConf(int val, int def, int imin, int imax) {
	return evalIntConf(&val, def, imin, imax);
}
int evalIntConf(int *val, int def, int imin, int imax) {
	if (*val == 0 && (*val < imin || *val > imax))
		*val = def;
	else
		*val = constrain(*val, imin, imax);
	return *val;
}

const string &evalStrConf(const string &val, const string &def) {
	return val.empty() ? def : val;
}
const string &evalStrConf(string *val, const string &def) {
	*val = evalStrConf(*val, def);
	return *val;
}

bool split(vector<string> &vec, const string &str, const string &delim, bool destructive) {
	vec.clear();

	if (delim.empty()) {
		vec.push_back(str);
		return false;
	}

	std::string::size_type i = 0;
	std::string::size_type j = 0;

	while (true) {
		j = str.find(delim,i);
		if (j == std::string::npos) {
			vec.push_back(str.substr(i));
			break;
		}

		if (!destructive) {
			j += delim.size();
		}

		vec.push_back(str.substr(i,j-i));

		if (destructive) {
			i = j + delim.size();
		}

		if (i == str.size()) {
			vec.push_back(std::string());
			break;
		}
	}

	return true;
}

string strreplace(string orig, const string &search, const string &replace) {
	string::size_type pos = orig.find(search, 0);
	while (pos != string::npos) {
		orig.replace(pos, search.length(), replace);
		pos = orig.find(search, pos + replace.length());
	}
	return orig;
}

string cmdclean(string cmdline) {
	string spchars = "\\`$();|{}&'\"*?<>[]!^~-#\n\r ";
	for (uint32_t i = 0; i < spchars.length(); i++) {
		string curchar = spchars.substr(i, 1);
		cmdline = strreplace(cmdline, curchar, "\\" + curchar);
	}
	return cmdline;
}

int intTransition(int from, int to, int32_t tickStart, int32_t duration, int32_t tickNow) {
	if (tickNow < 0) tickNow = SDL_GetTicks();
	float elapsed = (float)(tickNow - tickStart) / duration;
	//                    elapsed                 increments
	return min((int)round(elapsed * (to - from)), (int)max(from, to));
}

string exec(const char* cmd) {
	FILE* pipe = popen(cmd, "r");
	if (!pipe) return "";
	char buffer[128];
	string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}

string real_path(const string &path) {
	char real_path[PATH_MAX];
	char *ptr;

	ptr = realpath(path.c_str(), real_path);

	if (ptr == NULL && errno == ENOENT) {
		string outpath;
		vector<string> vpath;
		split(vpath, path, "/");

		if (vpath.size() > 2) {
			int i = 1;
			vector<string>::iterator it = vpath.begin() + 1;

			while (it < vpath.end()) {
				if (*it == "." || it->empty()) {
					vpath.erase(vpath.begin() + i);
				} else if (*it == "..") {
					vpath.erase(vpath.begin() + i);
					vpath.erase(vpath.begin() + i - 1);
					it = vpath.begin() + 1;
					i = 1;
				} else {
					it++;
					i++;
				}
			}

			outpath = vpath.at(0) + "/";
			for(vector<string>::iterator it = vpath.begin() + 1; it < vpath.end() - 1; ++it) {
				outpath += *it + "/";
			}
			outpath += vpath.back();

		}
		return outpath;
	}
	return (string)real_path;
}

string dir_name(const string &path) {
	string::size_type p = path.rfind("/");
	if (p == path.size() - 1) p = path.rfind("/", p - 1);
	return real_path("/" + path.substr(0, p));
}

string base_name(string path, bool strip_extension) {
	string::size_type p = path.rfind("/");
	if (p == path.size() - 1) p = path.rfind("/", p - 1);

	path = path.substr(p + 1);

	if (strip_extension) {
		p = path.rfind('.');
		path = path.substr(0, p);
	}

	return path;
}

string file_ext(const string &path, bool tolower) {
	string ext = "";
	string::size_type pos = path.rfind(".");
	if (pos != string::npos && pos > 0) {
		ext = path.substr(pos);
		if (tolower)
			return lowercase(ext);
	}
	return ext;
}

string lowercase(string s) {
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

bool file_copy(const string &src, const string &dst) {
	FILE *fs, *fd;

	fs = fopen(src.c_str(), "r");
	if (fs == NULL) {
		ERROR("Cannot open source file %s\n", src.c_str());
		return false;
	}

	fd = fopen(dst.c_str(), "w");
	if (fd == NULL) {
		ERROR("Cannot open destiny file %s\n", src.c_str());
		return false;
	}

	// Read contents from file
	int c = fgetc(fs);
	while (c != EOF) {
		fputc(c, fd);
		c = fgetc(fs);
	}

	fclose(fs);
	fclose(fd);
	return true;
}

string unique_filename(string path, string ext) {
	uint32_t x = 0;
	string fname = path + ext;
	while (file_exists(fname)) {
		stringstream ss;
		ss << x;
		ss >> fname;
		fname = path + fname + ext;
		x++;
	}
	return fname;
}

string exe_path() {
	char real_path[PATH_MAX];
	memset(real_path, 0, PATH_MAX);
	readlink("/proc/self/exe", real_path, PATH_MAX);
	return dir_name(real_path);
}

string disk_free(const char *path) {
	string df = "N/A";
	struct statvfs b;

	if (statvfs(path, &b) == 0) {
		// Make sure that the multiplication happens in 64 bits.
		uint32_t freeMiB = ((uint64_t)b.f_bfree * b.f_bsize) / (1024 * 1024);
		uint32_t totalMiB = ((uint64_t)b.f_blocks * b.f_frsize) / (1024 * 1024);
		stringstream ss;
		if (totalMiB >= 10000) {
			ss	<< (freeMiB / 1024) << "." << ((freeMiB % 1024) * 10) / 1024 << "/"
				<< (totalMiB / 1024) << "." << ((totalMiB % 1024) * 10) / 1024 << "GiB";
		} else {
			ss	<< freeMiB << "/" << totalMiB << "MiB";
		}
		ss >> df;
	} else {
		WARNING("statvfs failed with error '%s'", strerror(errno));
	}

	return df;
}

const string get_date_time() {
#if !defined(TARGET_LINUX)
	system("hwclock --hctosys &");
#endif

	char buf[80];
	time_t now = time(0);
	struct tm tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%F %R", &tstruct);
	return buf;
}

void sync_date_time(time_t t) {
#if !defined(TARGET_LINUX)
	struct timeval tv = { t, 0 };
	settimeofday(&tv, NULL);
	system("hwclock --systohc &");
#endif
}


void init_date_time() {
	time_t now = time(0);
	const uint32_t t = __BUILDTIME__;

	if (now < t) {
		sync_date_time(t);
	}
}

void build_date_time() {
	const uint32_t t = __BUILDTIME__;

		sync_date_time(t);
}

void set_date_time(const char* timestamp) {
	int imonth, iday, iyear, ihour, iminute;

	sscanf(timestamp, "%d-%d-%d %d:%d", &iyear, &imonth, &iday, &ihour, &iminute);

	struct tm datetime = { 0 };

	datetime.tm_year = iyear - 1900;
	datetime.tm_mon  = imonth - 1;
	datetime.tm_mday = iday;
	datetime.tm_hour = ihour;
	datetime.tm_min  = iminute;
	datetime.tm_sec  = 0;

	if (datetime.tm_year < 0) datetime.tm_year = 0;

	time_t t = mktime(&datetime);

	sync_date_time(t);
}

// char *ms2hms(uint32_t t, bool mm = true, bool ss = true) {
// 	static char buf[10];

// 	t = t / 1000;
// 	int s = (t % 60);
// 	int m = (t % 3600) / 60;
// 	int h = (t % 86400) / 3600;
// 	// int d = (t % (86400 * 30)) / 86400;

// 	if (!ss) sprintf(buf, "%02d:%02d", h, m);
// 	else if (!mm) sprintf(buf, "%02d", h);
// 	else sprintf(buf, "%02d:%02d:%02d", h, m, s);
// 	return buf;
// };
